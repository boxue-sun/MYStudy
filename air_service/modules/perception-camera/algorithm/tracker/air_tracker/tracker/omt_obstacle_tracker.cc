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

#include "omt_obstacle_tracker.h"

#include <fcntl.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "air_tracker/common_operator/linear_assignment.h"
#include "air_tracker/track_common/geometry_common.h"
#include "air_tracker/track_common/math_functions.h"
#include "air_tracker/track_common/util.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace algorithm {

bool OMTObstacleTracker::Init(
    const airos::perception::algorithm::BaseObjectTracker::InitParam &para) {
  std::string omt_config = airos::base::FileUtil::GetAbsolutePath(
      para.model_dir, "camera_config.pt");
  CHECK(airos::base::ParseProtobufFromFile<omt::OmtParam>(
      omt_config, &omt_param_))
      << para.id << " Read config failed: " << omt_config;

  LOG_INFO << "load omt parameters from " << omt_config
           << " \nParams: " << omt_param_.DebugString();

  track_id_  = 0;
  frame_num_ = 0;
  frame_list_.Init(omt_param_.img_capability());
  width_  = para.width;
  height_ = para.height;

  for (int i = 0; i < omt_param_.access().size(); ++i) {
    min_height_map_[omt_param_.access(i).camera_name()] =
        omt_param_.access(i).min_height();
    init_height_map_[omt_param_.access(i).camera_name()] =
        omt_param_.access(i).init_height();
    img_border_map_[omt_param_.access(i).camera_name()] =
        omt_param_.access(i).img_border();
  }
  CHECK(img_border_map_.find("default") != img_border_map_.end())
      << "Should have default access param";
  std::string conf_map_file = airos::base::FileUtil::GetAbsolutePath(
      para.model_dir, omt_param_.conf_map_file());
  CHECK(LoadConfidenceMap(conf_map_file, &confidence_map_));
  std::string type_change_cost = airos::base::FileUtil::GetAbsolutePath(
      para.model_dir, omt_param_.type_change_cost());
  std::ifstream fin(type_change_cost);
  CHECK(fin.is_open());
  kTypeAssociatedCost_.clear();
  int n_type = static_cast<int>(track::ObjectSubType::MAX_OBJECT_TYPE);
  for (int i = 0; i < n_type; ++i) {
    kTypeAssociatedCost_.emplace_back(std::vector<float>(n_type, 0));
    for (int j = 0; j < n_type; ++j) {
      fin >> kTypeAssociatedCost_[i][j];
    }
  }

  max_iou_distance_    = omt_param_.weight().mdiou();
  max_cosine_distance_ = omt_param_.weight().mdcos();
  max_age_             = track2d::MAX_AGE;
  n_init_              = track2d::NUM_INIT;
  nn_budget_           = track2d::NN_BUDGET;
  kf_                  = new track2d::KalmanFilter();
  return true;
}

OMTObstacleTracker::~OMTObstacleTracker() {
  delete kf_;
  kf_ = nullptr;
}

void OMTObstacleTracker::ProjectBox(
    const airos::base::BBox2DF &box_origin, const Eigen::Matrix3d &transform,
    airos::base::BBox2DF *box_projected) {
  Eigen::Vector3d point;
  //  top left
  point << box_origin.xmin, box_origin.ymin, 1;
  point               = transform * point;
  box_projected->xmin = static_cast<float>(point[0] / point[2]);
  box_projected->ymin = static_cast<float>(point[1] / point[2]);
  //  bottom right
  point << box_origin.xmax, box_origin.ymax, 1;
  point               = transform * point;
  box_projected->xmax = static_cast<float>(point[0] / point[2]);
  box_projected->ymax = static_cast<float>(point[1] / point[2]);
}

bool OMTObstacleTracker::Predict(double timestamp) {
  for (auto &target : targets_) {
    target.Predict(timestamp, kf_);
  }
  return true;
}

int OMTObstacleTracker::CreateNewTarget(const TrackObjectPtrs &objects) {
  int created_count = 0;
  for (int i = 0; i < objects.size(); ++i) {
    if (!used_[i]) {
      auto det_box = objects[i]->tracker_info->detect.box;
      airos::base::RectF rect(
          det_box.left_top.x, det_box.left_top.y, det_box.Width(),
          det_box.Height());
      if (((objects[i]->tracker_info->detect.is_truncated ==
            airos::perception::algorithm::TriStatus::TRUE) &&
           OutOfValidX(rect, width_, height_, current_img_border_ * 10)) ||
          OutOfValidX(rect, width_, height_, current_img_border_)) {
        continue;
      }

      if (rect.height > current_init_height_) {
        Target target(omt_param_.target_param());
        int xmin = rect.x;
        int ymin = rect.y;
        int w    = rect.width;
        int h    = rect.height;
        track2d::DetectionRow detection;
        detection.tlwh      = DETECTBOX(xmin, ymin, w, h);
        KAL_DATA data       = kf_->initiate(detection.to_xyah());
        KAL_MEAN mean       = data.first;
        KAL_COVA covariance = data.second;

        target.Add(objects[i], mean, covariance);
        targets_.push_back(target);
        LOG_INFO << "Target " << target.id() << " is created by "
                 << objects[i]->indicator.frame_id << " ("
                 << objects[i]->indicator.patch_id << ")";
        created_count += 1;
      }
    }
  }
  return created_count;
}

bool OMTObstacleTracker::Associate(
    const std::vector<airos::perception::algorithm::ObjectDetectInfoPtr>
        &detect_result,
    TrackFrame *frame) {
  frame_list_.Add(frame);
  std::string camera_name = frame->camera_name;
  if (img_border_map_.find(camera_name) == img_border_map_.end()) {
    camera_name = "default";
  }
  current_img_border_  = img_border_map_[camera_name];
  current_init_height_ = init_height_map_[camera_name];
  current_min_height_  = min_height_map_[camera_name];

  for (auto &target : targets_) {
    target.RemoveOld(frame_list_.OldestFrameId());
  }

  // track prepare
  track2d::Detections detections;
  std::vector<int> detections_y;
  cur_frame_id_    = frame->frame_id;
  cnt_cur_objects_ = detect_result.size();
  detections.reserve(cnt_cur_objects_);
  detections_y.reserve(cnt_cur_objects_);
  for (int det_idx = 0; det_idx < detect_result.size(); ++det_idx) {
    const auto det_obj = detect_result[det_idx];

    track2d::DetectionRow tmp_det_row;
    int xmin = static_cast<int>(det_obj->box.left_top.x);
    int ymin = static_cast<int>(det_obj->box.left_top.y);
    int w =
        static_cast<int>(det_obj->box.right_bottom.x - det_obj->box.left_top.x);
    int h =
        static_cast<int>(det_obj->box.right_bottom.y - det_obj->box.left_top.y);
    airos::base::RectF rect(xmin, ymin, w, h);
    tmp_det_row.tlwh       = DETECTBOX(xmin, ymin, w, h);
    tmp_det_row.confidence = det_obj->type_id_confidence;

    if (((det_obj->is_truncated ==
          airos::perception::algorithm::TriStatus::TRUE) &&
         OutOfValidX(rect, width_, height_, current_img_border_ * 10)) ||
        OutOfValidX(rect, width_, height_, current_img_border_)) {
      continue;
    }

    auto track_ptr = std::make_shared<TrackObject>();
    track_ptr->tracker_info =
        std::make_shared<airos::perception::algorithm::ObjectTrackInfo>();
    track_ptr->tracker_info->detect = *det_obj;
    track_ptr->id                   = det_idx;
    track_ptr->timestamp            = frame->timestamp;
    track_ptr->confidence           = det_obj->type_id_confidence;
    track_ptr->indicator =
        PatchIndicator(frame->frame_id, det_idx, frame->camera_name);
    ProjectBox(
        airos::base::BBox2DF(rect), project_matrix_,
        &(track_ptr->projected_box));
    tmp_det_row.object.swap(track_ptr);
    detections.push_back(tmp_det_row);
    detections_y.emplace_back(ymin + h);
  }

  DYNAMICM trackers_iou =
      Eigen::MatrixXf::Zero(targets_.size(), targets_.size());
  for (int tg_outer_idx = 0; tg_outer_idx < targets_.size(); ++tg_outer_idx) {
    if (targets_[tg_outer_idx].lost_age_ > 3) {
      continue;
    }
    DETECTBOX bbox_outer = targets_[tg_outer_idx].GetLastTrackedObjectBox();
    for (int tg_inner_idx = 0; tg_inner_idx < targets_.size(); ++tg_inner_idx) {
      if (targets_[tg_inner_idx].lost_age_ > 3) {
        continue;
      }
      DETECTBOX bbox_inner = targets_[tg_inner_idx].GetLastTrackedObjectBox();

      float xmin_outer = bbox_outer(0);
      float xmax_outer = xmin_outer + bbox_outer(2);
      float ymin_outer = bbox_outer(2);
      float ymax_outer = ymin_outer + bbox_outer(3);
      float xmin_inner = bbox_inner(0);
      float xmax_inner = xmin_inner + bbox_inner(2);
      float ymin_inner = bbox_inner(2);
      float ymax_inner = ymin_inner + bbox_inner(3);
      if (xmin_outer > xmax_inner || xmin_inner > xmax_outer ||
          ymin_outer > ymax_inner || ymin_inner > ymax_outer) {
        continue;
      }

      DETECTBOXSS tmp_candidate(1, 4);
      tmp_candidate.row(0)                     = bbox_inner;
      Eigen::VectorXf tmp_iou                  = iou(bbox_outer, tmp_candidate);
      trackers_iou(tg_outer_idx, tg_inner_idx) = tmp_iou(0);
    }
  }
  std::vector<MATCH_DATA> large_iou_track_pairs;
  float iou_th = 0.5;
  for (int outer_idx = 0; outer_idx < targets_.size(); ++outer_idx) {
    for (int inner_idx = 0; inner_idx < outer_idx; ++inner_idx) {
      if (trackers_iou(outer_idx, inner_idx) > iou_th) {
        std::pair<int, int> tmp_pair(outer_idx, inner_idx);
        large_iou_track_pairs.emplace_back(tmp_pair);
      }
    }
  }

  TRACKER_MATCHED res;
  _match(detections, res);
  std::vector<MATCH_DATA> &matches = res.matches;
  std::vector<float> &confs        = res.confs;
  for (int outer_idx = 0; outer_idx < large_iou_track_pairs.size();
       ++outer_idx) {
    int lhs_track_idx = large_iou_track_pairs[outer_idx].first;
    int rhs_track_idx = large_iou_track_pairs[outer_idx].second;
    int lhs_det_idx   = -1;
    int rhs_det_idx   = -1;
    int lhs_match_idx = -1;
    int rhs_match_idx = -1;
    for (int inner_idx = 0; inner_idx < matches.size(); ++inner_idx) {
      if (matches[inner_idx].first == lhs_track_idx) {
        lhs_det_idx   = matches[inner_idx].second;
        lhs_match_idx = inner_idx;
        break;
      }
    }
    for (int inner_idx = 0; inner_idx < matches.size(); ++inner_idx) {
      if (matches[inner_idx].first == rhs_track_idx) {
        rhs_det_idx   = matches[inner_idx].second;
        rhs_match_idx = inner_idx;
        break;
      }
    }
    if (lhs_det_idx < 0 || rhs_det_idx < 0) {
      continue;
    }

    float det_y_diff   = detections_y[lhs_det_idx] - detections_y[rhs_det_idx];
    float track_y_diff = targets_[lhs_track_idx].GetLastTrackedObjectYCoord() -
                         targets_[rhs_track_idx].GetLastTrackedObjectYCoord();
    if (det_y_diff * track_y_diff < 0) {
      int tmp_idx                   = matches[rhs_match_idx].second;
      matches[rhs_match_idx].second = matches[lhs_match_idx].second;
      matches[lhs_match_idx].second = tmp_idx;
      float tmp_conf                = confs[lhs_match_idx];
      confs[lhs_match_idx]          = confs[rhs_match_idx];
      confs[rhs_match_idx]          = tmp_conf;
    }
  }

  for (int idx = 0; idx < matches.size(); idx++) {
    const MATCH_DATA &data = matches[idx];
    const float conf       = confs[idx];
    int track_idx          = data.first;
    int detection_idx      = data.second;
    targets_[track_idx].Update2D(frame, this->kf_, detections[detection_idx]);

    detections[detection_idx].object->confidence = conf;
    targets_[track_idx].Add(detections[detection_idx].object);
    targets_[track_idx].UpdateType(frame);
    targets_[track_idx].UpdateYaw(frame);
  }

  const std::vector<int> &unmatched_tracks = res.unmatched_tracks;
  for (const int &track_idx : unmatched_tracks) {
    targets_[track_idx].mark_missed();
  }
  const std::vector<int> &unmatched_detections = res.unmatched_detections;
  for (const int &detection_idx : unmatched_detections) {
    _initiate_track(detections[detection_idx]);
  }
  std::vector<Target>::iterator it;
  for (it = targets_.begin(); it != targets_.end();) {
    if ((*it).is_deleted())
      it = targets_.erase(it);
    else
      ++it;
  }

  frame->tracked_objects.clear();
  ClearTargets();
  used_.clear();

  for (Target &target : targets_) {
    if (target.IsTemporaryLost() || !target.is_confirmed()) {
      LOG_INFO << "timestamp:" << frame->timestamp
               << ",tid:" << target[-1]->tracker_info->tracker.track_id;
      continue;
    }
    frame->tracked_objects.push_back(target[-1]->tracker_info);
  }

  return true;
}

void OMTObstacleTracker::_match(
    const track2d::Detections &detections, TRACKER_MATCHED &res) {
  std::vector<int> confirmed_tracks;
  std::vector<int> unconfirmed_tracks;
  int idx = 0;
  for (auto &t : targets_) {
    if (t.is_confirmed()) {
      confirmed_tracks.push_back(idx);
    } else {
      unconfirmed_tracks.push_back(idx);
    }
    idx++;
  }
  TRACKER_MATCHED matcha =
      track2d::linear_assignment::getInstance()->matching_cascade(
          this, &OMTObstacleTracker::gated_matric, max_cosine_distance_,
          this->max_age_, this->targets_, detections, confirmed_tracks);
  std::vector<int> iou_track_candidates;
  iou_track_candidates.assign(
      unconfirmed_tracks.begin(), unconfirmed_tracks.end());
  std::vector<int>::iterator it;
  for (it = matcha.unmatched_tracks.begin();
       it != matcha.unmatched_tracks.end();) {
    int idx = *it;
    if (targets_[idx].lost_age_ == 1) {  // push into unconfirmed
      iou_track_candidates.push_back(idx);
      it = matcha.unmatched_tracks.erase(it);
      continue;
    }
    ++it;
  }
  TRACKER_MATCHED matchb =
      track2d::linear_assignment::getInstance()->min_cost_matching(
          this, &OMTObstacleTracker::iou_cost, this->max_iou_distance_,
          this->targets_, detections, iou_track_candidates,
          matcha.unmatched_detections);
  // get result
  res.matches.assign(matcha.matches.begin(), matcha.matches.end());
  res.matches.insert(
      res.matches.end(), matchb.matches.begin(), matchb.matches.end());
  // confs
  res.confs.assign(matcha.confs.begin(), matcha.confs.end());
  res.confs.insert(res.confs.end(), matchb.confs.begin(), matchb.confs.end());
  // unmatched_tracks
  res.unmatched_tracks.assign(
      matcha.unmatched_tracks.begin(), matcha.unmatched_tracks.end());
  res.unmatched_tracks.insert(
      res.unmatched_tracks.end(), matchb.unmatched_tracks.begin(),
      matchb.unmatched_tracks.end());
  res.unmatched_detections.assign(
      matchb.unmatched_detections.begin(), matchb.unmatched_detections.end());
}

void OMTObstacleTracker::_initiate_track(
    const track2d::DetectionRow &detection) {
  auto det_box = detection.object->tracker_info->detect.box;
  airos::base::RectF rect(
      det_box.left_top.x, det_box.left_top.y, det_box.Width(),
      det_box.Height());
  if (detection.object->tracker_info->detect.is_truncated ==
      airos::perception::algorithm::TriStatus::TRUE) {
    return;
  }
  KAL_DATA data       = kf_->initiate(detection.to_xyah());
  KAL_MEAN mean       = data.first;
  KAL_COVA covariance = data.second;
  Target target(omt_param_.target_param());
  target.Add(detection.object, mean, covariance);
  targets_.push_back(target);
}

DYNAMICM OMTObstacleTracker::gated_matric(
    std::vector<Target> &tracks, const track2d::Detections &dets,
    const std::vector<int> &track_indices,
    const std::vector<int> &detection_indices) {
  std::vector<int> targets;
  for (int i : track_indices) {
    targets.push_back(tracks[i].id_);
  }
  DYNAMICM cost_matrix =
      Eigen::MatrixXf::Zero(targets.size(), detection_indices.size());
  cost_matrix = track2d::linear_assignment::getInstance()->gate_cost_matrix(
      this->kf_, cost_matrix, tracks, dets, track_indices, detection_indices);

  DYNAMICM res = cost_matrix;
  DYNAMICM motion_shape_matrix =
      motion_shape_cost(tracks, dets, track_indices, detection_indices);

  int rows = track_indices.size();
  int cols = detection_indices.size();
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (res(i, j) != INFTY_COST) {
        res(i, j) = res(i, j) * omt_param_.weight().wapp() +
                    motion_shape_matrix(i, j) * omt_param_.weight().wmot();
      }
    }
  }
  return res;
}

DYNAMICM OMTObstacleTracker::iou_cost(
    std::vector<Target> &tracks, const track2d::Detections &dets,
    const std::vector<int> &track_indices,
    const std::vector<int> &detection_indices) {
  int rows             = track_indices.size();
  int cols             = detection_indices.size();
  DYNAMICM cost_matrix = Eigen::MatrixXf::Zero(rows, cols);
  for (int i = 0; i < rows; i++) {
    int track_idx = track_indices[i];
    if (tracks[track_idx].lost_age_ > 1) {
      cost_matrix.row(i) = Eigen::RowVectorXf::Constant(cols, INFTY_COST);
      continue;
    }
    DETECTBOX bbox = tracks[track_idx].to_tlwh();
    int csize      = detection_indices.size();
    DETECTBOXSS candidates(csize, 4);
    for (int k = 0; k < csize; k++) {
      candidates.row(k) = dets[detection_indices[k]].tlwh;
    }
    Eigen::RowVectorXf rowV =
        (1. - iou(bbox, candidates).array()).matrix().transpose();
    cost_matrix.row(i) = rowV;
  }

  return cost_matrix;
}

Eigen::VectorXf OMTObstacleTracker::iou(
    DETECTBOX &bbox, DETECTBOXSS &candidates) {
  float bbox_tl_1 = bbox[0];
  float bbox_tl_2 = bbox[1];
  float bbox_br_1 = bbox[0] + bbox[2];
  float bbox_br_2 = bbox[1] + bbox[3];
  float area_bbox = bbox[2] * bbox[3];

  Eigen::Matrix<float, -1, 2> candidates_tl;
  Eigen::Matrix<float, -1, 2> candidates_br;

  candidates_tl = candidates.leftCols(2);
  candidates_br = candidates.rightCols(2) + candidates_tl;

  int size = static_cast<int>(candidates.rows());
  Eigen::VectorXf res(size);
  for (int i = 0; i < size; i++) {
    float tl_1 = std::max(bbox_tl_1, candidates_tl(i, 0));
    float tl_2 = std::max(bbox_tl_2, candidates_tl(i, 1));
    float br_1 = std::min(bbox_br_1, candidates_br(i, 0));
    float br_2 = std::min(bbox_br_2, candidates_br(i, 1));

    float w                 = br_1 - tl_1;
    w                       = (w < 0 ? 0 : w);
    float h                 = br_2 - tl_2;
    h                       = (h < 0 ? 0 : h);
    float area_intersection = w * h;
    float area_candidates   = candidates(i, 2) * candidates(i, 3);
    res[i] =
        area_intersection / (area_bbox + area_candidates - area_intersection);
  }
  return res;
}

DYNAMICM OMTObstacleTracker::motion_shape_cost(
    std::vector<Target> &tracks, const track2d::Detections &dets,
    const std::vector<int> &track_indices,
    const std::vector<int> &detection_indices) {
  int rows                     = track_indices.size();
  int cols                     = detection_indices.size();
  DYNAMICM motion_shape_matrix = Eigen::MatrixXf::Zero(rows, cols);

  for (int i = 0; i < rows; i++) {
    int track_idx        = track_indices[i];
    DETECTBOX track_bbox = tracks[track_idx].to_tlwh();
    int dsize            = detection_indices.size();
    DETECTBOXSS candidates(dsize, 4);
    for (int j = 0; j < dsize; j++) {
      candidates.row(j) = dets[detection_indices[j]].tlwh;
    }
    Eigen::RowVectorXf rowV = motion_shape_distance(track_bbox, candidates)
                                  .array()
                                  .matrix()
                                  .transpose();
    motion_shape_matrix.row(i) = rowV;
  }
  return motion_shape_matrix;
}

Eigen::VectorXf OMTObstacleTracker::motion_shape_distance(
    DETECTBOX &bbox, DETECTBOXSS &candidates) {
  float track_bbox_tl_x = bbox[0];
  float track_bbox_tl_y = bbox[1];
  float track_bbox_w    = abs(bbox[2]);
  float track_bbox_h    = abs(bbox[3]);
  float _weight_motion  = -0.5;
  float _weight_shape   = -1.5;

  int size = static_cast<int>(candidates.rows());
  Eigen::VectorXf res(size);
  for (int i = 0; i < size; i++) {
    float detect_bbox_tl_x = candidates(i, 0);
    float detect_bbox_tl_y = candidates(i, 1);
    float detect_bbox_w    = candidates(i, 2);
    float detect_bbox_h    = candidates(i, 3);

    // motion cost
    float factor_motion_1 =
        pow((track_bbox_tl_x - detect_bbox_tl_x) / detect_bbox_w, 2);
    float factor_motion_2 =
        pow((track_bbox_tl_y - detect_bbox_tl_y) / detect_bbox_h, 2);
    float motion_cost =
        exp(_weight_motion * (factor_motion_1 + factor_motion_2));
    // shape cost
    float factor_shape_1 =
        abs(track_bbox_h - detect_bbox_h) / (track_bbox_h + detect_bbox_h);
    float factor_shape_2 =
        abs(track_bbox_w - detect_bbox_w) / (track_bbox_w + detect_bbox_w);
    float shape_cost = exp(_weight_shape * (factor_shape_1 + factor_shape_2));
    res[i]           = (1 - motion_cost) + (1 - shape_cost);
  }
  return res;
}

void OMTObstacleTracker::ClearTargets() {
  // move the tails to blank fronts, and remove the tails
  int left = 0;
  int end  = static_cast<int>(targets_.size() - 1);
  while (left <= end) {
    if ((targets_[left].Size() == 0)) {
      while ((left < end) && (targets_[end].Size() == 0)) {
        --end;
      }
      if (left >= end) {
        break;
      }
      targets_[left] = targets_[end];
      --end;
    }
    ++left;
  }
  targets_.erase(targets_.begin() + left, targets_.end());
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
