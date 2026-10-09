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

#include "target.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <numeric>

#include "air_tracker/track_common/geometry_basic.h"
#include "air_tracker/track_common/math_functions.h"
#include "air_tracker/track_common/object_types.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

int Target::global_track_id_ = 0;

void Target::Init(const omt::TargetParam &param) {
  target_param_ = param;
  id_           = -1;
  lost_age_     = 0;
  max_age_      = track2d::MAX_AGE;
  n_init_       = track2d::NUM_INIT;
  hits_         = 1;
  state_        = TrackState::Tentative;

  world_lwh_.SetWindow(param.world_lhw_history());
  world_center_.variance_ *= target_param_.world_center().init_variance();
  world_center_.process_noise_ *= 1;
  world_center_.measure_noise_ *= 1;

  type_probs_.assign(
      static_cast<int>(track::ObjectSubType::MAX_OBJECT_TYPE), 0.0f);
  world_velocity_.SetWindow(param.mean_filter_window());
  velocity_.SetWindow(param.mean_filter_window());
  vel_max_distance_        = param.vel_max_distance();
  vel_max_                 = param.vel_max();
  objec_list_thresh_       = param.objec_list_thresh_();
  velocity_history_thresh_ = param.velocity_history_thresh_();

  // record orientation of displacement
  displacement_theta_.SetWindow(param.mean_filter_window());
  direction_.SetAlpha(param.direction_filter_ratio());
  // Init constant position Kalman Filter
  world_center_const_.covariance_.setIdentity();
  world_center_const_.measure_noise_.setIdentity();
  world_center_const_.process_noise_.setIdentity();
  world_center_const_.covariance_ *=
      target_param_.world_center().init_variance();
}

Target::Target(const omt::TargetParam &param) {
  Init(param);
}

int Target::Size() const {
  return static_cast<int>(tracked_objects_.size());
}

void Target::Clear() {
  tracked_objects_.clear();
}

TrackObjectPtr const &Target::operator[](int index) const {
  return GetObject(index);
}

TrackObjectPtr const &Target::GetObject(int index) const {
  CHECK(tracked_objects_.size() > 0);                          // NOLINT
  CHECK(index < static_cast<int>(tracked_objects_.size()));    // NOLINT
  CHECK(index >= -static_cast<int>(tracked_objects_.size()));  // NOLINT
  return tracked_objects_
      [(index + tracked_objects_.size()) % tracked_objects_.size()];
}

void Target::Add(TrackObjectPtr object) {
  if (tracked_objects_.empty()) {
    start_ts_ = object->timestamp;
    id_       = Target::global_track_id_++;
    type_     = object->tracker_info->detect.type_id;
  }
  // set track_id when confirmed
  if (is_confirmed()) {
    object->tracker_info->tracker.track_id = id_;
  }
  object->tracking_time       = object->timestamp - start_ts_;
  object->latest_tracked_time = object->timestamp;
  lost_age_                   = 0;
  tracked_objects_.emplace_back(object);
}

void Target::Add(TrackObjectPtr object, KAL_MEAN &mean, KAL_COVA &covariance) {
  start_ts_                   = object->timestamp;
  id_                         = Target::global_track_id_++;
  type_                       = object->tracker_info->detect.type_id;
  object->tracking_time       = object->timestamp - start_ts_;
  object->latest_tracked_time = object->timestamp;
  lost_age_                   = 0;
  tracked_objects_.emplace_back(object);
  mean_       = mean;
  covariance_ = covariance;
}

void Target::RemoveOld(int frame_id) {
  int index = 0;
  while (index < tracked_objects_.size() &&
         tracked_objects_[index]->indicator.frame_id < frame_id) {
    ++index;
  }
  tracked_objects_.erase(
      tracked_objects_.begin(), tracked_objects_.begin() + index);
}

void Target::Predict(double timestamp, track2d::KalmanFilter *kf) {
  ++lost_age_;
  ++age_;
  auto delta_t = static_cast<float>(timestamp - GetObject(-1)->timestamp);
  if (delta_t < 0) {
    return;
  }
  kf->predict(mean_, covariance_);
  float acc_variance = target_param_.world_center().process_variance();
  float delta_t_2    = delta_t * delta_t;
  float pos_variance = 0.25f * acc_variance * delta_t_2 * delta_t_2;
  float vel_variance = acc_variance * delta_t_2;
  world_center_.process_noise_(0, 0) = pos_variance;
  world_center_.process_noise_(1, 1) = pos_variance;
  world_center_.process_noise_(2, 2) = vel_variance;
  world_center_.process_noise_(3, 3) = vel_variance;
  world_center_.Predict(delta_t);

  // const position kalman predict
  world_center_const_.process_noise_.setIdentity();
  world_center_const_.process_noise_(0, 0) = vel_variance * delta_t_2;
  world_center_const_.process_noise_(1, 1) =
      world_center_const_.process_noise_(0, 0);
  world_center_const_.Predict(delta_t);
}

void Target::Update2D(
    TrackFrame *frame, track2d::KalmanFilter *const kf,
    const track2d::DetectionRow &detection) {
  pro_        = kf->project(this->mean_, this->covariance_);
  KAL_DATA pa = kf->update(this->mean_, this->covariance_, detection.to_xyah());
  this->mean_ = pa.first;
  this->covariance_ = pa.second;
  ++hits_;
  lost_age_ = 0;

  if (state_ == TrackState::Tentative && hits_ >= n_init_) {
    state_ = TrackState::Confirmed;
  }
}

void Target::mark_missed() {
  if (this->state_ == TrackState::Tentative) {
    this->state_ = TrackState::Deleted;
  } else if (this->lost_age_ > this->max_age_) {
    this->state_ = TrackState::Deleted;
  }
}

bool Target::is_confirmed() {
  return this->state_ == TrackState::Confirmed;
}

bool Target::is_deleted() {
  return this->state_ == TrackState::Deleted;
}

bool Target::is_tentative() {
  return this->state_ == TrackState::Tentative;
}

DETECTBOX Target::to_tlwh() const {
  DETECTBOX ret = mean_.leftCols(4);
  ret(2) *= ret(3);
  ret.leftCols(2) -= (ret.rightCols(2) / 2);
  return ret;
}

void Target::UpdateYaw(TrackFrame *frame) {
  auto info = GetObject(-1)->tracker_info.get();
  if (track_theta_.IsAligned()) {
    info->tracker.theta       = track_theta_.GetState()(2);
    info->tracker.direction.x = cos(info->tracker.theta);
    info->tracker.direction.y = sin(info->tracker.theta);
    info->tracker.direction.z = 0;
  }
}

void Target::UpdateType(TrackFrame *frame) {
  auto info = GetObject(-1)->tracker_info.get();
  if (!IsTemporaryLost()) {
    float confidence = std::max(info->detect.type_id_confidence, 0.01f);
    for (int i = 0; i < type_probs_.size(); ++i) {
      if (type_probs_[i] < 0.01) {
        continue;
      }
      type_probs_[i] *= 0.618;
    }
    type_probs_[info->detect.type_id] += confidence;
    auto max_prob = std::max_element(type_probs_.begin(), type_probs_.end());
    auto index = static_cast<int>(std::distance(type_probs_.begin(), max_prob));
    auto pre_type = type_;
    type_         = index;
    if (type_ != pre_type) {
      LOG_DEBUG << "Target " << id_ << " change type from "
                << info->detect.type_id << " to "
                << "0(" << type_probs_[0] << "), "
                << "1(" << type_probs_[1] << "), "
                << "2(" << type_probs_[2] << "), "
                << "3(" << type_probs_[3] << "), "
                << "4(" << type_probs_[4] << "), "
                << "5(" << type_probs_[5] << "), "
                << "6(" << type_probs_[6] << "), "
                << "7(" << type_probs_[7] << "), "
                << "8(" << type_probs_[8] << "), "
                << "9(" << type_probs_[9] << "), "
                << "10(" << type_probs_[10] << "), "
                << "11(" << type_probs_[11] << ")";
    }
    info->detect.type_id = type_;
    info->detect.type    = DetectObjectType(type_);
    Eigen::Vector4d size_measurement;
    size_measurement << confidence, info->detect.size.length,
        info->detect.size.width, info->detect.size.height;
    world_lwh_.AddMeasure(size_measurement);
  }
}

bool Target::IsTracked() const {
  return Size() >= target_param_.tracked_life();
}

bool Target::IsTemporaryLost() const {
  return lost_age_ > 0;
}

bool Target::IsLost() const {
  return lost_age_ > target_param_.reserve_age();
}

int Target::type() const {
  return type_;
}

int Target::id() const {
  return id_;
}

int Target::lost_age() const {
  return lost_age_;
}

double Target::start_ts() const {
  return start_ts_;
}

float Target::GetLastTrackedObjectConf() const {
  return tracked_objects_.back()->confidence;
}

float Target::GetLastTrackedObjectYCoord() const {
  auto det_box = tracked_objects_.back()->tracker_info->detect.box;
  airos::base::RectF tmp_rect(
      det_box.left_top.x, det_box.left_top.y, det_box.Width(),
      det_box.Height());
  return tmp_rect.y + tmp_rect.height;
  // return tmp_rect.bottom;
}

DETECTBOX Target::GetLastTrackedObjectBox() const {
  auto det_box = tracked_objects_.back()->tracker_info->detect.box;
  airos::base::RectF tmp_box(
      det_box.left_top.x, det_box.left_top.y, det_box.Width(),
      det_box.Height());
  return DETECTBOX(tmp_box.x, tmp_box.y, tmp_box.width, tmp_box.height);
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
