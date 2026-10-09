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
#include <list>
#include <string>
#include <vector>

#include <boost/circular_buffer.hpp>

#include "air_service/modules/perception-camera/algorithm/tracker/air_tracker/tracker/omt.pb.h"

#include "air_tracker/common_operator/curve_fitting.h"
#include "air_tracker/common_operator/filters.h"
#include "air_tracker/common_operator/kalmanfilter.h"
#include "air_tracker/common_operator/track_util.h"
#include "frame_list.h"
#include "track_object.h"

namespace airos {
namespace perception {
namespace algorithm {

// Target represent a object in time sequence

class Target {
 public:
  explicit Target(const omt::TargetParam &param);

  void Init(const omt::TargetParam &param);

  void Add(TrackObjectPtr object);

  void Add(TrackObjectPtr object, KAL_MEAN &mean, KAL_COVA &covariance);

  void RemoveOld(int frame_id);

  void Clear();

  void Predict(double timestamp, track2d::KalmanFilter *kf);

  void Update2D(
      TrackFrame *frame, track2d::KalmanFilter *const kf,
      const track2d::DetectionRow &detection);

  void UpdateYaw(TrackFrame *frame);

  void UpdateType(TrackFrame *frame);

  int Size() const;

  TrackObjectPtr const &GetObject(int index) const;
  TrackObjectPtr const &operator[](int index) const;

  bool IsTracked() const;
  bool IsTemporaryLost() const;
  bool IsLost() const;
  int lost_age() const;
  double start_ts() const;
  int id() const;
  int type() const;

  enum TrackState { Tentative = 1, Confirmed, Deleted };
  void update(
      track2d::KalmanFilter *const kf, const track2d::DetectionRow &detection);
  void mark_missed();
  bool is_confirmed();
  bool is_deleted();
  bool is_tentative();
  DETECTBOX to_tlwh() const;
  float GetLastTrackedObjectConf() const;
  float GetLastTrackedObjectYCoord() const;
  DETECTBOX GetLastTrackedObjectBox() const;

  int age_;

  KAL_MEAN mean_;
  KAL_COVA covariance_;
  KAL_HDATA pro_;
  MeanFilter quality_;
  // ObjectSubType type_ = ObjectSubType::MAX_OBJECT_TYPE;
  int type_ = 99;

  int hits_;
  int n_init_;
  int max_age_;
  TrackState state_;
  int lost_age_    = 0;
  int id_          = 0;
  double start_ts_ = 0.0;

 protected:
  FirstOrderRCLowPassFilter direction_;
  // constant velocity kalman state
  KalmanFilterConstVelocity world_center_;
  // constant position kalman state
  KalmanFilterConstState<2> world_center_const_;
  // displacement theta
  MeanFilter displacement_theta_;
  // curve_fitting theta
  CurveFitting track_theta_;
  float vel_max_distance_ = 1000.0f;
  float vel_max_          = 50;
  std::list<TrackObjectPtr> object_list_;
  int objec_list_thresh_ = 2;
  std::deque<double> velocity_history_x_;
  std::deque<double> velocity_history_y_;
  int velocity_history_thresh_ = 3;
  MaxNMeanFilter world_lwh_;
  MeanFilter world_velocity_;
  MeanFilter velocity_;
  std::vector<float> type_probs_;
  omt::TargetParam target_param_;

  KalmanFilterConstVelocity image_tl_;
  KalmanFilterConstVelocity image_br_;
  MeanFilter image_tl_const_;
  KalmanFilterConstState<2> image_br_const_;

  TrackObjectPtrs tracked_objects_;

  static int global_track_id_;
};
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
