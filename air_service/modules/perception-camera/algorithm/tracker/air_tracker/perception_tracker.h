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

#include "air_service/modules/perception-camera/algorithm/interface/object_tracker.h"
#include "tracker/omt_obstacle_tracker.h"
#include "tracker/track_frame.h"
namespace airos {
namespace perception {
namespace algorithm {

class AirObjectTracker
    : public airos::perception::algorithm::BaseObjectTracker {
 public:
  AirObjectTracker() = default;
  ~AirObjectTracker() {}

  std::string ModelName() {
    return "ObjectTrack";
  }
  std::string ID() const {
    return para_.id;
  }
  bool Init(const InitParam& para) override;
  int Process(
      double timestamp,
      const std::vector<airos::perception::algorithm::ObjectDetectInfoPtr>&
          detect_result,
      std::vector<airos::perception::algorithm::ObjectTrackInfoPtr>&
          object_result);

 private:
  std::shared_ptr<OMTObstacleTracker> tracker_;
  airos::perception::algorithm::BaseObjectTracker::InitParam para_;
  bool _use_filter = false;
};

REGISTER_OBSTACLE_TRACKER(AirObjectTracker);

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
