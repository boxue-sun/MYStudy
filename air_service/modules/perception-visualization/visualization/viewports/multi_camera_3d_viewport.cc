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

#include "air_service/modules/perception-visualization/visualization/viewports/multi_camera_3d_viewport.h"

#include <set>
#include <string>
#include <vector>

#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"
#include "base/common/singleton.h"
// #include
// "air_service/modules/perception-visualization/visualization/visualizer/glfw_viewer.h"
#include <iostream>  // fgq

#include "air_service/modules/perception-visualization/config/viz_config_manager.h"
// using ::idg::perception::common::SensorManager;
// #include "common/sensor_manager/sensor_manager.h"
// #include "camera/common/omnidirectional_undistortion_handler.h"

namespace airos {
namespace perception {
namespace visualization {

// using visualization::GLRasterTextSingleton;
using visualization::DisplayOptionsManager;
using visualization::DrawRect2D;
using visualization::OBJECT_TYPE_COLOR_TABLE;

void MultiCamera3DViewport::draw() {
  if (!inited_) {
    visualization::GenCloudVaoAndVbo(
        kVAOCloudNum_, kVBOCloudNum_, vao_cloud_, buffers_cloud_, cloud_verts_,
        sizeof(cloud_verts_));
    visualization::GenCirclesVaoAndVbo(
        kVAOCircleNum_, kVBOCircleNum_, vao_circle_, main_sensor_name_);
    inited_ = true;
  }

  if (!frame_content_) {
    return;
  }

  if (this->camera_names_.empty()) {
    return;
  }
  std::string camera_id = camera_names_[0];

  Eigen::Matrix4d sensor2world_pose;

  sensor2world_pose =
      frame_content_->get_camera_to_world_pose(main_sensor_name_);

  const auto& global_offset = frame_content_->global_position_offset();
  sensor2world_pose(0, 3) += global_offset(0);
  sensor2world_pose(1, 3) += global_offset(1);
  sensor2world_pose(2, 3) += global_offset(2);

  auto display_opt_manager = DisplayOptionsManager::Instance();

  bool draw_polygon  = display_opt_manager->get_option("show_polygon");
  bool draw_cube     = !draw_polygon;
  bool draw_velocity = true;
  bool draw_track_id = display_opt_manager->get_option("show_track_id_3d");
  bool draw_error_ellipse =
      display_opt_manager->get_option("show_error_ellipse");
  bool draw_object_cloud = false;
  // display_opt_manager->get_option("show_object_cloud");
  bool draw_with_class_color = false;

  // draw camera results from multi-cameras
  if (display_opt_manager->get_option("show_camera_objects_3d")) {
    // draw camera objects
    std::set<int> track_ids_set;
    std::vector<camera::ObjectInfo> all_camera_objects;
    std::vector<int> all_camera_objects_id;
    for (int i = 0; i < this->camera_names_.size(); ++i) {
      if (!display_opt_manager->get_option("show_" + this->camera_names_[i])) {
        continue;
      }
      // camera_names_[0] is main camera
      if (i == 0) {
        all_camera_objects =
            frame_content_->get_camera_objects(this->camera_names_[i]);
        for (auto obj : all_camera_objects) {
          track_ids_set.insert(obj.track_id);
          all_camera_objects_id.push_back(i);
        }
        continue;
      }
      std::vector<camera::ObjectInfo> camera_objects =
          frame_content_->get_camera_objects(this->camera_names_[i]);
      for (auto obj : camera_objects) {
        all_camera_objects_id.push_back(i);
        all_camera_objects.push_back(obj);
      }
    }
    Eigen::Vector3d cam_color(255, 255, 0);
    draw_with_class_color = true;
    DrawObjects(
        all_camera_objects, all_camera_objects_id, global_offset, draw_cube,
        draw_polygon, draw_velocity, draw_track_id, draw_error_ellipse,
        cam_color, draw_with_class_color);
  }
  DrawCircles();

  DrawCarForwardDir();
  // draw camera fov region
  for (int i = 0; i < camera_names_.size(); ++i) {
    DrawFrontCameraFOV(
        this->camera_names_[i], 15.0, Eigen::Vector3d(179, 212, 214), 0.1);
  }
  // DrawFrontCameraFOV(this->camera_names_[0], 15.0,
  //     Eigen::Vector3d(179, 212, 214), 0.1);
  // DrawFrontCameraFOV(this->camera_names_[1], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[2], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[3], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[4], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[5], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[6], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  // DrawFrontCameraFOV(this->camera_names_[7], 15.0,
  //     Eigen::Vector3d(211, 214, 179), 0.0);
  DrawHudText();
}

void MultiCamera3DViewport::DrawCircles() {
  Eigen::Matrix4d s2w_pose =
      frame_content_->get_camera_to_world_pose(main_sensor_name_);

  const auto& global_offset = frame_content_->global_position_offset();
  s2w_pose(0, 3) += global_offset(0);
  s2w_pose(1, 3) += global_offset(1);
  s2w_pose(2, 3) += global_offset(2);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  GLdouble mat[16] = {
      s2w_pose(0, 0), s2w_pose(1, 0), s2w_pose(2, 0), s2w_pose(3, 0),
      s2w_pose(0, 1), s2w_pose(1, 1), s2w_pose(2, 1), s2w_pose(3, 1),
      s2w_pose(0, 2), s2w_pose(1, 2), s2w_pose(2, 2), s2w_pose(3, 2),
      s2w_pose(0, 3), s2w_pose(1, 3), s2w_pose(2, 3), s2w_pose(3, 3)};
  glMultMatrixd(mat);
  // visualization::DrawCircles(
  //     kVAOCircleNum_, kVBOCircleNum_, vao_circle_);
  int ref_circle_num = base::Singleton<VizConfigManager>::get_instance()
                           ->GetVizConfig()
                           .ref_circle_num();
  visualization::DrawCircles(ref_circle_num, kVBOCircleNum_, vao_circle_);
  glPopMatrix();
}

void MultiCamera3DViewport::DrawCarForwardDir() {
  Eigen::Vector3d scene_center;
  Eigen::Vector3d forward_dir;
  Eigen::Matrix4d s2w_pose =
      frame_content_->get_camera_to_world_pose(main_sensor_name_);
  const auto& global_offset = frame_content_->global_position_offset();
  s2w_pose(0, 3) += global_offset(0);
  s2w_pose(1, 3) += global_offset(1);
  s2w_pose(2, 3) += global_offset(2);
  scene_center = (s2w_pose * CAMERA_SCENE_CENTER).head(3);
  forward_dir  = (s2w_pose * CAMERA_FORWARD_DIR).head(3);
  auto viz_config =
      base::Singleton<VizConfigManager>::get_instance()->GetVizConfig();
  double main_car_length = viz_config.main_car_size().length();
  double main_car_width  = viz_config.main_car_size().width();
  double main_car_height = viz_config.main_car_size().height();
  Eigen::Vector3d forward_pt =
      scene_center + forward_dir * main_car_length * 0.5;

  glColor3f(1.0, 0.5, 0.17);
  glLineWidth(2);

  glBegin(GL_LINES);
  glVertex3f(scene_center(0), scene_center(1), scene_center(2));
  glVertex3f(forward_pt(0), forward_pt(1), forward_pt(2));
  glEnd();

  glLineWidth(1);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  // draw main car box
  std::vector<Eigen::Vector3d> main_car_points;
  GetObject3DBBoxPoints(
      scene_center, forward_dir, main_car_length, main_car_width,
      main_car_height, 0, &main_car_points);
  Eigen::Vector3d main_car_color(255, 128, 0);
  Draw3DBBox(main_car_points, 1, main_car_color, 0.4);
}

void MultiCamera3DViewport::DrawFrontCameraFOV(
    const std::string& camera_name, double fov_region_radius,
    const Eigen::Vector3d& color, double z_offset) {
  Eigen::Matrix4d s2w_pose;
  Eigen::Vector3d scene_center;
  s2w_pose = frame_content_->get_camera_to_world_pose(camera_name);

  const auto& global_offset = frame_content_->global_position_offset();

  s2w_pose(0, 3) += global_offset(0);
  s2w_pose(1, 3) += global_offset(1);
  s2w_pose(2, 3) += global_offset(2);
  scene_center = (s2w_pose * CAMERA_SCENE_CENTER).head(3);

  double focal_length_x = 2700;
  double image_width    = 1080;

  double half_fov             = std::atan(0.5 * image_width / focal_length_x);
  Eigen::Vector4d forward_vec = Eigen::Vector4d(0, 0, fov_region_radius, 1);
  Eigen::Vector4d right_forward_vec = Eigen::Vector4d(
      fov_region_radius * std::sin(half_fov), 0,
      fov_region_radius * std::cos(half_fov), 1);
  Eigen::Vector4d left_forward_vec = Eigen::Vector4d(
      -fov_region_radius * std::sin(half_fov), 0,
      fov_region_radius * std::cos(half_fov), 1);

  Eigen::Vector3d fov_region_right_bound_pt =
      (s2w_pose * right_forward_vec).head(3);
  Eigen::Vector3d fov_region_left_bound_pt =
      (s2w_pose * left_forward_vec).head(3);

  scene_center(2) -= z_offset;
  fov_region_right_bound_pt(2) -= z_offset;
  fov_region_left_bound_pt(2) -= z_offset;

  DrawDashedLine3D(scene_center, fov_region_right_bound_pt, 2, color);
  DrawDashedLine3D(scene_center, fov_region_left_bound_pt, 2, color);

  glColor4f(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 0.1f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(scene_center(0), scene_center(1), scene_center(2));
  glVertex3f(
      fov_region_right_bound_pt(0), fov_region_right_bound_pt(1),
      fov_region_right_bound_pt(2));
  glVertex3f(
      fov_region_left_bound_pt(0), fov_region_left_bound_pt(1),
      fov_region_left_bound_pt(2));
  glEnd();
  glLineWidth(1);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void MultiCamera3DViewport::DrawHudText() {
  // if (this->camera_names_.empty() || this->lidar_names_.empty() ||
  //     this->radar_names_.empty()) {
  //   return;
  // }
  if (this->camera_names_.empty()) {
    return;
  }
  std::string camera_id = this->camera_names_[0];
  // std::string radar_id = this->radar_names_[0];
  // std::string lidar_id = this->lidar_names_[0];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glClear(GL_DEPTH_BUFFER_BIT);

  // GLRasterTextSingleton *raster_text =
  //   lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText* raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();

  // draw 'title'
  std::string str_title = "MULTI-CAMERAS";
  int x_title           = 10;
  int y_title           = 20;
  raster_text->DrawString(str_title.c_str(), x_title, y_title, 0, 0, 255);

  std::string camera_ts_str =
      "camera timestamp: " +
      std::to_string(frame_content_->get_camera_timestamp(camera_id));

  raster_text->DrawString(
      camera_ts_str.c_str(), x_title, y_title + 15, 255, 255, 255);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

REGISTER_GLVIEWPORT(MultiCamera3DViewport);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
