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

#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"

namespace airos {
namespace perception {
namespace usecase {

constexpr double kMathEpsilon = 1e-10;

inline double CrossProd(
    const Vec2d& point1, const Vec2d& point2, const Vec2d& point3) {
  return (point2.X() - point1.X()) * (point3.Y() - point1.Y()) -
         (point3.X() - point1.X()) * (point2.Y() - point1.Y());
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
