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

#include "middleware/device_service/modules/camera/ipcamera/include/IPCameraDriver.h"

#include <unistd.h>

#include <list>
#include <sstream>

#include "base/common/log.h"

namespace airos {
namespace base {
namespace device {
IPCameraDriver *IPCameraDriver::_S_instance = nullptr;
std::mutex IPCameraDriver::_S_lock_instance;

void IPCameraDriver::__log_thread_func() {
  std::lock_guard<std::mutex> g(_S_instance->_M_lock_map);
  LOG_INFO << "<DBG> Size: " << _S_instance->_M_map_private.size();
  for (auto &item : _S_instance->_M_map_private) {
    LOG_INFO << "<DBG> " << item.second->_M_ip << ": "
             << item.second->_M_buffer->debug_info();
  }
}

IPCameraDriver *IPCameraDriver::getInstance() {
  if (!_S_instance) {
    std::lock_guard<std::mutex> g(_S_lock_instance);
    if (!_S_instance) {
      _S_instance = new IPCameraDriver();
      std::thread([]() {
        IPCameraDriver::__camera_pool_thread();
      }).detach();
      std::thread([]() {
        IPCameraDriver::__camera_reset_thread();
      }).detach();
      std::thread([]() {
        while (nullptr != _S_instance) {
          IPCameraDriver::__ffmpeg_thread_func();
          sleep(2);
        }
      }).detach();
      std::thread([]() {
        while (nullptr != _S_instance) {
          IPCameraDriver::__log_thread_func();
          sleep(2);
        }
      }).detach();
    }
  }
  return _S_instance;
}

bool IPCameraDriver::newInstanceHandle(
    Vendor vendor, const unsigned char ip[4], FuncDriverCallbackT callback,
    int stream_num, int channel_num, unsigned int port,
    const std::string &username, const std::string &passwd) {
  uint32_t key = ip[3];
  key <<= 8;
  key += ip[2];
  key <<= 8;
  key += ip[1];
  key <<= 8;
  key += ip[0];
  std::string ip_str = std::to_string(static_cast<int>(ip[3])) + "." +
                       std::to_string(static_cast<int>(ip[2])) + "." +
                       std::to_string(static_cast<int>(ip[1])) + "." +
                       std::to_string(static_cast<int>(ip[0]));
  std::lock_guard<std::mutex> g(_M_lock_map);
  if (_M_map_private.find(key) != _M_map_private.end()) {
    LOG_WARN << "Found the same IP in map. Ignore.";
    return false;
  }
  auto item = std::make_shared<PrivateInfoT>(
      key,
      24,
      vendor,
      ip_str,
      port,
      stream_num,
      channel_num,
      username,
      passwd,
      std::move(callback));
  _M_map_private[key] = item;
  return true;
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
