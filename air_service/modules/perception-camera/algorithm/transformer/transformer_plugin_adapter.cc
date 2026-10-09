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

#include "air_service/modules/perception-camera/algorithm/transformer/transformer_plugin_adapter.h"

#include <string>
#include <utility>

#include "base/common/log.h"
#include "base/io/protobuf_util.h"
#include "middleware/device_service/framework/camera/camera_service.h"

namespace airos {
namespace perception {
namespace camera {

inline void Transform3DInfo2ObjectInfo(
    const algorithm::ObjectTransform3DInfo& in, ObjectInfo& out) {
  out.local_center       = in.local_center;
  out.center             = in.center;
  out.size               = in.size;
  out.theta              = in.theta;
  out.theta_variance     = in.theta_variance;
  out.center_uncertainty = in.center_uncertainty;
  out.direction          = in.direction;
  out.alpha              = in.alpha;
  out.pts8               = in.pts8;
}

inline bool GetAlgorithmParam(
    const TransformerParam& transformer_param,
    algorithm::BaseObjectTransformer3D::InitParam& ret) {
  using airos::middleware::device_service::CameraService;
  airos::middleware::device_service::CameraParam camera_param;
  if (CameraService::get_instance().GetCameraParam(
          transformer_param.camera_name(), &camera_param) == false) {
    return false;
  }
  ret.image_width        = camera_param.intrinsic_params.camera_width;
  ret.image_height       = camera_param.intrinsic_params.camera_height;
  ret.camera2world_pose  = camera_param.camera2world_pose;
  ret.ground_plane_coffe = camera_param.ground_plane_coffe;
  // trans param
  Eigen::VectorXf matrix(9 + 5);
  for (size_t i = 0; i < camera_param.intrinsic_params.K.size(); ++i) {
    matrix(i) = camera_param.intrinsic_params.K[i];
  }
  for (size_t i = 0; i < camera_param.intrinsic_params.D.size(); ++i) {
    matrix(9 + i) = camera_param.intrinsic_params.D[i];
  }
  Eigen::Matrix3f camera_k_matrix;
  camera_k_matrix(0, 0) = matrix(0);
  camera_k_matrix(0, 1) = matrix(1);
  camera_k_matrix(0, 2) = matrix(2);
  camera_k_matrix(1, 0) = matrix(3);
  camera_k_matrix(1, 1) = matrix(4);
  camera_k_matrix(1, 2) = matrix(5);
  camera_k_matrix(2, 0) = matrix(6);
  camera_k_matrix(2, 1) = matrix(7);
  camera_k_matrix(2, 2) = matrix(8);
  ret.camera_k_matrix   = camera_k_matrix;

  return true;
}

bool TransformerPluginAdapter::ReadConfFile(const std::string& conf_file) {
  if (airos::base::ParseProtobufFromFile<TransformerParam>(
          conf_file, &transformer_param_) == false)
    return false;
  return true;
}

bool TransformerPluginAdapter::Init(const airos::base::PluginParam& param) {
  if (ReadConfFile(param.config_file) == false) return false;
  p_transformer.reset(
      algorithm::BaseObjectTransformer3DRegisterer::GetInstanceByName(
          transformer_param_.transformer_name()));
  if (p_transformer == nullptr) return false;
  algorithm::BaseObjectTransformer3D::InitParam alg_param;
  if (GetAlgorithmParam(transformer_param_, alg_param) == false) return false;
  camera_k_matrix_    = alg_param.camera_k_matrix;
  camera2world_pose_  = alg_param.camera2world_pose;
  ground_plane_coffe_ = alg_param.ground_plane_coffe;
  return p_transformer->Init(alg_param);
}

bool TransformerPluginAdapter::Run(PerceptionFrame& data) {
  for (unsigned int i = 0; i < data.track_ret.size(); i++) {
    algorithm::ObjectTransform3DInfo transform_ret;
    int ret_code = p_transformer->Process(*data.track_ret[i], transform_ret);
    if (ret_code != 0) {
      LOG_ERROR << "transform fail";
    }
    Transform3DInfo2ObjectInfo(transform_ret, data.objects[i]);
    data.transform3d_ret.emplace_back(std::move(transform_ret));
  }
  data.camera_k_matrix    = camera_k_matrix_;
  data.camera2world_pose  = camera2world_pose_;
  data.ground_plane_coffe = ground_plane_coffe_;
  return true;
}
}  // namespace camera
}  // namespace perception
}  // namespace airos
