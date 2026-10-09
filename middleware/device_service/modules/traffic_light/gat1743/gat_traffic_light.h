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
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <memory>
#include <thread>

#include "base/device_connect/traffic_light/device_base.h"
#include "gat_worker.h"

namespace os {
namespace v2x {
namespace device {

/**
 * @brief GAT1743信号机设备类
 *
 */
class GatTrafficLight : public TrafficLightDevice {
 public:
  GatTrafficLight(const TrafficLightCallBack& cb)
      : TrafficLightDevice(cb) {
    gat_worker_  = std::make_shared<GatWorker>();
    output_data_ = std::make_shared<os::v2x::device::TrafficLightBaseData>();
  }

  virtual ~GatTrafficLight() = default;

  bool Init(const std::string& config_file) override;

  void Start() override;

  void WriteToDevice(const std::shared_ptr<const TrafficLightReceiveData>&
                         re_proto) override;  // 实现将信息写入设备

  TrafficLightDeviceState GetState() override {
    return TrafficLightDeviceState::RUNNING;
  }

  void SendData();
  static void TimeoutSender(__sigval_t arg);

  void WritePulseData(
      const std::vector<std::pair<uint8_t, uint8_t>>& lanes) override;
  void WriteTrafficFlowData(
      const std::vector<CoilFlowData>& coils) override;
  std::shared_ptr<os::v2x::device::TrafficLightBaseData> output_data_;

 private:
  bool InitSenderTimer();
  bool isIPv4(const std::string& str);
  std::string getHostByName(std::string name);
  std::shared_ptr<GatWorker> gat_worker_ = nullptr;
  timer_t timer_fd_sender_               = nullptr;
  bool has_send_once 					 = false;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
