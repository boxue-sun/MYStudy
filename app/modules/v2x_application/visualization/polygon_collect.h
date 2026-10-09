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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "app/modules/v2x_application/usecase/common/tensor.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"
#include "opencv2/opencv.hpp"

namespace airos {
namespace perception {
namespace usecase {

class PolygonCollector {
 public:
  PolygonCollector()  = default;
  ~PolygonCollector() = default;

  void AddPolygon(
      const std::string& nm,
      const std::vector<std::vector<airos::perception::usecase::Vec2d>>& poly) {
    usecase_polygon_[nm] = poly;
  }

  void AddSinglePoint(const std::string& nm, const std::vector<Tensor>& point) {
    single_point_t_[nm] = point;
  }

  void AddSignalPoint(
      const std::string& nm, const std::map<std::string, Tensor>& point) {
    signal_point_[nm] = point;
  }

  void AddTurnType(
      const std::vector<std::pair<Tensor, std::string>>& turn_type) {
    turn_type_ = turn_type;
  }

  void AddGridMap(const cv::Mat& image) {
    grid_map_ = image;
  }

  void AddHighLightObj(
      const std::string& nm, const std::vector<int64_t>& obj_ids) {
    high_light_obj_[nm] = obj_ids;
  }

  void AddLaneIds(
      const std::string& nm, const std::vector<std::string>& lane_ids) {
    high_light_lane_ids[nm] = lane_ids;
  }

  void AddTextMessage(
      const std::string& nm,
      const std::vector<std::pair<Tensor, std::string>>& messages) {
    text_message_[nm] = messages;
  }

  std::map<
      std::string, std::vector<std::vector<airos::perception::usecase::Vec2d>>>
  GetPolygon() {
    return usecase_polygon_;
  }

  std::map<std::string, std::vector<Tensor>> GetSinglePointT() {
    return single_point_t_;
  }

  std::map<std::string, std::map<std::string, Tensor>> GetSignalPoint() {
    return signal_point_;
  }

  std::vector<std::pair<Tensor, std::string>> GetTurnType() {
    return turn_type_;
  }

  std::map<std::string, std::vector<int64_t>> GetHighLightObj() {
    return high_light_obj_;
  }

  std::map<std::string, std::vector<std::pair<Tensor, std::string>>>
  GetTextMessage() {
    return text_message_;
  }

  std::map<std::string, std::vector<std::string>> GetLaneIds() {
    return high_light_lane_ids;
  }

 private:
  void Init();
  std::map<
      std::string, std::vector<std::vector<airos::perception::usecase::Vec2d>>>
      usecase_polygon_;
  std::map<std::string, std::vector<Tensor>> single_point_t_;
  std::map<std::string, std::map<std::string, Tensor>> signal_point_;
  std::map<std::string, std::vector<int64_t>> high_light_obj_;
  std::map<std::string, std::vector<std::pair<Tensor, std::string>>>
      text_message_;
  std::map<std::string, std::vector<std::string>> high_light_lane_ids;
  std::vector<std::pair<Tensor, std::string>> turn_type_;
  cv::Mat grid_map_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
