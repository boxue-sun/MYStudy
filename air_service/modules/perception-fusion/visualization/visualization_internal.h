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
#include <mutex>
#include <thread>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/modules/perception-fusion/proto/multi_sensor_fusion_config.pb.h"

#include "air_service/modules/perception-fusion/visualization/window_wrapper.h"

namespace airos {
namespace perception {
namespace msf {

class MSFVisualKernal {
 public:
  MSFVisualKernal() {}

  ~MSFVisualKernal() {
    is_exit_ = true;
    disp_thread_->join();
  };

  int Init(int mode = 1, double chisquare = 10.0);

  void SetMode(int mode = 1);
  bool UpdateObstacles(
      const std::vector<std::shared_ptr<
          const airos::perception::PerceptionObstacles>>& fusion_input_list,
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          fusion_output,
      MsfConfig& fusion_param);
  void DrawObstaclesError(double x, double y, double error) {
    window_wrapper_.DrawObstaclesError(x, y, error);
  }

 private:
  void DispThread();

 private:
  std::vector<msf::FusionInput> fusion_input_list_;
  msf::FusionOutput fusion_output_;
  int mode_         = 1;
  double chisquare_ = 10.0;
  WindowWrapper window_wrapper_;
  std::unique_ptr<std::thread> disp_thread_;
  bool is_exit_ = false;
  std::mutex data_mutex_;
};

}  // namespace msf
}  // namespace perception
}  // namespace airos
