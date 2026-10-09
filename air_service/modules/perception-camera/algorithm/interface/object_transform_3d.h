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

#include <memory>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include "base/plugin/registerer.h"
#include "object_track_info.h"

namespace airos {
namespace perception {
namespace algorithm {

struct ObjectTransform3DInfo {
  Eigen::Vector3f local_center;
  Eigen::Vector3d center;
  SizeShape size;
  float theta          = 0.0f;
  float theta_variance = 0.0f;
  Eigen::Matrix3f center_uncertainty;
  Point3f direction;
  float alpha = 0;  // 朝向
  CubeBox2f pts8;
};

using ObjectTransform3DInfoPtr = std::shared_ptr<ObjectTransform3DInfo>;

class BaseObjectTransformer3D {
 public:
  struct InitParam {
    Eigen::Matrix3f camera_k_matrix;
    Eigen::Affine3d camera2world_pose;
    Eigen::Vector4d ground_plane_coffe;
    int image_width;
    int image_height;
    // cv::Mat_<cv::Vec3s> depth_map;
  };

 public:
  virtual ~BaseObjectTransformer3D() = default;

  virtual bool Init(const InitParam& init_param) = 0;

  // 0 success; otherwise fail
  virtual int Process(
      const ObjectTrackInfo& object, ObjectTransform3DInfo&) = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseObjectTransformer3D);
#define REGISTER_OBSTACLE_TRANSFORMER(name) \
  PERCEPTION_REGISTER_CLASS(BaseObjectTransformer3D, name)

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
