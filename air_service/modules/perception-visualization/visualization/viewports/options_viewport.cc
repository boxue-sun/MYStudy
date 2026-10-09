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

#include "air_service/modules/perception-visualization/visualization/viewports/options_viewport.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "base/common/singleton.h"

// #include "base/object_types.h"

#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"

namespace airos {
namespace perception {
namespace visualization {

void ToggleButton::draw() const {
  Eigen::Vector3d color_button(
      toggled_color_[0], toggled_color_[1], toggled_color_[2]);
  auto display_opt_manager = DisplayOptionsManager::Instance();
  if (!display_opt_manager->get_option(option_name_)) {
    color_button[0] = untoggled_color_[0];
    color_button[1] = untoggled_color_[1];
    color_button[2] = untoggled_color_[2];
  }
  Eigen::Vector2d ul_pt(x_, y_);
  Eigen::Vector2d lr_pt(x_ + width_ - 1, y_ + height_ - 1);

  GLint polygon_mode;
  glGetIntegerv(GL_FRONT_AND_BACK, &polygon_mode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glColor3ub(color_button[0], color_button[1], color_button[2]);
  glBegin(GL_QUADS);
  glVertex2f(ul_pt[0], ul_pt[1]);
  glVertex2f(ul_pt[0], lr_pt[1]);
  glVertex2f(lr_pt[0], lr_pt[1]);
  glVertex2f(lr_pt[0], ul_pt[1]);
  glEnd();

  // GLRasterTextSingleton* raster_text =
  //   lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText *raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();
  raster_text->DrawString(
      text_.c_str(), ul_pt[0] + 2, lr_pt[1] + 20, 255, 0, 0);
  raster_text->DrawString(
      hint_text_.c_str(), lr_pt[0] + 5, ul_pt[1] + 20, 255, 255, 0);

  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
}

void TextLabel::draw() const {
  Eigen::Vector2d ul_pt(x_, y_);
  Eigen::Vector2d lr_pt(x_ + width_ - 1, y_ + height_ - 1);

  // GLRasterTextSingleton* raster_text =
  //   lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText *raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();
  raster_text->DrawString(text_.c_str(), ul_pt[0], ul_pt[1] + 20, 255, 255, 0);
}

void OptionsViewport::init_buttons() {
  if (inited_) {
    return;
  }
  auto display_opt_manager = DisplayOptionsManager::Instance();

  int x_btn = border_size_;
  int y_btn = border_size_;

  // switch cameras text label
  label_switch_camera_.reset(
      new TextLabel(x_btn, y_btn, button_size_, button_size_));
  label_switch_camera_->set_text("Press `c` to switch camera_2d views");
  update_button_position(&x_btn, &y_btn);

  // capture screen text label
  label_capture_screen_.reset(
      new TextLabel(x_btn, y_btn, button_size_, button_size_));
  label_capture_screen_->set_text("Press `a` to save screenshots");
  update_button_position(&x_btn, &y_btn);

  // capture screen text label
  label_switch_view_type_.reset(
      new TextLabel(x_btn, y_btn, button_size_, button_size_));
  label_switch_view_type_->set_text("Press `e` to switch 3d view type");
  update_button_position(&x_btn, &y_btn);

  auto color = OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::VEHICLE);
  btn_car_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_car_color_->set_hint_text(" Car");
  btn_car_color_->set_untoggled_color(color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  color = OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::BICYCLE);
  btn_cyclist_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_cyclist_color_->set_hint_text(" Cyclist");
  btn_cyclist_color_->set_untoggled_color(color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  color = OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::PEDESTRIAN);
  btn_pedestrian_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_pedestrian_color_->set_hint_text(" Pedestrian");
  btn_pedestrian_color_->set_untoggled_color(color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  color = OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::UNKNOWN);
  btn_unknown_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_unknown_color_->set_hint_text(" Unknown");
  btn_unknown_color_->set_untoggled_color(color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  color =
      OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE);
  btn_unknown_unmovable_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_unknown_unmovable_color_->set_hint_text(" Unknown_unmovable");
  btn_unknown_unmovable_color_->set_untoggled_color(
      color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  color =
      OBJECT_TYPE_COLOR_TABLE.at(algorithm::ObjectMajorType::UNKNOWN_MOVABLE);
  btn_unknown_movable_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_unknown_movable_color_->set_hint_text(" Unknown_movable");
  btn_unknown_movable_color_->set_untoggled_color(color[0], color[1], color[2]);
  update_button_position(&x_btn, &y_btn);

  // show polygon / box
  btn_show_polygon_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_show_polygon_->set_option_name("show_polygon");
  btn_show_polygon_->set_text("show polygon/box");
  btn_show_polygon_->set_hint_text("(Press `p`)");
  update_button_position(&x_btn, &y_btn);

  // show track 2d id
  btn_show_track_id_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_show_track_id_->set_option_name("show_track_id_2d");
  btn_show_track_id_->set_text("show track 2d id");
  btn_show_track_id_->set_hint_text("(Press `z`)");
  update_button_position(&x_btn, &y_btn);

  // show track 2d id
  btn_show_track_color_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_show_track_color_->set_option_name("show_track_color");
  btn_show_track_color_->set_text("show track color");
  btn_show_track_color_->set_hint_text("(Press `t`)");
  update_button_position(&x_btn, &y_btn);

  // show 2d bbox
  btn_show_2dbbox_.reset(
      new ToggleButton(x_btn, y_btn, button_size_, button_size_));
  btn_show_2dbbox_->set_option_name("show_2dbbox");
  btn_show_2dbbox_->set_text("show 2d bbox");
  btn_show_2dbbox_->set_hint_text("(Press `y`)");
  update_button_position(&x_btn, &y_btn);

  inited_ = true;
}

void OptionsViewport::draw() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, 0.0, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // GLRasterTextSingleton* raster_text =
  //   lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText *raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();
  raster_text->DrawString(
      "Press 'H' to show/hide this view", 100, 20, 255, 200, 0);

  int x_btn = border_size_;
  int y_btn = border_size_;

  if (label_switch_camera_) {
    label_switch_camera_->draw();
    label_switch_camera_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (label_capture_screen_) {
    label_capture_screen_->draw();
    label_capture_screen_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }

  if (label_switch_view_type_) {
    label_switch_view_type_->draw();
    label_switch_view_type_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }

  if (btn_car_color_) {
    btn_car_color_->draw();
    btn_car_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_cyclist_color_) {
    btn_cyclist_color_->draw();
    btn_cyclist_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_pedestrian_color_) {
    btn_pedestrian_color_->draw();
    btn_pedestrian_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_unknown_color_) {
    btn_unknown_color_->draw();
    btn_unknown_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_unknown_unmovable_color_) {
    btn_unknown_unmovable_color_->draw();
    btn_unknown_unmovable_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_unknown_movable_color_) {
    btn_unknown_movable_color_->draw();
    btn_unknown_movable_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
  if (btn_show_polygon_) {
    btn_show_polygon_->draw();
    btn_show_polygon_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }

  if (btn_show_track_id_) {
    btn_show_track_id_->draw();
    btn_show_track_id_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }

  if (btn_show_track_color_) {
    btn_show_track_color_->draw();
    btn_show_track_color_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }

  if (btn_show_2dbbox_) {
    btn_show_2dbbox_->draw();
    btn_show_2dbbox_->set_position(x_btn, y_btn);
    update_button_position(&x_btn, &y_btn);
  }
}

void OptionsViewport::on_mouse_button_click(int xpos, int ypos) {
  auto display_opt_manager = DisplayOptionsManager::Instance();
  if (btn_show_polygon_ && btn_show_polygon_->clicked(xpos, ypos)) {
    display_opt_manager->invert_option("show_polygon");
  }

  if (btn_show_track_id_ && btn_show_track_id_->clicked(xpos, ypos)) {
    display_opt_manager->invert_option("show_track_2d_id");
  }

  if (btn_show_track_color_ && btn_show_track_color_->clicked(xpos, ypos)) {
    display_opt_manager->invert_option("show_track_color");
  }

  if (btn_show_2dbbox_ && btn_show_2dbbox_->clicked(xpos, ypos)) {
    display_opt_manager->invert_option("show_track_color");
  }
}

REGISTER_GLVIEWPORT(OptionsViewport);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
