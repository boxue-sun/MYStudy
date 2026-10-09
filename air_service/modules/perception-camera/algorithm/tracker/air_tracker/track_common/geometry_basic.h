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
#include <iostream>
#include <limits>
#include <vector>

#include <Eigen/Core>

namespace airos {
namespace perception {
namespace common {

// @brief cross production on two vectors
// one is from pt1 to pt2, another is from pt1 to pt3
// the type of points could be double or float
// old name: cross_prod
template <typename Type>
inline Type CrossProduct(
    const Eigen::Matrix<Type, 2, 1> &point1,
    const Eigen::Matrix<Type, 2, 1> &point2,
    const Eigen::Matrix<Type, 2, 1> &point3) {
  return (point2.x() - point1.x()) * (point3.y() - point1.y()) -
         (point3.x() - point1.x()) * (point2.y() - point1.y());
}

// @brief cross production on two vectors
// one is from pt1 to pt2, another is from pt1 to pt3
// the type of points could be double or float
// old name: cross_prod
template <typename PointT>
inline typename PointT::PointType CrossProduct(
    const PointT &point1, const PointT &point2, const PointT &point3) {
  return (point2.x - point1.x) * (point3.y - point1.y) -
         (point3.x - point1.x) * (point2.y - point1.y);
}

// @brief calculate the Eucliden distance between two points
// old name: euclidean_dist
template <typename PointT>
inline typename PointT::PointType CalculateEuclidenDist(
    const PointT &pt1, const PointT &pt2) {
  typename PointT::PointType dist = (pt1.x - pt2.x) * (pt1.x - pt2.x);
  dist += (pt1.y - pt2.y) * (pt1.y - pt2.y);
  dist += (pt1.z - pt2.z) * (pt1.z - pt2.z);
  return sqrt(dist);
}

// @brief calculate the Eucliden distance between two points in X-Y plane
// old name: euclidean_dist_2d_xy
template <typename PointT>
inline typename PointT::PointType CalculateEuclidenDist2DXY(
    const PointT &pt1, const PointT &pt2) {
  typename PointT::PointType dist = (pt1.x - pt2.x) * (pt1.x - pt2.x);
  dist += (pt1.y - pt2.y) * (pt1.y - pt2.y);
  return sqrt(dist);
}

// @brief calculate cos value of the rotation angle
// between two vectors in X-Y plane
// old name: vector_cos_theta_2d_xy
template <typename Type>
Type CalculateCosTheta2DXY(
    const Eigen::Matrix<Type, 3, 1> &v1, const Eigen::Matrix<Type, 3, 1> &v2) {
  Type v1_len = sqrt((v1.head(2).cwiseProduct(v1.head(2))).sum());
  Type v2_len = sqrt((v2.head(2).cwiseProduct(v2.head(2))).sum());
  if (v1_len < std::numeric_limits<Type>::epsilon() ||
      v2_len < std::numeric_limits<Type>::epsilon()) {
    return 0.0;
  }
  Type cos_theta =
      (v1.head(2).cwiseProduct(v2.head(2))).sum() / (v1_len * v2_len);
  return cos_theta;
}

// @brief calculate the rotation angle between two vectors in X-Y plane
// old name: vector_theta_2d_xy
template <typename Type>
Type CalculateTheta2DXY(
    const Eigen::Matrix<Type, 3, 1> &v1, const Eigen::Matrix<Type, 3, 1> &v2) {
  Type v1_len = sqrt((v1.head(2).cwiseProduct(v1.head(2))).sum());
  Type v2_len = sqrt((v2.head(2).cwiseProduct(v2.head(2))).sum());
  if (v1_len < std::numeric_limits<Type>::epsilon() ||
      v2_len < std::numeric_limits<Type>::epsilon()) {
    return 0.0;
  }
  Type cos_theta =
      (v1.head(2).cwiseProduct(v2.head(2))).sum() / (v1_len * v2_len);
  Type sin_theta = (v1(0) * v2(1) - v1(1) * v2(0)) / (v1_len * v2_len);
  cos_theta      = cos_theta > 1.0 ? 1.0 : cos_theta;
  cos_theta      = cos_theta < -1.0 ? -1.0 : cos_theta;
  Type theta     = acos(cos_theta);
  if (sin_theta < 0.0) {
    theta = -theta;
  }
  return theta;
}

// @brief calculate the rotation matrix
// transform from v1 axis coordinate to v2 axis coordinate
// old name: vector_rot_mat_2d_xy
template <typename Type>
Eigen::Matrix<Type, 3, 3> CalculateRotationMat2DXY(
    const Eigen::Matrix<Type, 3, 1> &v1, const Eigen::Matrix<Type, 3, 1> &v2) {
  Type v1_len = sqrt((v1.head(2).cwiseProduct(v1.head(2))).sum());
  Type v2_len = sqrt((v2.head(2).cwiseProduct(v2.head(2))).sum());
  if (v1_len < std::numeric_limits<Type>::epsilon() ||
      v2_len < std::numeric_limits<Type>::epsilon()) {
    return Eigen::Matrix<Type, 3, 3>::Zero(3, 3);
  }

  Type cos_theta =
      (v1.head(2).cwiseProduct(v2.head(2))).sum() / (v1_len * v2_len);
  Type sin_theta = (v1(0) * v2(1) - v1(1) * v2(0)) / (v1_len * v2_len);

  Eigen::Matrix<Type, 3, 3> rot_mat;
  rot_mat << cos_theta, sin_theta, 0, -sin_theta, cos_theta, 0, 0, 0, 1;
  return rot_mat;
}

// @brief calculate the project vector from one vector to another
// old name: compute_2d_xy_project_vector
template <typename Type>
Eigen::Matrix<Type, 3, 1> Calculate2DXYProjectVector(
    const Eigen::Matrix<Type, 3, 1> &projected_vector,
    const Eigen::Matrix<Type, 3, 1> &project_vector) {
  if (projected_vector.head(2).norm() < std::numeric_limits<Type>::epsilon() ||
      project_vector.head(2).norm() < std::numeric_limits<Type>::epsilon()) {
    return Eigen::Matrix<Type, 3, 1>::Zero(3, 1);
  }
  Eigen::Matrix<Type, 3, 1> project_dir = project_vector;
  project_dir(2)                        = 0.0;
  project_dir.normalize();
  Type projected_vector_project_dir_inner_product =
      projected_vector(0) * project_dir(0) +
      projected_vector(1) * project_dir(1);
  Type projected_vector_project_dir_angle_cos =
      projected_vector_project_dir_inner_product /
      (projected_vector.head(2).norm() * project_dir.head(2).norm());
  Type projected_vector_norm_on_project_dir =
      projected_vector.head(2).norm() * projected_vector_project_dir_angle_cos;
  Eigen::Matrix<Type, 3, 1> result_vector =
      project_dir * projected_vector_norm_on_project_dir;
  return result_vector;
}

// @brief convert point xyz in Cartesian coordinate to polar coordinate
// old name: xyz_to_polar_coordinate
template <typename PointT>
void ConvertCartesiantoPolarCoordinate(
    const PointT &xyz, typename PointT::PointType *h_angle_in_degree,
    typename PointT::PointType *v_angle_in_degree,
    typename PointT::PointType *dist) {
  typedef typename PointT::PointType Type;
  const Type radius_to_degree = 180.0 / M_PI;
  Type x                      = xyz.x;
  Type y                      = xyz.y;
  Type z                      = xyz.z;

  (*dist)      = sqrt(x * x + y * y + z * z);
  Type dist_xy = sqrt(x * x + y * y);

  (*h_angle_in_degree) = acos(x / dist_xy) * radius_to_degree;
  if (y < 0) {
    (*h_angle_in_degree) = 360 - (*h_angle_in_degree);
  }

  (*v_angle_in_degree) = acos(dist_xy / (*dist)) * radius_to_degree;
  if (z < 0) {
    (*v_angle_in_degree) = -(*v_angle_in_degree);
  }
}

}  // namespace common
}  // namespace perception
}  // namespace airos
