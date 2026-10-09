
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

#include "app/modules/v2x_application/usecase/base/frame_data.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "app/modules/v2x_application/usecase/common/util.h"
#include "base/common/singleton.h"
#include "base/io/protobuf_util.h"
#include "opencv2/core/types.hpp"
#include "opencv2/opencv.hpp"

namespace airos {
namespace perception {
namespace usecase {

void FrameTrackData::Init() {
  auto framedata_param = Factory<BaseParams>::Instance().GetShared("dataframe");
  base::Singleton<ConfigManager>::get_instance()->LoadFor(framedata_param);

  stop_speed_thre_ = framedata_param->GetVal("stop_speed_thre").Cast<float>();
  stop_speed_cnt_thre_ =
      framedata_param->GetVal("stop_speed_cnt_thre").Cast<float>();
  stop_shift_cnt_thre_ =
      framedata_param->GetVal("stop_shift_cnt_thre").Cast<float>();
  tracked_life_ = framedata_param->GetVal("tracked_life").Cast<int>();

  auto usecare_param = Factory<BaseParams>::Instance().GetShared("usecase");
  base::Singleton<ConfigManager>::get_instance()->LoadFor(usecare_param);

  if (airos::base::ParseProtobufFromFile(
          usecare_param->GetVal("lane_param_path").Cast<std::string>(),
          &mini_map_params_)) {
    cross_center_.x = mini_map_params_.feature_position().x();
    cross_center_.y = mini_map_params_.feature_position().y();
    cross_center_.z = mini_map_params_.feature_position().z();
  }
}

void FrameTrackData::Update(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        obstacles) {
  timestamp_sec_ = obstacles->header().timestamp_sec();

  // check whether the tracked obj needs to be cleaned up
  for (auto iter = trackeds_.begin(); iter != trackeds_.end();) {
    iter->CheckLost();
    if (iter->Life() < 0) {
      std::iter_swap(trackeds_.begin(), iter++);
      trackeds_.pop_front();
    } else {
      ++iter;
    }
  }

  // find need matched cur-frame-obj & tracked-obj
  track_ids_.clear();
  std::unordered_set<size_t> visited_tracked;
  std::unordered_set<size_t> visited_objs;
  std::unordered_set<int> need_match_obj;
  std::unordered_set<int> need_match_tracked;
  std::map<int64_t, int> cur_frame_pre_ids;
  std::unordered_set<int> track_per_ids;

  for (size_t i = 0; i < trackeds_.size(); ++i) {
    const auto& track = trackeds_[i];
    track_ids_.insert(track.Id());
    track_per_ids.insert(track.PerceptionId());
  }
  for (int j = 0; j < obstacles->perception_obstacle_size(); ++j) {
    const auto& obj             = obstacles->perception_obstacle(j);
    cur_frame_pre_ids[obj.id()] = j;
    if (!track_per_ids.count(obj.id())) {
      need_match_obj.insert(j);
    }
  }
  for (size_t i = 0; i < trackeds_.size(); ++i) {
    auto& tracked = trackeds_[i];
    if (!cur_frame_pre_ids.count(tracked.PerceptionId())) {
      need_match_tracked.insert(i);
    } else {
      int j           = cur_frame_pre_ids[tracked.PerceptionId()];
      const auto& obj = obstacles->perception_obstacle(j);
      tracked.Add(obj);
      visited_tracked.insert(i);
      visited_objs.insert(j);
    }
  }

  std::vector<Score> match_scores;
  match_scores.reserve(need_match_obj.size() * need_match_tracked.size());
  Score score;
  for (const auto& j : need_match_obj) {
    const auto& obj = obstacles->perception_obstacle(j);
    for (const auto& i : need_match_tracked) {
      auto& tracked = trackeds_[i];
      score.target  = i;
      score.object  = j;
      score.score   = MatchScore(obj, tracked);
      match_scores.push_back(score);
    }
  }
  std::sort(match_scores.begin(), match_scores.end(), std::greater<Score>());
  // Warning: the match strategy may mis objs: score in (ignore_thresh_,
  // diou_thresh_) Only happen when an new obj flash near another one But now
  // the strategy is work well
  for (const auto& score : match_scores) {
    if (visited_tracked.count(score.target) ||
        visited_objs.count(score.object)) {
      continue;
    }
    if (score.score < ignore_thresh_) {
      continue;
    }

    auto& track_obj = trackeds_[score.target];
    const auto& obj = obstacles->perception_obstacle(score.object);
    if (score.score > diou_thresh_) {
      track_obj.Add(obj);
    }
    visited_tracked.insert(score.target);
    visited_objs.insert(score.object);
  }

  // add new appear obj
  for (int j = 0; j < obstacles->perception_obstacle_size(); ++j) {
    if (visited_objs.find(j) == visited_objs.end()) {
      const auto& obj = obstacles->perception_obstacle(j);
      TrackObj track_obj(
          global_id_++, tracked_life_, stop_speed_thre_, stop_speed_cnt_thre_,
          stop_shift_cnt_thre_, cross_center_);
      track_obj.Add(obj);
      trackeds_.push_back(std::move(track_obj));
    }
  }
}

float FrameTrackData::DIoU(
    const airos::perception::PerceptionObstacle& obstacle,
    const TrackObj& obj) {
  if (std::fabs(obstacle.length()) < 0.00000000000001 ||
      std::fabs(obstacle.width()) < 0.00000000000001 ||
      std::fabs(obstacle.height()) < 0.00000000000001) {
    return -1.0;
  }

  Tensor pos   = obj.StopPos();
  float width  = obj.StopWidth();
  float length = obj.StopLength();
  float theta  = obj.StopTheta();

  float cx_1 = pos.x;
  float cy_1 = pos.y;
  float cx_2 = obstacle.position().x();
  float cy_2 = obstacle.position().y();

  float ex_xmin = std::numeric_limits<float>::max();
  float ex_ymin = std::numeric_limits<float>::max();
  float ex_xmax = std::numeric_limits<float>::min();
  float ex_ymax = std::numeric_limits<float>::min();

  float to_degree        = 180.0 / M_PI;
  cv::RotatedRect z_rect = cv::RotatedRect(
      cv::Point2f(cx_1, cy_1), cv::Size2f(width, length), theta * to_degree);
  cv::RotatedRect o_rect = cv::RotatedRect(
      cv::Point2f(obstacle.position().x(), obstacle.position().y()),
      cv::Size2f(obstacle.width(), obstacle.length()),
      obstacle.theta() * to_degree);

  cv::Point2f poly[4];
  z_rect.points(poly);
  for (int i = 0; i < 4; ++i) {
    ex_xmin = std::min(poly[i].x, ex_xmin);
    ex_ymin = std::min(poly[i].y, ex_ymin);
    ex_xmax = std::max(poly[i].x, ex_xmax);
    ex_ymax = std::max(poly[i].y, ex_ymax);
    // if (poly[i].x < ex_xmin) {
    //     ex_xmin = poly[i].x;
    // }
    // if (poly[i].y < ex_ymin) {
    //     ex_ymin = poly[i].y;
    // }
    // if (poly[i].x > ex_xmax) {
    //     ex_xmax = poly[i].x;
    // }
    // if (poly[i].y > ex_ymax) {
    //     ex_ymax = poly[i].y;
    // }
  }
  o_rect.points(poly);
  for (int i = 0; i < 4; ++i) {
    ex_xmin = std::min(poly[i].x, ex_xmin);
    ex_ymin = std::min(poly[i].y, ex_ymin);
    ex_xmax = std::max(poly[i].x, ex_xmax);
    ex_ymax = std::max(poly[i].y, ex_ymax);
    // if (poly[i].x < ex_xmin) {
    //     ex_xmin = poly[i].x;
    // }
    // if (poly[i].y < ex_ymin) {
    //     ex_ymin = poly[i].y;
    // }
    // if (poly[i].x > ex_xmax) {
    //     ex_xmax = poly[i].x;
    // }
    // if (poly[i].y > ex_ymax) {
    //     ex_ymax = poly[i].y;
    // }
  }

  float c_dist  = (cx_1 - cx_2) * (cx_1 - cx_2) + (cy_1 - cy_2) * (cy_1 - cy_2);
  float ex_dist = (ex_xmin - ex_xmax) * (ex_xmin - ex_xmax) +
                  (ex_ymin - ex_ymax) * (ex_ymin - ex_ymax);

  std::vector<cv::Point2f> rect;
  int intersectionType = -1;

  try {
    intersectionType = cv::rotatedRectangleIntersection(z_rect, o_rect, rect);
  } catch (...) {
    return 1.0;
  }

  if (intersectionType == 0) {
    return 0 - c_dist / ex_dist;
  }

  if (intersectionType == -1) {
    return 1 - c_dist / ex_dist;
  }

  double insec  = cv::contourArea(rect);
  double area_z = width * length;
  double area_o = obstacle.width() * obstacle.length();
  double iou    = insec / (area_z + area_o - insec);

  return iou - c_dist / ex_dist;
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
