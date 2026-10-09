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

#include "air_service/modules/perception-camera/proto/detector_param.pb.h"

#include "air_service/modules/perception-camera/algorithm/interface/object_detecter.h"
#include "air_service/modules/perception-camera/algorithm/interface/perception_frame.h"
#include "base/plugin/algorithm_plugin.h"

namespace airos {
namespace perception {
namespace camera {

class DetectorPluginAdapter
    : public airos::base::PluginElement<PerceptionFrame> {
 public:
  DetectorPluginAdapter()  = default;
  ~DetectorPluginAdapter() = default;
  bool Init(const airos::base::PluginParam& param) override;
  bool Run(PerceptionFrame& data) override;

 private:
  bool ReadConfFile(const std::string& conf_file);

 private:
  std::shared_ptr<algorithm::BaseObjectDetecter> p_detector_;
  DetectorParam detector_param_;
};

// REGISTER_ALGORITHM_PLUGIN_ELEMENT(DetectorPluginAdapter, PerceptionFrame);

}  // namespace camera
}  // namespace perception
}  // namespace airos
