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

#include "obj_mapper.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

bool ObjMapper::set_camera_name(const std::string &camera_name) {
  camera_name_ = camera_name;
  return true;
}

bool ObjMapper::set_coffe_ground(const Eigen::Vector4d &coffe_ground) {
  for (int i = 0; i < 4; i++) {
    coffe_ground_[i] = coffe_ground(i, 0);
  }
  return true;
}

double ObjMapper::GetDepthFromGroundPlane(const Eigen::Vector3f &point) const {
  double tmp =
      (coffe_ground_[0] * point[0] / point[2] +
       coffe_ground_[1] * point[1] / point[2] + coffe_ground_[2]);
  double Z = -1.0 * coffe_ground_[3] / tmp;
  return Z;
}

bool ObjMapper::GetGroundCenterPoints(
    const ObjMapperOptions &options,
    Eigen::Matrix<float, 3, 1> &matrix_bottom) {
  float bbox3d_bottom[2] = {options.bbox3d_bottom[0], options.bbox3d_bottom[1]};
  Eigen::Vector3f img_p;
  Eigen::Vector3f point_in_camera;
  float depth = 0.0;

  float v_p_cam[3];
  Eigen::Vector3d v_p_cam_vec;
  float *img_p_2d = new float[2];
  img_p << bbox3d_bottom[0], bbox3d_bottom[1], 1;
  point_in_camera = camera_k_matrix_.inverse() * img_p;
  depth           = GetDepthFromGroundPlane(point_in_camera);

  img_p_2d[0] = bbox3d_bottom[0];
  img_p_2d[1] = bbox3d_bottom[1];
  i_backproject_canonical(img_p_2d, k_mat_, depth, v_p_cam);
  if (depth > 0.0) {
    v_p_cam_vec << v_p_cam[0], v_p_cam[1], v_p_cam[2];
  } else {
    v_p_cam_vec << 0.0, 0.0, 0.0;
  }
  for (int j = 0; j < 3; j++) {
    matrix_bottom(j, 0) = v_p_cam_vec(j);
  }

  delete[] img_p_2d;
  return true;
}

bool ObjMapper::Compute_obj_center(
    const ObjMapperOptions &options, const Eigen::Vector3d &normal_vector,
    const Eigen::Matrix<float, 3, 1> &matrix_bottom, float hwl[3],
    float center[3]) {
  for (int i = 0; i < 3; i++) {
    hwl[i] = options.hwl[i];
  }
  for (int i = 0; i < 3; i++) {
    center[i] = matrix_bottom(i, 0) + normal_vector[i] * options.hwl[0] / 2.0;
  }
  return true;
}

void ObjMapper::GetReliableCameraBottomPoint(
    const ObjMapperOptions &obj_mapper_options, float object_center[3],
    float dimension_hwl[3]) {
  Eigen::Matrix<float, 3, 1> matrix_bottom;
  Eigen::Vector3d normal_vector(
      coffe_ground_[0], coffe_ground_[1], coffe_ground_[2]);
  if (normal_vector[1] > 0) {
    normal_vector = -1 * normal_vector;
  }

  GetGroundCenterPoints(obj_mapper_options, matrix_bottom);

  Compute_obj_center(
      obj_mapper_options, normal_vector, matrix_bottom, dimension_hwl,
      object_center);
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
