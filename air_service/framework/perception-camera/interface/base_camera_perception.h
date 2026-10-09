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
#include <memory>
#include <string>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/framework/proto/airos_visual_traffic_light.pb.h"

#include "base/device_connect/camera/camera_base.h"
#include "base/plugin/registerer.h"
namespace airos {
namespace perception {
namespace camera {

struct CameraPerceptionParam {
  std::string camera_name;
  std::string sdk_path;
  std::string conf_path;
  unsigned int gpu_id = 0;
};

class BaseCameraPerception {
 public:
  BaseCameraPerception()                                        = default;
  BaseCameraPerception(const BaseCameraPerception &)            = delete;
  BaseCameraPerception &operator=(const BaseCameraPerception &) = delete;
  virtual ~BaseCameraPerception()                               = default;
  virtual bool Init(const CameraPerceptionParam &param)         = 0;
  virtual bool Perception(
      const base::device::CameraImageData &camera_data,
      PerceptionObstacles *result, TrafficLightDetection *traffic_light) = 0;
  virtual std::string Name() const                                       = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseCameraPerception);
#define REGISTER_CAMERA_PERCEPTION(name) \
  PERCEPTION_REGISTER_CLASS(BaseCameraPerception, name)

}  // namespace camera
}  // namespace perception
}  // namespace airos
