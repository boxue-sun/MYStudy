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

#include "curve_fitting.h"

namespace airos {
namespace perception {
namespace algorithm {

int CurveFitting::size() const {
  return measures_.size();
}

void CurveFitting::AddMeasure(const cv::Point2d& z) {
  measures_.push_back(z);

  translation_.clear();
  x_min_ = 1e9;
  y_min_ = 1e9;
  x_max_ = 0;
  y_max_ = 0;
  for (auto iter = measures_.rbegin();
       iter != measures_.rbegin() + window_ && iter != measures_.rend();
       ++iter) {
    x_max_ = iter->x > x_max_ ? iter->x : x_max_;
    y_max_ = iter->y > y_max_ ? iter->y : y_max_;
    x_min_ = iter->x < x_min_ ? iter->x : x_min_;
    y_min_ = iter->y < y_min_ ? iter->y : y_min_;
    translation_.push_back(*iter);
  }
  for (auto& point : translation_) {
    point.x -= x_min_;
    point.y -= y_min_;
  }
}

Eigen::Vector3d CurveFitting::GetState() {
  cv::Mat mat_k = PloyFit(translation_, order_);
  // translation_ is reversed
  cv::Point2d point1(translation_.begin()->x, 0);
  cv::Point2d point2((translation_.begin() + 2)->x, 0);

  for (int j = 0; j < order_ + 1; ++j) {
    point1.y += mat_k.at<double>(j, 0) * std::pow(point1.x, j);
    point2.y += mat_k.at<double>(j, 0) * std::pow(point2.x, j);
  }

  double theta = std::atan2(point1.y - point2.y, point1.x - point2.x);

  return Eigen::Vector3d(0, 0, theta);
}

cv::Mat CurveFitting::PloyFit(
    const std::vector<cv::Point2d>& in_point, int order) {
  int size = in_point.size();
  // 所求未知数个数
  int x_num = order + 1;
  // 构造矩阵U和Y
  cv::Mat mat_u(size, x_num, CV_64F);
  cv::Mat mat_y(size, 1, CV_64F);

  for (int i = 0; i < mat_u.rows; ++i) {
    for (int j = 0; j < mat_u.cols; ++j) {
      mat_u.at<double>(i, j) = std::pow(in_point[i].x, j);
    }
  }
  for (int i = 0; i < mat_y.rows; ++i) {
    mat_y.at<double>(i, 0) = in_point[i].y;
  }

  // 矩阵运算，获得系数矩阵K
  cv::Mat mat_k(x_num, 1, CV_64F);
  mat_k = (mat_u.t() * mat_u).inv() * mat_u.t() * mat_y;
  return mat_k;
}

bool CurveFitting::IsAligned() {
  return translation_.size() > aligned_thresh_num_ &&
         (std::abs(x_max_ - x_min_) > aligned_thresh_meter_ ||
          std::abs(y_max_ - y_min_) > aligned_thresh_meter_);
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
