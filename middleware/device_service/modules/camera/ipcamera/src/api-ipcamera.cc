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

#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-ipcamera.h"

namespace airos {
namespace base {
namespace device {

bool api_init_instance(
    uint32_t drv_mode, const std::string &ipport,
    const std::string &channel_str, int32_t cudadev, airos::base::Color imgmode,
    const std::string &vendor, int stream_num, int channel_num,
    const std::string &username, const std::string &passwd,
    const CameraImageCallBack cb) {
  return DriverAPI::addHandle(
      drv_mode,
      ipport,
      channel_str,
      cudadev,
      imgmode,
      vendor,
      stream_num,
      channel_num,
      username,
      passwd,
      cb);
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
