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

#include "app/modules/v2x_application/usecase/common/polygon2d.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

#include "app/modules/v2x_application/usecase/common/math_utils.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace usecase {

Polygon2d::Polygon2d(std::vector<Vec2d> points)
    : _points(std::move(points)) {
  BuildFromPoints();
}

bool Polygon2d::IsPointOnBoundary(const Vec2d& point) const {
  CHECK_GE(_points.size(), 3);
  return std::any_of(
      _segments.begin(), _segments.end(), [&](const Segment2d& poly_seg) {
        return poly_seg.IsPointIn(point);
      });
}

bool Polygon2d::IsPointIn(const Vec2d& point) const {
  CHECK_GE(_points.size(), 3);
  if (IsPointOnBoundary(point)) {
    return true;
  }
  int j = _num_points - 1;
  int c = 0;
  for (int i = 0; i < _num_points; ++i) {
    if ((_points[i].Y() > point.Y()) != (_points[j].Y() > point.Y())) {
      const double side = CrossProd(point, _points[i], _points[j]);
      if (_points[i].Y() < _points[j].Y() ? side > 0.0 : side < 0.0) {
        ++c;
      }
    }
    j = i;
  }
  return c & 1;
}

void Polygon2d::BuildFromPoints() {
  _num_points = _points.size();
  CHECK_GE(_num_points, 3);

  // Make sure the points are in ccw order.
  _area = 0.0;
  for (int i = 1; i < _num_points; ++i) {
    _area += CrossProd(_points[0], _points[i - 1], _points[i]);
  }
  if (_area < 0) {
    _area = -_area;
    std::reverse(_points.begin(), _points.end());
  }
  _area /= 2.0;
  CHECK_GT(_area, kMathEpsilon);

  // Construct segments.
  _segments.reserve(_num_points);
  for (int i = 0; i < _num_points; ++i) {
    _segments.emplace_back(_points[i], _points[Next(i)]);
  }

  // Check convexity.
  _is_convex = true;
  for (int i = 0; i < _num_points; ++i) {
    if (CrossProd(_points[Prev(i)], _points[i], _points[Next(i)]) <=
        -kMathEpsilon) {
      _is_convex = false;
      break;
    }
  }

  // Compute aabox.
  _min_x = _points[0].X();
  _max_x = _points[0].X();
  _min_y = _points[0].Y();
  _max_y = _points[0].Y();
  for (const auto& point : _points) {
    _min_x = std::min(_min_x, point.X());
    _max_x = std::max(_max_x, point.X());
    _min_y = std::min(_min_y, point.Y());
    _max_y = std::max(_max_y, point.Y());
  }
}

int Polygon2d::Next(int at) const {
  return at >= _num_points - 1 ? 0 : at + 1;
}

int Polygon2d::Prev(int at) const {
  return at == 0 ? _num_points - 1 : at - 1;
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
