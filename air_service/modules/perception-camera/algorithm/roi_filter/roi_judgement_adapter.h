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
#include <vector>

#include "air_service/modules/perception-camera/proto/roi_filter_param.pb.h"

#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "air_service/modules/perception-camera/algorithm/interface/roi_judgement.h"
#include "base/plugin/algorithm_plugin.h"

namespace airos {
namespace perception {
namespace camera {

class RoiPluginAdapter : public airos::base::PluginElement<PerceptionFrame> {
 public:
  RoiPluginAdapter()  = default;
  ~RoiPluginAdapter() = default;
  bool Init(const airos::base::PluginParam& param) override;
  bool Run(PerceptionFrame& data) override;

 private:
  bool ReadConfFile(const std::string& conf_file);

 private:
  std::shared_ptr<algorithm::BaseROIJudgement> p_roi_filter_;
  RoiFilterParam roi_param_;
};

// REGISTER_ALGORITHM_PLUGIN_ELEMENT(RoiPluginAdapter, PerceptionFrame);

}  // namespace camera
}  // namespace perception
}  // namespace airos
