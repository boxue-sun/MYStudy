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

#include "app/modules/v2x_application/usecase/base/tracked_obj.h"

#include <algorithm>

namespace airos {
namespace perception {
namespace usecase {

size_t TrackObj::buffer_size_ = 75;

void TrackObj::Add(const airos::perception::PerceptionObstacle& object) {
  for (auto it = buffer_.begin(); it != buffer_.end(); ++it) {
    while (it->second.size() >= buffer_size_) {
      it->second.pop_front();
    }
  }

  perception_id_ = object.id();
  type_          = object.type();
  sub_type_      = object.sub_type();
  time_stamp_    = object.timestamp();
  dy_life_       = life_;

  buffer_["camera_id"].clear();
  buffer_["lane_ids"].clear();

  Tensor tensor;
  tensor.x = object.position().x();
  tensor.y = object.position().y();
  tensor.z = object.position().z();
  cur_pos_ = tensor;
  buffer_["pos"].emplace_back(tensor);

  tensor.x = object.velocity().x();
  tensor.y = object.velocity().y();
  tensor.z = object.velocity().z();
  buffer_["speed"].emplace_back(tensor);

  length_ = object.length();
  width_  = object.width();
  theta_  = object.theta();
  buffer_["length"].emplace_back(length_);
  buffer_["width"].emplace_back(width_);
  buffer_["theta"].emplace_back(theta_);
  buffer_["id"].emplace_back(int64_t(object.id()));

  buffer_["in_region"].emplace_back(true);
  buffer_["in_lane"].emplace_back(true);
  buffer_["lane_heading"].emplace_back(1);

  buffer_["polygon_point"].clear();
  for (const auto& polygon_point : object.polygon_point()) {
    tensor.x = polygon_point.x();
    tensor.y = polygon_point.y();
    tensor.z = polygon_point.z();
    buffer_["polygon_point"].emplace_back(tensor);
  }

  buffer_["speed_covariance"].clear();

  JudgeStop();
}

int TrackObj::IdChange() const {
  std::unordered_set<int64_t> ids;
  for (const auto& id : buffer_.at("id")) {
    ids.insert(id.Cast<int64_t>());
  }
  return ids.size();
}

bool TrackObj::IdChange(int64_t id) const {
  for (auto it = buffer_.at("id").rbegin(); it != buffer_.at("id").rend();
       ++it) {
    if (it->Cast<int64_t>() != id) {
      return true;
    }
  }
  return false;
}

bool TrackObj::JudgeStable() {
  // check velocity
  auto speed_info = buffer_["speed"];
  if (speed_info.size() < static_cast<int>(buffer_size_ * 0.7)) {
    return false;
  }

  int vec_break_cnt = 0;
  int vec_slack     = 0;
  for (auto it = speed_info.rbegin(); it != speed_info.rend(); ++it) {
    if ((*it).Cast<Tensor>().abs() >= stop_speed_thre_) {
      ++vec_break_cnt;
      vec_slack = 0;
      if (vec_break_cnt > stop_speed_cnt_thre_) {
        vec_break_cnt = 0;
        return false;
      }
    } else {
      ++vec_slack;
      if (vec_slack > 0) {
        vec_break_cnt = 0;
        vec_slack     = 0;
      }
    }
  }

  // check pos
  float pos_shift = std::min(std::max(length_, width_) * 0.4, 3.0);
  if (is_stop_) {
    float dx   = cur_pos_.x - stop_pos_.x;
    float dy   = cur_pos_.y - stop_pos_.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > pos_shift) {
      pos_break_cnt_ += 1;
      if (pos_break_cnt_ > stop_shift_cnt_thre_) {
        pos_break_cnt_ = 0;
        return false;
      }
    } else {
      pos_break_cnt_ = 0;
    }
  }
  return true;
}

bool TrackObj::JudgeStop() {
  if (!JudgeStable()) {
    is_stop_ = false;
    return is_stop_;
  }

  if (!is_stop_) {
    is_stop_     = true;
    stop_time_   = time_stamp_;
    stop_pos_    = cur_pos_;
    stop_length_ = length_;
    stop_width_  = width_;
    stop_theta_  = theta_;
  }
  return true;
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
