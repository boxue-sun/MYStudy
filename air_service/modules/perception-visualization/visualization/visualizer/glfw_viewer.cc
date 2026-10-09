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

#include "air_service/modules/perception-visualization/visualization/visualizer/glfw_viewer.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <opencv2/opencv.hpp>

#include "base/common/log.h"
// #include "common/sensor_manager/sensor_manager.h"
// #include "lib/config_manager/config_manager.h"
#include "air_service/modules/perception-visualization/config/viz_config_manager.h"
#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/common/bmp.h"
#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/viewports/camera_2d_viewport.h"
#include "air_service/modules/perception-visualization/visualization/viewports/camera_proj_2d_viewport.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"
#include "base/common/singleton.h"
#include "base/plugin/registerer.h"
// using ::idg::perception::common::SensorManager;
// using ::idg::perception::lib::Singleton;

namespace airos {
namespace perception {
namespace visualization {

// using visualizer::common::DisplayOptionsManager;

double GLFWFusionViewer::s_main_car_length_ = 1.0f;
double GLFWFusionViewer::s_main_car_width_  = 1.0f;
double GLFWFusionViewer::s_main_car_height_ = 1.0f;
// double GLFWFusionViewer::s_ref_circle_radius_inc_in_meters_ = 20.0;
int GLFWFusionViewer::s_ref_circle_num_ = 10;

GLFWFusionViewer::GLFWFusionViewer()
    : init_(false)
    , window_(nullptr)
    , pers_camera_(nullptr)
    , win_width_(2560)
    , win_height_(1440)
    , scene_width_(1280)
    , scene_height_(720)
    , mouse_prev_x_(0)
    , mouse_prev_y_(0)
    , bg_color_(0.2, 0.2, 0.2)
    , mode_mat_(Eigen::Matrix4d::Identity())
    , view_mat_(Eigen::Matrix4d::Identity())
    , frame_content_(nullptr)
    , rgba_buffer_(nullptr)
    , capture_video_(false)
    , render_mode_idx_(0)
    , view_type_(0) {}

GLFWFusionViewer::~GLFWFusionViewer() {
  close();
  if (pers_camera_) {
    delete pers_camera_;
  }
  if (rgba_buffer_) {
    delete[] rgba_buffer_;
    rgba_buffer_ = nullptr;
  }
}

bool GLFWFusionViewer::initialize(const VisualizerInitOptions &options) {
  LOG_INFO << "GLFWFusionViewer::initialize()" << std::endl;
  if (init_) {
    LOG_INFO << " GLFWFusionViewer is already initialized !" << std::endl;
    return false;
  }
  VizConfigManager *viz_config_manager =
      base::Singleton<VizConfigManager>::get_instance();
  // model_name_ = options.conf_model_name;
  // screen_output_dir_ = options.screen_output_dir;
  screen_output_dir_ = viz_config_manager->GetVizConfig().screen_output_dir();
  // if (!init_model_config()) {
  //   LOG_ERROR << "failed to init model config";
  //   return false;
  // }
  if (!init_config()) {
    LOG_ERROR << "failed to init config";
    return false;
  }
  if (!init_window()) {
    LOG_ERROR << " Failed to initialize the window !" << std::endl;
    return false;
  }

  if (!init_camera()) {
    LOG_ERROR << " Failed to initialize the camera !" << std::endl;
    return false;
  }

  if (!init_opengl()) {
    LOG_ERROR << " Failed to initialize opengl !" << std::endl;
    return false;
  }

  // init viewports
  if (!init_viewports()) {
    LOG_ERROR << "init_viewports failed";
    return false;
  }

  init_display_options();

  init_ = true;

  return true;
}

void GLFWFusionViewer::spin() {
  while (!glfwWindowShouldClose(window_) && frame_content_) {
    glfwPollEvents();
    render();
    glfwSwapBuffers(window_);
  }
  glfwDestroyWindow(window_);
}

void GLFWFusionViewer::spin_once() {
  if (!frame_content_) {
    return;
  }
  glfwPollEvents();
  render();
  glfwSwapBuffers(window_);
}

void GLFWFusionViewer::close() {
  glfwTerminate();
}

void GLFWFusionViewer::set_camera_para(
    Eigen::Vector3d i_position, Eigen::Vector3d i_scn_center,
    Eigen::Vector3d i_up_vector) {
  pers_camera_->set_position(i_position);
  pers_camera_->setscene_center(i_scn_center);
  pers_camera_->setup_vector(i_up_vector);
  pers_camera_->look_at(i_scn_center);

  GLdouble v_mat[16];
  pers_camera_->get_model_view_matrix(v_mat);
  view_mat_ << v_mat[0], v_mat[4], v_mat[8], v_mat[12], v_mat[1], v_mat[5],
      v_mat[9], v_mat[13], v_mat[2], v_mat[6], v_mat[10], v_mat[14], v_mat[3],
      v_mat[7], v_mat[11], v_mat[15];
}

bool GLFWFusionViewer::init_window() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize glfw ! " << strerror(errno) << "\n";
    return false;
  }

  win_width_  = scene_width_ * 2;
  win_height_ = scene_height_ * 2;
  window_     = glfwCreateWindow(
      win_width_, win_height_, "gl_fusion_visualizer", nullptr, nullptr);
  if (window_ == nullptr) {
    std::cerr << "Failed to create glfw window!\n";
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(window_, this);

  // set callback functions
  glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

  glfwSetKeyCallback(window_, key_callback);
  glfwSetMouseButtonCallback(window_, mouse_button_callback);
  glfwSetCursorPosCallback(window_, mouse_cursor_position_callback);
  glfwSetScrollCallback(window_, mouse_scroll_callback);

  glfwShowWindow(window_);
  return true;
}

bool GLFWFusionViewer::init_camera() {
  // perspective cameras
  pers_camera_ = new Camera;
  pers_camera_->set_type(Camera::Type::PERSPECTIVE);
  pers_camera_->setscene_radius(1000);
  pers_camera_->set_position(Eigen::Vector3d(0, 0, -30));
  pers_camera_->setscreen_widthandheight(scene_width_, scene_height_);
  pers_camera_->look_at(Eigen::Vector3d(0, 0, 0));
  double fov = 45 * (M_PI / 180.0);
  pers_camera_->setfield_of_view(fov);

  return true;
}

bool GLFWFusionViewer::init_opengl() {
  glViewport(0, 0, scene_width_, scene_height_);
  glClearColor(bg_color_(0), bg_color_(1), bg_color_(2), 1.0);
  glClearDepth(1.0f);
  glShadeModel(GL_SMOOTH);
  glDepthFunc(GL_LEQUAL);
  // lighting
  GLfloat mat_shininess[]  = {20.0};
  GLfloat lmodel_ambient[] = {.5, .5, .5, 1.0};
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    LOG_ERROR << "init GLEW failed";
    return false;
  }

  return true;
}

bool GLFWFusionViewer::set_sensor_names(
    const std::vector<std::string> &lidar_names,
    const std::vector<std::string> &radar_names,
    const std::vector<std::string> &camera_names,
    const std::vector<std::string> &ultrasonic_names) {
  if (lidar_names.empty() || radar_names.empty() || camera_names.empty()) {
    return false;
  }
  lidar_names_.clear();
  lidar_names_.assign(lidar_names.begin(), lidar_names.end());

  radar_names_.clear();
  radar_names_.assign(radar_names.begin(), radar_names.end());

  camera_names_.clear();
  camera_names_.assign(camera_names.begin(), camera_names.end());

  ultrasonic_names_.clear();
  ultrasonic_names_.assign(ultrasonic_names.begin(), ultrasonic_names.end());

  return true;
}

bool GLFWFusionViewer::init_config() {
  auto &viz_config =
      base::Singleton<VizConfigManager>::get_instance()->GetVizConfig();

  screen_output_dir_ = viz_config.screen_output_dir();
  main_sensor_name_  = viz_config.camera_config().main_camera_id();
  for (int i = 0; i < viz_config.camera_config().camera_ids_size(); ++i) {
    camera_names_.push_back(viz_config.camera_config().camera_ids(i));
  }

  s_main_car_length_ = viz_config.main_car_size().length();
  s_main_car_width_  = viz_config.main_car_size().width();
  s_main_car_height_ = viz_config.main_car_size().height();

  // s_ref_circle_radius_inc_in_meters_ = viz_config.ref_circle_radius_inc();

  // read viewports name from config
  std::vector<std::string> viewports_name;
  std::string render_mode = viz_config.render_config().render_mode();
  render_modes_names_.push_back(render_mode);

  std::string viewport_name = "";
  for (int i = 0; i < viz_config.render_config().viewports_name_size(); ++i) {
    viewport_name             = viz_config.render_config().viewports_name(i);
    viewports_[viewport_name] = nullptr;
    viewports_name.push_back(viewport_name);
  }
  render_modes_[render_mode] = viewports_name;

  FrameContent::s_max_data_buffer_size_ = viz_config.max_data_buffer_size();
  FrameContent::s_max_buffer_interval_  = viz_config.max_buffer_interval();

  full_window_viewport_idx_ = viz_config.full_window_viewport_idx();
  // crop_box_nrmd_ = viz_config.crop_box_normalized();
  for (int i = 0; i < viz_config.crop_box_normalized_size(); ++i) {
    crop_box_nrmd_.push_back(viz_config.crop_box_normalized(i));
  }
  s_ref_circle_num_ = viz_config.ref_circle_num();

  display_opt_init_filepath_ = viz_config.display_opt_init_filepath();

  return true;
}

bool GLFWFusionViewer::init_viewports() {
  for (auto &kv : viewports_) {
    auto &viewport_name = kv.first;
    viewports_[viewport_name].reset(
        BaseGLViewportRegisterer::GetInstanceByName(viewport_name));
    if (!viewports_[viewport_name]) {
      LOG_ERROR << "failed to get viewport instance: " << viewport_name;
      return false;
    }

    viewports_[viewport_name]->set_camera_names(camera_names_);
  }

  options_viewport_.reset(new OptionsViewport);
  options_viewport_->set_viewport(0, 0, win_width_, win_height_);
  // must set viewport size before init buttons
  options_viewport_->init_buttons();

  return true;
}

void GLFWFusionViewer::init_display_options() {
  auto display_opt_manager = DisplayOptionsManager::Instance();

  display_opt_manager->set_option("show_polygon", false);
  display_opt_manager->set_option("show_lidar_objects_3d", true);
  display_opt_manager->set_option("show_radar_objects_3d", true);
  display_opt_manager->set_option("show_camera_objects_3d", true);
  display_opt_manager->set_option("show_ultrasonic_origin_objects_3d", true);
  display_opt_manager->set_option("show_ultrasonic_objects_3d", true);
  display_opt_manager->set_option("show_map_roi_3d", true);
  display_opt_manager->set_option("show_map_road_boundary_3d", true);
  display_opt_manager->set_option("show_track_id_3d", true);
  display_opt_manager->set_option("show_error_ellipse", false);
  display_opt_manager->set_option("show_camera_smartereye", true);
  display_opt_manager->set_option("show_camera_left_forward", true);
  display_opt_manager->set_option("show_camera_right_forward", true);
  display_opt_manager->set_option("show_camera_rear", true);
  display_opt_manager->set_option("show_camera_left_backward", true);
  display_opt_manager->set_option("show_camera_right_backward", true);
  display_opt_manager->set_option("show_camera_front_obstacle", true);
  display_opt_manager->set_option("show_camera_front_narrow", true);
  display_opt_manager->set_option("show_onsemi_obstacle", true);
  display_opt_manager->set_option("show_onsemi_narrow", true);
  display_opt_manager->set_option("show_range_image", false);
  display_opt_manager->set_option("show_origin_cloud", true);
  display_opt_manager->set_option("show_raw_radar_point", false);

  display_opt_manager->set_option("show_camera_3d_side", true);

  display_opt_manager->set_option("show_options_view", false);

  display_opt_manager->set_option("show_object_cloud", true);
  display_opt_manager->set_option("show_track_color", false);
  display_opt_manager->set_option("show_camera_ego_lane", false);
  display_opt_manager->set_option("show_camera_ego_lane_only", false);
  display_opt_manager->set_option("show_track_id_2d", true);

  display_opt_manager->set_option("show_plate_id_2d", true);

  if (display_opt_init_filepath_ != "") {
    display_opt_manager->ReadOptionsFromINIFile(display_opt_init_filepath_);
  }
}

void GLFWFusionViewer::pre_draw() {
  glClearColor(bg_color_(0), bg_color_(1), bg_color_(2), 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  pers_camera_->load_projection_matrix();

  // column major
  GLdouble mode_mat[16] = {
      mode_mat_(0, 0), mode_mat_(1, 0), mode_mat_(2, 0), mode_mat_(3, 0),
      mode_mat_(0, 1), mode_mat_(1, 1), mode_mat_(2, 1), mode_mat_(3, 1),
      mode_mat_(0, 2), mode_mat_(1, 2), mode_mat_(2, 2), mode_mat_(3, 2),
      mode_mat_(0, 3), mode_mat_(1, 3), mode_mat_(2, 3), mode_mat_(3, 3)};
  GLdouble view_mat[16] = {
      view_mat_(0, 0), view_mat_(1, 0), view_mat_(2, 0), view_mat_(3, 0),
      view_mat_(0, 1), view_mat_(1, 1), view_mat_(2, 1), view_mat_(3, 1),
      view_mat_(0, 2), view_mat_(1, 2), view_mat_(2, 2), view_mat_(3, 2),
      view_mat_(0, 3), view_mat_(1, 3), view_mat_(2, 3), view_mat_(3, 3)};
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(mode_mat);
  glMultMatrixd(view_mat);
}

void GLFWFusionViewer::render() {
  pre_draw();
  auto &cur_render_mode_name = render_modes_names_[render_mode_idx_];
  auto &cur_viewports_names  = render_modes_[cur_render_mode_name];
  // _viewports are arranged as follows
  // TODO(guiyilin): support configurable layouts
  /*
      ___________________
      |        |        |
      |    2   |    3   |
      |        |        |
      |________|________|
      |        |        |
      |    0   |    1   |
      |        |        |
      |________|________|
   y|
    |
    ----x
  */
  auto display_opt_manager = DisplayOptionsManager::Instance();
  if (!display_opt_manager->get_option("show_options_view")) {
    if (full_window_viewport_idx_ == -1) {
      viewports_[cur_viewports_names[0]]->set_viewport(
          0, 0, scene_width_, scene_height_);
      viewports_[cur_viewports_names[0]]->set_frame_content(frame_content_);
      viewports_[cur_viewports_names[0]]->render();

      viewports_[cur_viewports_names[1]]->set_viewport(
          scene_width_, 0, scene_width_, scene_height_);
      viewports_[cur_viewports_names[1]]->set_frame_content(frame_content_);
      viewports_[cur_viewports_names[1]]->render();

      viewports_[cur_viewports_names[2]]->set_viewport(
          0, scene_height_, scene_width_, scene_height_);
      viewports_[cur_viewports_names[2]]->set_frame_content(frame_content_);
      viewports_[cur_viewports_names[2]]->render();

      // viewports_[cur_viewports_names[3]]->set_viewport(
      //     scene_width_, scene_height_, scene_width_, scene_height_);
      // viewports_[cur_viewports_names[3]]->set_frame_content(frame_content_);
      // viewports_[cur_viewports_names[3]]->render();
    } else if (
        full_window_viewport_idx_ >= 0 &&
        full_window_viewport_idx_ < cur_viewports_names.size()) {
      viewports_[cur_viewports_names[full_window_viewport_idx_]]->set_viewport(
          0, 0, win_width_, win_height_);
      viewports_[cur_viewports_names[full_window_viewport_idx_]]
          ->set_frame_content(frame_content_);
      viewports_[cur_viewports_names[full_window_viewport_idx_]]->render();
    }
  } else {
    options_viewport_->set_viewport(0, 0, win_width_, win_height_);
    options_viewport_->set_frame_content(frame_content_);
    options_viewport_->render();
  }
  // save images
  static int no_frame = 0;
  if (capture_video_) {
    if (camera_names_.empty()) {
      return;
    }

    double time_stamp = frame_content_->get_camera_timestamp(main_sensor_name_);

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "./%.12f.jpg", time_stamp);
    std::string file_name = screen_output_dir_ + buffer;
    capture_screen(file_name);
  }

  no_frame++;
}

/************************callback functions************************/

void GLFWFusionViewer::framebuffer_size_callback(
    GLFWwindow *window, int width, int height) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }

  GLFWFusionViewer *vis = static_cast<GLFWFusionViewer *>(user_data);
  vis->resize_framebuffer(width, height);
}

void GLFWFusionViewer::window_size_callback(
    GLFWwindow *window, int width, int height) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }

  GLFWFusionViewer *vis = static_cast<GLFWFusionViewer *>(user_data);
  vis->resize_window(width, height);
}

void GLFWFusionViewer::key_callback(
    GLFWwindow *window, int key, int scancode, int action, int mods) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }
  if (action == GLFW_PRESS) {
    GLFWFusionViewer *vis = static_cast<GLFWFusionViewer *>(user_data);
    LOG_INFO << "key_value: " << key;
    vis->keyboard(key, scancode, action, mods);
  }
}

void GLFWFusionViewer::mouse_button_callback(
    GLFWwindow *window, int button, int action, int mods) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }
  GLFWFusionViewer *vis    = static_cast<GLFWFusionViewer *>(user_data);
  auto display_opt_manager = DisplayOptionsManager::Instance();

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double xpos;
    double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (display_opt_manager->get_option("show_options_view")) {
      vis->options_viewport_->on_mouse_button_click(xpos, ypos);
    }
  }

  if (mods == GLFW_MOD_CONTROL && button == GLFW_MOUSE_BUTTON_LEFT &&
      action == GLFW_PRESS && vis->full_window_viewport_idx_ == -1) {
    double xpos;
    double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // note: here we use window coordinates
    if (xpos >= 0 && ypos >= 0 && xpos < vis->scene_width_ &&
        ypos < vis->scene_height_) {
      vis->full_window_viewport_idx_                = 2;
      Camera2DViewport::full_window_viewport_index_ = 2;
    } else if (
        xpos >= vis->scene_width_ && ypos >= 0 &&
        xpos < 2 * vis->scene_width_ && ypos < vis->scene_height_) {
      vis->full_window_viewport_idx_                = 3;
      Camera2DViewport::full_window_viewport_index_ = 3;
    } else if (
        xpos >= 0 && ypos >= vis->scene_height_ && xpos < vis->scene_width_ &&
        ypos < 2 * vis->scene_height_) {
      vis->full_window_viewport_idx_                = 0;
      Camera2DViewport::full_window_viewport_index_ = 0;
    } else if (
        xpos >= vis->scene_width_ && ypos >= vis->scene_height_ &&
        xpos < 2 * vis->scene_width_ && ypos < 2 * vis->scene_height_) {
      vis->full_window_viewport_idx_                = 1;
      Camera2DViewport::full_window_viewport_index_ = 1;
    }
  }

  if (mods == GLFW_MOD_CONTROL && button == GLFW_MOUSE_BUTTON_RIGHT &&
      action == GLFW_PRESS) {
    vis->full_window_viewport_idx_                = -1;
    Camera2DViewport::full_window_viewport_index_ = -1;
  }

  // calib visual
  if (mods == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_LEFT &&
      action == GLFW_PRESS) {
    double xpos;
    double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // note: here we use window coordinates

    if ((vis->full_window_viewport_idx_ == -1 && xpos >= 0 && ypos >= 0 &&
         xpos < vis->scene_width_ && ypos < 2 * vis->scene_height_) ||
        vis->full_window_viewport_idx_ == 2 ||
        vis->full_window_viewport_idx_ == 0) {
      Camera2DViewport::xpos_       = xpos;
      Camera2DViewport::ypos_       = ypos;
      Camera2DViewport::choose_obs_ = true;
    }
  }

  if (mods == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_RIGHT &&
      action == GLFW_PRESS) {
    Camera2DViewport::choose_track_id_ = -2;
    Camera2DViewport::choose_obs_      = false;
  }
}

void GLFWFusionViewer::mouse_cursor_position_callback(
    GLFWwindow *window, double xpos, double ypos) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }
  GLFWFusionViewer *vis = static_cast<GLFWFusionViewer *>(user_data);
  vis->mouse_move(xpos, ypos);
}

void GLFWFusionViewer::mouse_scroll_callback(
    GLFWwindow *window, double xoffset, double yoffset) {
  void *user_data = glfwGetWindowUserPointer(window);
  if (user_data == nullptr) {
    return;
  }
  GLFWFusionViewer *vis = static_cast<GLFWFusionViewer *>(user_data);
  vis->mouse_wheel(yoffset);
}

void GLFWFusionViewer::error_callback(int error, const char *description) {
  std::cout << "ERROR - " << error << "  " << description << "\n";
}

/************************callback assistants************************/
void GLFWFusionViewer::resize_window(int width, int height) {
  if (width == win_width_ && height == win_height_) {
    return;
  }
  if (rgba_buffer_ != nullptr) {
    delete[] rgba_buffer_;
    rgba_buffer_ = nullptr;
  }
  rgba_buffer_  = new unsigned char[width * height * 4];
  win_width_    = width;
  win_height_   = height;
  scene_width_  = win_width_ * 0.5;
  scene_height_ = win_height_ * 0.5;

  pers_camera_->setscreen_widthandheight(scene_width_, scene_height_);
}

void GLFWFusionViewer::resize_framebuffer(int width, int height) {
  if (width == win_width_ && height == win_height_) {
    return;
  }
  if (rgba_buffer_ != nullptr) {
    delete[] rgba_buffer_;
    rgba_buffer_ = nullptr;
  }
  rgba_buffer_  = new unsigned char[width * height * 4];
  win_width_    = width;
  win_height_   = height;
  scene_width_  = win_width_ * 0.5;
  scene_height_ = win_height_ * 0.5;

  pers_camera_->setscreen_widthandheight(scene_width_, scene_height_);
}

void GLFWFusionViewer::mouse_move(double xpos, double ypos) {
  int state_left  = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
  int state_right = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT);
  int x_delta     = xpos - mouse_prev_x_;
  int y_delta     = ypos - mouse_prev_y_;
  if (state_left == GLFW_PRESS) {
    Eigen::Quaterniond rot =
        pers_camera_->get_rotatation_by_mouse_from_qgwidget(
            mouse_prev_x_, mouse_prev_y_, xpos, ypos);
    Eigen::Matrix3d rot_mat    = rot.inverse().toRotationMatrix();
    Eigen::Vector3d scn_center = pers_camera_->scene_center();
    Eigen::Vector4d scn_center_h(
        scn_center(0), scn_center(1), scn_center(2), 1);
    scn_center_h                       = mode_mat_ * view_mat_ * scn_center_h;
    scn_center                         = scn_center_h.head(3);
    Eigen::Vector3d r_multi_scn_center = rot_mat * scn_center;
    Eigen::Vector3d t                  = scn_center - r_multi_scn_center;
    Eigen::Matrix4d cur_mat            = Eigen::Matrix4d::Identity();
    cur_mat.topLeftCorner(3, 3)        = rot_mat;
    cur_mat.topRightCorner(3, 1)       = t;
    mode_mat_                          = cur_mat * mode_mat_;
  } else if (state_right == GLFW_PRESS) {
    mode_mat_(0, 3) += 0.1 * x_delta;
    mode_mat_(1, 3) -= 0.1 * y_delta;
  }

  mouse_prev_x_ = xpos;
  mouse_prev_y_ = ypos;
}

void GLFWFusionViewer::mouse_wheel(double delta) {
  mode_mat_(2, 3) -= delta;
}

void GLFWFusionViewer::reset() {
  mode_mat_ = Eigen::Matrix4d::Identity();
}

void GLFWFusionViewer::keyboard(int key, int scancode, int action, int mods) {
  auto display_opt_manager = DisplayOptionsManager::Instance();
  switch (key) {
    case GLFW_KEY_R:
      reset();
      break;
    case GLFW_KEY_P:
      display_opt_manager->invert_option("show_polygon");
      break;
    case GLFW_KEY_C:  // switch camera
      // TODO(guiyilin): add camera idx
      Camera2DViewport::s_camera_idx_ =
          (Camera2DViewport::s_camera_idx_ + 1) % camera_names_.size();
      CameraProj2DViewport::s_camera_idx_ =
          (CameraProj2DViewport::s_camera_idx_ + 1) % camera_names_.size();
      break;
    case GLFW_KEY_A:
      capture_video_ = true;
      break;
    case GLFW_KEY_E:  // 'E'
      view_type_ = (view_type_ + 1) % 2;
      break;
    case GLFW_KEY_H:
      display_opt_manager->invert_option("show_options_view");
      break;
    case GLFW_KEY_Y:
      Camera2DViewport::display_2dbbox_ = !Camera2DViewport::display_2dbbox_;
      break;
    case GLFW_KEY_T:
      display_opt_manager->invert_option("show_track_color");
      break;
    case GLFW_KEY_Z:
      display_opt_manager->invert_option("show_track_id_2d");
      break;
    case GLFW_KEY_W:
      if (call_back_func_) {
        call_back_func_();
      }
      break;
    default:
      break;
  }
}

void GLFWFusionViewer::SetCallback(
    const std::function<void()> &call_back_func) {
  call_back_func_ = call_back_func;
}

void GLFWFusionViewer::capture_screen(const std::string &file_name) {
  if (rgba_buffer_ == nullptr) {
    rgba_buffer_ = new unsigned char[4 * win_width_ * win_height_];
    if (rgba_buffer_ == nullptr) {
      LOG_ERROR << "Failed to create screen capture buffer \n";
      return;
    }
  }
  glReadPixels(
      0, 0, win_width_, win_height_, GL_BGRA, GL_UNSIGNED_BYTE, rgba_buffer_);
  auto gl_align = [](int width) {
    return 4 * ((width) / 4 + ((width) % 4 != 0));
  };
  // use OpenCV to save screenshots as .jpg file
  cv::Mat4b output_img(
      win_height_, win_width_, (cv::Vec4b *)rgba_buffer_,
      gl_align(win_width_ * 4));
  cv::flip(output_img, output_img, 0);
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
  compression_params.push_back(100);
  cv::Rect crop_rect(
      static_cast<int>(crop_box_nrmd_[0] * output_img.cols),
      static_cast<int>(crop_box_nrmd_[1] * output_img.rows),
      static_cast<int>(
          (crop_box_nrmd_[2] - crop_box_nrmd_[0]) * output_img.cols),
      static_cast<int>(
          (crop_box_nrmd_[3] - crop_box_nrmd_[1]) * output_img.rows));
  if (crop_box_nrmd_[0] > 0.0 || crop_box_nrmd_[1] > 0.0 ||
      crop_box_nrmd_[2] < 1.0 || crop_box_nrmd_[3] < 1.0) {
    if (crop_rect.width % 2 != 0) {
      crop_rect.width -= 1;
    }
    if (crop_rect.height % 2 != 0) {
      crop_rect.height -= 1;
    }
    cv::Mat crop_img = output_img(crop_rect).clone();
    cv::imwrite(file_name, crop_img, compression_params);
  } else {
    cv::imwrite(file_name, output_img, compression_params);
  }
  // SaveRGBAImageToBMP<unsigned char>(rgba_buffer_,
  //     win_width_, win_height_, file_name.c_str());
}

void GLFWFusionViewer::set_top_view() {
  if (frame_content_ == nullptr) {
    return;
  }
  Eigen::Matrix4d s2w_pose;
  Eigen::Vector3d up_dir_world;
  Eigen::Vector3d view_point_world;
  Eigen::Vector3d camera_center_world;
  // SensorManager *sensor_manager =
  // lib::Singleton<SensorManager>::get_instance(); if
  // (sensor_manager->IsCamera(main_sensor_name_)) {
  s2w_pose = frame_content_->get_camera_to_world_pose(main_sensor_name_);
  const auto &global_offset = frame_content_->global_position_offset();
  s2w_pose(0, 3) += global_offset(0);
  s2w_pose(1, 3) += global_offset(1);
  s2w_pose(2, 3) += global_offset(2);
  up_dir_world        = (s2w_pose * CAMERA_UP_DIR).head(3);
  view_point_world    = (s2w_pose * CAMERA_SCENE_CENTER).head(3);
  camera_center_world = (s2w_pose * CAMERA_CENTER).head(3);
  // }

  set_camera_para(camera_center_world, view_point_world, up_dir_world);
}

void GLFWFusionViewer::set_forward_view() {
  if (frame_content_ == nullptr || camera_names_.empty()) {
    return;
  }

  Eigen::Matrix4d pose_c2w =
      frame_content_->get_camera_to_world_pose(camera_names_[0]);
  const auto &global_offset = frame_content_->global_position_offset();
  pose_c2w(0, 3) += global_offset(0);
  pose_c2w(1, 3) += global_offset(1);
  pose_c2w(2, 3) += global_offset(2);

  Eigen::Vector3d up_world = (pose_c2w * Eigen::Vector4d(0, -1, 0, 0)).head(3);
  Eigen::Vector3d view_point_world =
      (pose_c2w * Eigen::Vector4d(0, 0, 10, 1)).head(3);
  Eigen::Vector3d camera_center_world =
      (pose_c2w * Eigen::Vector4d(0, -9, -15, 1)).head(3);

  Eigen::Vector3d forward_world =
      (pose_c2w * Eigen::Vector4d(0, 0, 1, 0)).head(3);

  set_camera_para(camera_center_world, view_point_world, up_world);
}

void GLFWFusionViewer::set_frame_content(FrameContent *frame_content) {
  frame_content_ = frame_content;
  if (view_type_ == 0) {
    set_top_view();
  } else {
    set_forward_view();
  }
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
