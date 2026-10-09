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

#include "middleware/device_service/modules/camera/ipcamera/include/dahua_camera.h"

#include <unistd.h>

#include <chrono>
#include <thread>

#include "base/common/log.h"
#include "dhnetsdk.h"

namespace airos {
namespace base {
namespace device {

dahua_camera::dahua_camera(
    cameraInfo const &camera_info, streamParam const &stream_param)
    : _camera_info(camera_info)
    , _stream_param(stream_param)
    , _logined(false)
    , _fetched(false)
    , _userId(-1)
    , _is_new_stream(true) {}

dahua_camera::~dahua_camera() {
  LOG_WARN << "<dhcamera> camera:" << _camera_info.ip << " ~camera()";
  stop();
}

bool dahua_camera::start() {
  std::lock_guard<std::mutex> g(_mutex);
  LOG_WARN << "<dhcamera> camera:" << _camera_info.ip << " start";
  bool ret = login();
  if (!ret) return false;

  // 拉取视频流，失败则登出相机
  ret = start_stream();
  if (!ret) {
    logout();
    return false;
  }

  return ret;
}

bool dahua_camera::stop() {
  std::lock_guard<std::mutex> g(_mutex);
  LOG_WARN << "<dhcamera> camera:" << _camera_info.ip << " stop";

  stop_stream();
  bool ret = logout();
  return ret;
}

bool dahua_camera::reset() {
  bool ret = stop();
  if (!ret) {
    LOG_WARN << "<dhcamera> camera:" << _camera_info.ip << " reset failed!";
    return false;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  ret = start();

  return ret;
}

void dahua_camera::stream_callback(unsigned char *buf, unsigned int buflen) {
  if (!_stream_param.cb) {
    LOG_ERROR << "<dhcamera> camera cb is invalid!";
    return;
  }

  _stream_param.cb(_is_new_stream, buf, buflen, _stream_param.user_key);

  if (_is_new_stream) {
    _is_new_stream = false;
  }
}

bool dahua_camera::login() {
  // 检查是否已经登录
  if (_logined) {
    //
    LOG_ERROR << "<dhcamera> camera:" << _camera_info.ip
              << " has login, userId:" << _userId;
    return true;
  }

  const char *pchDVRIP    = _camera_info.ip.c_str();
  WORD wDVRPort           = _camera_info.port;
  const char *pchUserName = _camera_info.username.c_str();
  const char *pchPassword = _camera_info.password.c_str();

  NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
  memset(&stInparam, 0, sizeof(stInparam));
  stInparam.dwSize = sizeof(stInparam);
  strncpy(stInparam.szIP, pchDVRIP, sizeof(stInparam.szIP) - 1);
  strncpy(stInparam.szPassword, pchPassword, sizeof(stInparam.szPassword) - 1);
  strncpy(stInparam.szUserName, pchUserName, sizeof(stInparam.szUserName) - 1);
  stInparam.nPort     = wDVRPort;
  stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

  NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
  memset(&stOutparam, 0, sizeof(stOutparam));
  stOutparam.dwSize = sizeof(stOutparam);
  auto handle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
  if (0 == handle) {
    LOG_ERROR << "<dhcamera> login camera:" << _camera_info.ip
              << " failed, error code:" << (0x7ffffff & CLIENT_GetLastError());
    return false;
  } else {
    _logined = true;
    _userId  = handle;
    _sn      = std::string(
        reinterpret_cast<char *>(stOutparam.stuDeviceInfo.sSerialNumber),
        DH_SERIALNO_LEN);

    //
    LOG_WARN << "<dhcamera> login camera:" << _camera_info.ip
             << " success, userId:" << _userId;
    return true;
  }
}

bool dahua_camera::logout() {
  // 检查是否未登录
  if (!_logined) {
    //
    LOG_ERROR << "<dhcamera> camera:" << _camera_info.ip << " has not login";
    return true;
  }

  auto ret = CLIENT_Logout(_userId);
  if (FALSE == ret) {
    LOG_ERROR << "<dhcamera> logout camera:" << _camera_info.ip
              << " failed, error code:" << (0x7ffffff & CLIENT_GetLastError());
    return false;
  }
  _logined = false;
  LOG_ERROR << "<dhcamera> camera:" << _camera_info.ip << " log out!";

  return true;
}

std::string dahua_camera::get_ip() {
  return _camera_info.ip;
}

static void CALLBACK RealDataCallBackEx(
    LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize,
    LLONG lParam, LDWORD dwUser) {
  dahua_camera *camera = reinterpret_cast<dahua_camera *>(dwUser);
  if (camera &&
      (NET_DATA_CALL_BACK_VALUE + EM_REAL_DATA_TYPE_H264) == dwDataType) {
    NET_STREAMCONVERT_INFO *info =
        reinterpret_cast<NET_STREAMCONVERT_INFO *>(lParam);
    if (!info || EM_STREAMCONVERT_FRAMEDATA_VIDEO != info->emDataType) {
      LOG_ERROR << "<dhcamera> camera:" << camera->get_ip()
                << "get invalid video frame.";
      return;
    }

    camera->stream_callback(pBuffer, dwBufSize);
  }
}

bool dahua_camera::start_stream() {
  if (_fetched) {
    //
    LOG_ERROR << "<dhcamera> camera:" << _camera_info.ip
              << " has start stream, streamId:" << _streamId;
    return true;
  }

  NET_IN_REALPLAY_BY_DATA_TYPE stuIn = {sizeof(stuIn)};
  stuIn.nChannelID                   = 0;
  stuIn.emDataType                   = EM_REAL_DATA_TYPE_H264;
  stuIn.hWnd                         = 0;
  // 当前只支持主、子码流
  stuIn.rType  = (_stream_param.stream_type == 0) ? DH_RType_Realplay
                                                  : DH_RType_Realplay_1;
  stuIn.dwUser = reinterpret_cast<LDWORD>(this);

  stuIn.cbRealDataEx = RealDataCallBackEx;
  // stuIn.cbRealDataEx2 = nullptr;
  stuIn.cbRealData  = nullptr;
  stuIn.emAudioType = EM_AUDIO_DATA_TYPE_DEFAULT;

  NET_OUT_REALPLAY_BY_DATA_TYPE stuOut = {sizeof(stuOut)};

  LLONG lRet = CLIENT_RealPlayByDataType(_userId, &stuIn, &stuOut, 5000);
  if (0 == lRet) {
    DWORD dwError = CLIENT_GetLastError();
    LOG_ERROR << "<dhcamera> CLIENT_RealPlayByDataType failed:"
              << (dwError & 0x7ffffff);
    return false;
  } else {
    LOG_ERROR << "<dhcamera> CLIENT_RealPlayByDataType success!";
  }

  _streamId      = lRet;
  _fetched       = true;
  _is_new_stream = true;
  return true;
}

bool dahua_camera::stop_stream() {
  LOG_ERROR << "<dhcamera> camera:" << _camera_info.ip << " stop_stream!";
  _fetched       = false;
  BOOL bRealPlay = CLIENT_StopRealPlayEx(_streamId);
  if (!bRealPlay) {
    LOG_ERROR << "<dhcamera> stop_stream failed:"
              << (0x7ffffff & CLIENT_GetLastError());
    return false;
  }

  return true;
}

static void onDisConnect(
    LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser) {
  LOG_ERROR << "<dhcamera> " << std::string(pchDVRIP) << "," << nDVRPort
            << " disconnected!";
}

static void onReConnect(
    LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser) {
  LOG_ERROR << "<dhcamera> " << std::string(pchDVRIP) << "," << nDVRPort
            << " reconnected!";
}

bool dahua_camera::init() {
  auto ret = CLIENT_Init(onDisConnect, (LDWORD) nullptr);
  if (!ret) {
    LOG_ERROR << "<dhcamera> CLIENT_Init failed:" << ret;
    return false;
  } else {
    LOG_WARN << "<dhcamera> camera sdk init success.";
    CLIENT_SetAutoReconnect(onReConnect, -1);
    LOG_SET_PRINT_INFO stLogPrintInfo = {sizeof(stLogPrintInfo)};
    stLogPrintInfo.bSetFilePath       = TRUE;
    snprintf(
        stLogPrintInfo.szLogFilePath,
        MAX_LOG_PATH_LEN,
        "/home/caros/xlog/log/dhcamera.log");
    CLIENT_LogOpen(&stLogPrintInfo);

    return true;
  }
}

bool dahua_camera::uninit() {
  CLIENT_Cleanup();

  return true;
}

}  // namespace device
}  // namespace base
}  // namespace airos
