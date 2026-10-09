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

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "base/device_connect/camera/camera_base.h"
#include "middleware/device_service/modules/camera/ipcamera/include/api-ipcamera.h"
#include "middleware/runtime/src/air_middleware_node.h"

namespace airos {
namespace base {
namespace device {

class DriverAPI final {
 private:
  ~DriverAPI() {}

  DriverAPI(uint32_t mode);

  bool __add_handle(
      const std::string &ipport, const std::string &channel_str,
      int32_t cudadev, airos::base::Color imgmode, const std::string &vendor,
      int stream_num, int channel_num, const std::string &username,
      const std::string &passwd, const CameraImageCallBack cb);

  void __convert_img(
      std::shared_ptr<GPUImage> decoded_img,
      std::shared_ptr<CameraImageData> output_data);

  static const char *const INPUT_CHANNEL_PREFIX;
  static std::mutex _S_lock_instance;
  static DriverAPI *_S_instance;
  const uint32_t _M_mode;
  bool _M_initflag;
  // ================= ChannelSet ==============
  std::mutex _M_lock3_channelset_input;
  std::mutex _M_lock4_channelset_output;
  std::unordered_set<std::string> _M_channelset_input;
  std::unordered_set<std::string> _M_channelset_output;
  // ================ Offline Mode Only =============
  std::shared_ptr<airos::middleware::AirMiddlewareNode> _M_node_offline =
      nullptr;
  std::shared_ptr<airos::middleware::AirMiddlewareNode> _M_debug_node = nullptr;
  std::unordered_map<uint32_t, std::map<int64_t, double>> timestamp_cache;

 public:
  static bool addHandle(
      uint32_t drv_modes, const std::string &ipport,
      const std::string &channel_str, int32_t cudadev,
      airos::base::Color imgmode, const std::string &vendor, int stream_num,
      int channel_num, const std::string &username, const std::string &passwd,
      const CameraImageCallBack cb);

 public:  // DELETED
  DriverAPI(const DriverAPI &) = delete;

  DriverAPI &operator=(const DriverAPI &) = delete;

  DriverAPI(DriverAPI &&) = delete;

  DriverAPI &operator=(DriverAPI &&) = delete;
};

}  // END namespace device
}  // END namespace base
}  // namespace airos
