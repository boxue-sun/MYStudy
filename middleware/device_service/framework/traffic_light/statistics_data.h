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

#include <vector>

#include "base/common/network/serializable_data.h"

namespace os {
namespace v2x {
namespace device {

using namespace afl::base;

// 单路检测通道统计数据（对应雷达协议 4.14.2，字段与GA/T 1743 A.52逐字段对齐）
struct RadarStatisticLane : public afl::base::SerializableData {
  int lane_no   = 0;  // 检测通道序号（检测器编号、线圈编号）0~255
  int volume    = 0;  // 流量（车辆数量）0~255
  int occupancy = 0;  // 占有率 0~200，单位0.5%
  int speed     = 0;  // 车辆平均行驶速度 1~255（255表示溢出），单位km/h
  int length    = 0;  // 车辆平均车长 1~255（255表示溢出），单位0.1m
  int distance  = 0;  // 车辆平均车头时距 1~255（255表示溢出），单位s

  void serialize(afl::base::json& j) override {
    JsonSerialize(lane_no,   "lane_no",   j, false);
    JsonSerialize(volume,    "volume",    j, false);
    JsonSerialize(occupancy, "occupancy", j, false);
    JsonSerialize(speed,     "speed",     j, false);
    JsonSerialize(length,    "length",    j, false);
    JsonSerialize(distance,  "distance",  j, false);
  }

  void deserialize(const afl::base::json& j) override {
    JsonDeserialize(lane_no,   "lane_no",   j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(volume,    "volume",    j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(occupancy, "occupancy", j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(speed,     "speed",     j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(length,    "length",    j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(distance,  "distance",  j, afl::base::g_flag_empty_novalid);
  }
};

// 雷达统计数据（对应雷达协议 4.14 统计数据结构）
// head 和 version 不关心，只提取 statistics
struct RadarStatisticData : public afl::base::SerializableData {
  std::vector<RadarStatisticLane> statistics;  // 单路检测通道统计数据列表

  void serialize(afl::base::json& j) override {
    JsonSerialize(statistics, "statistics", j, false);
  }

  void deserialize(const afl::base::json& j) override {
    JsonDeserialize(statistics, "statistics", j, afl::base::g_flag_empty_novalid);
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
