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

#include "air_service/modules/perception-fusion/proto/fusion_params.pb.h"

#include "air_service/modules/perception-fusion/algorithm/air_fusion/fusion/fusion.h"
#include "air_service/modules/perception-fusion/algorithm/interface/multi_sensor_fusion.h"

namespace airos {
namespace perception {
namespace msf {

struct InputChn {
  const std::vector<FusionInput>& chn;
  std::vector<int> consumed_times;
};

class AirObjectFusion : public BaseMultiSensorFusion {
 public:
  AirObjectFusion() = default;

  ~AirObjectFusion();

  virtual bool Init(const InitParam&) override;

  virtual int Process(
      const std::vector<FusionInput>& input, FusionOutput& output) override;

 private:
  const std::vector<base::Object>& GetFusedObjects() {
    return fused_objects_;
  }
  const std::vector<std::vector<base::Object>>& GetFusionResult() {
    return fusion_result_;
  }
  virtual double Timestamp() const override {
    return pre_mean_time_;
  }
  const base::Error& Error() const {
    return error_;
  }

 private:
  bool CheckAllTimestamp(const std::vector<FusionInput>& input);
  bool CheckSingleTimestamp(const FusionInput& input, int& consumed_times);
  bool IsReferenceSequence(int seq);
  bool UpdateTimestamp();

 private:
  unsigned int channel_num_ = 0;

  bool is_exit_    = false;
  bool debug_mode_ = false;
  Fusion fusion_;
  std::vector<base::Object> fused_objects_;
  std::vector<std::vector<base::Object>> fusion_result_;
  double init_time_     = -1.0;
  double pre_mean_time_ = -1.0;
  double min_timestamp_ = -1.0;
  std::vector<double> timestamps_;
  std::set<int> reference_seq_;
  double time_diff_              = -1.0;
  double missing_time_threshold_ = -1.0;
  double reverse_time_threshold_ = -1.0;
  base::Error error_             = base::Error::NONE;
  std::vector<int> chn_consumed_times_;
  airos::perception::msf::FusionParams fusion_params_;
};

}  // namespace msf
}  // namespace perception
}  // namespace airos
