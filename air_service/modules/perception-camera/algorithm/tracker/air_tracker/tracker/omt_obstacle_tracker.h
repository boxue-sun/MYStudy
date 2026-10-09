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

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "air_service/modules/perception-camera/algorithm/tracker/air_tracker/tracker/omt.pb.h"

#include "air_service/modules/perception-camera/algorithm/interface/object_tracker.h"
#include "air_tracker/common_operator/track_util.h"
#include "frame_list.h"
#include "target.h"

namespace airos {
namespace perception {
namespace algorithm {

class OMTObstacleTracker {
 public:
  OMTObstacleTracker() = default;

  ~OMTObstacleTracker();

  bool Init(const airos::perception::algorithm::BaseObjectTracker::InitParam&
                options);
  // @brief: predict candidate obstales in the new image.
  // @param [in]: timestamp
  // candidate obstacle 2D boxes should be filled, required.
  bool Predict(double timestamp);

  // @brief: associate obstales by 2D information.
  // @param [in/out]: frame
  // associated obstacles with tracking id should be filled, required,
  // smoothed 2D information can be filled, optional.
  bool Associate(
      const std::vector<airos::perception::algorithm::ObjectDetectInfoPtr>&
          detect_result,
      TrackFrame* frame);

  std::vector<Target> targets_;

 public:
  typedef DYNAMICM (OMTObstacleTracker::*GATED_METRIC_FUNC)(
      std::vector<Target>& tracks, const track2d::Detections& dets,
      const std::vector<int>& track_indices,
      const std::vector<int>& detection_indices);

 private:
  DYNAMICM gated_matric(
      std::vector<Target>& tracks, const track2d::Detections& dets,
      const std::vector<int>& track_indices,
      const std::vector<int>& detection_indices);

  DYNAMICM iou_cost(
      std::vector<Target>& tracks, const track2d::Detections& dets,
      const std::vector<int>& track_indices,
      const std::vector<int>& detection_indices);

  Eigen::VectorXf iou(DETECTBOX& bbox, DETECTBOXSS& candidates);

  float iou(airos::base::BBox2DF& lhs_bbox, airos::base::BBox2DF& rhs_bbox);

  DYNAMICM motion_shape_cost(
      std::vector<Target>& tracks, const track2d::Detections& dets,
      const std::vector<int>& track_indices,
      const std::vector<int>& detection_indices);

  Eigen::VectorXf motion_shape_distance(
      DETECTBOX& box, DETECTBOXSS& candidates);
  void _match(const track2d::Detections& detections, TRACKER_MATCHED& matchs);
  void _initiate_track(const track2d::DetectionRow& detection);

 private:
  void ClearTargets();
  int CreateNewTarget(const TrackObjectPtrs& objects);
  void ProjectBox(
      const airos::base::BBox2DF& box_origin, const Eigen::Matrix3d& transform,
      airos::base::BBox2DF* box_projected);

 private:
  float max_iou_distance_;
  float max_cosine_distance_;
  int max_age_;
  int n_init_;
  int nn_budget_;
  track2d::KalmanFilter* kf_;
  Eigen::Vector4d coffe_ground;

 private:
  omt::OmtParam omt_param_;
  FrameList frame_list_;

  std::vector<bool> used_;
  float current_min_height_  = 0.0f;
  float current_init_height_ = 0.0f;
  float current_img_border_  = 0.0f;
  std::map<std::string, float> min_height_map_;
  std::map<std::string, float> init_height_map_;
  std::map<std::string, float> img_border_map_;
  float roi_3d_max_distance_ = 1000.0f;
  std::vector<std::vector<float>> kTypeAssociatedCost_;
  std::vector<float> confidence_map_;
  int track_id_  = 0;
  int frame_num_ = 0;
  int gpu_id_    = 0;
  float width_   = 0.0f;
  float height_  = 0.0f;
  Eigen::Affine3d world2camera_pose_;
  int cur_frame_id_;
  int cnt_cur_objects_;

  // narrow to obstacle projected_matrix
  Eigen::Matrix3d project_matrix_ = Eigen::Matrix3d::Identity();
};
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
