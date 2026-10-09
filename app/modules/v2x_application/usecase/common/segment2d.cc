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

#include "app/modules/v2x_application/usecase/common/segment2d.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

#include "app/modules/v2x_application/usecase/common/math_utils.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace usecase {

namespace {

bool IsWithin(double val, double bound1, double bound2) {
  if (bound1 > bound2) {
    std::swap(bound1, bound2);
  }
  return val >= bound1 - kMathEpsilon && val <= bound2 + kMathEpsilon;
}

}  // namespace

Segment2d::Segment2d() {
  _unit_direction = Vec2d(1, 0);
}

Segment2d::Segment2d(const Vec2d& start, const Vec2d& end)
    : _start(start)
    , _end(end) {
  const double dx = _end.X() - _start.X();
  const double dy = _end.Y() - _start.Y();
  _length         = hypot(dx, dy);
  _unit_direction =
      (_length <= kMathEpsilon ? Vec2d(0, 0)
                               : Vec2d(dx / _length, dy / _length));
  _heading = _unit_direction.Angle();

  // 计算与正北方向的夹角
  _heading_north = 90.0 - (_heading * 180.0 / M_PI);
  if (_heading_north < 0.0) {
    _heading_north += 360.0;
  }
}

double Segment2d::Length() const {
  return _length;
}

bool Segment2d::IsPointIn(const Vec2d& point) const {
  if (_length <= kMathEpsilon) {
    return std::abs(point.X() - _start.X()) <= kMathEpsilon &&
           std::abs(point.Y() - _start.Y()) <= kMathEpsilon;
  }
  const double prod = CrossProd(point, _start, _end);
  if (std::abs(prod) > kMathEpsilon) {
    return false;
  }
  return IsWithin(point.X(), _start.X(), _end.X()) &&
         IsWithin(point.Y(), _start.Y(), _end.Y());
}

double Segment2d::DistanceTo(const Vec2d& point) const {
  if (_length <= kMathEpsilon) {
    return point.DistanceTo(_start);
  }
  const double x0   = point.X() - _start.X();
  const double y0   = point.Y() - _start.Y();
  const double proj = x0 * _unit_direction.X() + y0 * _unit_direction.Y();
  if (proj <= 0.0) {
    return hypot(x0, y0);
  }
  if (proj >= _length) {
    return point.DistanceTo(_end);
  }
  return std::abs(x0 * _unit_direction.Y() - y0 * _unit_direction.X());
}

/**
 * @brief 计算点到直线的垂直距离
 * @param point 
 * @return 
 */
double Segment2d::PerpendicularDistanceTo(const Vec2d& point) const {
  if (_length <= kMathEpsilon) {
    return point.DistanceTo(_start);
  }
  const double x0 = point.X() - _start.X();
  const double y0 = point.Y() - _start.Y();
  return std::abs(x0 * _unit_direction.Y() - y0 * _unit_direction.X());
}

double Segment2d::DistanceTo(
    const Vec2d& point, Vec2d* const nearest_pt) const {
  CHECK_NOTNULL(nearest_pt);
  if (_length <= kMathEpsilon) {
    *nearest_pt = _start;
    return point.DistanceTo(_start);
  }
  const double x0   = point.X() - _start.X();
  const double y0   = point.Y() - _start.Y();
  const double proj = x0 * _unit_direction.X() + y0 * _unit_direction.Y();
  if (proj < 0.0) {
    *nearest_pt = _start;
    return hypot(x0, y0);
  }
  if (proj > _length) {
    *nearest_pt = _end;
    return point.DistanceTo(_end);
  }
  *nearest_pt = _start + _unit_direction * proj;
  return std::abs(x0 * _unit_direction.Y() - y0 * _unit_direction.X());
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
