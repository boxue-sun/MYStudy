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

#ifndef PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_OPTIONS_VIEWPORT_H_  // NOLINT
#define PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_OPTIONS_VIEWPORT_H_  // NOLINT

#include <memory>
#include <string>

#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/base_gl_viewport.h"

namespace airos {
namespace perception {
namespace visualization {

class ToggleButton {
 public:
  ToggleButton(int x, int y, int width, int height)
      : x_(x)
      , y_(y)
      , width_(width)
      , height_(height) {}
  ~ToggleButton() = default;

  void set_text(const std::string& text) {
    text_ = text;
  }

  void set_hint_text(const std::string& text) {
    hint_text_ = text;
  }

  void set_option_name(const std::string& option_name) {
    option_name_ = option_name;
  }

  void set_position(int x, int y) {
    x_ = x;
    y_ = y;
  }

  void set_toggled_color(int r, int g, int b) {
    toggled_color_[0] = r;
    toggled_color_[1] = g;
    toggled_color_[2] = b;
  }

  void set_untoggled_color(int r, int g, int b) {
    untoggled_color_[0] = r;
    untoggled_color_[1] = g;
    untoggled_color_[2] = b;
  }

  void draw() const;

  bool clicked(int x, int y) const {
    return x >= x_ && y >= y_ && x < x_ + width_ && y < y_ + height_;
  }

 private:
  int x_;
  int y_;
  int width_;
  int height_;
  std::string text_;
  std::string hint_text_;
  std::string option_name_;

  int toggled_color_[3]   = {255, 255, 255};
  int untoggled_color_[3] = {100, 100, 100};
};  // class ToggleButton

class TextLabel {
 public:
  TextLabel(int x, int y, int width, int height)
      : x_(x)
      , y_(y)
      , width_(width)
      , height_(height) {}
  ~TextLabel() = default;

  void set_text(const std::string& text) {
    text_ = text;
  }

  void set_position(int x, int y) {
    x_ = x;
    y_ = y;
  }

  void draw() const;

 private:
  int x_;
  int y_;
  int width_;
  int height_;
  std::string text_;
};  // class TextLabel

class OptionsViewport : public BaseGLViewport {
 public:
  OptionsViewport()  = default;
  ~OptionsViewport() = default;

  void init_buttons();
  void on_mouse_button_click(int xpos, int ypos);

 protected:
  void draw() override;

 private:
  inline void update_button_position(int* x, int* y) {
    *y += button_vertical_space_;
    if (*y >= get_height() - border_size_) {
      *x += button_horizontal_space_;
      *y = border_size_;
    }
  }

 private:
  bool inited_                 = false;
  int border_size_             = 50;
  int button_size_             = 35;
  int button_vertical_space_   = 80;
  int button_horizontal_space_ = 400;

  std::shared_ptr<ToggleButton> btn_show_origin_cloud_ = nullptr;
  std::shared_ptr<ToggleButton> btn_show_polygon_      = nullptr;

  std::shared_ptr<ToggleButton> btn_show_track_id_    = nullptr;
  std::shared_ptr<ToggleButton> btn_show_track_color_ = nullptr;
  std::shared_ptr<ToggleButton> btn_show_2dbbox_      = nullptr;

  std::shared_ptr<TextLabel> label_switch_view_type_ = nullptr;
  std::shared_ptr<TextLabel> label_switch_camera_    = nullptr;
  std::shared_ptr<TextLabel> label_capture_screen_   = nullptr;

  // these buttons are not clickable
  std::shared_ptr<ToggleButton> btn_car_color_               = nullptr;
  std::shared_ptr<ToggleButton> btn_cyclist_color_           = nullptr;
  std::shared_ptr<ToggleButton> btn_pedestrian_color_        = nullptr;
  std::shared_ptr<ToggleButton> btn_unknown_color_           = nullptr;
  std::shared_ptr<ToggleButton> btn_unknown_unmovable_color_ = nullptr;
  std::shared_ptr<ToggleButton> btn_unknown_movable_color_   = nullptr;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos

#endif  // PERCEPTION_VISUALIZATION_GL_FUSION_VISUALIZER_VIEWPORTS_OPTIONS_VIEWPORT_H_
        // // NOLINT
