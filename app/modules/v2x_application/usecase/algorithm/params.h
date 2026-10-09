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

#include <map>
#include <string>

#include "app/modules/v2x_application/usecase/base/config_manager.h"

namespace airos {
namespace perception {
namespace usecase {

class UseCaseParam : public BaseParams {
 public:
  std::string module_name{"usecase"};
  bool active;
  float same_lane_heading_thresh;
  std::string lane_param_path;
  std::string traffic_light_channel;
  std::string usecase_output_channel;

 public:
  bool ParseNode(const YAML::Node& node) override;
  void PrintConfig() override;
  std::string Name() const override {
    return module_name;
  }
};

class DataFrameParam : public BaseParams {
 public:
  std::string module_name{"dataframe"};
  float stop_speed_thre     = 1.0;
  float stop_speed_cnt_thre = 37;
  float stop_shift_cnt_thre = 27;
  int tracked_life;

 public:
  bool ParseNode(const YAML::Node& node) override;
  void PrintConfig() override;
  std::string Name() const override {
    return module_name;
  }
};

class VizParam : public BaseParams {
 public:
  std::string module_name{"visiualization"};
  std::string usecase_roi;

 public:
  bool ParseNode(const YAML::Node& node) override;
  void PrintConfig() override;
  std::string Name() const override {
    return module_name;
  }
};

class LaneCongestionParam : public BaseParams {
 public:
  std::string module_name{"lane_congestion"};
  bool active;
  bool mock;
  int peroid_window_size;
  int observe_sec;
  int polygon_id;
  int output_peroid;
  double congestion_coeff_a;
  double congestion_coeff_b;
  double congestion_coeff_c;
  int congestion_car_num_limit;
  bool debug;

 public:
  bool ParseNode(const YAML::Node& node) override;
  void PrintConfig() override;
  std::string Name() const override {
    return module_name;
  }
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
