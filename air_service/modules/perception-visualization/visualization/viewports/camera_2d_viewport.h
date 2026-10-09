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

#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/viewports/base_gl_viewport.h"

namespace airos {
namespace perception {
namespace visualization {

class Camera2DViewport : public BaseGLViewport {
 public:
  Camera2DViewport()  = default;
  ~Camera2DViewport() = default;

 protected:
  void draw() override;

  void draw_title();

 public:
  static int s_camera_idx_;
  // static bool display_calib_points_ground_;
  static bool display_2dbbox_;
  static bool display_3dbbox_8pts_;
  static bool display_3dbbox_bottom_uv_;
  static double xpos_;
  static double ypos_;
  static bool choose_obs_;
  static int choose_track_id_;
  static int full_window_viewport_index_;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
