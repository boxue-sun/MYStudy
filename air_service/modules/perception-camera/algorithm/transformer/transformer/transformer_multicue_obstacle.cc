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

#include "transformer_multicue_obstacle.h"

#include <algorithm>
#include <memory>

#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace algorithm {

TransformerMultiCueObstacle::TransformerMultiCueObstacle()
    : mapper_(std::make_shared<ObjMapper>()) {}

bool TransformerMultiCueObstacle::Init(const InitParam& param) {
  init_param_ = param;
  if (!mapper_->set_coffe_ground(param.ground_plane_coffe)) {
    return false;
  }
  for (size_t i = 0; i < 3; i++) {
    size_t i3 = i * 3;
    for (size_t j = 0; j < 3; j++) {
      k_mat_[i3 + j] = param.camera_k_matrix(i, j);
    }
  }

  mapper_->Init(k_mat_, param.image_width, param.image_height);
  return true;
}

void TransformerMultiCueObstacle::SetObjMapperOptions(
    const ObjectTrackInfo* const obj, Eigen::Matrix3f const& camera_k_matrix,
    Eigen::Affine3d const& camera2world_pose, int width_image, int height_image,
    ObjMapperOptions* obj_mapper_options, float* theta_ray) {
  float bbox2d[4] = {
      obj->detect.box.left_top.x, obj->detect.box.left_top.y,
      obj->detect.box.right_bottom.x, obj->detect.box.right_bottom.y};

  bbox2d[0] = std::min(width_image - 2.0f, std::max(0.0f, bbox2d[0]));
  bbox2d[1] = std::min(height_image - 2.0f, std::max(0.0f, bbox2d[1]));
  bbox2d[2] = std::max(bbox2d[0] + 1.0f, bbox2d[2]);
  bbox2d[3] = std::max(bbox2d[1] + 1.0f, bbox2d[3]);

  float dimension_hwl[3] = {
      obj->detect.size.height, obj->detect.size.width, obj->detect.size.length};

  float bbox3d_bottom[2] = {0};
  bbox3d_bottom[0]       = obj->detect.bottom_uv.x;
  bbox3d_bottom[1]       = obj->detect.bottom_uv.y;

  if (bbox3d_bottom[1] > bbox2d[3] &&
      (obj->detect.is_truncated == TriStatus::FALSE)) {
    bbox3d_bottom[1] = bbox2d[3] - 1;
  }

  float box_cent_x = (bbox2d[0] + bbox2d[2]) / 2;
  Eigen::Vector3f image_point_low_center(box_cent_x, bbox2d[3], 1);
  Eigen::Vector3f point_in_camera =
      camera_k_matrix.inverse() * image_point_low_center;

  *theta_ray = atan2(point_in_camera.x(), point_in_camera.z());

  float rotation_y = *theta_ray + obj->detect.alpha;
  float alpha      = obj->detect.alpha;
  NormalAngle(rotation_y);
  NormalAngle(alpha);

  obj_mapper_options->alpha = alpha;
  obj_mapper_options->ry    = rotation_y;
  memcpy(obj_mapper_options->bbox, bbox2d, sizeof(float) * 4);
  memcpy(obj_mapper_options->hwl, dimension_hwl, sizeof(float) * 3);
  memcpy(obj_mapper_options->bbox3d_bottom, bbox3d_bottom, sizeof(float) * 2);
}

int TransformerMultiCueObstacle::Process(
    const ObjectTrackInfo& obj, ObjectTransform3DInfo& trans_info) {
  ObjMapperOptions obj_mapper_options;
  float object_center[3] = {0};
  float dimension_hwl[3] = {0};
  float rotation_y       = 0.0f;

  if ((obj.detect.box.right_bottom.x <= 1) && (obj.detect.size.height < 0.1)) {
    LOG_DEBUG << "Zero object input to transformer.";
    return 1;
  }

  float theta_ray = 0.0f;
  SetObjMapperOptions(
      &obj, init_param_.camera_k_matrix, init_param_.camera2world_pose,
      init_param_.image_width, init_param_.image_height, &obj_mapper_options,
      &theta_ray);

  mapper_->GetReliableCameraBottomPoint(
      obj_mapper_options, object_center, dimension_hwl);
  rotation_y = obj_mapper_options.ry;
  FillResults(
      object_center, dimension_hwl, rotation_y, init_param_.camera2world_pose,
      theta_ray, &obj, &trans_info);

  return StatusCode::SUCCESS;
}

void TransformerMultiCueObstacle::FillResults(
    float object_center[3], float dimension_hwl[3], float rotation_y,
    Eigen::Affine3d const& camera2world_pose, float theta_ray,
    const ObjectTrackInfo* obj, ObjectTransform3DInfo* trans_res) {
  trans_res->local_center(0) = object_center[0];
  trans_res->local_center(1) = object_center[1];
  trans_res->local_center(2) = object_center[2];
  trans_res->center(0)       = static_cast<double>(object_center[0]);
  trans_res->center(1)       = static_cast<double>(object_center[1]);
  trans_res->center(2)       = static_cast<double>(object_center[2]);
  trans_res->center          = camera2world_pose * trans_res->center;

  trans_res->size.height = dimension_hwl[0];
  trans_res->size.width  = dimension_hwl[1];
  trans_res->size.length = dimension_hwl[2];

  NormalAngle(rotation_y);
  trans_res->theta_variance = 100.0;

  float theta = rotation_y;
  Eigen::Vector3d dir =
      (camera2world_pose.matrix().block(0, 0, 3, 3) *
       Eigen::Vector3d(cos(theta), 0, -sin(theta)));
  trans_res->direction.x = static_cast<float>(dir[0]);
  trans_res->direction.y = static_cast<float>(dir[1]);
  trans_res->direction.z = static_cast<float>(dir[2]);
  trans_res->theta       = static_cast<float>(atan2(dir[1], dir[0]));  // OriP
  trans_res->alpha       = rotation_y - theta_ray;
  NormalAngle(trans_res->alpha);
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
