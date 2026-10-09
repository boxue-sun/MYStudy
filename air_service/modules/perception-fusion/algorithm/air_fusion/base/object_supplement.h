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
#include <vector>

#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/common.h"
#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/object_types.h"
#include "base/common/box.h"

namespace airos {
namespace perception {
namespace msf {
namespace base {

using airos::base::BBox2DF;

// @brief general object status
enum class ObstacleStatus {
  UNKNOWN = 0,
  TRUE    = 1,
  FALSE   = 2,
};

// @brief wheel type
enum class WheelType {
  LEFT_FRONT  = 0,
  LEFT_REAR   = 1,
  RIGHT_FRONT = 2,
  RIGHT_REAR  = 3,
  OTHER       = 4,
  UNKNOWN     = 5,
};

// @brief wheel structure
struct Wheel {
  BBox2DF box    = BBox2DF();
  WheelType type = WheelType::UNKNOWN;
  // @brief confidence of wheel visibility
  float confidence = 0.0;
  // @brief relative orientation in camera coords
  float alpha = 0.0;
  // @brief orientation in camera coords
  float theta = 0.0;
};

struct alignas(16) CameraObjectSupplement {
  CameraObjectSupplement() {
    Reset();
  }

  void Reset() {
    sensor_name.clear();
    timestamp      = 0.0;
    local_track_id = -1;
    box            = BBox2DF();
  }

  // @brief camera sensor name
  std::string sensor_name;  // 2

  // @brief timestamp
  double timestamp = 0.0;  // 2
  // @brief local camera frame id
  int frame_id = -1;  //?????????????????????

  // @brief  2d box
  BBox2DF box;  // 2

  // @brief local track id
  int local_track_id = -1;  // 2
};
typedef std::shared_ptr<CameraObjectSupplement> CameraObjectSupplementPtr;
typedef std::shared_ptr<const CameraObjectSupplement>
    CameraObjectSupplementConstPtr;

struct SensorObjectMeasurement {
  void Reset() {
    sensor_id = "unknonw_sensor";
    timestamp = 0.0;
    track_id  = -1;
    box       = BBox2DF();
  }

  std::string sensor_id = "unknown_sensor";
  double timestamp      = 0.0;
  int track_id          = -1;
  // @brief only for camera measurement
  BBox2DF box;
};  // 和proto的measurement一致 输入输出都有

struct alignas(16) FusionObjectSupplement {
  FusionObjectSupplement() {
    measurements.reserve(5);
  }
  void Reset() {
    on_use = false;
    measurements.clear();
  }
  bool on_use = false;
  std::vector<SensorObjectMeasurement> measurements;
};

}  // namespace base
}  // namespace msf
}  // namespace perception
}  // namespace airos
