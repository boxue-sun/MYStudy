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

#include "base/device_connect/lidar/device_base.h"
#include "base/plugin/registerer.h"

namespace airos {
namespace perception {
namespace lidar {

struct LidarPerceptionParam {
  std::string lidar_name;
  std::string alg_path;
  std::string conf_path;
};

using LidarPonitCloud = os::v2x::device::LidarPBDataTypePtr;

class BaseLidarPerception {
 public:
  BaseLidarPerception()                                       = default;
  BaseLidarPerception(const BaseLidarPerception &)            = delete;
  BaseLidarPerception &operator=(const BaseLidarPerception &) = delete;
  virtual ~BaseLidarPerception()                              = default;
  virtual bool Init(const LidarPerceptionParam &param)        = 0;
  virtual bool Perception(
      const LidarPonitCloud &lidar_data, PerceptionObstacles *result) = 0;
  virtual std::string Name() const                                    = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseLidarPerception);
#define REGISTER_LIDAR_PERCEPTION(name) \
  PERCEPTION_REGISTER_CLASS(BaseLidarPerception, name)

}  // namespace lidar
}  // namespace perception
}  // namespace airos
