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

#include <cmath>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "air_service/modules/perception-fusion/proto/multi_sensor_fusion_config.pb.h"

#include "air_service/framework/perception-fusion/interface/base_perception_fusion.h"
#include "air_service/modules/perception-fusion/algorithm/interface/multi_sensor_fusion.h"

namespace airos {
namespace perception {
namespace msf {
using namespace airos::perception::fusion;

class AirosPerceptionFusion : public BasePerceptionFusion {
 public:
  AirosPerceptionFusion()  = default;
  ~AirosPerceptionFusion() = default;
  bool Init(const PerceptionFusionParam &param) override;
  bool Process(
      const std::vector<std::shared_ptr<const PerceptionObstacles>> &input,
      PerceptionObstacles *result) override;
  std::string Name() const override {
    return "AirosPerceptionFusion";
  }

 private:
  void SerializeMsg(
      const msf::FusionOutput &message, PerceptionObstacles *output_msg,
      int error_code);

 private:
  PerceptionFusionParam param_;
  MsfConfig air_config_;
  std::shared_ptr<msf::BaseMultiSensorFusion> air_fusion_;
  int seq_num_ = 0;
};

REGISTER_PERCEPTION_FUSION(AirosPerceptionFusion);

}  // namespace msf
}  // namespace perception
}  // namespace airos
