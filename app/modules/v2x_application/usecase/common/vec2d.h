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

#include <cmath>
#include <string>

namespace airos {
namespace perception {
namespace usecase {

class Vec2d {
 public:
  constexpr Vec2d() noexcept
      : Vec2d(0, 0) {}
  constexpr Vec2d(const double x, const double y) noexcept
      : _x(x)
      , _y(y) {}

  double X() const {
    return _x;
  }
  double Y() const {
    return _y;
  }
  double Angle() const {
    return atan2(_y, _x);
  }
  void SetX(const double x) {
    _x = x;
  }
  void SetY(const double y) {
    _y = y;
  }

  double DistanceTo(const Vec2d &other) const {
    return std::hypot(_x - other._x, _y - other._y);
  }

  Vec2d operator+(const Vec2d &other) const {
    return Vec2d(_x + other.X(), _y + other.Y());
  }
  Vec2d operator*(const double ratio) const {
    return Vec2d(_x * ratio, _y * ratio);
  }

 protected:
  double _x = 0.0;
  double _y = 0.0;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
