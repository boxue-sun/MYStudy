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

#include "ipcamera/instance/simulation_camera.h"

#include "ipcamera/include/api-ipcamera.h"

namespace airos {
namespace base {
namespace device {

bool SimulationCamera::Init(const CameraInitConfig& config) {
  bool res = api_init_instance(
      API_MODE_OFFLINE,
      config.ip,
      config.camera_name,
      config.gpu_id,
      config.img_mode,
      "hikvision",
      config.stream_num,
      config.channel_num,
      "",
      "",
      image_sender_);
  return res;
}

AIROS_CAMERA_REG_FACTORY(SimulationCamera, "simulation_camera");

}  // namespace device
}  // namespace base
}  // namespace airos
