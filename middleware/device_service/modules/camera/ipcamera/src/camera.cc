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

#include "middleware/device_service/modules/camera/ipcamera/include/camera.h"

#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>

// #include
// "middleware/device_service/modules/camera/ipcamera/include/dahua_camera.h"
#include "base/common/log.h"
#include "middleware/device_service/modules/camera/ipcamera/include/hikvision_camera.h"

namespace airos {
namespace base {
namespace device {
camera::camera() {}

camera::~camera() {}

bool camera::init() {
  if (!hikvision_camera::init()) {
    return false;
  }

  // if (!dahua_camera::init()) {
  //   hikvision_camera::uninit();
  //   return false;
  // }
  return true;
}

bool camera::uninit() {
  hikvision_camera::uninit();
  // dahua_camera::uninit();

  return true;
}

std::shared_ptr<camera> camera::create_camera_instance(
    cameraInfo const& camera_info, streamParam const& stream_param) {
  if (Vendor::HIKVISION == camera_info.vendor) {
    return std::make_shared<hikvision_camera>(camera_info, stream_param);
    // } else if (Vendor::DAHUA == camera_info.vendor) {
    //   return std::make_shared<dahua_camera>(camera_info, stream_param);
  } else {
    LOG_ERROR << "Unsupported vendor!";
    return nullptr;
  }
}

}  // namespace device
}  // namespace base
}  // namespace airos
