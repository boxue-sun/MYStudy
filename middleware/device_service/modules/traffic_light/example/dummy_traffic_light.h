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

#include "base/device_connect/traffic_light/device_base.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief 演示如何注册一个信号灯采集设备到信号灯工厂
 *
 */
class DummyTrafficLight : public TrafficLightDevice {
 public:
  DummyTrafficLight(const TrafficLightCallBack& cb)
      : TrafficLightDevice(cb) {}

  virtual ~DummyTrafficLight() = default;

  bool Init(const std::string& config_file) override;

  void Start() override;

  void WriteToDevice(const std::shared_ptr<const TrafficLightReceiveData>&
                         re_proto) override;  // 实现将信息写入设备

  TrafficLightDeviceState GetState() override {
    return TrafficLightDeviceState::RUNNING;
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
