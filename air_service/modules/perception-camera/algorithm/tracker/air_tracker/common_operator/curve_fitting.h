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

#include <unordered_map>
#include <vector>

#include <Eigen/Core>
#include <opencv2/opencv.hpp>

namespace airos {
namespace perception {
namespace algorithm {
class CurveFitting {
 public:
  CurveFitting() = default;
  // void Init();
  void AddMeasure(const cv::Point2d& z);
  int size() const;
  bool IsAligned();
  Eigen::Vector3d GetState();

 private:
  cv::Mat PloyFit(const std::vector<cv::Point2d>& in_point, int order);

 private:
  int window_ = 20;
  int order_  = 1;

  std::vector<cv::Point2d> measures_;
  std::vector<cv::Point2d> translation_;
  double x_min_                = 1e9;
  double y_min_                = 1e9;
  double x_max_                = 0;
  double y_max_                = 0;
  double aligned_thresh_meter_ = 4.0;
  double aligned_thresh_num_   = 10;

  Eigen::Vector3d state_;
};
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
