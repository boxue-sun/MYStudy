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

#include "perception_tracker.h"

#include "air_tracker/tracker/omt_obstacle_tracker.h"

namespace airos {
namespace perception {
namespace algorithm {
using ObjectDetectInfoPtr = airos::perception::algorithm::ObjectDetectInfoPtr;
using ObjectTrackInfoPtr  = airos::perception::algorithm::ObjectTrackInfoPtr;
using ObjectTrackInfo     = airos::perception::algorithm::ObjectTrackInfo;
using ObjectDetectInfo    = airos::perception::algorithm::ObjectDetectInfo;

bool AirObjectTracker::Init(const InitParam& para) {
  para_ = para;
  if (para_.width < 10 || para_.height < 10) {
    LOG_FATAL << "track init width or height too small, width " << para_.width
              << ", height " << para_.height;
  }
  tracker_.reset(new OMTObstacleTracker());
  CHECK(tracker_ != nullptr);
  CHECK(tracker_->Init(para_)) << para_.id << "Failed to init ";
  return true;
}

int AirObjectTracker::Process(
    double timestamp, const std::vector<ObjectDetectInfoPtr>& detect_result,
    std::vector<ObjectTrackInfoPtr>& object_result) {
  static int frame_sn = 0;
  TrackFrame frame;
  frame.timestamp   = timestamp;
  frame.frame_id    = ++frame_sn;
  frame.camera_name = "default";

  LOG_INFO << "frame sn " << frame_sn << " , timestamp " << timestamp
           << ", detect result " << detect_result.size();

  tracker_->Predict(frame.timestamp);
  tracker_->Associate(detect_result, &frame);

  object_result = frame.tracked_objects;

  return 0;
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
