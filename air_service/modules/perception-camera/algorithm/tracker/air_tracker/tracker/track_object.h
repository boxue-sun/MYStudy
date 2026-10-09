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

#include <Eigen/Eigen>

#include "air_service/modules/perception-camera/algorithm/interface/object_detect_info.h"
#include "air_service/modules/perception-camera/algorithm/interface/object_track_info.h"
#include "base/common/box.h"

namespace airos {
namespace perception {
namespace algorithm {
struct PatchIndicator {
  PatchIndicator() {
    frame_id = patch_id = -1;
    sensor_name         = "";
  }

  PatchIndicator(int frame_id, int patch_id) {
    this->frame_id = frame_id;
    this->patch_id = patch_id;
    sensor_name    = "";
  }
  PatchIndicator(int frame_id, int patch_id, const std::string &sensor_name) {
    this->frame_id    = frame_id;
    this->patch_id    = patch_id;
    this->sensor_name = sensor_name;
  }
  bool operator==(const PatchIndicator &indicator) {
    return (frame_id == indicator.frame_id && patch_id == indicator.patch_id);
  }

  std::string to_string() const {
    std::stringstream str;
    str << sensor_name << " | " << frame_id << " (" << patch_id << ")";
    return str.str();
  }

  int frame_id;
  int patch_id;
  std::string sensor_name;
};

struct TrackObject {
  PatchIndicator indicator;
  double timestamp = 0.0;
  airos::base::BBox2DF projected_box;
  float confidence = 0.0f;
  Eigen::Vector3d center;

  //
  airos::perception::algorithm::ObjectTrackInfoPtr tracker_info;

  // @breif object id per frame, required
  int id = -1;
  // @brief age of the tracked object, required
  double tracking_time = 0.0;
  // @brief timestamp of latest measurement, required
  double latest_tracked_time = 0.0;
};
typedef std::shared_ptr<TrackObject> TrackObjectPtr;
typedef std::vector<TrackObjectPtr> TrackObjectPtrs;

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
