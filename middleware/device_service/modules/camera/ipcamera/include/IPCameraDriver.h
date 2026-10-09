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

#pragma once

#ifndef __cplusplus
#error "This is a c++ header!"
#endif

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "Performance.h"

extern "C" {
#include <libavformat/avformat.h>
}

#include "middleware/device_service/modules/camera/ipcamera/include/RingBuffer.h"
#include "middleware/device_service/modules/camera/ipcamera/include/camera.h"
#include "middleware/device_service/modules/camera/ipcamera/include/frequency_counter.h"

namespace airos {
namespace base {
namespace device {

typedef std::function<void(
    int64_t starttime_us, int64_t recvtime_us, const AVStream *stream,
    const AVPacket *pkt)>
    FuncDriverCallbackT;

class PrivateInfoT;

class IPCameraDriver final {
 public:
  static void realstream_callback(
      bool is_new_stream, unsigned char *buf, unsigned int buflen,
      unsigned int key);

  IPCameraDriver(const IPCameraDriver &) = delete;

  const IPCameraDriver &operator=(const IPCameraDriver &) = delete;

  IPCameraDriver(IPCameraDriver &&) = delete;

  IPCameraDriver &operator=(IPCameraDriver &&) = delete;

  /**
   * 创建一个新的相机驱动 Handle
   * @param vendor    要连接的相机厂家
   * @param ip        要连接的相机IP
   * @param callback  回调函数
   * @param port      相机控制端口
   * @param username  用户名
   * @param passwd    密码
   * @return          是否成功
   */
  bool newInstanceHandle(
      Vendor vendor, const unsigned char ip[4], FuncDriverCallbackT callback,
      int stream_num, int channel_num, unsigned int port = 8000,
      const std::string &username = "admin",
      const std::string &passwd   = "baidu123");

  /**
   * 获取驱动单例，线程安全
   * @return Instance
   */
  static IPCameraDriver *getInstance();
  std::map<unsigned int, std::shared_ptr<PrivateInfoT>> _M_map_private;
  std::mutex _M_lock_map;

 private:  // STATIC
  static IPCameraDriver *_S_instance;
  static std::mutex _S_lock_instance;

  static void __log_thread_func();

  static void __camera_pool_thread();

  static void __ffmpeg_thread_func();

  static void __camera_reset_thread();

  static int __ffmpeg_callback(void *opaque, uint8_t *buf, int buf_size);

  IPCameraDriver() = default;

  ~IPCameraDriver() = default;
};

class PrivateInfoT final {
  friend class IPCameraDriver;

 public:
  /**
   * @brief 重置视频流连接
   *
   */
  void reset_stream() {
    _M_flag0_hik = false;
  }

  /**
   * @brief 设置视频流连接
   *
   */
  void set_stream() {
    _M_flag0_hik = true;
  }

  /**
   * @brief 查询视频流状态
   *
   * @return true 连接状态
   * @return false 非连接状态
   */
  bool stream_status() {
    return _M_flag0_hik;
  }

  std::string get_vendor() {
    return _M_vendor == Vendor::HIKVISION ? "hikvision" : "dahua";
  }

  __attribute__((used)) PrivateInfoT(
      int key, unsigned int buffer_size_exp, Vendor vendor, std::string ip_str,
      unsigned int port, int stream_num, int channel_num, std::string username,
      std::string password, FuncDriverCallbackT callback)
      : _M_ip(std::move(ip_str))
      , _M_key(key)
      , _M_buffer(std::make_shared<RingBuffer>(buffer_size_exp))
      , _M_vendor(vendor)
      , _M_port(port)
      , _M_username(std::move(username))
      , _M_password(std::move(password))
      , _M_stream_num(stream_num)
      , _M_channel_num(channel_num)
      , _M_callback(std::move(callback))
      , _M_flag0_hik(false) {}

  ~PrivateInfoT() = default;

  int64_t single_camera_sequence     = 0;
  int64_t single_camera_hik_sequence = 0;
  const std::string _M_ip;
  CameraPerf perf;
  double _M_frame_rate = -1;
  std::string _M_before_time;

  FrequencyCounter _frequency_counter;

  bool _M_trickerror;             // 是否出现了trick错误
  int64_t _trickererr_starttime;  // 出现trick error时的开始时间

  int32_t _frames_counts_test = 0;

 private:
  const uint32_t _M_key;
  const std::shared_ptr<RingBuffer> _M_buffer;

  const Vendor _M_vendor;
  // const std::string _M_ip;
  const unsigned int _M_port;
  const std::string _M_username;
  const std::string _M_password;

  int _M_stream_num;
  int _M_channel_num;

  std::mutex _M_lock;
  std::condition_variable _M_cond;
  struct timespec _M_start_timespec = {0, 0};
  FuncDriverCallbackT _M_callback;
  void *_M_handle_analyzedata = nullptr;
  std::atomic<bool> _M_flag0_hik;
  bool _M_flag1_ffmpeg              = false;
  bool _M_flag2_ffmpeg_need_rebuild = false;
  bool _M_flag_reserved3            = false;
  bool _M_flag_reserved4            = false;
  bool _M_flag_reserved5            = false;
  bool _M_flag_reserved6            = false;
  bool _M_flag_reserved7            = false;
};

}  // END namespace device
}  // END namespace base
}  // namespace airos
