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

// 单路检测通道过车信息（对应协议 4.13.2）
struct PulseLane : public afl::base::SerializableData {
  int lane_no   = 0;  // 检测通道序号 0~255
  int direction = 0;  // 车辆方向：0 离开检测区域，1 进入检测区域

  void serialize(afl::base::json& j) override {
    JsonSerialize(lane_no,   "lane_no",   j, false);
    JsonSerialize(direction, "direction", j, false);
  }

  void deserialize(const afl::base::json& j) override {
    JsonDeserialize(lane_no,   "lane_no",   j, afl::base::g_flag_empty_novalid);
    JsonDeserialize(direction, "direction", j, afl::base::g_flag_empty_novalid);
  }
};

// 脉冲数据（对应协议 4.13 脉冲数据结构）
// head 和 version 不关心，只提取 lanes
struct PulseData : public afl::base::SerializableData {
  std::vector<PulseLane> lanes;  // 检测通道列表

  void serialize(afl::base::json& j) override {
    JsonSerialize(lanes, "lanes", j, false);
  }

  void deserialize(const afl::base::json& j) override {
    JsonDeserialize(lanes, "lanes", j, afl::base::g_flag_empty_novalid);
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
