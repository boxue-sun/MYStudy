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

#include "air_service/modules/perception-camera/algorithm/tracker/tracker_plugin_adapter.h"

#include <string>

#include "base/common/log.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace camera {

inline void TrackInfo2ObjectInfo(
    const algorithm::ObjectTrackInfo& in, ObjectInfo& out) {
  auto& detect_in = in.detect;
  auto& track_in  = in.tracker;

  out.type_id            = detect_in.type_id;
  out.type               = detect_in.type;
  out.type_id_confidence = detect_in.type_id_confidence;
  out.sub_type_probs     = detect_in.sub_type_probs;
  out.box                = detect_in.box;
  out.size               = detect_in.size;
  out.bottom_uv          = detect_in.bottom_uv;
  out.alpha              = detect_in.alpha;
  out.direction_alpha    = detect_in.direction_alpha;
  out.is_truncated       = detect_in.is_truncated;
  out.is_occluded        = detect_in.is_occluded;
  out.caseflag           = detect_in.caseflag;

  out.track_id  = track_in.track_id;
  out.theta     = track_in.theta;
  out.direction = track_in.direction;

  return;
}

inline algorithm::BaseObjectTracker::InitParam GetAlgorithmParam(
    const TrackParam& track_param) {
  algorithm::BaseObjectTracker::InitParam ret;
  ret.id        = track_param.id();
  ret.model_dir = track_param.model_dir();
  ret.width     = track_param.width();
  ret.height    = track_param.height();
  return ret;
}

bool TrackerPluginAdapter::ReadConfFile(const std::string& conf_file) {
  if (airos::base::ParseProtobufFromFile<TrackParam>(
          conf_file, &track_param_) == false)
    return false;
  return true;
}

bool TrackerPluginAdapter::Init(const airos::base::PluginParam& param) {
  if (ReadConfFile(param.config_file) == false) return false;
  p_tracker_.reset(algorithm::BaseObjectTrackerRegisterer::GetInstanceByName(
      track_param_.tracker_name()));
  if (p_tracker_ == nullptr) return false;
  return p_tracker_->Init(GetAlgorithmParam(track_param_));
}

bool TrackerPluginAdapter::Run(PerceptionFrame& data) {
  int ret =
      p_tracker_->Process(data.timestamp, data.detect_ret, data.track_ret);
  if (ret != 0) {
    LOG_ERROR << "track fail";
    return false;
  }
  for (auto& tracked_obj : data.track_ret) {
    data.objects.emplace_back(ObjectInfo());
    TrackInfo2ObjectInfo(*tracked_obj, data.objects.back());
  }
  return true;
}
}  // namespace camera
}  // namespace perception
}  // namespace airos
