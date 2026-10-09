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

#include "air_service/modules/perception-camera/algorithm/interface/object_transform_3d.h"
#include "obj_mapper.h"

namespace airos {
namespace perception {
namespace algorithm {

enum StatusCode {
  /// Successfully
  SUCCESS = 0,
  /// Invalid license, no license file or license expired
  FAIL = 1
};

class TransformerMultiCueObstacle : public BaseObjectTransformer3D {
 public:
  TransformerMultiCueObstacle();

  bool Init(const InitParam &param) override;

  int Process(const ObjectTrackInfo &object, ObjectTransform3DInfo &trans_info)
      override;

 private:
  void SetObjMapperOptions(
      const ObjectTrackInfo *const obj, Eigen::Matrix3f const &camera_k_matrix,
      Eigen::Affine3d const &camera2world_pose, int width_image,
      int height_image, ObjMapperOptions *obj_mapper_options, float *theta_ray);

  void FillResults(
      float object_center[3], float dimension_hwl[3], float rotation_y,
      Eigen::Affine3d const &camera2world_pose, float theta_ray,
      const ObjectTrackInfo *obj, ObjectTransform3DInfo *trans_res);

 private:
  std::shared_ptr<ObjMapper> mapper_;
  InitParam init_param_;
  float k_mat_[9] = {0};
};

REGISTER_OBSTACLE_TRANSFORMER(TransformerMultiCueObstacle);

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
