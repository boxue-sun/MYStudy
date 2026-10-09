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
#include <float.h>

#include <algorithm>
#include <cstdlib>
#include <utility>
#include <vector>

#include <Eigen/Dense>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

template <typename T>
class Constant;
template <>
class Constant<double> {
 public:
  static const double PI() {
    return 3.1415926535897932384626433832795;
  } /*pi*/
};
template <>
class Constant<float> {
 public:
  static const float PI() {
    return 3.1415926535897932384626433832795f;
  } /*pi*/
};

template <typename T>
void NormalAngle(T &angle) {
  const T PI = Constant<T>::PI();
  if (angle < -PI) {
    angle += 2 * PI;
  } else if (angle >= PI) {
    angle -= 2 * PI;
  }
}

/*Compute 1/a, should not template this function, i_rec(int) should return
 * double*/
inline float i_rec(float a) {
  return ((a != 0.0f) ? ((1.0f) / a) : 1.0f);
}
inline double i_rec(int a) {
  return ((a != 0) ? ((1.0) / a) : 1.0);
}
inline double i_rec(unsigned int a) {
  return ((a != 0) ? ((1.0) / a) : 1.0);
}
inline double i_rec(double a) {
  return ((a != 0.0) ? ((1.0) / a) : 1.0);
}

template <typename T>
inline void i_backproject_canonical(
    const T x[2], const T K[9], T depth, T X[3]) {
  X[0] = (x[0] - K[2]) * depth * i_rec(K[0]);
  X[1] = (x[1] - K[5]) * depth * i_rec(K[4]);
  X[2] = depth;
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
