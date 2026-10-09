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

#include "air_service/modules/perception-visualization/visualization/viewports/camera_proj_2d_viewport.h"

#include <memory>
#include <string>
#include <vector>
// #include <codecvt>
#include <locale>

#include "air_service/modules/perception-camera/algorithm/interface/algorithm_base.h"
#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"
#include "base/common/singleton.h"

#define DEBUG_LOG(msg) \
  { std::cout << msg << std::endl; }

namespace airos {
namespace perception {
namespace visualization {

int CameraProj2DViewport::s_camera_idx_ = 0;

void CameraProj2DViewport::draw() {
  if (!frame_content_ || this->camera_names_.empty() || s_camera_idx_ < 0 ||
      s_camera_idx_ >= camera_names_.size()) {
    return;
  }

  std::string camera_name = this->camera_names_[s_camera_idx_];
  // auto display_opt_manager = DisplayOptionsManager::Instance();

  // 1. draw color/range image as background
  std::shared_ptr<airos::base::Blob<uint8_t>> image_data =
      frame_content_->get_camera_color_image(camera_name);
  if (!image_data) {
    draw_title();
    return;
  }

  int image_width  = 0;
  int image_height = 0;
  if (image_data->num_axes() == 3) {
    image_height = image_data->shape(1);
    image_width  = image_data->shape(0);
  } else if (image_data->num_axes() == 4) {
    image_height = image_data->shape(1);
    image_width  = image_data->shape(2);
  } else {
    LOG_ERROR << "unsupported image_data->num_axes: " << image_data->num_axes();
  }

  glMatrixMode(GL_PROJECTION);  // Operate on projection matrix
  glPushMatrix();
  glLoadIdentity();
  // here is a trick, set 'bottom' to 'this->_height' and 'top' to '0'
  glOrtho(0, get_width(), get_height(), 0, 0.0, 100.0);

  glMatrixMode(GL_MODELVIEW);  // Operate on model-view matrix
  glPushMatrix();
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  GLuint image_tex = ImageToGlTexture(
      image_data, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP);

  // draw a quad, and map image onto it
  glBegin(GL_QUADS);
  glTexCoord2i(0, 0);
  glVertex2i(0, 0);

  glTexCoord2i(0, 1);
  glVertex2i(0, get_height());

  glTexCoord2i(1, 1);
  glVertex2i(get_width(), get_height());

  glTexCoord2i(1, 0);
  glVertex2i(get_width(), 0);
  glEnd();

  glDeleteTextures(1, &image_tex);
  glDisable(GL_TEXTURE_2D);

  // 2. draw camera  3d bboxes' projections on image
  const std::vector<camera::ObjectInfo>& camera_objects =
      frame_content_->get_camera_objects(camera_name);
  // GLRasterTextSingleton* raster_text =
  //     lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText* raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();
  Eigen::Matrix4d pose_w2c =
      frame_content_->get_camera_to_world_pose(camera_name).inverse();

  Eigen::Matrix3f intrinsic_params =
      frame_content_->get_camera_intrinsic_params(camera_name);

  // scale factors
  double sx = 1.0 * get_width() / image_width;
  double sy = 1.0 * get_height() / image_height;

  // draw 3d bboxes' projections
  for (const auto& obj : camera_objects) {
    // if (obj.camera_supplement.sensor_name != camera_name) { //TODO fgq
    //   continue;
    // }
    auto type  = OBJECT_TYPE_TABLE.at(obj.type);
    auto color = OBJECT_TYPE_COLOR_TABLE.at(type);
    // const auto &bbox2d = obj.camera_supplement.box;  //TODO fgq
    // Eigen::Vector2d ul_pt(bbox2d.xmin * sx, bbox2d.ymin * sy);
    // Eigen::Vector2d lr_pt(bbox2d.xmax * sx, bbox2d.ymax * sy);
    const auto& bbox2d = obj.box;
    Eigen::Vector2d ul_pt(bbox2d.left_top.x * sx, bbox2d.left_top.y * sy);
    Eigen::Vector2d lr_pt(
        bbox2d.right_bottom.x * sx, bbox2d.right_bottom.y * sy);

    // get 3d bbox
    std::vector<Eigen::Vector3d> bbox3d_pts;
    Eigen::Vector3d direction(
        obj.direction.x, obj.direction.y, obj.direction.z);

    GetObject3DBBoxPoints(
        obj.center, direction.cast<double>(),  // TODO fgq
        obj.size.length, obj.size.width, obj.size.height, obj.theta,
        &bbox3d_pts);
    std::vector<Eigen::Vector2d> bbox3d_pts_proj;

    for (int i = 0; i < bbox3d_pts.size(); ++i) {
      Eigen::Vector2d pt2d;
      if (!ProjectPoint(
              pose_w2c, intrinsic_params, camera_name,  // TODO fgq
              bbox3d_pts[i], &pt2d)) {
        continue;
      }
      bbox3d_pts_proj.push_back(pt2d);
    }
    // for(auto &pt: obj.pts8.pts8) {  //直接用回3D后的八点
    //   bbox3d_pts_proj.push_back({pt.x, pt.y});
    // }

    if (bbox3d_pts_proj.size() != bbox3d_pts.size()) {
      continue;
    }
    std::transform(
        bbox3d_pts_proj.begin(), bbox3d_pts_proj.end(), bbox3d_pts_proj.begin(),
        [&](Eigen::Vector2d pt) {
          return Eigen::Vector2d(pt[0] * sx, pt[1] * sy);
        });
    Eigen::Vector3d color_top = color;
    Eigen::Vector3d color_bottom(255, 0, 0);
    Eigen::Vector3d color_side = color;
    Draw3DBBoxProjection(
        bbox3d_pts_proj, 2, color_top, color_bottom, color_side);

    // track id
    std::string track_id_str = std::to_string(obj.track_id);
    raster_text->DrawString(
        track_id_str.c_str(), ul_pt[0], lr_pt[1] - 30, 255, 0, 0);
  }

  draw_title();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void CameraProj2DViewport::draw_title() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glClear(GL_DEPTH_BUFFER_BIT);

  // GLRasterTextSingleton* raster_text =
  //     lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText* raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();

  // draw 'title'
  std::string camera_name = this->camera_names_[s_camera_idx_];
  std::string str_title   = "CAMERA 3D PROJ " + camera_name;
  int x_title             = 10;
  int y_title             = 20;
  raster_text->DrawString(str_title.c_str(), x_title, y_title, 0, 0, 255);

  std::string camera_ts_str =
      "camera timestamp: " +
      std::to_string(frame_content_->get_camera_timestamp(camera_name));
  raster_text->DrawString(
      camera_ts_str.c_str(), x_title, y_title + 15, 255, 255, 255);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

REGISTER_GLVIEWPORT(CameraProj2DViewport);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
