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

#include "air_service/modules/perception-fusion/algorithm/air_fusion/air_object_fusion.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <future>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "base/common/box.h"
#include "base/io/protobuf_util.h"  //TODO fgq to replace protofbuf reading function

namespace airos {
namespace perception {
namespace msf {

using base::Object;

static inline airos::base::BBox2DF TransBox2f(const Box2f& box) {
  airos::base::BBox2DF f;
  f.xmin = box.left_top.x;
  f.ymin = box.left_top.y;
  f.xmax = box.right_bottom.x;
  f.ymax = box.right_bottom.y;
  return f;
}

static inline Box2f TransBBox2DF(const airos::base::BBox2DF& f) {
  Box2f box;
  box.left_top.x     = f.xmin;
  box.left_top.y     = f.ymin;
  box.right_bottom.x = f.xmax;
  box.right_bottom.y = f.ymax;
  return box;
}

static base::Object TransPerceptionObjectToBaseObject(
    const PerceptionObject& o) {
  base::Object obj;
  Eigen::Matrix3d var;
  obj.sensor_type                   = o.sensor_type;
  obj.camera_supplement.sensor_name = o.sensor_name;
  obj.camera_supplement.timestamp   = o.timestamp;
  obj.camera_supplement.box         = TransBox2f(o.box);
  obj.timestamp                     = o.timestamp;
  obj.frame_id                      = o.sensor_name;
  obj.track_id                      = o.track_id;
  obj.position.Set(o.center, o.center_uncertainty);
  obj.main_theta = o.main_theta;
  obj.theta.Set(o.theta, o.theta_variance);
  var.setIdentity();
  obj.size.Set(
      Eigen::Vector3d(
          static_cast<double>(o.size.length), static_cast<double>(o.size.width),
          static_cast<double>(o.size.height)),
      var);

  obj.type           = o.type;
  obj.type_probs     = o.type_probs;
  obj.sub_type       = o.sub_type;
  obj.sub_type_probs = o.sub_type_probs;

  var.setIdentity();
  for (auto& p : o.polygon) {
    base::Info3d po;
    po.Set(p.polygon, p.variance);
    obj.polygon.push_back(po);
  }
  // Eigen::Vector3d(o.velocity.x,o.velocity.y,o.velocity.z);
  obj.velocity.Set(
      Eigen::Vector3d(o.velocity.x, o.velocity.y, o.velocity.z),
      o.velocity_uncertainty);
  obj.occ_state     = o.occ_state;
  obj.is_truncation = o.is_truncation;
  return obj;
}

static FusionObject TransBaseObjectToFusionObject(const base::Object& obj) {
  FusionObject out;
  out.timestamp     = obj.timestamp;
  out.tracking_time = obj.tracking_time;
  out.track_id      = obj.track_id;
  for (auto& m : obj.fusion_supplement.measurements) {
    ObjectMeasurement mea;
    mea.sensor_name = m.sensor_id;
    mea.timestamp   = m.timestamp;
    mea.track_id    = m.track_id;
    mea.box         = TransBBox2DF(m.box);
    out.measurements.push_back(mea);
  }
  out.type               = obj.type;
  out.type_probs         = obj.type_probs;
  out.sub_type           = obj.sub_type;
  out.sub_type_probs     = obj.sub_type_probs;
  out.center             = obj.position.Value();
  out.center_uncertainty = obj.position.Variance();
  out.theta              = obj.theta.Value();
  out.theta_variance     = obj.theta.Variance();
  out.size.length        = obj.size.Value()(0);
  out.size.width         = obj.size.Value()(1);
  out.size.height        = obj.size.Value()(2);
  out.velocity.x         = obj.velocity.Value()(0);
  out.velocity.y         = obj.velocity.Value()(1);
  out.velocity.z         = obj.velocity.Value()(2);
  for (auto& p : obj.polygon) {
    out.polygon.push_back(p.Value());
  }
  return out;
}

AirObjectFusion::~AirObjectFusion() {
  is_exit_ = true;
}

bool AirObjectFusion::Init(const InitParam& params) {
  channel_num_            = params.channel_num;
  reference_seq_          = params.reference_seq;
  time_diff_              = params.time_diff;
  missing_time_threshold_ = params.missing_time_threshold;
  reverse_time_threshold_ = params.reverse_time_threshold;
  CHECK(
      airos::base::ParseProtobufFromFile(
          params.fusion_params_file, &fusion_params_) == true);
  fusion_.Init(fusion_params_);
  std::vector<int> tmp(channel_num_);
  chn_consumed_times_.swap(tmp);
  return true;
}

int AirObjectFusion::Process(
    const std::vector<FusionInput>& input, FusionOutput& output) {
  if (input.size() > channel_num_) {
    return static_cast<int>(base::Error::PROCESS);
  }

  error_ = base::Error::NONE;
  timestamps_.clear();
  {
    std::vector<int> tmp(channel_num_);
    chn_consumed_times_.swap(tmp);
  }

  if (input.size() == 0) {
    LOG_ERROR << "No channel input.";
    error_ = base::Error::MESSAGE;
    return static_cast<int>(error_);
  }

  if (init_time_ < 0.0) {
    if (!CheckAllTimestamp(input)) {
      error_ = base::Error::INIT_TIMESTAMP;
      return static_cast<int>(error_);
    }
  }

  std::vector<base::Object>().swap(fused_objects_);
  std::vector<std::vector<base::Object>>().swap(fusion_result_);

  for (size_t i = 0; i < input.size(); ++i) {
    auto& message = input[i];
    if (!CheckSingleTimestamp(message, chn_consumed_times_[i])) {
      continue;
    }

    std::vector<base::Object> objects;

    for (auto it : message.objects) {
      objects.push_back(TransPerceptionObjectToBaseObject(it));
    }
    fusion_.CombineNewResource(objects, &fused_objects_, &fusion_result_);
  }

  if (!UpdateTimestamp()) {
    error_ = base::Error::TIMESTAMP;
  } else {
    for (auto& it : fused_objects_) {
      output.objects.push_back(TransBaseObjectToFusionObject(it));
    }
  }

  return static_cast<int>(error_);
}

bool AirObjectFusion::CheckAllTimestamp(const std::vector<FusionInput>& input) {
  for (size_t i = 0; i < input.size(); ++i) {
    auto& chn = input[i];
    chn_consumed_times_[i]++;
    if (IsReferenceSequence(i)) {
      if (chn.timestamp > 0.0) {
        timestamps_.push_back(chn.timestamp);
      } else {
        LOG_ERROR << std::setprecision(16) << "Reference timestamp empty: <seq "
                  << i << ", " << chn.sensor_name << ", " << chn.timestamp
                  << ">.";
      }
    }
  }

  double min_timestamp  = -1.0;
  double max_timestamp  = -1.0;
  double mean_timestamp = -1.0;
  if (timestamps_.empty()) {
    std::string reference_seq;
    for (int iter : reference_seq_) {
      reference_seq = absl::StrCat(reference_seq, " ", iter);
    }

    LOG_ERROR << "No reference timestamp available in initialization, the "
                 "reference are: <"
              << reference_seq << ">";
    return false;
  } else if (timestamps_.size() < 2) {
    min_timestamp  = timestamps_.at(0);
    max_timestamp  = timestamps_.at(0);
    mean_timestamp = timestamps_.at(0);
  } else {
    min_timestamp = *(std::min_element(timestamps_.begin(), timestamps_.end()));
    max_timestamp = *(std::max_element(timestamps_.begin(), timestamps_.end()));
    mean_timestamp = (min_timestamp + max_timestamp) / 2.0;
  }

  if (max_timestamp - min_timestamp > time_diff_) {
    std::string time_str;

    for (const auto& time : timestamps_) {
      time_str = absl::StrCat(time_str, " ", std::to_string(time));
    }
    LOG_ERROR << std::setprecision(16)
              << "Reference timestamp diff is larger than " << 1
              << ", which are " << time_str;
    return false;
  }

  init_time_     = min_timestamp;
  pre_mean_time_ = mean_timestamp;

  return true;
}

bool AirObjectFusion::CheckSingleTimestamp(
    const FusionInput& input, int& consumed_times) {
  if (consumed_times > 1 /*  || message.objects.empty() */) {
    LOG_IF(ERROR, input.sensor_name != "")
        << std::setprecision(16) << "<" << input.sensor_name << ", seq "
        << input.sequence_num << "> no new message";
    return false;
  }
  consumed_times++;

  if (input.timestamp < 0.0) {
    LOG_ERROR << std::setprecision(16) << "<" << input.sensor_name << ", seq "
              << input.sequence_num << "> timestamp < 0: " << input.timestamp;
    return false;
  }

  if (input.timestamp < init_time_ - reverse_time_threshold_) {
    LOG_ERROR << std::setprecision(16) << "<" << input.sensor_name << ", seq "
              << input.sequence_num
              << "> is earlier than init time: " << init_time_
              << ", while current time: " << input.timestamp;
    return false;
  }

  if (input.timestamp < pre_mean_time_ - reverse_time_threshold_) {
    LOG_ERROR << std::setprecision(16) << "<" << input.sensor_name << ", seq"
              << input.sequence_num << "> fall back "
              << input.timestamp - pre_mean_time_
              << "s, the pre time and current time:" << pre_mean_time_ << ", "
              << input.timestamp;
    return false;
  }

  if (IsReferenceSequence(input.index)) {
    timestamps_.push_back(input.timestamp);
  }
  return true;
}

bool AirObjectFusion::IsReferenceSequence(int seq) {
  if (reference_seq_.empty()) {
    LOG_FIRST_N(ERROR, 5)
        << "No reference sequence specified, set all sequences valid";
    return true;
  } else if (reference_seq_.find(seq) == reference_seq_.end()) {
    return false;
  } else {
    return true;
  }
}

bool AirObjectFusion::UpdateTimestamp() {
  double min_timestamp  = -1.0;
  double max_timestamp  = -1.0;
  double mean_timestamp = -1.0;

  if (timestamps_.empty()) {
    std::string reference_seq;
    for (int iter : reference_seq_) {
      reference_seq = absl::StrCat(reference_seq, " ", iter);
    }

    LOG_ERROR << "No reference timestamp available when update, the "
                 "reference are: <"
              << reference_seq << ">";
    return false;
  } else if (timestamps_.size() < 2) {
    min_timestamp  = timestamps_.at(0);
    max_timestamp  = timestamps_.at(0);
    mean_timestamp = timestamps_.at(0);
  } else {
    min_timestamp = *(std::min_element(timestamps_.begin(), timestamps_.end()));
    max_timestamp = *(std::max_element(timestamps_.begin(), timestamps_.end()));
    mean_timestamp = (min_timestamp + max_timestamp) / 2.0;
  }

  pre_mean_time_ = mean_timestamp;
  min_timestamp_ = min_timestamp;

  return true;
}

}  // namespace msf
}  // namespace perception
}  // namespace airos
