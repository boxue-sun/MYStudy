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

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"

#include "app/modules/v2x_application/usecase/base/meta_data.h"
#include "app/modules/v2x_application/usecase/common/tensor.h"

namespace airos {
namespace perception {
namespace usecase {

class TrackObj {
 public:
  using Buffer = std::map<std::string, std::deque<Meta>>;

  TrackObj(
      int64_t cur_id, int life, float thre_speed, float thre_speed_cnt,
      float thre_pos_cnt, Tensor cross_center)
      : id_(cur_id)
      , life_(life)
      , stop_speed_thre_(thre_speed)
      , stop_speed_cnt_thre_(thre_speed_cnt)
      , stop_shift_cnt_thre_(thre_pos_cnt)
      , cross_center_(cross_center) {}

  const Buffer& Info() const {
    return buffer_;
  }

  const std::deque<Meta>& GetInfo(const std::string& key) const {
    if (buffer_.find(key) != buffer_.end()) {
      return buffer_.at(key);
    }
    return buffer_.at("_empty_");
  }

  const std::vector<Sensor>& GetSensor() const {
    return sensor_;
  }

  int64_t Id() const {
    return id_;
  }

  int64_t PerceptionId() const {
    return perception_id_;
  }

  double TimeStamp() const {
    return time_stamp_;
  }

  int Type() const {
    return type_;
  }

  int SubType() const {
    return sub_type_;
  }

  bool IsCar() const {
    return type_ == airos::perception::PerceptionObstacle_Type_VEHICLE;
  }

  Tensor StopPos() const {
    return is_stop_ ? stop_pos_ : cur_pos_;
  }

  float StopLength() const {
    return is_stop_ ? stop_length_ : length_;
  }

  float StopWidth() const {
    return is_stop_ ? stop_width_ : width_;
  }

  float StopTheta() const {
    return is_stop_ ? stop_theta_ : theta_;
  }

  double StopTime() const {
    return is_stop_ ? stop_time_ : time_stamp_;
  }

  double StopDura() const {
    return is_stop_ ? time_stamp_ - stop_time_ : 0;
  }

  bool IsStop() const {
    return is_stop_;
  }

  int IdChange() const;

  bool IdChange(int64_t id) const;

  void CheckLost() {
    if (dy_life_ >= 0) {
      --dy_life_;
    } else {
      buffer_.clear();
    }
  }

  int Life() const {
    return dy_life_;
  }

  int Size() const {
    return buffer_.empty() ? 0 : buffer_.at("pos").size();
  }

  bool IsExists() const {
    if (dy_life_ == life_) {
      if (cross_center_.abs() < 0.1) {
        return true;
      }
      auto dist = cross_center_ - cur_pos_;
      if (dist.abs() < 1000) {
        return true;
      } else {
        APP_LOG_ERROR << "debug-> dist out of range: " << perception_id_;
      }
    }
    return false;
  }

  void Add(const airos::perception::PerceptionObstacle& object);
  bool InPoly(const std::vector<Tensor>& poly) const;
  bool InPolyStable(const std::vector<Tensor>& poly) const;

 private:
  bool JudgeStop();
  bool JudgeStable();

 private:
  Buffer buffer_;
  static size_t buffer_size_;
  std::vector<Sensor> sensor_;
  int64_t id_;
  int64_t perception_id_;
  int type_;
  int sub_type_;
  double time_stamp_;
  double stop_time_;
  int dy_life_;
  int life_ = 30;

  // for stable
  bool is_stop_ = false;
  Tensor stop_pos_;
  float stop_length_;
  float stop_width_;
  float stop_theta_;
  Tensor cur_pos_;
  int pos_break_cnt_ = 0;

  float velocity_thresh_ = 1.0;
  float length_;
  float width_;
  float theta_;

  float stop_speed_thre_;
  float stop_speed_cnt_thre_;
  float stop_shift_cnt_thre_;
  Tensor cross_center_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
