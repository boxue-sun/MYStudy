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

#include "dummy_radar.h"

#include <chrono>
#include <fstream>
#include <thread>

#include "base/device_connect/radar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

bool DummyRadar::Init(const std::string& config_file) {
  std::ofstream fs;
  fs.open(config_file, std::ios::in);
  if (!fs.is_open()) {
    return false;
  }
  /*
    解析config_file参数，初始化相机各项参数

    code here...

  */

  return true;
}

void DummyRadar::Start() {
  int i = 0;
  while (i < 5) {
    // 设备数据产生与格式化制备
    auto data = std::make_shared<RadarObstacles>();
    /*
      填充格式化的输出数据
      code here...

    */
    data->mutable_header()->set_sequence_num(i++);

    // 将结构化数据输出给回调函数
    sender_(data);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return;
}

V2XOS_RADAR_REG_FACTORY(DummyRadar, "dummy_radar");

}  // namespace device
}  // namespace v2x
}  // namespace os
