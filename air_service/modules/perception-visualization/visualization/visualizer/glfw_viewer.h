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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
// #include "base/camera.h"
// #include "base/distortion_model.h"
#include "air_service/modules/perception-visualization/visualization/base/base_visualizer.h"
#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/common/camera.h"
#include "air_service/modules/perception-visualization/visualization/viewports/base_gl_viewport.h"
#include "air_service/modules/perception-visualization/visualization/viewports/options_viewport.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace visualization {

class FrameContent;
// class OptionsViewport;

class GLFWFusionViewer {
 public:
  GLFWFusionViewer();
  virtual ~GLFWFusionViewer();

  GLFWFusionViewer(const GLFWFusionViewer &)            = delete;
  GLFWFusionViewer &operator=(const GLFWFusionViewer &) = delete;

  bool initialize(const VisualizerInitOptions &options);

  void set_frame_content(FrameContent *frame_content);

  void spin();

  void spin_once();

  void close();

  void set_full_window_viewport_idx(const int vp_idx) {
    if (vp_idx >= -1 && vp_idx <= 3) {
      full_window_viewport_idx_ = vp_idx;
    }
  }

  void set_model_name(const std::string &model_name) {
    model_name_ = model_name;
  }

  void set_background_color(Eigen::Vector3d i_bg_color) {
    bg_color_ = i_bg_color;
  }

  void set_camera_para(
      Eigen::Vector3d i_position, Eigen::Vector3d i_scn_center,
      Eigen::Vector3d i_up_vector);

  void SetCallback(const std::function<void()> &call_back_func);

  // callback assistants
  void resize_framebuffer(int width, int height);

  void mouse_move(double xpos, double ypos);

  void mouse_wheel(double delta);

  void reset();

  void keyboard(int key, int scancode, int action, int mods);

  void resize_window(int width, int height);

  // callback functions
  static void framebuffer_size_callback(
      GLFWwindow *window, int width, int height);

  static void window_size_callback(GLFWwindow *window, int width, int height);

  // input related
  static void key_callback(
      GLFWwindow *window, int key, int scancode, int action, int mods);

  static void mouse_button_callback(
      GLFWwindow *window, int button, int action, int mods);

  static void mouse_cursor_position_callback(
      GLFWwindow *window, double xpos, double ypos);

  static void mouse_scroll_callback(
      GLFWwindow *window, double xoffset, double yoffset);

  // error handling
  static void error_callback(int error, const char *description);

  // main car dimensions in meters
  static double s_main_car_length_;
  static double s_main_car_width_;
  static double s_main_car_height_;

  // ref-circle radius incremental; if this value is 20, the ref-circles'
  // radius are [20, 40, 60, 80, ...]
  // static double s_ref_circle_radius_inc_in_meters_;

  static int s_ref_circle_num_;

 private:
  bool init_window();

  bool init_camera();

  bool init_opengl();

  bool set_sensor_names(
      const std::vector<std::string> &lidar_names,
      const std::vector<std::string> &radar_names,
      const std::vector<std::string> &camera_names,
      const std::vector<std::string> &ultrasonic_names);

  bool init_config();

  bool init_viewports();

  void init_display_options();

  void pre_draw();

  void render();

 protected:
  // capture screen
  void capture_screen(const std::string &file_name);

  // set viewpoint for 3d rendering
  void set_top_view();
  void set_forward_view();

 private:
  bool init_;

  GLFWwindow *window_;
  visualization::Camera *pers_camera_;

  std::string model_name_;
  std::string screen_output_dir_;

  std::string display_opt_init_filepath_;

  Eigen::Vector3d bg_color_;
  int win_width_;
  int win_height_;
  int scene_width_;
  int scene_height_;
  int mouse_prev_x_;
  int mouse_prev_y_;
  Eigen::Matrix4d mode_mat_;
  Eigen::Matrix4d view_mat_;

  FrameContent *frame_content_;
  unsigned char *rgba_buffer_;

  bool capture_video_ = false;

  // render mode
  int render_mode_idx_;  // current index of render_modes_names_
  int view_type_;        // 0: top view; 1: forward view

  // sensor ids
  std::vector<std::string> lidar_names_;
  std::vector<std::string> radar_names_;
  std::vector<std::string> camera_names_;
  std::vector<std::string> ultrasonic_names_;

  std::string main_sensor_name_;

  // viewports: viewport name -> viewport instance
  std::unordered_map<std::string, std::shared_ptr<BaseGLViewport> > viewports_;

  std::shared_ptr<OptionsViewport> options_viewport_;

  std::vector<std::string> render_modes_names_;

  // rendering modes: render mode name -> viewport names
  std::unordered_map<std::string, std::vector<std::string> > render_modes_;

  int full_window_viewport_idx_ = -1;

  std::vector<double> crop_box_nrmd_;

  std::function<void()> call_back_func_ = std::function<void()>();
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
