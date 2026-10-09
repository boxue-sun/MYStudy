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

#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"

#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>

#include <Eigen/Eigenvalues>

#include "air_service/modules/perception-visualization/config/viz_config_manager.h"
#include "base/common/log.h"
#include "base/common/singleton.h"
// #include "common/sensor_manager/sensor_manager.h"
#include "air_service/modules/perception-visualization/visualization/base/base_visualizer.h"
#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/camera_2d_viewport.h"
#include "air_service/modules/perception-visualization/visualization/viewports/camera_proj_2d_viewport.h"

// using ::idg::perception::common::SensorManager;  //TODO fgq

namespace airos {
namespace perception {
namespace visualization {

void DrawLine2D(
    const Eigen::Vector2d &p1, const Eigen::Vector2d &p2, int line_width,
    const Eigen::Vector3d &color_rgb) {
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINES);
  glVertex2i(p1[0], p1[1]);
  glVertex2i(p2[0], p2[1]);
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void DrawLines2D(
    const std::vector<Eigen::Vector2d> &pts, int line_width,
    const Eigen::Vector3d &color_rgb) {
  if (pts.size() < 2) {
    return;
  }
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINES);
  const int pts_size = pts.size();
  for (int i = 1; i < pts_size; ++i) {
    glVertex2i(pts[i - 1][0], pts[i - 1][1]);
    glVertex2i(pts[i][0], pts[i][1]);
  }
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void Draw3DBBoxProjection(
    const std::vector<Eigen::Vector2d> &points, int line_width,
    const Eigen::Vector3d &color_top, const Eigen::Vector3d &color_bottom,
    const Eigen::Vector3d &color_side) {
  assert(points.size() == 8);

  DrawLine2D(points[0], points[1], line_width, color_top);
  DrawLine2D(points[1], points[2], line_width, color_top);
  DrawLine2D(points[2], points[3], line_width, color_top);
  DrawLine2D(points[3], points[0], line_width, color_top);

  DrawLine2D(points[4], points[5], line_width, color_bottom);
  DrawLine2D(points[5], points[6], line_width, color_bottom);
  DrawLine2D(points[6], points[7], line_width, color_bottom);
  DrawLine2D(points[7], points[4], line_width, color_bottom);

  DrawLine2D(points[4], points[3], line_width, color_side);
  DrawLine2D(points[2], points[7], line_width, color_side);
  DrawLine2D(points[1], points[6], line_width, color_side);
  DrawLine2D(points[5], points[0], line_width, color_side);
}

void DrawRect2D(
    const Eigen::Vector2d &upper_left, const Eigen::Vector2d &bottom_right,
    int line_width, const Eigen::Vector3d &color_rgb) {
  const double &x1 = upper_left[0];
  const double &y1 = upper_left[1];
  const double &x2 = bottom_right[0];
  const double &y2 = bottom_right[1];

  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINES);
  glVertex2i(x1, y1);
  glVertex2i(x1, y2);

  glVertex2i(x1, y2);
  glVertex2i(x2, y2);

  glVertex2i(x2, y2);
  glVertex2i(x2, y1);

  glVertex2i(x2, y1);
  glVertex2i(x1, y1);
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void DrawPoints2D(
    const std::vector<Eigen::Vector2d> &points, int point_size,
    const Eigen::Vector3d &color_rgb) {
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glPointSize(point_size);

  glBegin(GL_POINTS);
  for (const auto &pt : points) {
    glVertex2i(pt[0], pt[1]);
  }
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glPointSize(1);
}

GLuint ImageToGlTexture(
    const std::shared_ptr<airos::base::Blob<uint8_t>> &image_blob,
    GLenum min_filter, GLenum mag_filter, GLenum wrap_filter) {
  // TODO(guiyilin): use Image8U instead of Blob
  int img_height      = 0;
  int img_width       = 0;
  int img_channel_num = 0;
  if (image_blob->num_axes() == 3) {
    img_height      = image_blob->shape(1);
    img_width       = image_blob->shape(0);
    img_channel_num = image_blob->shape(2);
  } else if (image_blob->num_axes() == 4) {
    img_height      = image_blob->shape(1);
    img_width       = image_blob->shape(2);
    img_channel_num = image_blob->shape(3);
  } else {
    LOG_ERROR << "unsupported image_blob->num_axes: " << image_blob->num_axes();
    return 0;
  }

  // Generate a number for our texture_id's unique handle
  GLuint texture_id;
  glGenTextures(1, &texture_id);

  // Bind to our texture handle
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // Catch silly-mistake texture interpolation method for magnification
  if (mag_filter == GL_LINEAR_MIPMAP_LINEAR ||
      mag_filter == GL_LINEAR_MIPMAP_NEAREST ||
      mag_filter == GL_NEAREST_MIPMAP_LINEAR ||
      mag_filter == GL_NEAREST_MIPMAP_NEAREST) {
    // cout << "You can't use MIPMAPs for magnification - setting filter to
    // GL_LINEAR" << endl;
    mag_filter = GL_LINEAR;
  }

  // Set texture interpolation methods for minification and magnification
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

  // Set texture clamping method
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_filter);

  // Set incoming texture format
  GLenum input_color_format = GL_BGR;
  if (img_channel_num == 1) {
    input_color_format = GL_LUMINANCE;
  } else if (img_channel_num == 4) {
    input_color_format = GL_BGRA;
  } else {
    // Unknown color format
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Create the texture
  glTexImage2D(
      GL_TEXTURE_2D,            // Type of texture
      0,                        // Pyramid level (for mip-mapping) - 0 is
                                // the top level
      GL_RGB,                   // Internal colour format to convert to
      img_width,                // Image width  i.e. 640 for Kinect in
                                // standard mode
      img_height,               // Image height i.e. 480 for Kinect in
                                // standard mode
      0,                        // Border width in pixels (can either be
                                // 1 or 0)
      input_color_format,       // Input image format (i.e. GL_RGB,
                                // GL_RGBA, GL_BGR etc.)
      GL_UNSIGNED_BYTE,         // Image data type
      image_blob->cpu_data());  // The actual image data itself

  // If we're using mipmaps then generate them. Note: This requires OpenGL
  // 3.0 or higher
  if (min_filter == GL_LINEAR_MIPMAP_LINEAR ||
      min_filter == GL_LINEAR_MIPMAP_NEAREST ||
      min_filter == GL_NEAREST_MIPMAP_LINEAR ||
      min_filter == GL_NEAREST_MIPMAP_NEAREST) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  return texture_id;
}

static Eigen::Vector2d Project(
    const Eigen::Vector3d &point3d, const Eigen::Matrix3f &intrinsic_params) {
  Eigen::Vector2d pt2d;

  pt2d[0] =
      point3d[0] / point3d[2] * intrinsic_params(0, 0) + intrinsic_params(0, 2);
  pt2d[1] =
      point3d[1] / point3d[2] * intrinsic_params(1, 1) + intrinsic_params(1, 2);

  return pt2d;
}

bool ProjectPoint(
    const Eigen::Matrix4d &w2c_pose, const Eigen::Matrix3f &intrinsic_params,
    const std::string &camera_name, const Eigen::Vector3d &pt3d,
    Eigen::Vector2d *pt2d) {
  // transform pt3d from world coord. to camera coord.
  Eigen::Vector3d pt3d_cam =
      (w2c_pose * Eigen::Vector4d(pt3d[0], pt3d[1], pt3d[2], 1)).head(3);
  // behind camera
  if (pt3d_cam[2] < 0) {
    return false;
  }

  *pt2d = Project(pt3d_cam, intrinsic_params);

  return true;
}

void Get8Points(
    double width, double height, double length,
    std::vector<Eigen::Vector3d> *points) {
  points->clear();
  points->push_back(Eigen::Vector3d(-width / 2.0, 0, length / 2.0));
  points->push_back(Eigen::Vector3d(width / 2.0, 0, length / 2.0));
  points->push_back(Eigen::Vector3d(width / 2.0, 0, -length / 2.0));
  points->push_back(Eigen::Vector3d(-width / 2.0, 0, -length / 2.0));
  points->push_back(Eigen::Vector3d(-width / 2.0, -height, length / 2.0));
  points->push_back(Eigen::Vector3d(width / 2.0, -height, length / 2.0));
  points->push_back(Eigen::Vector3d(width / 2.0, -height, -length / 2.0));
  points->push_back(Eigen::Vector3d(-width / 2.0, -height, -length / 2.0));
}

void GetObject3DBBoxPoints(
    const Eigen::Vector3d &center, const Eigen::Vector3d &direction,
    double length, double width, double height, double theta,
    std::vector<Eigen::Vector3d> *points) {
  points->resize(8);

  double x1 = length / 2;
  double x2 = -x1;
  double y1 = width / 2;
  double y2 = -y1;

  double len = sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
  double cos_theta = direction[0] / len;
  double sin_theta = -direction[1] / len;

  /*
      1  ________ 2
       /|       /|
    0 /________/3|
      | |      | |
      |6|_____ |_| 7
      |/_______|/
     5          4
  */

  // set x y
  (*points)[0][0] = x1 * cos_theta + y1 * sin_theta + center[0];
  (*points)[5][0] = (*points)[0][0];
  (*points)[0][1] = y1 * cos_theta - x1 * sin_theta + center[1];
  (*points)[5][1] = (*points)[0][1];

  (*points)[3][0] = x1 * cos_theta + y2 * sin_theta + center[0];
  (*points)[4][0] = (*points)[3][0];
  (*points)[3][1] = y2 * cos_theta - x1 * sin_theta + center[1];
  (*points)[4][1] = (*points)[3][1];

  (*points)[1][0] = x2 * cos_theta + y1 * sin_theta + center[0];
  (*points)[6][0] = (*points)[1][0];
  (*points)[1][1] = y1 * cos_theta - x2 * sin_theta + center[1];
  (*points)[6][1] = (*points)[1][1];

  (*points)[2][0] = x2 * cos_theta + y2 * sin_theta + center[0];
  (*points)[7][0] = (*points)[2][0];
  (*points)[2][1] = y2 * cos_theta - x2 * sin_theta + center[1];
  (*points)[7][1] = (*points)[2][1];

  // set z
  (*points)[0][2] = center[2] + height / 2;
  (*points)[1][2] = (*points)[0][2];
  (*points)[2][2] = (*points)[0][2];
  (*points)[3][2] = (*points)[0][2];

  (*points)[4][2] = center[2] - height / 2;
  (*points)[5][2] = (*points)[4][2];
  (*points)[6][2] = (*points)[4][2];
  (*points)[7][2] = (*points)[4][2];
}

void GetObject3DBBoxPointsInScreen(
    std::vector<Eigen::Vector3d> *points3d,
    std::vector<Eigen::Vector2d> *points2d) {
  points2d->resize(8);

  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int offset_x     = viewport[0];
  int offset_y     = viewport[1];
  int image_width  = viewport[2];
  int image_height = viewport[3];

  GLdouble mv_mat[16];
  GLdouble proj_mat[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, mv_mat);
  glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);
  for (int i = 0; i < points3d->size(); ++i) {
    Eigen::Vector4d eye_pos;
    eye_pos[0] = mv_mat[0] * (*points3d)[i][0] + mv_mat[4] * (*points3d)[i][1] +
                 mv_mat[8] * (*points3d)[i][2] + mv_mat[12];
    eye_pos[1] = mv_mat[1] * (*points3d)[i][0] + mv_mat[5] * (*points3d)[i][1] +
                 mv_mat[9] * (*points3d)[i][2] + mv_mat[13];
    eye_pos[2] = mv_mat[2] * (*points3d)[i][0] + mv_mat[6] * (*points3d)[i][1] +
                 mv_mat[10] * (*points3d)[i][2] + mv_mat[14];
    eye_pos[3] = mv_mat[3] * (*points3d)[i][0] + mv_mat[7] * (*points3d)[i][1] +
                 mv_mat[11] * (*points3d)[i][2] + mv_mat[15];

    Eigen::Vector4d clip_pos;
    clip_pos[0] = proj_mat[0] * eye_pos[0] + proj_mat[4] * eye_pos[1] +
                  proj_mat[8] * eye_pos[2] + proj_mat[12] * eye_pos[3];
    clip_pos[1] = proj_mat[1] * eye_pos[0] + proj_mat[5] * eye_pos[1] +
                  proj_mat[9] * eye_pos[2] + proj_mat[13] * eye_pos[3];
    clip_pos[2] = proj_mat[2] * eye_pos[0] + proj_mat[6] * eye_pos[1] +
                  proj_mat[10] * eye_pos[2] + proj_mat[14] * eye_pos[3];
    clip_pos[3] = proj_mat[3] * eye_pos[0] + proj_mat[7] * eye_pos[1] +
                  proj_mat[11] * eye_pos[2] + proj_mat[15] * eye_pos[3];

    Eigen::Vector3d ndc_pos;
    ndc_pos[0] = clip_pos[0] / clip_pos[3];
    ndc_pos[1] = clip_pos[1] / clip_pos[3];
    ndc_pos[2] = clip_pos[2] / clip_pos[3];

    (*points2d)[i][0] = offset_x + image_width * 0.5 * (1 + ndc_pos[0]);
    (*points2d)[i][1] = offset_y + image_height * 0.5 * (1 - ndc_pos[1]);

    glMatrixMode(GL_PROJECTION);  // Operate on projection matrix
    glPushMatrix();
    glLoadIdentity();
    glOrtho(
        offset_x, offset_x + image_width, offset_y + image_height, offset_y,
        0.0, 100.0);
    glMatrixMode(GL_MODELVIEW);  // Operate on model-view matrix
    glPushMatrix();
    glLoadIdentity();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
}

void DrawLine3D(
    const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, int line_width,
    const Eigen::Vector3d &color_rgb) {
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINES);
  glVertex3d(p1[0], p1[1], p1[2]);
  glVertex3d(p2[0], p2[1], p2[2]);
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void DrawDashedLine3D(
    const Eigen::Vector3d &p1, const Eigen::Vector3d &p2, int line_width,
    const Eigen::Vector3d &color_rgb) {
  glPushAttrib(GL_ENABLE_BIT);
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glLineStipple(1, 0x8888);  // 1000 1000 1000 1000
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINES);
  glVertex3d(p1[0], p1[1], p1[2]);
  glVertex3d(p2[0], p2[1], p2[2]);
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
  glPopAttrib();
}

void DrawLines3D(
    const std::vector<Eigen::Vector3d> &pts, int line_width,
    const Eigen::Vector3d &color_rgb) {
  if (pts.size() < 2) {
    return;
  }
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINES);
  const int pts_size = pts.size();
  for (int i = 1; i < pts_size; ++i) {
    glVertex3d(pts[i - 1][0], pts[i - 1][1], pts[i - 1][2]);
    glVertex3d(pts[i][0], pts[i][1], pts[i][2]);
  }
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void Draw3DBBoxDashed(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb) {
  assert(points.size() == 8);

  int indices[16] = {0, 1, 2, 3, 4, 5, 6, 7, 4, 3, 0, 5, 6, 1, 2, 7};
  glPushAttrib(GL_ENABLE_BIT);
  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glLineStipple(1, 0x8888);  // 1000 1000 1000 1000
  glEnable(GL_LINE_STIPPLE);

  glBegin(GL_LINE_STRIP);
  int j = 0;
  for (j = 0; j < 16; j++) {
    glVertex3d(
        points[indices[j]][0], points[indices[j]][1], points[indices[j]][2]);
  }
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
  glPopAttrib();
}

void Draw3DBBox(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb) {
  assert(points.size() == 8);

  int indices[16] = {0, 1, 2, 3, 4, 5, 6, 7, 4, 3, 0, 5, 6, 1, 2, 7};

  glColor3ub(color_rgb[0], color_rgb[1], color_rgb[2]);
  glLineWidth(line_width);

  glBegin(GL_LINE_STRIP);
  int j = 0;
  for (j = 0; j < 16; j++) {
    glVertex3d(
        points[indices[j]][0], points[indices[j]][1], points[indices[j]][2]);
  }
  glEnd();

  glColor3ub(255, 255, 255);  // reset the color to white
  glLineWidth(1);
}

void Draw3DBBox(
    const std::vector<Eigen::Vector3d> &points, int line_width,
    const Eigen::Vector3d &color_rgb, double alpha) {
  assert(points.size() == 8);

  int indices[24] = {0, 1, 2, 3, 4, 5, 6, 7, 4, 3, 0, 5,
                     6, 1, 2, 7, 2, 3, 4, 7, 0, 1, 6, 5};

  glColor4f(
      color_rgb[0] / 255.f, color_rgb[1] / 255.f, color_rgb[2] / 255.f, alpha);
  glLineWidth(line_width);

  glBegin(GL_QUADS);
  int j = 0;
  for (j = 0; j < 24; j++) {
    glVertex3d(
        points[indices[j]][0], points[indices[j]][1], points[indices[j]][2]);
  }
  glEnd();

  glColor4f(1.f, 1.f, 1.f, 1.f);
  glLineWidth(1);
}

void GetVelocitySrcPosition(
    const Eigen::Vector3d &center, const Eigen::Vector3d &direction,
    const Eigen::Vector3d &velocity, double length, double width, double height,
    Eigen::Vector3d *velocity_src) {
  std::vector<Eigen::Vector2d> bbox_2d_vertices(4);
  double x1  = length / 2;
  double x2  = -x1;
  double y1  = width / 2;
  double y2  = -y1;
  double len = sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
  double cos_theta = direction[0] / len;
  double sin_theta = -direction[1] / len;

  bbox_2d_vertices[0][0] = x1 * cos_theta + y1 * sin_theta + center[0];
  bbox_2d_vertices[0][1] = y1 * cos_theta - x1 * sin_theta + center[1];

  bbox_2d_vertices[1][0] = x2 * cos_theta + y1 * sin_theta + center[0];
  bbox_2d_vertices[1][1] = y1 * cos_theta - x2 * sin_theta + center[1];

  bbox_2d_vertices[2][0] = x2 * cos_theta + y2 * sin_theta + center[0];
  bbox_2d_vertices[2][1] = y2 * cos_theta - x2 * sin_theta + center[1];

  bbox_2d_vertices[3][0] = x1 * cos_theta + y2 * sin_theta + center[0];
  bbox_2d_vertices[3][1] = y2 * cos_theta - x1 * sin_theta + center[1];

  std::vector<Eigen::Vector2d> midpoints_of_bbox_2d_edges(4);
  for (int i = 0; i < 4; ++i) {
    midpoints_of_bbox_2d_edges[i] =
        0.5 * (bbox_2d_vertices[i] + bbox_2d_vertices[(i + 1) % 4]);
  }

  double max_cos_val = std::numeric_limits<double>::min();
  Eigen::Vector2d min_angle_midpoint;  // min angle <==> max e
  for (int i = 0; i < 4; ++i) {
    Eigen::Vector2d line_center_to_midpoint =
        midpoints_of_bbox_2d_edges[i] - center.head(2);
    double cos_val =
        line_center_to_midpoint.normalized().dot(velocity.head(2).normalized());
    if (cos_val > max_cos_val) {
      min_angle_midpoint = midpoints_of_bbox_2d_edges[i];
      max_cos_val        = cos_val;
    }
  }
  (*velocity_src)[0] = min_angle_midpoint[0];
  (*velocity_src)[1] = min_angle_midpoint[1];
  (*velocity_src)[2] = center[2] - 0.5 * height;
}

void GenCloudVaoAndVbo(
    int vao_num, int kVboNum, GLuint vao_cloud[],
    GLuint buffers_cloud[][VBO_TYPE::NUM_VBOS], GLfloat cloud_verts[][3],
    GLsizei cloud_verts_size) {
  GLfloat cloud_colors[kVboNum][3];
  GLuint cloud_indices[kVboNum];
  for (int i = 0; i < kVboNum; ++i) {
    cloud_colors[i][0] = 0.7;
    cloud_colors[i][1] = 0.7;
    cloud_colors[i][2] = 0.7;
    cloud_indices[i]   = (GLuint)i;
  }

  glGenVertexArrays(vao_num, vao_cloud);
  for (int i = 0; i < vao_num; ++i) {
    glBindVertexArray(vao_cloud[i]);
    // change buffer object data
    glGenBuffers(VBO_TYPE::NUM_VBOS, buffers_cloud[i]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::VERTICES]);
    glBufferData(
        GL_ARRAY_BUFFER, cloud_verts_size, cloud_verts, GL_STREAM_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::COLORS]);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(cloud_colors), cloud_colors, GL_STREAM_DRAW);
    glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::ELEMENTS]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(cloud_indices), cloud_indices,
        GL_STREAM_DRAW);
  }
}

// support per-point color setting
void GenCloudVaoAndVbo(
    int vao_num, int kVboNum, GLuint vao_cloud[],
    GLuint buffers_cloud[][VBO_TYPE::NUM_VBOS], GLfloat cloud_verts[][3],
    GLfloat cloud_vert_colors[][3], GLsizei cloud_verts_size) {
  GLuint cloud_indices[kVboNum];
  for (int i = 0; i < kVboNum; ++i) {
    cloud_vert_colors[i][0] = 0.7;
    cloud_vert_colors[i][1] = 0.7;
    cloud_vert_colors[i][2] = 0.7;
    cloud_indices[i]        = (GLuint)i;
  }

  glGenVertexArrays(vao_num, vao_cloud);
  for (int i = 0; i < vao_num; ++i) {
    glBindVertexArray(vao_cloud[i]);
    // change buffer object data
    glGenBuffers(VBO_TYPE::NUM_VBOS, buffers_cloud[i]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::VERTICES]);
    glBufferData(
        GL_ARRAY_BUFFER, cloud_verts_size, cloud_verts, GL_STREAM_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::COLORS]);
    glBufferData(
        GL_ARRAY_BUFFER, cloud_verts_size, cloud_vert_colors, GL_STREAM_DRAW);
    glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers_cloud[i][VBO_TYPE::ELEMENTS]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(cloud_indices), cloud_indices,
        GL_STREAM_DRAW);
  }
}

void PointLabel2Rgb(uint8_t label, GLfloat rgb[]) {
  switch (label) {
    case 0:  // unknown
      rgb[0] = rgb[1] = rgb[2] = 0.7f;
      break;
    case 1:  // roi
      rgb[0] = 1.0f, rgb[1] = 0, rgb[2] = 0.0f;
      break;
    case 2:  // ground
      rgb[0] = 0, rgb[1] = 1.0f, rgb[2] = 0.0f;
      break;
    case 3:  // object
      rgb[0] = 0.0f, rgb[1] = 0.0f, rgb[2] = 1.0f;
      break;
    default:
      rgb[0] = rgb[1] = rgb[2] = 0.7f;
      break;
  }
}

void GenCirclesVaoAndVbo(
    const int vao_num, const int kVBONum, GLuint vao_circle[],
    const std::string &sensor_name) {
  GLuint buffers_circle[vao_num][VBO_TYPE::NUM_VBOS];
  GLfloat circle_verts[kVBONum][3];
  GLfloat circle_colors[kVBONum][3];
  GLuint circle_indices[kVBONum];
  // SensorManager *sensor_manager =
  // lib::Singleton<SensorManager>::get_instance(); bool is_camera =
  // sensor_manager->IsCamera(sensor_name);
  bool is_camera = true;
  float d_theta  = 2 * 3.1415926f / static_cast<float>(kVBONum);
  int i          = 0;
  int vao_idx    = 0;
  for (i = 0; i < kVBONum; ++i) {
    if (is_camera) {
      circle_verts[i][1] = 1.7;  // camera ground, y = 1.7
    } else {
      circle_verts[i][2] = -1.7;  // lidar/novatel ground, z = -1.7
    }

    circle_colors[i][0] = 0.0;
    circle_colors[i][1] = 0.7;
    circle_colors[i][2] = 0.7;

    circle_indices[i] = (GLuint)i;
  }

  glGenVertexArrays(vao_num, vao_circle);

  auto circle_radius_inc = base::Singleton<VizConfigManager>::get_instance()
                               ->GetVizConfig()
                               .ref_circle_radius_inc();
  for (vao_idx = 0; vao_idx < vao_num; ++vao_idx) {
    for (i = 0; i < kVBONum; i++) {
      float theta        = static_cast<float>(i) * d_theta;
      circle_verts[i][0] = circle_radius_inc * (vao_idx + 1) * cos(theta);
      if (is_camera) {
        circle_verts[i][2] = circle_radius_inc * (vao_idx + 1) * sin(theta);
      } else {
        circle_verts[i][1] = circle_radius_inc * (vao_idx + 1) * sin(theta);
      }
    }
    glBindVertexArray(vao_circle[vao_idx]);
    glGenBuffers(VBO_TYPE::NUM_VBOS, buffers_circle[vao_idx]);
    glBindBuffer(GL_ARRAY_BUFFER, buffers_circle[vao_idx][VBO_TYPE::VERTICES]);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(circle_verts), circle_verts, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, buffers_circle[vao_idx][VBO_TYPE::COLORS]);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(circle_colors), circle_colors, GL_STATIC_DRAW);
    glColorPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER, buffers_circle[vao_idx][VBO_TYPE::ELEMENTS]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(circle_indices), circle_indices,
        GL_STATIC_DRAW);
  }
}

void DrawCircles(int vao_circle_num, int vbo_circle_num, GLuint vao_circle[]) {
  for (int vao_idx = 0; vao_idx < vao_circle_num; ++vao_idx) {
    glBindVertexArray(vao_circle[vao_idx]);
    glDrawElements(
        GL_LINE_LOOP, vbo_circle_num, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
    glBindVertexArray(0);
  }
}

// void DrawObjects(const std::vector<camera::ObjectInfo> &objects,
//                  const Eigen::Vector3d &offset, bool draw_cube,
//                  bool draw_polygon, bool draw_velocity, bool draw_track_id,
//                  bool draw_error_ellipse, const Eigen::Vector3d &color,
//                  bool use_class_color, bool draw_cloud) {
//   std::vector<camera::ObjectInfo> objects_after_offset(objects.size());
//   std::transform(
//       objects.begin(), objects.end(), objects_after_offset.begin(),
//       [&offset](camera::ObjectInfo object) {
//         // base::Object object = *obj_cptr;
//         object.center(0) += offset(0);
//         object.center(1) += offset(1);
//         object.center(2) += offset(2);
//         const int cloud_size = object.lidar_supplement.cloud_world.size();
//         for (int i = 0; i < cloud_size; ++i) {
//           base::PointD &pc = object.lidar_supplement.cloud_world.at(i);
//           pc.x += offset(0);
//           pc.y += offset(1);
//           pc.z += offset(2);
//         }
//         const int polygon_pt_size = object.polygon.size();
//         for (int i = 0; i < polygon_pt_size; ++i) {
//           base::PointD &pc = object.polygon.at(i);
//           pc.x += offset(0);
//           pc.y += offset(1);
//           pc.z += offset(2);
//         }
//         return std::move(object);
//       });

//   auto display_opt_manager = DisplayOptionsManager::Instance();
//   if (draw_cube) {
//     for (const auto &obj : objects_after_offset) {
//       std::vector<Eigen::Vector3d> points_3d_bbox;
//       GetObject3DBBoxPoints(obj.center, obj.direction.cast<double>(),
//                             obj.size[0], obj.size[1], obj.size[2], obj.theta,
//                             &points_3d_bbox);
//       // if (obj.camera_supplement != nullptr &&
//       //   obj.camera_supplement->is_cipv) {
//       //   Eigen::Vector3d color_cipv(255, 255, 255);
//       //   Draw3DBBox(points_3d_bbox, 1, color_cipv);
//       //   continue;
//       // }
//       if (use_class_color) {
//         Eigen::Vector3d color_cls;
//         GetObjectClassColor(obj.type, &color_cls);  ////TODO fgq type_id
//         if (display_opt_manager->get_option("show_track_color")) {
//           GetObjectTrackColor(obj.track_id, &color_cls);
//         }
//         Draw3DBBox(points_3d_bbox, 1, color_cls);
//         DrawDashedLine3D(
//             obj.center,
//             obj.center + obj.direction.cast<double>() * 0.5 * obj.size[0], 1,
//             color_cls);
//       } else {
//         Draw3DBBox(points_3d_bbox, 1, color);
//         DrawDashedLine3D(
//             obj.center,
//             obj.center + obj.direction.cast<double>() * 0.5 * obj.size[0], 1,
//             color);
//       }
//     }
//   }

//   // if (draw_error_ellipse) {
//   //   Eigen::Vector3d color_ellipse(255, 102, 255);
//   //   for (const auto& obj : objects) {
//   //     DrawErrorEllipse2D(obj, SQRT_CHISQUARE_VALUE_FOR_ERR_ELLIPSE_2D,
//   //     color_ellipse);
//   //   }
//   // }

//   if (draw_track_id) {
//     for (int i = 0; i < objects_after_offset.size(); ++i) {
//       Eigen::Vector3d pt(0, 0, 0);
//       if (!draw_polygon) {   // TODO fgq
//         pt = objects_after_offset[i].center;  // bbox center
//       }
//       // else {
//       //   // polygon center
//       //   for (int j = 0; j < objects_after_offset[i].polygon.size(); ++j) {
//       //     auto &polygon_pt = objects_after_offset[i].polygon.at(j);
//       //     pt[0] += polygon_pt.x;
//       //     pt[1] += polygon_pt.y;
//       //   }
//       //   pt[0] /= objects_after_offset[i].polygon.size();
//       //   pt[1] /= objects_after_offset[i].polygon.size();
//       //   pt[2] = objects_after_offset[i].center[2];
//       // }

//       Eigen::Vector3f color(1, 0, 0);
//       std::stringstream ss;
//       ss << objects_after_offset[i].track_id;
//       Draw3DText(pt, color, ss.str());
//     }
//   }

//   // if (draw_polygon) {
//   //   int i = 0;
//   //   int j = 0;
//   //   int rgb[3] = {0, 0, 0};
//   //   for (i = 0; i < objects_after_offset.size(); ++i) {
//   //     if (use_class_color) {
//   //       Eigen::Vector3d color_cls;
//   //       GetObjectClassColor(objects_after_offset[i].type, &color_cls);
//   //       if (display_opt_manager->get_option("show_track_color")) {
//   //         GetObjectTrackColor(objects_after_offset[i].track_id,
//   &color_cls);
//   //       }
//   //       rgb[0] = color_cls[0];
//   //       rgb[1] = color_cls[1];
//   //       rgb[2] = color_cls[2];
//   //     } else {
//   //       rgb[0] = color[0];
//   //       rgb[1] = color[1];
//   //       rgb[2] = color[2];
//   //     }

//   //     glColor3ub(rgb[0], rgb[1], rgb[2]);
//   //     glBegin(GL_LINE_LOOP);

//   //     for (j = 0; j < objects_after_offset[i].polygon.size(); ++j) {
//   //       auto &pt = objects_after_offset[i].polygon.at(j);
//   //       glVertex3f((GLfloat)pt.x, (GLfloat)pt.y,
//   //                  objects_after_offset[i].center[2]);
//   //     }
//   //     glEnd();
//   //     glFlush();
//   //   }
//   // }

//   // if (draw_cloud) {
//   //   Eigen::Vector3d cloud_color(1, 0, 0);
//   //   glBegin(GL_POINTS);
//   //   for (size_t i = 0; i < objects_after_offset.size(); i++) {
//   //     if (use_class_color) {
//   //       GetObjectClassColor(objects_after_offset[i].type, &cloud_color);
//   //     }
//   //     if (display_opt_manager->get_option("show_track_color")) {
//   //       GetObjectTrackColor(objects_after_offset[i].track_id,
//   &cloud_color);
//   //     }
//   //     glColor3f(cloud_color[0], cloud_color[1], cloud_color[2]);
//   //     const base::PointDCloud &obj_cloud =
//   //         objects_after_offset[i].lidar_supplement.cloud_world;
//   //     for (size_t j = 0; j < obj_cloud.size(); j++) {
//   //       auto &pt = obj_cloud.at(j);
//   //       glVertex3d(pt.x, pt.y, pt.z);
//   //     }
//   //   }
//   //   glEnd();
//   // }

//   if (draw_velocity) {
//     int i = 0;
//     Eigen::Vector3d velocity_src;
//     Eigen::Vector3d velocity_dst;
//     int rgb[3] = {1, 1, 0};
//     for (i = 0; i < objects_after_offset.size(); ++i) {
//       if (use_class_color) {
//         Eigen::Vector3d color_cls;
//         GetObjectClassColor(objects_after_offset[i].type, &color_cls);
//         if (display_opt_manager->get_option("show_track_color")) {
//           GetObjectTrackColor(objects_after_offset[i].track_id, &color_cls);
//         }
//         rgb[0] = color_cls[0];
//         rgb[1] = color_cls[1];
//         rgb[2] = color_cls[2];
//       } else {
//         rgb[0] = color[0];
//         rgb[1] = color[1];
//         rgb[2] = color[2];
//       }

//       GetVelocitySrcPosition(objects_after_offset[i].center,
//                              objects_after_offset[i].direction.cast<double>(),
//                              objects_after_offset[i].velocity.cast<double>(),
//                              objects_after_offset[i].size[0],
//                              objects_after_offset[i].size[1],
//                              objects_after_offset[i].size[2], &velocity_src);
//       velocity_dst[0] = velocity_src[0] +
//       objects_after_offset[i].velocity[0]; velocity_dst[1] = velocity_src[1]
//       + objects_after_offset[i].velocity[1]; velocity_dst[2] =
//       velocity_src[2];

//       Eigen::Vector3d vel_color(rgb[0], rgb[1], rgb[2]);
//       DrawLine3D(velocity_src, velocity_dst, 1, vel_color);
//     }
//   }

//   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
// }

bool MouseIn3DBox(std::vector<Eigen::Vector2d> *points2d) {
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int offset_x     = viewport[0];
  int offset_y     = viewport[1];
  int image_width  = viewport[2];
  int image_height = viewport[3];

  double x_max = -1000000.0, y_max = -1000000.0, x_min = 1000000.0,
         y_min = 1000000.0;
  for (int i = 0; i < points2d->size(); ++i) {
    x_max = (x_max > (*points2d)[i][0]) ? x_max : (*points2d)[i][0];
    y_max = (y_max > (*points2d)[i][1]) ? y_max : (*points2d)[i][1];
    x_min = (x_min < (*points2d)[i][0]) ? x_min : (*points2d)[i][0];
    y_min = (y_min < (*points2d)[i][1]) ? y_min : (*points2d)[i][1];
  }

  if (Camera2DViewport::full_window_viewport_index_ == -1) {
    y_max += image_height;
    y_min += image_height;
  }

  return Camera2DViewport::xpos_ >= x_min && Camera2DViewport::xpos_ <= x_max &&
         Camera2DViewport::ypos_ >= y_min && Camera2DViewport::ypos_ <= y_max;
}

void DrawObjects(
    const std::vector<camera::ObjectInfo> &objects,
    const std::vector<int> &all_camera_objects_id,
    const Eigen::Vector3d &offset, bool draw_cube, bool draw_polygon,
    bool draw_velocity, bool draw_track_id, bool draw_error_ellipse,
    const Eigen::Vector3d &color, bool use_class_color, bool draw_cloud) {
  std::vector<camera::ObjectInfo> objects_after_offset(objects.size());
  std::transform(
      objects.begin(), objects.end(), objects_after_offset.begin(),
      [&offset](camera::ObjectInfo object) {
        object.center(0) += offset(0);
        object.center(1) += offset(1);
        object.center(2) += offset(2);
        return std::move(object);
      });
  auto display_opt_manager = DisplayOptionsManager::Instance();
  int count                = 0;
  if (draw_cube) {
    for (const auto &obj : objects_after_offset) {
      std::vector<Eigen::Vector3d> points_3d_bbox;
      Eigen::Vector3d direction(
          obj.direction.x, obj.direction.y, obj.direction.z);

      GetObject3DBBoxPoints(
          obj.center, direction, obj.size.length, obj.size.width,
          obj.size.height, obj.theta, &points_3d_bbox);

      Eigen::Vector3d color_cx;
      if (all_camera_objects_id[count] == 0 ||
          all_camera_objects_id[count] == 1) {
        Eigen::Vector3d color_c(255, 255, 255);
        color_cx = color_c;
      } else if (
          all_camera_objects_id[count] == 2 ||
          all_camera_objects_id[count] == 3) {
        Eigen::Vector3d color_c(0, 0, 255);
        color_cx = color_c;
      } else if (
          all_camera_objects_id[count] == 4 ||
          all_camera_objects_id[count] == 5) {
        Eigen::Vector3d color_c(0, 255, 0);
        color_cx = color_c;
      } else if (
          all_camera_objects_id[count] == 6 ||
          all_camera_objects_id[count] == 7) {
        Eigen::Vector3d color_c(255, 0, 0);
        color_cx = color_c;
      } else {
        Eigen::Vector3d color_c(255, 255, 0);
        color_cx = color_c;
      }
      count++;

      // get track car
      std::vector<Eigen::Vector2d> points_3d_bbox_screen;
      GetObject3DBBoxPointsInScreen(&points_3d_bbox, &points_3d_bbox_screen);
      if (Camera2DViewport::choose_obs_ &&
          MouseIn3DBox(&points_3d_bbox_screen)) {
        Camera2DViewport::choose_track_id_ = obj.track_id;
        Camera2DViewport::choose_obs_      = false;

        Camera2DViewport::s_camera_idx_     = all_camera_objects_id[count - 1];
        CameraProj2DViewport::s_camera_idx_ = all_camera_objects_id[count - 1];
      }

      // draw track car and other cars
      if (Camera2DViewport::choose_track_id_ == obj.track_id) {
        // Draw3DBBoxDashed(points_3d_bbox, 2, color_cx);
        Draw3DBBox(points_3d_bbox, 3, Eigen::Vector3d(241, 158, 194));
        DrawDashedLine3D(
            obj.center, obj.center + direction * 0.5 * obj.size.width, 2,
            Eigen::Vector3d(241, 158, 194));
      } else {
        Draw3DBBox(points_3d_bbox, 1, color_cx);
        DrawDashedLine3D(
            obj.center, obj.center + direction * 0.5 * obj.size.width, 1,
            color_cx);
      }

      // }
    }
  }

  if (draw_track_id) {
    for (int i = 0; i < objects_after_offset.size(); ++i) {
      Eigen::Vector3d pt(0, 0, 0);
      if (!draw_polygon) {                    // TODO fgq true
        pt = objects_after_offset[i].center;  // bbox center
      }
      // else {
      //   // polygon center
      //   for (int j = 0; j < objects_after_offset[i].polygon.size(); ++j) {
      //     auto &polygon_pt = objects_after_offset[i].polygon.at(j);
      //     pt[0] += polygon_pt.x;
      //     pt[1] += polygon_pt.y;
      //   }
      //   pt[0] /= objects_after_offset[i].polygon.size();
      //   pt[1] /= objects_after_offset[i].polygon.size();
      //   pt[2] = objects_after_offset[i].center[2];
      // }

      Eigen::Vector3f color(1, 0, 0);
      std::stringstream ss;
      ss << objects_after_offset[i].track_id;
      Draw3DText(pt, color, ss.str());
    }
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Draw3DText(
    const Eigen::Vector3d &position, const Eigen::Vector3f &color,
    const std::string &text) {
  // current only support digital number and capital letter
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  int offset_x     = viewport[0];
  int offset_y     = viewport[1];
  int image_width  = viewport[2];
  int image_height = viewport[3];

  GLdouble mv_mat[16];
  GLdouble proj_mat[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, mv_mat);
  glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);
  Eigen::Vector4d eye_pos;
  eye_pos[0] = mv_mat[0] * position[0] + mv_mat[4] * position[1] +
               mv_mat[8] * position[2] + mv_mat[12];
  eye_pos[1] = mv_mat[1] * position[0] + mv_mat[5] * position[1] +
               mv_mat[9] * position[2] + mv_mat[13];
  eye_pos[2] = mv_mat[2] * position[0] + mv_mat[6] * position[1] +
               mv_mat[10] * position[2] + mv_mat[14];
  eye_pos[3] = mv_mat[3] * position[0] + mv_mat[7] * position[1] +
               mv_mat[11] * position[2] + mv_mat[15];

  Eigen::Vector4d clip_pos;
  clip_pos[0] = proj_mat[0] * eye_pos[0] + proj_mat[4] * eye_pos[1] +
                proj_mat[8] * eye_pos[2] + proj_mat[12] * eye_pos[3];
  clip_pos[1] = proj_mat[1] * eye_pos[0] + proj_mat[5] * eye_pos[1] +
                proj_mat[9] * eye_pos[2] + proj_mat[13] * eye_pos[3];
  clip_pos[2] = proj_mat[2] * eye_pos[0] + proj_mat[6] * eye_pos[1] +
                proj_mat[10] * eye_pos[2] + proj_mat[14] * eye_pos[3];
  clip_pos[3] = proj_mat[3] * eye_pos[0] + proj_mat[7] * eye_pos[1] +
                proj_mat[11] * eye_pos[2] + proj_mat[15] * eye_pos[3];

  Eigen::Vector3d ndc_pos;
  ndc_pos[0] = clip_pos[0] / clip_pos[3];
  ndc_pos[1] = clip_pos[1] / clip_pos[3];
  ndc_pos[2] = clip_pos[2] / clip_pos[3];

  int ix = offset_x + image_width * 0.5 * (1 + ndc_pos[0]);
  int iy = offset_y + image_height * 0.5 * (1 - ndc_pos[1]);

  glMatrixMode(GL_PROJECTION);  // Operate on projection matrix
  glPushMatrix();
  glLoadIdentity();
  glOrtho(
      offset_x, offset_x + image_width, offset_y + image_height, offset_y, 0.0,
      100.0);
  glMatrixMode(GL_MODELVIEW);  // Operate on model-view matrix
  glPushMatrix();
  glLoadIdentity();

  // GLRasterTextSingleton *raster_text =
  //     idg::perception::lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText *raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();
  raster_text->DrawString(
      text.c_str(), ix, iy, color[0] * 255, color[1] * 255, color[2] * 255);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
