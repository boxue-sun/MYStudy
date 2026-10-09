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

#include "app/modules/v2x_application/usecase/common/vec2d.h"

namespace airos {
namespace perception {
namespace usecase {

using Vec2d = airos::perception::usecase::Vec2d;

class Segment2d {
 public:
  Segment2d();
  Segment2d(const Vec2d& start, const Vec2d& end);

  double Length() const;
  const Vec2d& Start() const {
    return _start;
  }
  const Vec2d& End() const {
    return _end;
  }
  const Vec2d& UnitDirection() const {
    return _unit_direction;
  }
  double Heading() const {
    return _heading;
  }

  /**
   * @brief 与正北方向的夹角
   * @return double 0 ~ 360
   */
  double HeadingNorth() const {
    return _heading_north;
  }

  double CosHeading() const {
    return _unit_direction.X();
  }
  double SinHeading() const {
    return _unit_direction.Y();
  }
  double DistanceTo(const Vec2d& point) const;
  double DistanceTo(const Vec2d& point, Vec2d* const nearest_pt) const;
  double PerpendicularDistanceTo(const Vec2d& point) const;
  bool IsPointIn(const Vec2d& point) const;

 protected:
  Vec2d _start;
  Vec2d _end;
  Vec2d _unit_direction;
  double _heading = 0.0;
  double _length  = 0.0;
  double _heading_north = 0.0;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
