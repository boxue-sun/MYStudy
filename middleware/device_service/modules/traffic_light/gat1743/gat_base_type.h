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

#include <stdint.h>

namespace os {
namespace v2x {
namespace device {

// 色步
struct GatLightStep {
  uint8_t color;  // 0-关灯 1-红 2-黄 3-绿  11-红闪 12-黄闪 13-绿闪
  uint8_t type;
  uint16_t duration;
};

// 灯态
struct GatLightState {
  uint8_t color;  // 0-关灯 1-红 2-黄 3-绿  11-红闪 12-黄闪 13-绿闪
  uint8_t type;
  uint8_t countdown;
  uint64_t timestamp;  // 更新时间
  int32_t entry_direction;   // 进口方向，取值范围[0, 359], 正北方向为起点的顺时针旋转角度，单位为度
};

}  // namespace device
}  // namespace v2x
}  // namespace os
