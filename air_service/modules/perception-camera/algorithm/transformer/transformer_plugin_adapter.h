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
#include <string>

#include "air_service/modules/perception-camera/proto/transform_param.pb.h"

#include "air_service/modules/perception-camera/algorithm/interface/object_transform_3d.h"
#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "base/plugin/algorithm_plugin.h"

namespace airos {
namespace perception {
namespace camera {

class TransformerPluginAdapter
    : public airos::base::PluginElement<PerceptionFrame> {
 public:
  TransformerPluginAdapter()  = default;
  ~TransformerPluginAdapter() = default;
  bool Init(const airos::base::PluginParam& param) override;
  bool Run(PerceptionFrame& data) override;

 private:
  bool ReadConfFile(const std::string& conf_file);

 private:
  std::shared_ptr<algorithm::BaseObjectTransformer3D> p_transformer;
  TransformerParam transformer_param_;
  Eigen::Matrix3f camera_k_matrix_;
  Eigen::Affine3d camera2world_pose_;
  Eigen::Vector4d ground_plane_coffe_;
};

// REGISTER_ALGORITHM_PLUGIN_ELEMENT(TransformerPluginAdapter, PerceptionFrame);

}  // namespace camera
}  // namespace perception
}  // namespace airos
