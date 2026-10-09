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

#include "air_service/modules/perception-visualization/visualization/viewports/camera_2d_viewport.h"

#include <memory>
#include <string>
#include <vector>
// #include <codecvt>
#include <locale>

#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"
#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace visualization {

int Camera2DViewport::s_camera_idx_ = 0;
// bool Camera2DViewport::display_calib_points_ground_ = false;
bool Camera2DViewport::display_2dbbox_            = true;
bool Camera2DViewport::display_3dbbox_8pts_       = false;
bool Camera2DViewport::display_3dbbox_bottom_uv_  = false;
double Camera2DViewport::xpos_                    = 0;
double Camera2DViewport::ypos_                    = 0;
bool Camera2DViewport::choose_obs_                = false;
int Camera2DViewport::choose_track_id_            = -2;
int Camera2DViewport::full_window_viewport_index_ = -1;

void Camera2DViewport::draw() {
  if (!frame_content_ || this->camera_names_.empty() || s_camera_idx_ < 0 ||
      s_camera_idx_ >= camera_names_.size()) {
    return;
  }

  std::string camera_name  = this->camera_names_[s_camera_idx_];
  auto display_opt_manager = DisplayOptionsManager::Instance();

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

  // 2. draw camera objects 2d bboxes on image
  const std::vector<camera::ObjectInfo>& camera_objects =
      frame_content_->get_camera_objects(camera_name);

  GLRasterText* raster_text = base::Singleton<GLRasterText>::get_instance();
  Eigen::Matrix4d pose_w2c =
      frame_content_->get_camera_to_world_pose(camera_name)
          .inverse();  // 外参 由发送方放在 viz_message 中

  // scale factors
  double sx = 1.0 * get_width() / image_width;
  double sy = 1.0 * get_height() / image_height;

  // draw 2d bboxes
  std::vector<int> seg_ground_index;
  for (const auto& obj : camera_objects) {
    // if (obj.camera_supplement.sensor_name != camera_name) {  //TODO fgq
    //   continue;
    // }
    algorithm::ObjectMajorType type = OBJECT_TYPE_TABLE.at(obj.type);
    auto color = OBJECT_TYPE_COLOR_TABLE.at(type);  // TODO fgq

    // auto color = OBJECT_TYPE_COLOR_TABLE.at(obj.type);

    if (display_opt_manager->get_option("show_track_color")) {
      GetObjectTrackColor(obj.track_id, &color);
    }

    // draw 2d bbox
    // const auto& bbox2d = obj.camera_supplement.box_visual; //TODO
    // Eigen::Vector2d ul_pt(bbox2d.xmin, bbox2d.ymin);
    // Eigen::Vector2d lr_pt(bbox2d.xmax, bbox2d.ymax);
    const auto& bbox2d = obj.box;
    Eigen::Vector2d ul_pt(bbox2d.left_top.x, bbox2d.left_top.y);
    Eigen::Vector2d lr_pt(bbox2d.right_bottom.x, bbox2d.right_bottom.y);
    ul_pt[0] *= sx;
    ul_pt[1] *= sy;
    lr_pt[0] *= sx;
    lr_pt[1] *= sy;

    if (choose_obs_ && xpos_ >= ul_pt[0] && xpos_ <= lr_pt[0] &&
        ypos_ >= ul_pt[1] && ypos_ <= lr_pt[1]) {
      choose_track_id_ = obj.track_id;
      choose_obs_      = false;
    }

    auto draw_3d_box = [&obj, &sx, &sy](int line_width) {
      std::vector<Eigen::Vector2d> points3d_8pts;
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[0].x * sx, obj.pts8.pts8[0].y * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[1].x * sx, obj.pts8.pts8[1].y * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[2].x * sx, obj.pts8.pts8[2].y * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[3].x * sx, obj.pts8.pts8[3].y * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[7].x * sx, obj.pts8.pts8[7].x * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[4].x * sx, obj.pts8.pts8[4].x * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[5].x * sx, obj.pts8.pts8[5].x * sy));
      points3d_8pts.push_back(
          Eigen::Vector2d(obj.pts8.pts8[6].x * sx, obj.pts8.pts8[6].x * sy));
      Draw3DBBoxProjection(
          points3d_8pts, line_width, Eigen::Vector3d(241, 158, 194),
          Eigen::Vector3d(255, 0, 0), Eigen::Vector3d(241, 158, 194));
    };
    if (choose_track_id_ == obj.track_id) {
      int line_width = 4;
      // seg_ground_index = obj.seg_ground_index; //TODO fgq
      if (display_2dbbox_) {
        DrawRect2D(ul_pt, lr_pt, line_width, Eigen::Vector3d(241, 158, 194));
      }
      if (display_3dbbox_8pts_) {
        draw_3d_box(line_width);
      }
      if (display_3dbbox_bottom_uv_) {
        std::vector<Eigen::Vector2d> points2d_bottom_center;
        points2d_bottom_center.push_back(
            Eigen::Vector2d(obj.bottom_uv.x * sx, obj.bottom_uv.y * sy));
        DrawPoints2D(
            points2d_bottom_center, line_width, Eigen::Vector3d(241, 158, 194));
      }
    } else {
      int line_width = 2;
      if (display_2dbbox_) {
        DrawRect2D(ul_pt, lr_pt, line_width, color);
      }
      if (display_3dbbox_8pts_) {
        draw_3d_box(line_width);
      }
      if (display_3dbbox_bottom_uv_) {
        std::vector<Eigen::Vector2d> points2d_bottom_center;
        points2d_bottom_center.push_back(
            Eigen::Vector2d(obj.bottom_uv.x * sx, obj.bottom_uv.y * sy));
        DrawPoints2D(
            points2d_bottom_center, line_width, Eigen::Vector3d(241, 158, 194));
      }
    }

    // distance
    Eigen::Vector3d center_in_camera_space =
        (pose_w2c *
         Eigen::Vector4d(obj.center[0], obj.center[1], obj.center[2], 1))
            .head(3);
    float distance = sqrt(
        center_in_camera_space.x() * center_in_camera_space.x() +
        center_in_camera_space.z() * center_in_camera_space.z() +
        center_in_camera_space.y() * center_in_camera_space.y());

    char dist_str_buf[512];
    snprintf(dist_str_buf, sizeof(dist_str_buf), "%.2f", distance);
    std::string dist_str(dist_str_buf);
    raster_text->DrawString(
        dist_str.c_str(), ul_pt[0], ul_pt[1] - 8, color[0], color[1], color[2]);
    // subtype name

    auto subtype_name = OBJECT_TYPE_NAME_TABLE.at(obj.type);

    raster_text->DrawString(
        subtype_name.c_str(), lr_pt[0], lr_pt[1] - 8, 0, 0, 255);
    // track id
    if (display_opt_manager->get_option("show_track_id_2d")) {
      std::string track_id_str = std::to_string(obj.track_id);

      raster_text->DrawString(
          track_id_str.c_str(), ul_pt[0],
          ul_pt[1] + (lr_pt[1] - ul_pt[1]) * 0.66, 255, 0, 0);
    }
  }

  draw_title();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void Camera2DViewport::draw_title() {
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
  std::string str_title   = "CAMERA " + camera_name;
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

REGISTER_GLVIEWPORT(Camera2DViewport);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
