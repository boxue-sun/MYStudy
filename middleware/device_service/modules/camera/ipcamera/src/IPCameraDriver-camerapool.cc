/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <unistd.h>

#include <fstream>
#include <list>
#include <map>

#include "AnalyzeDataNewInterface.h"
#include "HCNetSDK.h"
#include "base/common/log.h"
#include "middleware/device_service/modules/camera/ipcamera/include/IPCameraDriver.h"

namespace airos {
namespace base {
namespace device {

void IPCameraDriver::realstream_callback(
    bool is_new_stream, unsigned char *buf, unsigned int buflen,
    unsigned int key) {
  std::shared_ptr<PrivateInfoT> info = nullptr;
  {
    std::lock_guard<std::mutex> g(_S_instance->_M_lock_map);
    info = _S_instance->_M_map_private[key];
  }
  if (is_new_stream) {
    std::lock_guard<std::mutex> g(info->_M_lock);
    clock_gettime(CLOCK_MONOTONIC, &info->_M_start_timespec);
    info->_M_flag2_ffmpeg_need_rebuild = true;
    info->_M_buffer->clear();
    if (Vendor::DAHUA == info->_M_vendor) {
      auto ret = info->_M_buffer->putdata(buf, buflen);
      if (ret != buflen) {
        LOG_ERROR << "putdata partly failed, real:" << ret
                  << ",expected:" << buflen << "!";
      }
    }
    info->_M_cond.notify_all();
    LOG_WARN << "<camerapool> ip:" << info->_M_ip << " start new stream";
  } else {
    std::lock_guard<std::mutex> g(info->_M_lock);
    auto ret = info->_M_buffer->putdata(buf, buflen);
    if (ret != buflen) {
      LOG_ERROR << "putdata partly failed, real:" << ret
                << ",expected:" << buflen << "!";
    }
    info->_M_cond.notify_all();
  }
}

// 解决花屏问题临时方案,根据配置重新拉流
void IPCameraDriver::__camera_reset_thread() {
  LOG_ERROR << "Enter in __camera_reset_thread ....";

  auto last_reset_time = std::chrono::system_clock::now();
  while (nullptr != _S_instance) {
    std::condition_variable cond;
    std::unique_lock<std::mutex> lock(_S_instance->_M_lock_map);
    while (true !=
           cond.wait_for(lock, std::chrono::seconds(60), [last_reset_time] {
             // 重新拉流时间从配置文件读取,读取不到或者数据无效都不会重新拉流
             std::ifstream in("/home/caros/cyber/reset_stream");
             if (!in.is_open()) {
               LOG_ERROR << "could not open config "
                            "file:/home/caros/cyber/reset_stream!";
               return false;
             }

             uint16_t hour;
             in >> hour;
             if (hour >= 24) {
               LOG_ERROR << "Invalid reset config: " << hour;
               return false;
             }

             auto current_time = std::chrono::system_clock::now();
             auto c_time = std::chrono::system_clock::to_time_t(current_time);
             struct tm time;
             localtime_r(&c_time, &time);
             auto interval = std::chrono::duration_cast<std::chrono::hours>(
                 current_time - last_reset_time);
             // 距离上一次重新拉流需间隔1小时
             LOG_ERROR << "reset config:" << hour << ", now:" << time.tm_hour
                       << ", interval:" << interval.count();
             return time.tm_hour == hour && interval.count() > 0;
           })) {
      // do nothing
    }

    LOG_ERROR << "Reset begin...";
    last_reset_time = std::chrono::system_clock::now();
    for (auto &item : _S_instance->_M_map_private) {
      auto info = item.second;
      std::lock_guard<std::mutex> g2(info->_M_lock);
      info->_M_flag0_hik = false;
      LOG_ERROR << "camera reset:" << info->_M_ip << "!";
    }
    LOG_ERROR << "Reset finish...";
  }

  LOG_ERROR << "Exit __camera_reset_thread ....";
}

void IPCameraDriver::__camera_pool_thread() {
  auto ret = camera::init();
  if (!ret) {
    LOG_ERROR << "<camerapool> camera sdk init error";
    return;
  }
  /**
   * @brief 相机对象池
   */
  std::map<uint32_t, std::shared_ptr<camera>> camera_pool;
  while (nullptr != _S_instance) {
    /**
     * @brief 读取 全局 IPCameraDriver 内未连接的相机，并放入
     * stopped_cameras 列表中
     */
    std::list<std::shared_ptr<PrivateInfoT>> stopped_cameras;
    {
      std::lock_guard<std::mutex> g(_S_instance->_M_lock_map);
      for (auto &item : _S_instance->_M_map_private) {
        auto info = item.second;
        std::lock_guard<std::mutex> g2(info->_M_lock);
        if (!info->_M_flag0_hik) {
          stopped_cameras.emplace_back(info);
        }
      }
    }

    /**
     * @brief 如果没有待连接的相机，则睡眠
     */
    if (stopped_cameras.empty()) {
      sleep(4);
    }

    /**
     *
     * @brief 循环操作相机，进行登录,取流操作
     */
    for (auto &info : stopped_cameras) {
      auto iter = camera_pool.find(info->_M_key);
      if (iter != camera_pool.end()) {
        bool ret = iter->second->reset();
        if (ret) {
          info->set_stream();  // 设置链接
          LOG_WARN << "<camerapool> camera restart success, " << info->_M_ip;
        } else {
          LOG_ERROR << "<camerapool> camera restart failed, " << info->_M_ip;
        }
      } else {
        cameraInfo camera_info{
            info->_M_vendor,
            info->_M_ip,
            (uint16_t)info->_M_port,
            info->_M_username,
            info->_M_password};
        streamParam stream_param{
            &IPCameraDriver::realstream_callback,
            info->_M_key,
            (uint32_t)info->_M_stream_num,
            info->_M_channel_num};

        LOG_WARN << "<camerapool> cameraInfo: " << info->_M_ip
                 << ", port:" << info->_M_port << ",name:" << info->_M_username
                 << ",passwd:" << info->_M_password;
        LOG_WARN << "<camerapool> cameraInfo: " << info->_M_ip
                 << ", key: " << info->_M_key
                 << "stream_num:" << info->_M_stream_num << "channel_num"
                 << info->_M_channel_num;

        std::shared_ptr<camera> cm =
            camera::create_camera_instance(camera_info, stream_param);
        bool ret = cm->start();
        if (ret) {
          info->set_stream();
          LOG_WARN << "<camerapool> camera start success, " << info->_M_ip;
        } else {
          LOG_ERROR << "<camerapool> camera start failed, " << info->_M_ip;
        }
        camera_pool.emplace(info->_M_key, cm);
      }
    }
  }

  /**
   * @brief 清除相机对象，释放相机SDK资源
   */
  camera_pool.clear();
  camera::uninit();
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
