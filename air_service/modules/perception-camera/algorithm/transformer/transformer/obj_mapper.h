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

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <opencv/cv.hpp>

#include "air_service/modules/perception-camera/algorithm/interface/algorithm_base.h"
#include "base/common/log.h"
#include "twod_threed_util.h"

namespace airos {
namespace perception {
namespace algorithm {

struct ObjMapperOptions {
  float hwl[3]           = {0};
  float bbox[4]          = {0};
  float bbox3d_bottom[2] = {0};
  float ry               = 0.0f;
  float alpha            = 0.0f;
};

class ObjMapper {
 public:
  ObjMapper()
      : width_(0)
      , height_(0) {
    memset(k_mat_, 0, sizeof(float) * 9);
  }

  ~ObjMapper() {}

  void Init(const float *k_mat, int width, int height) {
    memcpy(k_mat_, k_mat, sizeof(float) * 9);
    width_  = width;
    height_ = height;
    camera_k_matrix_ << k_mat_[0], k_mat_[1], k_mat_[2], k_mat_[3], k_mat_[4],
        k_mat_[5], k_mat_[6], k_mat_[7], k_mat_[8];
  }

  bool set_camera_name(const std::string &camera_name);

  bool set_coffe_ground(const Eigen::Vector4d &coffe_ground);

  double GetDepthFromGroundPlane(const Eigen::Vector3f &point_in_camera) const;

  bool GetGroundCenterPoints(
      const ObjMapperOptions &options,
      Eigen::Matrix<float, 3, 1> &matrix_bottom);

  bool Compute_obj_center(
      const ObjMapperOptions &options, const Eigen::Vector3d &normal_vector,
      const Eigen::Matrix<float, 3, 1> &matrix_bottom, float hwl[3],
      float center[3]);

  void GetReliableCameraBottomPoint(
      const ObjMapperOptions &obj_mapper_options, float object_center[3],
      float dimension_hwl[3]);

 private:
  float k_mat_[9] = {0};
  int width_      = 0;
  int height_     = 0;
  Eigen::Matrix3f camera_k_matrix_;
  float coffe_ground_[4];

 public:
  std::string camera_name_;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
