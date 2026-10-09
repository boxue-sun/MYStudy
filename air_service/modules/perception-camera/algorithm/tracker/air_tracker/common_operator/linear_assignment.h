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

#include <map>
#include <utility>
#include <vector>

#include "air_tracker/tracker/omt_obstacle_tracker.h"
#include "air_tracker/tracker/target.h"
#include "track_util.h"

namespace airos {
namespace perception {
namespace algorithm {

class OMTObstacleTracker;

namespace track2d {

// for matching;
class linear_assignment {
  linear_assignment();
  linear_assignment(const linear_assignment&);
  linear_assignment& operator=(const linear_assignment&);
  static linear_assignment* instance;

 public:
  static linear_assignment* getInstance();
  TRACKER_MATCHED matching_cascade(
      OMTObstacleTracker* distance_metric,
      OMTObstacleTracker::GATED_METRIC_FUNC distance_metric_func,
      float max_distance, int cascade_depth, std::vector<Target>& tracks,
      const Detections& detections, std::vector<int>& track_indices,
      std::vector<int> detection_indices = std::vector<int>());
  TRACKER_MATCHED min_cost_matching(
      OMTObstacleTracker* distance_metric,
      OMTObstacleTracker::GATED_METRIC_FUNC distance_metric_func,
      float max_distance, std::vector<Target>& tracks,
      const Detections& detections, std::vector<int>& track_indices,
      std::vector<int>& detection_indices);
  DYNAMICM gate_cost_matrix(
      KalmanFilter* kf, DYNAMICM& cost_matrix, std::vector<Target>& tracks,
      const Detections& detections, const std::vector<int>& track_indices,
      const std::vector<int>& detection_indices, float gated_cost = INFTY_COST,
      bool only_position = false);

 private:
  KAL_HCOVA identity_ = Eigen::MatrixXf::Identity(4, 4);
  float alpha_        = 50.0;
};

}  // namespace track2d
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
