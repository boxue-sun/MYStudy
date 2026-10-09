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
#include <limits>
#include <vector>

#include <Eigen/Core>

#include "base/common/box.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace common {

// @brief check a point is in bounding-box or not
// old name: is_point_in_boundingbox
template <typename PointT>
bool IsPointInBBox(
    const Eigen::Matrix<typename PointT::PointType, 3, 1> &gnd_c,
    const Eigen::Matrix<typename PointT::PointType, 3, 1> &dir_x,
    const Eigen::Matrix<typename PointT::PointType, 3, 1> &dir_y,
    const Eigen::Matrix<typename PointT::PointType, 3, 1> &dir_z,
    const Eigen::Matrix<typename PointT::PointType, 3, 1> &size,
    const PointT &point) {
  typedef typename PointT::PointType Type;
  Eigen::Matrix<Type, 3, 1> eig(point.x, point.y, point.z);
  Eigen::Matrix<Type, 3, 1> diff = eig - gnd_c;
  Type x                         = diff.dot(dir_x);
  if (fabs(x) > size[0] * 0.5) {
    return false;
  }
  Type y = diff.dot(dir_y);
  if (fabs(y) > size[1] * 0.5) {
    return false;
  }
  Type z = diff.dot(dir_z);
  if (fabs(z) > size[2] * 0.5) {
    return false;
  }
  return true;
}

// @brief calculate the size and center of the bounding-box of a point cloud
// old name: compute_bbox_size_center_xy
template <typename PointCloudT>
void CalculateBBoxSizeCenter2DXY(
    const PointCloudT &cloud, const Eigen::Vector3f &dir, Eigen::Vector3f *size,
    Eigen::Vector3d *center, float minimum_edge_length = FLT_EPSILON) {
  // NOTE: direction should not be (0, 0, 1)
  Eigen::Matrix3d projection;
  Eigen::Vector3d dird(dir[0], dir[1], 0.0);
  dird.normalize();
  projection << dird[0], dird[1], 0.0, -dird[1], dird[0], 0.0, 0.0, 0.0, 1.0;
  Eigen::Vector3d min_pt(DBL_MAX, DBL_MAX, DBL_MAX);
  Eigen::Vector3d max_pt(-DBL_MAX, -DBL_MAX, -DBL_MAX);
  Eigen::Vector3d loc_pt(0.0, 0.0, 0.0);
  for (int i = 0; i < cloud.size(); i++) {
    loc_pt = projection * Eigen::Vector3d(cloud[i].x, cloud[i].y, cloud[i].z);

    min_pt(0) = std::min(min_pt(0), loc_pt(0));
    min_pt(1) = std::min(min_pt(1), loc_pt(1));
    min_pt(2) = std::min(min_pt(2), loc_pt(2));

    max_pt(0) = std::max(max_pt(0), loc_pt(0));
    max_pt(1) = std::max(max_pt(1), loc_pt(1));
    max_pt(2) = std::max(max_pt(2), loc_pt(2));
  }
  (*size)               = (max_pt - min_pt).cast<float>();
  Eigen::Vector3d coeff = (max_pt + min_pt) * 0.5;
  coeff(2)              = min_pt(2);
  *center               = projection.transpose() * coeff;

  float minimum_size =
      minimum_edge_length > FLT_EPSILON ? minimum_edge_length : FLT_EPSILON;
  (*size)(0) = (*size)(0) <= minimum_size ? minimum_size : (*size)(0);
  (*size)(1) = (*size)(1) <= minimum_size ? minimum_size : (*size)(1);
  (*size)(2) = (*size)(2) <= minimum_size ? minimum_size : (*size)(2);
}

// old name: compute_most_consistent_bbox_direction
template <typename Type>
void CalculateMostConsistentBBoxDir2DXY(
    const Eigen::Matrix<Type, 3, 1> &prev_dir,
    Eigen::Matrix<Type, 3, 1> *curr_dir) {
  Type dot_val_00 = prev_dir(0) * (*curr_dir)(0) + prev_dir(1) * (*curr_dir)(1);
  Type dot_val_01 = prev_dir(0) * (*curr_dir)(1) - prev_dir(1) * (*curr_dir)(0);
  if (fabs(dot_val_00) >= fabs(dot_val_01)) {
    if (dot_val_00 < 0) {
      (*curr_dir) = -(*curr_dir);
    }
  } else {
    if (dot_val_01 < 0) {
      (*curr_dir) =
          Eigen::Matrix<Type, 3, 1>((*curr_dir)(1), -(*curr_dir)(0), 0);
    } else {
      (*curr_dir) =
          Eigen::Matrix<Type, 3, 1>(-(*curr_dir)(1), (*curr_dir)(0), 0);
    }
  }
}

// @brief calculate the IOU (intersection-over-union) between two bbox
// old name:compute_2d_iou_bbox_to_bbox
template <typename Type>
Type CalculateIou2DXY(
    const Eigen::Matrix<Type, 3, 1> &center0,
    const Eigen::Matrix<Type, 3, 1> &size0,
    const Eigen::Matrix<Type, 3, 1> &center1,
    const Eigen::Matrix<Type, 3, 1> &size1) {
  Type min_x_bbox_0 = center0(0) - size0(0) * 0.5;
  Type min_x_bbox_1 = center1(0) - size1(0) * 0.5;
  Type max_x_bbox_0 = center0(0) + size0(0) * 0.5;
  Type max_x_bbox_1 = center1(0) + size1(0) * 0.5;
  Type start_x      = std::max(min_x_bbox_0, min_x_bbox_1);
  Type end_x        = std::min(max_x_bbox_0, max_x_bbox_1);
  Type length_x     = end_x - start_x;
  if (length_x <= 0) {
    return 0;
  }
  Type min_y_bbox_0 = center0(1) - size0(1) * 0.5;
  Type min_y_bbox_1 = center1(1) - size1(1) * 0.5;
  Type max_y_bbox_0 = center0(1) + size0(1) * 0.5;
  Type max_y_bbox_1 = center1(1) + size1(1) * 0.5;
  Type start_y      = std::max(min_y_bbox_0, min_y_bbox_1);
  Type end_y        = std::min(max_y_bbox_0, max_y_bbox_1);
  Type length_y     = end_y - start_y;
  if (length_y <= 0) {
    return 0;
  }
  Type intersection_area = length_x * length_y;
  Type bbox_0_area       = size0(0) * size0(1);
  Type bbox_1_area       = size1(0) * size1(1);
  Type iou =
      intersection_area / (bbox_0_area + bbox_1_area - intersection_area);
  return iou;
}
template <typename Type>
Type CalculateIOUBBox(
    const airos::base::BBox2D<Type> &box1,
    const airos::base::BBox2D<Type> &box2) {
  airos::base::Rect<Type> rect1(box1);
  airos::base::Rect<Type> rect2(box2);
  airos::base::Rect<Type> intersection = rect1 & rect2;
  airos::base::Rect<Type> unionsection = rect1 | rect2;
  return intersection.Area() / unionsection.Area();
}

// @brief given a point and segments,
// calculate the distance and direction to the nearest segment
// old name: calculate_distance_and_direction_to_segments_xy
template <typename PointCloudT>
bool CalculateDistAndDirToSegs(
    const Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> &pt,
    const PointCloudT &segs, typename PointCloudT::PointType::PointType *dist,
    Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> *dir) {
  if (segs.size() < 2) {
    return false;
  }

  typedef typename PointCloudT::PointType::PointType Type;
  Eigen::Matrix<Type, 3, 1> seg_point(segs[0].x, segs[0].y, 0);
  Type min_dist = (pt - seg_point).head(2).norm();

  Eigen::Matrix<Type, 3, 1> end_point_pre;
  Eigen::Matrix<Type, 3, 1> end_point_cur;
  Eigen::Matrix<Type, 3, 1> line_segment_dir;
  Eigen::Matrix<Type, 3, 1> line_segment_dir_pre;
  Eigen::Matrix<Type, 3, 1> end_point_to_pt_vec;

  line_segment_dir_pre << 0, 0, 0;

  Type line_segment_len        = 0;
  Type projected_len           = 0;
  Type point_to_line_dist      = 0;
  Type point_to_end_point_dist = 0;

  for (size_t i = 1; i < segs.size(); ++i) {
    end_point_pre << segs[i - 1].x, segs[i - 1].y, 0;
    end_point_cur << segs[i].x, segs[i].y, 0;
    line_segment_dir       = end_point_pre - end_point_cur;
    end_point_to_pt_vec    = pt - end_point_cur;
    end_point_to_pt_vec(2) = 0;
    line_segment_len       = line_segment_dir.head(2).norm();
    line_segment_dir       = line_segment_dir / line_segment_len;
    if (i == 1) {
      *dir = line_segment_dir;
    }
    projected_len = end_point_to_pt_vec.dot(line_segment_dir);
    // case 1. pt is in the range of current line segment, compute
    // the point to line distance
    if (projected_len >= 0 && projected_len <= line_segment_len) {
      point_to_line_dist = end_point_to_pt_vec.cross(line_segment_dir).norm();
      if (min_dist > point_to_line_dist) {
        min_dist = point_to_line_dist;
        *dir     = line_segment_dir;
      }
    } else {
      // case 2. pt is out of range of current line segment, compute
      // the point to end point distance
      point_to_end_point_dist = end_point_to_pt_vec.head(2).norm();
      if (min_dist > point_to_end_point_dist) {
        min_dist = point_to_end_point_dist;
        *dir     = line_segment_dir + line_segment_dir_pre;
        dir->normalize();
      }
    }
    line_segment_dir_pre = line_segment_dir;
  }
  *dist = min_dist;

  return true;
}

// @brief given a point and two boundaries,
// calculate the distance and direction to the nearer boundary
// old name: calculate_distance_and_direction_to_boundary_xy
template <typename PointCloudT>
void CalculateDistAndDirToBoundary(
    const Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> &pt,
    const PointCloudT &left_boundary, const PointCloudT &right_boundary,
    typename PointCloudT::PointType::PointType *dist,
    Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> *dir) {
  typedef typename PointCloudT::PointType::PointType Type;
  Type dist_to_left = std::numeric_limits<Type>::max();
  Eigen::Matrix<Type, 3, 1> direction_left;
  Type dist_to_right = std::numeric_limits<Type>::max();
  Eigen::Matrix<Type, 3, 1> direction_right;

  CalculateDistAndDirToSegs(pt, left_boundary, &dist_to_left, &direction_left);

  CalculateDistAndDirToSegs(
      pt, right_boundary, &dist_to_right, &direction_right);

  if (dist_to_left < dist_to_right) {
    (*dist) = dist_to_left;
    (*dir)  = direction_left;
  } else {
    (*dist) = dist_to_right;
    (*dir)  = direction_right;
  }
}

// @brief given a point and two boundaries sets,
// calculate the distance and direction to the nearest boundary
// old name: calculate_distance_and_direction_to_boundary_xy
template <typename PointCloudT>
void CalculateDistAndDirToBoundary(
    const Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> &pt,
    const std::vector<PointCloudT> &left_boundary,
    const std::vector<PointCloudT> &right_boundary,
    typename PointCloudT::PointType::PointType *dist,
    Eigen::Matrix<typename PointCloudT::PointType::PointType, 3, 1> *dir) {
  typedef typename PointCloudT::PointType::PointType Type;
  Type dist_to_left = std::numeric_limits<Type>::max();
  Eigen::Matrix<Type, 3, 1> direction_left;
  Type dist_to_right = std::numeric_limits<Type>::max();
  Eigen::Matrix<Type, 3, 1> direction_right;

  for (size_t i = 0; i < left_boundary.size(); i++) {
    Type dist_temp = std::numeric_limits<Type>::max();
    Eigen::Matrix<Type, 3, 1> dir_temp;
    if (CalculateDistAndDirToSegs(
            pt, left_boundary[i], &dist_temp, &dir_temp)) {
      if (dist_to_left > dist_temp) {
        dist_to_left   = dist_temp;
        direction_left = dir_temp;
      }
    }
  }

  for (size_t i = 0; i < right_boundary.size(); i++) {
    Type dist_temp = std::numeric_limits<Type>::max();
    Eigen::Matrix<Type, 3, 1> dir_temp;
    if (CalculateDistAndDirToSegs(
            pt, right_boundary[i], &dist_temp, &dir_temp)) {
      if (dist_to_right > dist_temp) {
        dist_to_right   = dist_temp;
        direction_right = dir_temp;
      }
    }
  }
  if (dist_to_left < dist_to_right) {
    (*dist) = dist_to_left;
    (*dir)  = direction_left;
  } else {
    (*dist) = dist_to_right;
    (*dir)  = direction_right;
  }
}

// @brief get 2d rectangle points in top-view given an obstacle's
// `center`, `direction` and `size`(l, w, h)
template <typename T>
void GetTopViewRectanglePoints2D(
    const Eigen::Matrix<T, 3, 1> &center,
    const Eigen::Matrix<T, 3, 1> &direction, const Eigen::Matrix<T, 3, 1> &size,
    std::vector<Eigen::Matrix<T, 2, 1> > *rect_pts_2d) {
  CHECK(rect_pts_2d != nullptr);
  rect_pts_2d->resize(4);

  const T length = size[0];
  const T width  = size[1];
  const T height = size[2];
  T x1           = length / 2;
  T x2           = -x1;
  T y1           = width / 2;
  T y2           = -y1;
  T len = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
  T cos_theta = direction[0] / len;
  T sin_theta = direction[1] / len;

  Eigen::Matrix<T, 2, 2> rot_mat;
  rot_mat << cos_theta, -sin_theta, sin_theta, cos_theta;
  const auto center2d = center.head(2);
  (*rect_pts_2d)[0]   = rot_mat * Eigen::Matrix<T, 2, 1>(x1, y1) + center2d;
  (*rect_pts_2d)[1]   = rot_mat * Eigen::Matrix<T, 2, 1>(x2, y1) + center2d;
  (*rect_pts_2d)[2]   = rot_mat * Eigen::Matrix<T, 2, 1>(x2, y2) + center2d;
  (*rect_pts_2d)[3]   = rot_mat * Eigen::Matrix<T, 2, 1>(x1, y2) + center2d;
}

// @brief return true if two 2D line segment `(p1, q1)` and
// `(p2, q2)` intersect
template <typename T>
bool IsLineSegments2DIntersect(
    const Eigen::Matrix<T, 2, 1> &p1, const Eigen::Matrix<T, 2, 1> &q1,
    const Eigen::Matrix<T, 2, 1> &p2, const Eigen::Matrix<T, 2, 1> &q2) {
  // find orientation of ordered triplet (p, q, r).
  // returns values:
  // 0 --> p, q and r are collinear
  // 1 --> Clockwise
  // 2 --> Counterclockwise
  auto find_orientation = [](const Eigen::Matrix<T, 2, 1> &p,
                             const Eigen::Matrix<T, 2, 1> &q,
                             const Eigen::Matrix<T, 2, 1> &r) -> int {
    int ret = (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1]);
    // collinear
    if (ret == 0) {
      return 0;
    }
    // clock or counterclock wise
    return (ret > 0) ? 1 : 2;
  };

  // given three 'collinear' points p, q, r
  // check whether point p is on line segment (q, r)
  auto is_on_segment = [](const Eigen::Matrix<T, 2, 1> &p,
                          const Eigen::Matrix<T, 2, 1> &q,
                          const Eigen::Matrix<T, 2, 1> &r) -> bool {
    return p[0] <= std::max(q[0], r[0]) && p[0] >= std::min(q[0], r[0]) &&
           p[1] <= std::max(q[1], r[1]) && p[1] >= std::min(q[1], r[1]);
  };

  // Two line segments (p1, q1) and (p2, q2) intersect <=>
  // (p1, q1, p2) and (p1, q1, q2) have different orientations and
  // (p2, q2, p1) and (p2, q2, q1) have different orientations.
  int orientation_p1_q1_p2 = find_orientation(p1, q1, p2);
  int orientation_p1_q1_q2 = find_orientation(p1, q1, q2);
  int orientation_p2_q2_p1 = find_orientation(p2, q2, p1);
  int orientation_p2_q2_q1 = find_orientation(p2, q2, q1);
  if (orientation_p1_q1_p2 != orientation_p1_q1_q2 &&
      orientation_p2_q2_p1 != orientation_p2_q2_q1) {
    return true;
  }

  // Special cases, two line segments are collinear
  if (orientation_p1_q1_p2 == 0 && is_on_segment(p2, p1, q1)) {
    return true;
  }
  if (orientation_p1_q1_q2 == 0 && is_on_segment(q2, p1, q1)) {
    return true;
  }
  if (orientation_p2_q2_p1 == 0 && is_on_segment(p1, p2, q2)) {
    return true;
  }
  if (orientation_p2_q2_q1 == 0 && is_on_segment(q1, p2, q2)) {
    return true;
  }
  return false;
}

// @brief return true if 2d line segment `(p, q)` and rectangle
// `rect_pts_2d` intersect
template <typename T>
bool IsLineSegmentAndRectangle2DIntersect(
    const Eigen::Matrix<T, 2, 1> &p, const Eigen::Matrix<T, 2, 1> &q,
    const std::vector<Eigen::Matrix<T, 2, 1> > &rect_pts_2d) {
  CHECK_EQ(rect_pts_2d.size(), 4);
  // check whether (p, q) intersects with any of four edges of rect_pts_2d
  const int rect_pts_2d_size = rect_pts_2d.size();
  for (int i = 0; i < rect_pts_2d_size; ++i) {
    if (IsLineSegments2DIntersect(
            p, q, rect_pts_2d[i], rect_pts_2d[(i + 1) % rect_pts_2d_size])) {
      return true;
    }
  }
  return false;
}

// @brief return true if point `p` is in rectangle `rect_pts_2d`
template <typename T>
bool IsPointInRectangle2D(
    const Eigen::Matrix<T, 2, 1> &p,
    const std::vector<Eigen::Matrix<T, 2, 1> > &rect_pts_2d) {
  CHECK_EQ(rect_pts_2d.size(), 4);
  // assume the rect is ABCD, the point is P
  // we can check the projections of the query point P on AB and BC:
  //   0 <= dot(AB, AP) <= dot(AB, AB) &&
  //   0 <= dot(BC, BP) <= dot(BC, BC)
  const auto &pt_a = rect_pts_2d[0];
  const auto &pt_b = rect_pts_2d[1];
  const auto &pt_c = rect_pts_2d[2];
  Eigen::Matrix<T, 2, 1> vec_ab(pt_b[0] - pt_a[0], pt_b[1] - pt_a[1]);
  Eigen::Matrix<T, 2, 1> vec_ap(p[0] - pt_a[0], p[1] - pt_a[1]);
  Eigen::Matrix<T, 2, 1> vec_bc(pt_c[0] - pt_b[0], pt_c[1] - pt_b[1]);
  Eigen::Matrix<T, 2, 1> vec_bp(p[0] - pt_b[0], p[1] - pt_b[1]);
  auto dot_ab_ap = vec_ab.dot(vec_ap);
  auto dot_ab_ab = vec_ab.dot(vec_ab);
  auto dot_bc_bp = vec_bc.dot(vec_bp);
  auto dot_bc_bc = vec_bc.dot(vec_bc);
  return dot_ab_ap >= 0 && dot_ab_ap <= dot_ab_ab && dot_bc_bp >= 0 &&
         dot_bc_bp <= dot_bc_bc;
}

// @brief return true if two rectangles collide
template <typename T>
bool IsRectangles2DCollide(
    const std::vector<Eigen::Matrix<T, 2, 1> > &rect_pts1,
    const std::vector<Eigen::Matrix<T, 2, 1> > &rect_pts2) {
  CHECK(rect_pts1.size() == 4 && rect_pts1.size() == rect_pts2.size());
  // check point in rect
  for (const auto &pt : rect_pts1) {
    if (IsPointInRectangle2D(pt, rect_pts2)) {
      return true;
    }
  }
  for (const auto &pt : rect_pts2) {
    if (IsPointInRectangle2D(pt, rect_pts1)) {
      return true;
    }
  }
  // check edges intersection
  const int rect_pts1_size = rect_pts1.size();
  const int rect_pts2_size = rect_pts2.size();
  for (int i = 0; i < rect_pts1_size; ++i) {
    auto p = rect_pts1[i];
    auto q = rect_pts1[(i + 1) % rect_pts1_size];
    if (IsLineSegmentAndRectangle2DIntersect(p, q, rect_pts2)) {
      return true;
    }
  }
  for (int i = 0; i < rect_pts2_size; ++i) {
    auto p = rect_pts2[i];
    auto q = rect_pts2[(i + 1) % rect_pts2_size];
    if (IsLineSegmentAndRectangle2DIntersect(p, q, rect_pts1)) {
      return true;
    }
  }
  return false;
}

// @brief calculate distance between point `(x, y)` and line segment
//   `(x1, y1), (x2, y2)`
template <typename T>
T CalculateDistPointToLineSegment(T x, T y, T x1, T y1, T x2, T y2) {
  T dot_prod = (x2 - x1) * (x - x1) + (y2 - y1) * (y - y1);
  if (dot_prod <= static_cast<T>(0)) {
    return std::sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
  }
  T d2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
  if (dot_prod >= d2) {
    return std::sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
  }
  T r  = dot_prod / d2;
  T px = x1 + (x2 - x1) * r;
  T py = y1 + (y2 - y1) * r;
  return std::sqrt((x - px) * (x - px) + (py - y) * (py - y));
}

}  // namespace common
}  // namespace perception
}  // namespace airos
