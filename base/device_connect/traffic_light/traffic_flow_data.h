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

#include <cstdint>
#include <vector>

namespace os {
namespace v2x {
namespace device {

// A13统计数据 - 单路检测通道统计数据（对应协议表A.52）
// A.51规定单路统计数据10字节，A.52定义前6个字段各占1字节，剩余4字节为保留
struct CoilFlowData {
  uint8_t coil_no;    // 检测通道编号（线圈编号），1~255
  uint8_t volume;     // 流量：车辆数量，0~255
  uint8_t occupancy;  // 占有率：车辆平均占有率，单位0.5%，0~200
  uint8_t avg_speed;  // 平均车速，单位km/h，1~255（255表示溢出）
  uint8_t avg_length; // 平均车长，单位0.1m，1~255（255表示溢出）
  uint8_t headway;    // 平均车头时距，单位s，1~255（255表示溢出）
  uint8_t reserved[4]; // 保留字节，填0
};

}  // namespace device
}  // namespace v2x
}  // namespace os
