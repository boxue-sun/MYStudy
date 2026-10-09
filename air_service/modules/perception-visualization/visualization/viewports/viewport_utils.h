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
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "base/blob/blob.h"

// #include "visualization/common/camera.h"
// #include "visualization/base/frame_content.h"
// #include "common/sensor_manager/sensor_manager.h"

namespace airos {
namespace perception {
namespace visualization {

#define BUFFER_OFFSET(offset) (reinterpret_cast<GLvoid *>(offset))

// rgb-color table -- RGB
const std::map<algorithm::ObjectMajorType, Eigen::Vector3d>
    OBJECT_TYPE_COLOR_TABLE = {
        {algorithm::ObjectMajorType::UNKNOWN, Eigen::Vector3d(160, 0, 255)},
        {algorithm::ObjectMajorType::UNKNOWN_MOVABLE,
         Eigen::Vector3d(0, 255, 255)},
        {algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE,
         Eigen::Vector3d(255, 255, 0)},
        {algorithm::ObjectMajorType::PEDESTRIAN,
         Eigen::Vector3d(255, 128, 128)},
        {algorithm::ObjectMajorType::BICYCLE, Eigen::Vector3d(100, 100, 255)},
        {algorithm::ObjectMajorType::VEHICLE, Eigen::Vector3d(0, 255, 0)}};

const std::map<algorithm::ObjectType, algorithm::ObjectMajorType>
    OBJECT_TYPE_TABLE = {
        {algorithm::ObjectType::UNKNOWN, algorithm::ObjectMajorType::UNKNOWN},
        {algorithm::ObjectType::UNKNOWN_MOVABLE,
         algorithm::ObjectMajorType::UNKNOWN_MOVABLE},
        {algorithm::ObjectType::UNKNOWN_UNMOVABLE,
         algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE},
        {algorithm::ObjectType::CAR, algorithm::ObjectMajorType::VEHICLE},
        {algorithm::ObjectType::VAN, algorithm::ObjectMajorType::VEHICLE},
        {algorithm::ObjectType::TRUCK, algorithm::ObjectMajorType::VEHICLE},
        {algorithm::ObjectType::BUS, algorithm::ObjectMajorType::VEHICLE},
        {algorithm::ObjectType::CYCLIST, algorithm::ObjectMajorType::BICYCLE},
        {algorithm::ObjectType::MOTORCYCLIST,
         algorithm::ObjectMajorType::BICYCLE},
        {algorithm::ObjectType::TRICYCLIST,
         algorithm::ObjectMajorType::BICYCLE},
        {algorithm::ObjectType::PEDESTRIAN,
         algorithm::ObjectMajorType::PEDESTRIAN},
        {algorithm::ObjectType::TRAFFICCONE,
         algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE},  // TODO
        {algorithm::ObjectType::SAFETY_TRIANGLE,
         algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE},
        {algorithm::ObjectType::BARRIER_DELINEATOR,
         algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE},
        {algorithm::ObjectType::BARRIER_WATER,
         algorithm::ObjectMajorType::UNKNOWN_UNMOVABLE},
        {algorithm::ObjectType::MAX_OBJECT_TYPE,
         algorithm::ObjectMajorType::MAX_OBJECT_TYPE}};

const std::map<algorithm::ObjectType, std::string> OBJECT_TYPE_NAME_TABLE = {
    {algorithm::ObjectType::UNKNOWN, "UNKNOWN"},
    {algorithm::ObjectType::UNKNOWN_MOVABLE, "UNKNOWN_MOVABLE"},
    {algorithm::ObjectType::UNKNOWN_UNMOVABLE, "UNMOVABLE"},
    {algorithm::ObjectType::CAR, "CAR"},
    {algorithm::ObjectType::VAN, "VAN"},
    {algorithm::ObjectType::TRUCK, "TRUCK"},
    {algorithm::ObjectType::BUS, "BUS"},
    {algorithm::ObjectType::CYCLIST, "CYCLIST"},
    {algorithm::ObjectType::MOTORCYCLIST, "MOTOR"},
    {algorithm::ObjectType::TRICYCLIST, "TRICYCLIST"},
    {algorithm::ObjectType::PEDESTRIAN, "PED"},
    {algorithm::ObjectType::TRAFFICCONE, "TRAFFICCONE"},
    {algorithm::ObjectType::SAFETY_TRIANGLE, "SAFETY_TRIANGLE"},
    {algorithm::ObjectType::BARRIER_DELINEATOR, "BARRIER_DELINEATOR"},
    {algorithm::ObjectType::BARRIER_WATER, "BARRIER_WATER"},
    // {algorithm::ObjectType::CONSTRUCTION_VEHICLE, "CONSTRUCTION_VEHICLE"},
    // //TODO fgq {algorithm::ObjectType::BARRIER_TEMPORARY_SIGN,
    // "BARRIER_TEMPORARY_SIGN"},
    {algorithm::ObjectType::MAX_OBJECT_TYPE, "MAX_OBJECT_TYPE"}};

const std::vector<Eigen::Vector3d> TRACKING_COLOR_LIST = {
    Eigen::Vector3d(230, 25, 75),   Eigen::Vector3d(60, 180, 75),
    Eigen::Vector3d(255, 225, 25),  Eigen::Vector3d(0, 130, 200),
    Eigen::Vector3d(70, 240, 240),  Eigen::Vector3d(240, 50, 230),
    Eigen::Vector3d(0, 128, 128),   Eigen::Vector3d(210, 245, 60),
    Eigen::Vector3d(250, 190, 190), Eigen::Vector3d(230, 190, 255),
    Eigen::Vector3d(255, 250, 200), Eigen::Vector3d(170, 255, 195),
    Eigen::Vector3d(245, 130, 48),  Eigen::Vector3d(255, 215, 180)};

enum VBO_TYPE { VERTICES, COLORS, ELEMENTS, NUM_VBOS };  // {0, 1, 2, 3}

// for setting opengl's view matrix in camera space
const Eigen::Vector4d CAMERA_CENTER       = Eigen::Vector4d(0, -150, 0, 1);
const Eigen::Vector4d CAMERA_SCENE_CENTER = Eigen::Vector4d(0, 0, 0, 1);
const Eigen::Vector4d CAMERA_FORWARD_DIR  = Eigen::Vector4d(0, 0, 1, 0);
const Eigen::Vector4d CAMERA_UP_DIR       = Eigen::Vector4d(-1, 0, 0, 0);

// for setting opengl's view matrix in novatel space
const Eigen::Vector4d NOVATEL_CENTER       = Eigen::Vector4d(0, 0, 100, 1);
const Eigen::Vector4d NOVATEL_SCENE_CENTER = Eigen::Vector4d(0, 0, 0, 1);
const Eigen::Vector4d NOVATEL_FORWARD_DIR  = Eigen::Vector4d(0, 1, 0, 0);
const Eigen::Vector4d NOVATEL_UP_DIR       = Eigen::Vector4d(-1, 0, 0, 0);

// see https://people.richland.edu/james/lecture/m170/tbl-chi.html
// 2 Dof Chi-Square, confidence 0.95
// sqrt(5.991) == 2.4477
// this value is for computing major/minor axis length of error ellipse
const double SQRT_CHISQUARE_VALUE_FOR_ERR_ELLIPSE_2D = 2.4477;

void DrawLine2D(
    const Eigen::Vector2d &p1, const Eigen::Vector2d &p2, int line_width,
    const Eigen::Vector3d &color_rgb);

void DrawLines2D(
    const std::vector<Eigen::Vector2d> &pts, int line_width,
    const Eigen::Vector3d &color_rgb);

void Draw3DBBoxProjection(
    const std::vector<Eigen::Vector2d> &points, int line_width,
    const Eigen::Vector3d &color_top, const Eigen::Vector3d &color_bottom,
    const Eigen::Vector3d &color_side);

void DrawRect2D(
    const Eigen::Vector2d &upper_left, const Eigen::Vector2d &bottom_right,
    int line_width, const Eigen::Vector3d &color_rgb);

void DrawPoints2D(
    const std::vector<Eigen::Vector2d> &points, int point_size,
    const Eigen::Vector3d &color);

GLuint ImageToGlTexture(
    const std::shared_ptr<airos::base::Blob<uint8_t>> &image_blob,
    GLenum min_filter, GLenum mag_filter, GLenum wrap_filter);

bool ProjectPoint(
    const Eigen::Matrix4d &w2c_pose, const Eigen::Matrix3f &intrinsic_params,
    const std::string &camera_name, const Eigen::Vector3d &pt3d,
    Eigen::Vector2d *pt2d);

bool ProjectPointKeepZ(
    const Eigen::Matrix4d &w2c_pose, const std::string &camera_name,
    const Eigen::Vector3d &pt3d, Eigen::Vector3d *pt2d);

inline bool GetObjectClassColor(
    const algorithm::ObjectMajorType &type, Eigen::Vector3d *color) {
  if (OBJECT_TYPE_COLOR_TABLE.find(type) == OBJECT_TYPE_COLOR_TABLE.end()) {
    return false;
  }
  *color = OBJECT_TYPE_COLOR_TABLE.at(type);
  return true;
}

inline void GetObjectTrackColor(int track_id, Eigen::Vector3d *color) {
  *color = TRACKING_COLOR_LIST.at(track_id % TRACKING_COLOR_LIST.size());
}

inline bool GetObjectClassColor(
    const algorithm::ObjectMajorType &type, int rgb[3]) {
  if (OBJECT_TYPE_COLOR_TABLE.find(type) == OBJECT_TYPE_COLOR_TABLE.end()) {
    return false;
  }
  const auto &color = OBJECT_TYPE_COLOR_TABLE.at(type);
  rgb[0]            = color[0];
  rgb[1]            = color[1];
  rgb[2]            = color[2];
  return true;
}

bool MouseIn3DBox(std::vector<Eigen::Vector2d> *points2d);

void GetObject3DBBoxPointsProjection(
    const std::string &camera_name, const Eigen::Vector3d &center,
    double length, double width, double height, double theta,
    std::vector<Eigen::Vector2d> *points);

void Get8Points(
    double width, double height, double length,
    std::vector<Eigen::Vector3d> *points);

void GetObject3DBBoxPoints(
    const Eigen::Vector3d &center, const Eigen::Vector3d &direction,
    double length, double width, double height, double theta,
    std::vector<Eigen::Vector3d> *points);

void GetObject3DBBoxPointsInScreen(
    std::vector<Eigen::Vector3d> *points3d,
    std::vector<Eigen::Vector2d> *points2d);

void DrawLine3D(
    const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, int line_width,
    const Eigen::Vector3d &color_rgb);

void DrawDashedLine3D(
    const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, int line_width,
    const Eigen::Vector3d &color_rgb);

void DrawLines3D(
    const std::vector<Eigen::Vector3d> &pts, int line_width,
    const Eigen::Vector3d &color_rgb);

// draw 3d bounding box in 3d space
void Draw3DBBox(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb);

void Draw3DBBoxDashed(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb);

void GetVelocitySrcPosition(
    const Eigen::Vector3d &center, const Eigen::Vector3d &direction,
    const Eigen::Vector3d &velocity, double length, double width, double height,
    Eigen::Vector3d *velocity_src);

void GenCloudVaoAndVbo(
    const int vao_num, const int vbo_num, GLuint vao_cloud[],
    GLuint buffers_cloud[][NUM_VBOS], GLfloat cloud_verts[][3],
    GLsizei cloud_verts_size);

void GenCloudVaoAndVbo(
    const int vao_num, const int vbo_num, GLuint vao_cloud[],
    GLuint buffers_cloud[][NUM_VBOS], GLfloat cloud_verts[][3],
    GLfloat cloud_vert_colors[][3], GLsizei cloud_verts_size);

void GenCirclesVaoAndVbo(
    const int vao_num, const int vbo_num, GLuint vao_circle[],
    const std::string &sensor_name);

void DrawCircles(int vao_circle_num, int vbo_circle_num, GLuint vao_circle[]);

void DrawObjects(
    const std::vector<camera::ObjectInfo> &objects,
    const Eigen::Vector3d &offset, bool draw_cube, bool draw_polygon,
    bool draw_velocity, bool draw_track_id, bool draw_error_ellipse,
    const Eigen::Vector3d &color, bool use_class_color,
    bool draw_cloud = false);

void DrawObjects(
    const std::vector<camera::ObjectInfo> &objects,
    const std::vector<int> &all_camera_objects_id,
    const Eigen::Vector3d &offset, bool draw_cube, bool draw_polygon,
    bool draw_velocity, bool draw_track_id, bool draw_error_ellipse,
    const Eigen::Vector3d &color, bool use_class_color,
    bool draw_cloud = false);

void Draw3DText(
    const Eigen::Vector3d &position, const Eigen::Vector3f &color,
    const std::string &text);

void Draw3DBBox(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb, double alpha);

template <typename T>
void DisplayDisparityInColor(
    unsigned char *canvas, const T *data, int width, int height,
    unsigned char bg_lum = 0, float dmin = 1.0f, float dmax = 100.0f) {
  // colorize in kitti standard format
  int num_pt  = width * height;
  float scale = 1.0f / (dmax - dmin);
  for (int i = 0; i < num_pt * 3; ++i) {
    canvas[i] = bg_lum;
  }

  // color map
  const float map[8][4] = {{0, 0, 0, 114}, {0, 0, 1, 185}, {1, 0, 0, 114},
                           {1, 0, 1, 174}, {0, 1, 0, 114}, {0, 1, 1, 185},
                           {1, 1, 0, 114}, {1, 1, 1, 0}};
  float sum             = 0;
  for (int i = 0; i < 8; i++) {
    sum += map[i][3];
  }

  float weights[8];  // relative weights
  memset(weights, 0, sizeof(float) * 8);
  float cumsum[8];  // cumulative weights
  cumsum[0] = 0;
  for (int i = 0; i < 7; i++) {
    weights[i]    = sum / map[i][3];
    cumsum[i + 1] = cumsum[i] + map[i][3] / sum;
  }

  unsigned char max_r = 0;
  unsigned char max_g = 0;
  unsigned char max_b = 0;
  unsigned char min_r = 255;
  unsigned char min_g = 255;
  unsigned char min_b = 255;

  // for all pixels do
  for (int v = 0; v < height; v++) {
    int vw = v * width;
    for (int u = 0; u < width; u++) {
      int id = 3 * (vw + u);

      if (data[vw + u] == (T)0) {
        canvas[id]     = bg_lum;
        canvas[id + 1] = bg_lum;
        canvas[id + 2] = bg_lum;
        continue;
      }

      // get normalized value
      float val = std::min(
          std::max(
              (static_cast<float>(data[v * width + u]) - dmin) * scale, 0.0f),
          1.0f);

      // find bin
      int i;
      for (i = 0; i < 6; i++) {
        if (val < cumsum[i + 1]) {
          break;
        }
      }

      // compute rgb values
      float w = 1.0f - (val - cumsum[i]) * weights[i];

      unsigned char r =
          (unsigned char)((w * map[i][0] + (1.0 - w) * map[i + 1][0]) * 255.f +
                          0.5f);
      unsigned char g =
          (unsigned char)((w * map[i][1] + (1.0 - w) * map[i + 1][1]) * 255.f +
                          0.5f);
      unsigned char b =
          (unsigned char)((w * map[i][2] + (1.0 - w) * map[i + 1][2]) * 255.f +
                          0.5f);

      // set pixel
      canvas[id]     = r;
      canvas[id + 1] = g;
      canvas[id + 2] = b;

      max_r = std::max(r, max_r);
      max_g = std::max(g, max_g);
      max_b = std::max(b, max_b);

      min_r = std::min(r, min_r);
      min_g = std::min(g, min_g);
      min_b = std::min(b, min_b);
    }
  }

  float scale_r =
      max_r > min_r
          ? 255.f / (static_cast<float>(max_r) - static_cast<float>(min_r))
          : 1.f;
  float scale_g =
      max_g > min_g
          ? 255.f / (static_cast<float>(max_g) - static_cast<float>(min_g))
          : 1.f;
  float scale_b =
      max_b > min_b
          ? 255.f / (static_cast<float>(max_b) - static_cast<float>(min_b))
          : 1.f;
  for (int v = 0; v < height; v++) {
    int vw = v * width;
    for (int u = 0; u < width; u++) {
      int id = 3 * (vw + u);
      if (canvas[id] != bg_lum && canvas[id + 1] != bg_lum &&
          canvas[id + 2] != bg_lum) {
        canvas[id] =
            (unsigned char)(static_cast<float>(canvas[id] - min_r) * scale_r);
        canvas[id + 1] =
            (unsigned char)(static_cast<float>(canvas[id + 1] - min_g) *
                            scale_g);
        canvas[id + 2] =
            (unsigned char)(static_cast<float>(canvas[id + 2] - min_b) *
                            scale_b);
      }
    }
  }
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
