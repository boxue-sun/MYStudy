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

#include "app/modules/v2x_application/visualization/visualization_internal.h"

#include <iostream>

#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace usecase {

void UsecaseVisualizer::Init() {
  window_wrapper_.Init();

  // polygon_collector_ = PolygonCollector::Instance();
  auto polygon_collector = base::Singleton<PolygonCollector>::get_instance();
  disp_thread_.reset(
      new std::thread(std::bind(&UsecaseVisualizer::DispThread, this)));
  return;
}

void UsecaseVisualizer::DispThread() {
  auto polygon_collector = base::Singleton<PolygonCollector>::get_instance();
  while (!is_exit_) {
    window_wrapper_.ResetBackground();

    {
      std::lock_guard<std::mutex> _lock(mutex_);
      if (!event_output_result_que_.empty()) {
        event_output_result_ = event_output_result_que_.front();
        event_output_result_que_.pop();
      }
    }

    if (event_output_result_ == nullptr ||
        event_output_result_->perception_obstacle_size() < 1) {
      window_wrapper_.Show();
      window_wrapper_.KeyCallbackFunc(cv::waitKey(1));
      continue;
    }

    window_wrapper_.CalculateScale(event_output_result_);
    window_wrapper_.DrawObstacle(event_output_result_);

    auto usecase_polygon = polygon_collector->GetPolygon();
    if (window_wrapper_.lane_congestion_) {
      DrawPolyWithID("lane_congestion", usecase_polygon);
      auto signal_point = polygon_collector->GetSignalPoint();
      DrawPoint("lane_congestion", signal_point);
      auto text_message = polygon_collector->GetTextMessage();
      auto message      = text_message["lane_congestion_obj_dura"];
      for (auto& iter : message) {
        auto pt  = iter.first;
        auto str = iter.second;
        cv::Point2d position(pt.x, pt.y);
        position = window_wrapper_.TranslatePosition(position);
        window_wrapper_.DrawText(str, position, cv::Scalar(204, 50, 153));
      }
    }

    if (window_wrapper_.draw_get_text_ == 1) {
      if (!window_wrapper_.get_text_doing) {
        window_wrapper_.get_text_doing = true;
        std::thread text_thread(
            std::bind(&WindowWrapper::GetText, &window_wrapper_));
        text_thread.detach();
      }
      window_wrapper_.draw_get_text_ = 0;
    }

    auto turn_types = polygon_collector->GetTurnType();
    for (size_t i = 0; i < turn_types.size(); ++i) {
      cv::Point2d position(turn_types[i].first.x, turn_types[i].first.y);
      position = window_wrapper_.TranslatePosition(position);
      window_wrapper_.DrawText(
          turn_types[i].second, position, cv::Scalar(0, 0, 255));
    }

    if (window_wrapper_.draw_polygon_ == 1) {
      window_wrapper_.ShowMakePolygon();
    } else if (window_wrapper_.draw_polygon_ == 2) {
      window_wrapper_.ShowMakeLanePolygon();
    }

    if (window_wrapper_.draw_lane_and_direction_ == 1) {
      window_wrapper_.DrawLanePolygon();
    } else if (window_wrapper_.draw_lane_and_direction_ == 2) {
      window_wrapper_.DrawLaneDirection();
    } else if (window_wrapper_.draw_lane_and_direction_ == 3) {
      window_wrapper_.DrawLaneDirection();
      window_wrapper_.DrawLanePolygon();
    }

    window_wrapper_.Show();
    window_wrapper_.KeyCallbackFunc(cv::waitKey(1));
  }
}

void UsecaseVisualizer::DrawPoly(
    const std::string& nm,
    std::map<
        std::string,
        std::vector<std::vector<airos::perception::usecase::Vec2d>>>
        polygons) {
  auto iter = polygons.find(nm);
  if (iter != polygons.end()) {
    for (auto poly_ : iter->second) {
      std::vector<cv::Point2d> polygon_boundary;
      for (const auto& pt : poly_) {
        cv::Point2d position(pt.X(), pt.Y());
        position = window_wrapper_.TranslatePosition(position);
        polygon_boundary.push_back(position);
      }
      window_wrapper_.DrawPolygon(polygon_boundary, cv::Scalar(0, 0, 255));
    }
  }
}

void UsecaseVisualizer::DrawPoint(
    const std::string& nm, std::map<std::string, std::vector<Tensor>> points) {
  auto iter = points.find(nm);
  if (iter != points.end()) {
    for (size_t i = 0; i < iter->second.size(); ++i) {
      cv::Point2d position(iter->second[i].x, iter->second[i].y);
      position = window_wrapper_.TranslatePosition(position);
      window_wrapper_.DrawPoint(position);
      // window_wrapper_.DrawText(std::to_string(i), position,
      // cv::Scalar(0,255,0));
    }
  }
}
void UsecaseVisualizer::DrawPoint(
    const std::string& nm,
    std::map<std::string, std::map<std::string, Tensor>> points) {
  auto iter = points.find(nm);
  if (iter != points.end()) {
    for (auto iter1 = iter->second.begin(); iter1 != iter->second.end();
         ++iter1) {
      cv::Point2d position(iter1->second.x, iter1->second.y);
      position = window_wrapper_.TranslatePosition(position);
      window_wrapper_.DrawPoint(position);
      window_wrapper_.DrawText(
          iter1->first, cv::Point2f(position.x, position.y),
          cv::Scalar(0, 255, 0));
    }
  }
}
void UsecaseVisualizer::DrawPolyWithID(
    const std::string& nm,
    std::map<
        std::string,
        std::vector<std::vector<airos::perception::usecase::Vec2d>>>
        polygons) {
  auto iter = polygons.find(nm);
  if (iter != polygons.end()) {
    int id = 0;
    for (auto poly_ : iter->second) {
      std::vector<cv::Point2d> polygon_boundary;
      for (const auto& pt : poly_) {
        cv::Point2d position(pt.X(), pt.Y());
        position = window_wrapper_.TranslatePosition(position);
        polygon_boundary.push_back(position);
      }
      window_wrapper_.DrawPolygon(polygon_boundary, cv::Scalar(0, 0, 255));
      if (id % 2 == 0) {
        float x = 0.0, y = 0.0;
        for (auto& poly_ : polygon_boundary) {
          x += poly_.x;
          y += poly_.y;
        }
        if (!polygon_boundary.empty()) {
          x /= polygon_boundary.size();
          y /= polygon_boundary.size();
        }
        window_wrapper_.DrawText(
            std::to_string(id / 2), cv::Point2f(x, y), cv::Scalar(255, 0, 0));
      }
      id++;
    }
  }
}

void UsecaseVisualizer::DrawHightLightObj(
    const std::string& nm, std::map<std::string, std::vector<int64_t>> objs) {
  auto iter = objs.find(nm);
  if (iter != objs.end()) {
    for (auto id : iter->second) {
      window_wrapper_.AddHighLightObj(id);
    }
  }
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
