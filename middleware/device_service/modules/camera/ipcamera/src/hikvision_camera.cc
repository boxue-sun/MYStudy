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

#include "middleware/device_service/modules/camera/ipcamera/include/hikvision_camera.h"

#include <unistd.h>

#include <chrono>
#include <functional>
#include <thread>

#include "AnalyzeDataNewInterface.h"
#include "HCNetSDK.h"
#include "base/common/log.h"

namespace airos {
namespace base {
namespace device {

std::mutex hikvision_camera::_mtx;
std::map<int, hikvision_camera*> hikvision_camera::_camera_set;

hikvision_camera::hikvision_camera(
    cameraInfo const& camera_info, streamParam const& stream_param)
    : _camera_info(camera_info)
    , _stream_param(stream_param)
    , _logined(false)
    , _fetched(false)
    , _userId(-1) {}

hikvision_camera::~hikvision_camera() {
  LOG_WARN << "camera:" << _camera_info.ip << " ~camera()";
  stop();
}

bool hikvision_camera::start() {
  std::lock_guard<std::mutex> g(_mutex);
  LOG_WARN << "camera:" << _camera_info.ip << " start";
  bool ret = login();
  if (!ret) return false;

  // 拉取视频流，失败则登出相机
  ret = start_stream();
  if (!ret) {
    logout();
    return false;
  }

  // 设置回调， 失败则 停止拉流，登出相机
  ret = set_stream_cb();
  if (!ret) {
    stop_stream();
    logout();
  }
  return ret;
}

bool hikvision_camera::stop() {
  std::lock_guard<std::mutex> g(_mutex);
  LOG_WARN << "camera:" << _camera_info.ip << " stop";
  unset_stream_cb();
  stop_stream();
  bool ret = logout();
  return ret;
}

bool hikvision_camera::reset() {
  LOG_WARN << "camera:" << _camera_info.ip << " reset start";
  bool ret = stop();
  if (!ret) return false;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  LOG_WARN << "camera:" << _camera_info.ip << " reset end";
  ret = start();
  return ret;
}

std::string hikvision_camera::get_ip() {
  return _camera_info.ip;
}

void* hikvision_camera::get_analyzedata() {
  return _analyze_data;
}

double hikvision_camera::get_framerate() {
  return _frame_rate;
}

void hikvision_camera::set_analyzedata(void* analyzedata) {
  _analyze_data = analyzedata;
}

void hikvision_camera::set_framerate(double framerate) {
  _frame_rate = framerate;
}

bool hikvision_camera::login() {
  // 检查是否已经登录
  if (_logined) {
    //
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " has login, userId:" << _userId;
    return true;
  }

  NET_DVR_DEVICEINFO_V30 devInfo = {};

  LONG userid = NET_DVR_Login_V30(
      const_cast<char*>(_camera_info.ip.c_str()),
      _camera_info.port,
      const_cast<char*>(_camera_info.username.c_str()),
      const_cast<char*>(_camera_info.password.c_str()),
      &devInfo);
  if (userid == -1) {
    LOG_ERROR << "<hkcamera> login camera:" << _camera_info.ip
              << " failed, error code:" << NET_DVR_GetLastError();
    return false;
  } else {
    _logined = true;
    _userId  = userid;
    _sn      = std::string(
        reinterpret_cast<char*>(devInfo.sSerialNumber), SERIALNO_LEN);

    std::lock_guard<std::mutex> lock(hikvision_camera::_mtx);
    hikvision_camera::_camera_set.emplace(
        inet_addr(_camera_info.ip.c_str()), this);

    LOG_WARN << "<hkcamera> login camera:" << _camera_info.ip
             << " success, userId:" << _userId;
    return true;
  }
}

bool hikvision_camera::logout() {
  // 检查是否未登录
  if (!_logined) {
    //
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip << " has not login";
    return true;
  }  // else {}

  BOOL ret = NET_DVR_Logout(_userId);
  if (ret == FALSE) {
    LOG_ERROR << "<hkcamera> logout camera:" << _camera_info.ip
              << " failed, error code:" << NET_DVR_GetLastError();
    return false;
  } else {
    _logined = false;
    _userId  = -1;

    std::lock_guard<std::mutex> lock(hikvision_camera::_mtx);
    hikvision_camera::_camera_set.erase(inet_addr(_camera_info.ip.c_str()));
    LOG_WARN << "<hkcamera> logout camera:" << _camera_info.ip << " success";
    return true;
  }
}

void hikvision_camera::callback(
    bool is_new_stream, unsigned char* buf, unsigned int buflen) {
  if (_stream_param.cb) {
    return _stream_param.cb(is_new_stream, buf, buflen, _stream_param.user_key);
  } else {
    LOG_ERROR << "<hkcamera> stream cb is invalid!";
  }
}

hikvision_camera* hikvision_camera::get_camera_by_key(int key) {
  std::lock_guard<std::mutex> lock(hikvision_camera::_mtx);
  if (hikvision_camera::_camera_set.find(key) ==
      hikvision_camera::_camera_set.end()) {
    return nullptr;
  }

  return hikvision_camera::_camera_set[key];
}

static void real_stream_callback(
    LONG handle, DWORD type, BYTE* buf, DWORD buflen, DWORD key) {
  hikvision_camera* camera =
      hikvision_camera::get_camera_by_key(static_cast<int>(key));
  if (!camera) {
    LOG_WARN << "<hkcamera> got invalid user data!";
    return;
  }
  auto analyzedata = camera->get_analyzedata();
  auto frame_rate  = camera->get_framerate();

  if (type == NET_DVR_SYSHEAD) {
    if (nullptr != analyzedata) {
      HIKANA_Destroy(analyzedata);
      analyzedata = nullptr;
    }
    analyzedata = HIKANA_CreateStreamEx(1 * 1024 * 1024, buf);
    HIKANA_SetOutputPacketType(analyzedata, ANALYZE_RAW_DATA);
    camera->set_analyzedata(analyzedata);
    LOG_WARN << "<hkcamera> ip:" << camera->get_ip() << " start new stream";

    return camera->callback(true, buf, buflen);
  } else if (type == NET_DVR_STREAMDATA) {
    if (frame_rate <= 0) {
      if (!analyzedata) {
        LOG_ERROR << "<hkcamera> handle is NULL!";
        return;
      }
      if (1 != HIKANA_InputData(analyzedata, buf, buflen)) {
        LOG_ERROR << "<hkcamera> Failed to input data!";
        return;
      }
      PACKET_INFO_EX pinfo;
      if (0 != HIKANA_GetOnePacketEx(analyzedata, &pinfo)) {
        unsigned int res = HIKANA_GetLastErrorH(analyzedata);
        if (res != ERROR_NEED_MOREDATA) {
          LOG_ERROR << "<hkcamera> Error: " << res;
        }
        return;
      }
      frame_rate = pinfo.dwFrameRate;
      camera->set_framerate(frame_rate);
      if (frame_rate > 0 && analyzedata != nullptr) {
        HIKANA_Destroy(analyzedata);
      }
    }

    return camera->callback(false, buf, buflen);
  }
}

bool hikvision_camera::start_stream() {
  if (_fetched) {
    //
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " has start stream, streamId:" << _streamId;
    return true;
  }  // else {}

  // start stream
  NET_DVR_PREVIEWINFO preInfo = {};
  preInfo.hPlayWnd            = 0;  // 不需要解码
  preInfo.bBlocked            = 1;  // 是否阻塞
  preInfo.dwLinkMode          = 0;  // 连接方式,可参考 SDK
  preInfo.byPreviewMode       = 0;  // 是否延迟
  preInfo.byProtoType         = 0;  // 0-私有协议，1-RTSP协议
  preInfo.bPassbackRecord     = 0;  // 是否启用录像回传
  preInfo.dwDisplayBufNum     = 1;
  preInfo.lChannel            = _stream_param.stream_channel;  // 通道号
  preInfo.dwStreamType        = _stream_param.stream_type;     // 码流号

  LONG handle = NET_DVR_RealPlay_V40(_userId, &preInfo, nullptr, nullptr);
  if (handle == -1) {
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " start stream failed, error code:" << NET_DVR_GetLastError();
    return false;
  } else {
    _fetched  = true;
    _streamId = handle;
    //
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " start stream success, streamId:" << _streamId;
    return true;
  }
}

bool hikvision_camera::stop_stream() {
  if (!_fetched) {
    //
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip << " not start stream";
    return true;
  }

  BOOL ret = NET_DVR_StopRealPlay(_streamId);
  if (ret == FALSE) {
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " stop stream failed, error code:" << NET_DVR_GetLastError();
    return false;
  } else {
    _fetched = false;
    //
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " stop stream success";
    return true;
  }
}

bool hikvision_camera::set_stream_cb() {
  if (!_fetched) {
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " not start stream, streamId is not valid when set stream cb";
    return true;
  }

  BOOL ret = NET_DVR_SetRealDataCallBack(
      _streamId,
      real_stream_callback,
      (DWORD)inet_addr(_camera_info.ip.c_str()));
  if (ret == FALSE) {
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " set callback failed, error code:" << NET_DVR_GetLastError();
    return false;
  } else {
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " set callback success";
    return true;
  }
}

bool hikvision_camera::unset_stream_cb() {
  if (!_fetched) {
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " not start stream, streamId is not valid when unset stream cb";
    return true;
  }

  BOOL ret =
      NET_DVR_SetRealDataCallBack(_streamId, nullptr, _stream_param.user_key);
  if (ret == FALSE) {
    LOG_ERROR << "<hkcamera> camera:" << _camera_info.ip
              << " unset callback failed, error code:"
              << NET_DVR_GetLastError();
    return false;
  } else {
    LOG_WARN << "<hkcamera> camera:" << _camera_info.ip
             << " unset callback success";
    return true;
  }
}

bool hikvision_camera::init() {
  BOOL ret = NET_DVR_Init();
  if (ret == FALSE) {
    auto err = NET_DVR_GetLastError();
    switch (err) {
      case NET_DVR_NOERROR:
        LOG_ERROR << "camera sdk init NET_DVR_NOERROR";
        break;
      case NET_DVR_ALLOC_RESOURCE_ERROR:
        LOG_ERROR << "camera sdk init NET_DVR_ALLOC_RESOURCE_ERROR";
        break;
      case NET_DVR_GETLOCALIPANDMACFAIL:
        LOG_ERROR << "camera sdk init NET_DVR_GETLOCALIPANDMACFAIL";
        break;
      default:
        break;
    }
    LOG_FATAL << "hk camera sdk init failed, error code:" << ret;
    return false;
  } else {
    LOG_WARN << "hk camera sdk init success";
    NET_DVR_SetLogToFile(3, const_cast<char*>("/home/caros/xlog/log/"), FALSE);
    NET_DVR_SetConnectTime(1000, 3);
    // 启动重连，设置重连间隔毫秒
    NET_DVR_SetReconnect(3000, true);

    return true;
  }
}

bool hikvision_camera::uninit() {
  BOOL ret = NET_DVR_Cleanup();
  if (ret == FALSE) {
    auto err = NET_DVR_GetLastError();
    switch (err) {
      case NET_DVR_NOERROR:
        LOG_ERROR << "<hkcamera> camera sdk uninit NET_DVR_NOERROR";
        break;
      case NET_DVR_NOINIT:
        LOG_ERROR << "<hkcamera> camera sdk uninit NET_DVR_NOINIT";
      default:
        break;
    }
    LOG_ERROR << "<hkcamera> camera sdk uninit failed, error code:" << ret;
    return false;
  } else {
    LOG_WARN << "<hkcamera> camera sdk uninit success";
    return true;
  }
}

}  // namespace device
}  // namespace base
}  // namespace airos
