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

#include "linear_assignment.h"

#include <Eigen/Cholesky>

#include "hungarianoper.h"

namespace airos {
namespace perception {
namespace algorithm {
namespace track2d {

linear_assignment* linear_assignment::instance = NULL;
linear_assignment::linear_assignment() {}

linear_assignment* linear_assignment::getInstance() {
  if (instance == NULL) {
    instance = new linear_assignment();
  }

  return instance;
}

TRACKER_MATCHED linear_assignment::matching_cascade(
    OMTObstacleTracker* distance_metric,
    OMTObstacleTracker::GATED_METRIC_FUNC distance_metric_func,
    float max_distance, int cascade_depth, std::vector<Target>& tracks,
    const Detections& detections, std::vector<int>& track_indices,
    std::vector<int> detection_indices) {
  TRACKER_MATCHED res;

  for (size_t i = 0; i < detections.size(); i++) {
    detection_indices.push_back(static_cast<int>(i));
  }

  std::vector<int> unmatched_detections;
  unmatched_detections.assign(
      detection_indices.begin(), detection_indices.end());
  res.matches.clear();
  res.confs.clear();
  std::vector<int> track_indices_l;

  std::map<int, int> matches_trackid;

  for (int level = 0; level < cascade_depth; level++) {
    if (unmatched_detections.size() == 0) {
      break;  // No detections left;
    }

    track_indices_l.clear();

    for (int k : track_indices) {
      if (tracks[k].lost_age_ == 1 + level) {
        track_indices_l.push_back(k);
      }
    }

    if (track_indices_l.size() == 0) {
      continue;  // Nothing to match at this level.
    }

    TRACKER_MATCHED tmp = min_cost_matching(
        distance_metric, distance_metric_func, max_distance, tracks, detections,
        track_indices_l, unmatched_detections);
    unmatched_detections.assign(
        tmp.unmatched_detections.begin(), tmp.unmatched_detections.end());

    for (size_t i = 0; i < tmp.matches.size(); i++) {
      MATCH_DATA pa = tmp.matches[i];
      float ca      = tmp.confs[i];
      res.matches.push_back(pa);
      res.confs.push_back(ca);
      matches_trackid.insert(pa);
    }
  }

  res.unmatched_detections.assign(
      unmatched_detections.begin(), unmatched_detections.end());

  for (size_t i = 0; i < track_indices.size(); i++) {
    int tid = track_indices[i];

    if (matches_trackid.find(tid) == matches_trackid.end()) {
      res.unmatched_tracks.push_back(tid);
    }
  }

  return res;
}

TRACKER_MATCHED linear_assignment::min_cost_matching(
    OMTObstacleTracker* distance_metric,
    OMTObstacleTracker::GATED_METRIC_FUNC distance_metric_func,
    float max_distance, std::vector<Target>& tracks,
    const Detections& detections, std::vector<int>& track_indices,
    std::vector<int>& detection_indices) {
  TRACKER_MATCHED res;

  if ((detection_indices.size() == 0) || (track_indices.size() == 0)) {
    res.matches.clear();
    res.confs.clear();
    res.unmatched_tracks.assign(track_indices.begin(), track_indices.end());
    res.unmatched_detections.assign(
        detection_indices.begin(), detection_indices.end());
    return res;
  }

  DYNAMICM cost_matrix = (distance_metric->*(distance_metric_func))(
      tracks, detections, track_indices, detection_indices);

  float tmp = 0;
  for (int i = 0; i < cost_matrix.rows(); i++) {
    for (int j = 0; j < cost_matrix.cols(); j++) {
      tmp = cost_matrix(i, j);

      if (tmp > max_distance) {
        cost_matrix(i, j) = max_distance + 1e-5;
      }
    }
  }

  Eigen::Matrix<float, -1, 2, Eigen::RowMajor> indices =
      HungarianOper::Solve(cost_matrix);
  res.matches.clear();
  res.confs.clear();
  res.unmatched_tracks.clear();
  res.unmatched_detections.clear();

  bool flag = false;
  for (size_t col = 0; col < detection_indices.size(); col++) {
    flag = false;

    for (int i = 0; i < indices.rows(); i++) {
      if (indices(i, 1) == col) {
        flag = true;
        break;
      }
    }

    if (flag == false) {
      res.unmatched_detections.push_back(detection_indices[col]);
    }
  }

  for (size_t row = 0; row < track_indices.size(); row++) {
    flag = false;

    for (int i = 0; i < indices.rows(); i++) {
      if (indices(i, 0) == row) {
        flag = true;
        break;
      }
    }

    if (flag == false) {
      res.unmatched_tracks.push_back(track_indices[row]);
    }
  }

  int row           = 0;
  int col           = 0;
  int track_idx     = 0;
  int detection_idx = 0;
  for (int i = 0; i < indices.rows(); i++) {
    row = indices(i, 0);
    col = indices(i, 1);

    track_idx     = track_indices[row];
    detection_idx = detection_indices[col];

    if (cost_matrix(row, col) > max_distance) {
      res.unmatched_tracks.push_back(track_idx);
      res.unmatched_detections.push_back(detection_idx);
    } else {
      res.matches.push_back(std::make_pair(track_idx, detection_idx));
      float tmp_conf = 1. - cost_matrix(row, col);
      if (tmp_conf < 0.0) {
        tmp_conf = 0.0;
      }
      if (tmp_conf > 1.0) {
        tmp_conf = 1.0;
      }
      res.confs.push_back(tmp_conf);
    }
  }

  return res;
}

DYNAMICM linear_assignment::gate_cost_matrix(
    KalmanFilter* kf, DYNAMICM& cost_matrix, std::vector<Target>& tracks,
    const Detections& detections, const std::vector<int>& track_indices,
    const std::vector<int>& detection_indices, float gated_cost,
    bool only_position) {
  int gating_dim          = (only_position == true ? 2 : 4);
  double gating_threshold = KalmanFilter::chi2inv95[gating_dim];
  std::vector<DETECTBOX, Eigen::aligned_allocator<DETECTBOX>> measurements;

  for (int i : detection_indices) {
    auto t = detections[i];
    measurements.push_back(t.to_xyah());
  }
  for (size_t i = 0; i < track_indices.size(); i++) {
    auto& track = tracks[track_indices[i]];

    KAL_HDATA pa = kf->project(track.mean_, track.covariance_);

    KAL_HMEAN mean1 = pa.first;
    // KAL_HCOVA covariance1 = pa.second;
    KAL_HCOVA covariance1 = pa.second + alpha_ * identity_;

    DETECTBOXSS d(measurements.size(), 4);
    int pos = 0;

    for (DETECTBOX box : measurements) {
      d.row(pos++) = box - mean1;
    }

    Eigen::Matrix<float, -1, -1, Eigen::RowMajor> factor =
        covariance1.llt().matrixL();
    Eigen::Matrix<float, -1, -1> z = factor.triangularView<Eigen::Lower>()
                                         .solve<Eigen::OnTheRight>(d)
                                         .transpose();
    Eigen::Matrix<float, -1, -1> zz = (z.array() * z.array()).matrix();
    auto gating_distance            = zz.colwise().sum();

    for (int j = 0; j < gating_distance.cols(); j++) {
      if (gating_distance(0, j) > gating_threshold) {
        cost_matrix(i, j) = gated_cost;
      }
    }
  }

  return cost_matrix;
}

}  // namespace track2d
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
