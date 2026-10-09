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
#include <cmath>
#include <utility>

namespace airos {
namespace perception {
namespace usecase {

class Tensor {
 public:
  Tensor()                         = default;
  Tensor(const Tensor&)            = default;
  Tensor& operator=(const Tensor&) = default;
  Tensor(float x, float y, float z)
      : x(x)
      , y(y)
      , z(z) {}
  explicit Tensor(float theta)
      : x(cos(theta))
      , y(sin(theta)) {}

  float abs() const {
    return std::sqrt(x * x + y * y + z * z);
  }
  Tensor norm() const {
    float abs_ = abs();
    Tensor res;
    if (abs_ == 0) {
      return res;
    }
    res.x = x / abs_;
    res.y = y / abs_;
    res.z = z / abs_;
    return res;
  }
  Tensor reverse() const {
    Tensor res = norm();
    res.y      = -res.y;
    res.x      = -res.x;
    return res;
  }
  Tensor v_left() const {
    Tensor res = norm();
    std::swap(res.x, res.y);
    res.y = -res.y;
    return res;
  }
  Tensor v_right() const {
    Tensor res = norm();
    std::swap(res.x, res.y);
    res.x = -res.x;
    return res;
  }
  float theta() const {
    Tensor unit = norm();
    return std::acos(unit.x) * (180 / pi_);
  }

  Tensor operator+(const Tensor& b) const {
    return Tensor(x + b.x, y + b.y, z + b.z);
  }
  Tensor operator-(const Tensor& b) const {
    return Tensor(x - b.x, y - b.y, z - b.z);
  }
  Tensor operator/(const Tensor& b) const;  // cross
  float operator%(const Tensor& b) const;   // angle
  float operator*(const Tensor& b) const;   // dot

 public:
  float x = 0;
  float y = 0;
  float z = 0;

 private:
  static float pi_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
