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

#include "app/modules/v2x_application/usecase/algorithm/params.h"

#include <fstream>

#include "app/modules/v2x_application/usecase/common/factory.hpp"

namespace airos {
namespace perception {
namespace usecase {

#define STR(name) #name ": " << name << "\n"
#define LINE_DASH "----------"
#define LOAD_VAL(val, type)              \
  if (node[#val].IsDefined()) {          \
    val         = node[#val].as<type>(); \
    mass_[#val] = Meta(val);             \
  }
#define REG_VAL(val) mass_[#val] = Meta(val);

bool UseCaseParam::ParseNode(const YAML::Node& node) {
  try {
    LOAD_VAL(active, bool);
    LOAD_VAL(same_lane_heading_thresh, float);
    LOAD_VAL(lane_param_path, std::string);
    LOAD_VAL(traffic_light_channel, std::string);
    LOAD_VAL(usecase_output_channel, std::string);
  } catch (...) {
    APP_LOG_FATAL << module_name << " params parse error," << strerror(errno);
    return false;
  }
  return true;
}
void UseCaseParam::PrintConfig() {
  APP_LOG_INFO << LINE_DASH << module_name << " params" << LINE_DASH;
  APP_LOG_INFO << STR(active) << STR(same_lane_heading_thresh)
           << STR(lane_param_path) << STR(traffic_light_channel)
           << STR(usecase_output_channel);
  APP_LOG_INFO << LINE_DASH << LINE_DASH;
}

bool DataFrameParam::ParseNode(const YAML::Node& node) {
  try {
    LOAD_VAL(stop_speed_thre, float);
    LOAD_VAL(stop_speed_cnt_thre, float);
    LOAD_VAL(stop_shift_cnt_thre, float);
    LOAD_VAL(tracked_life, int);
  } catch (...) {
    APP_LOG_FATAL << module_name << " params parse error," << strerror(errno);
    return false;
  }
  return true;
}
void DataFrameParam::PrintConfig() {
  APP_LOG_INFO << LINE_DASH << module_name << " params" << LINE_DASH;
  APP_LOG_INFO << STR(stop_speed_thre) << STR(stop_speed_cnt_thre)
           << STR(stop_shift_cnt_thre) << STR(tracked_life);
  APP_LOG_INFO << LINE_DASH << LINE_DASH;
}

bool VizParam::ParseNode(const YAML::Node& node) {
  try {
    LOAD_VAL(usecase_roi, std::string);
  } catch (...) {
    APP_LOG_FATAL << module_name << " params parse error," << strerror(errno);
    return false;
  }
  return true;
}
void VizParam::PrintConfig() {
  APP_LOG_INFO << LINE_DASH << module_name << " params" << LINE_DASH;
  APP_LOG_INFO << STR(usecase_roi);
  APP_LOG_INFO << LINE_DASH << LINE_DASH;
}

bool LaneCongestionParam::ParseNode(const YAML::Node& node) {
  try {
    LOAD_VAL(active, bool);
    LOAD_VAL(mock, bool);
    LOAD_VAL(peroid_window_size, int);
    LOAD_VAL(observe_sec, int);
    LOAD_VAL(polygon_id, int);
    LOAD_VAL(output_peroid, int);
    LOAD_VAL(congestion_coeff_a, double);
    LOAD_VAL(congestion_coeff_b, double);
    LOAD_VAL(congestion_coeff_c, double);
    LOAD_VAL(congestion_car_num_limit, int);
    LOAD_VAL(debug, bool);
  } catch (...) {
    APP_LOG_FATAL << module_name << " params parse error," << strerror(errno);
    return false;
  }
  return true;
}
void LaneCongestionParam::PrintConfig() {
  APP_LOG_INFO << LINE_DASH << module_name << " params" << LINE_DASH;
  APP_LOG_INFO << STR(active) << STR(peroid_window_size) << STR(observe_sec);
  APP_LOG_INFO << LINE_DASH << LINE_DASH;
}

USECASE_REGISTER_FACTORY(BaseParams, UseCaseParam, "usecase")
USECASE_REGISTER_FACTORY(BaseParams, DataFrameParam, "dataframe")
USECASE_REGISTER_FACTORY(BaseParams, VizParam, "visiualization")
USECASE_REGISTER_FACTORY(BaseParams, LaneCongestionParam, "lane_congestion")

}  // namespace usecase
}  // namespace perception
}  // namespace airos
