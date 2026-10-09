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

#include <string>
#include <vector>

#include "app/modules/v2x_application/usecase/common/segment2d.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"

namespace airos {
namespace perception {
namespace usecase {

class Polygon2d {
 public:
  Polygon2d() = default;
  explicit Polygon2d(std::vector<Vec2d> points);

  const std::vector<Vec2d>& Points() const {
    return _points;
  }
  const std::vector<Segment2d>& Segments() const {
    return _segments;
  }

  bool IsPointIn(const Vec2d& point) const;
  bool IsPointOnBoundary(const Vec2d& point) const;

 protected:
  void BuildFromPoints();

 protected:
  int Next(int at) const;
  int Prev(int at) const;

  std::vector<Vec2d> _points;
  int _num_points = 0;
  std::vector<Segment2d> _segments;
  bool _is_convex = false;
  double _area    = 0.0;
  double _min_x   = 0.0;
  double _max_x   = 0.0;
  double _min_y   = 0.0;
  double _max_y   = 0.0;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
