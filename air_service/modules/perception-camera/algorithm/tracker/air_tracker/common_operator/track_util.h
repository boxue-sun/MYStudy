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

#include <cstddef>
#include <iostream>
#include <vector>

#include <Eigen/Core>
#include <Eigen/StdVector>

#include "air_tracker/tracker/track_object.h"
#include "op_utils.h"

namespace airos {
namespace perception {
namespace algorithm {
namespace track2d {

// tracker setting
const float MAX_COS_DIS = 0.25;
const float MAX_IOU_DIS = 0.85;
const int MAX_AGE       = 20;
const int NUM_INIT      = 2;
const int NN_BUDGET     = 10;

// object
class DetectionRow : public DETECTION_ROW {
 public:
  TrackObjectPtr object;
};

typedef std::vector<DetectionRow> Detections;

}  // namespace track2d
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
