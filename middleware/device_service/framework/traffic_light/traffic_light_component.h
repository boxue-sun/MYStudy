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

#include <thread>

#include "middleware/device_service/framework/proto/traffic_light_config.pb.h"

#include "base/device_connect/traffic_light/device_base.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "middleware/runtime/src/air_middleware_component.h"
#include "middleware/runtime/src/air_middleware_reader.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
#include "pulse_data.h"
#include "statistics_data.h"
#include "base/device_connect/traffic_light/traffic_flow_data.h"
namespace os {
namespace v2x {
namespace device {

class AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)
    : public airos::middleware::ComponentAdapter<
          os::v2x::device::TrafficLightReceiveData> {
 public:
  AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)(){
      mec_monitor_ = std::make_shared<airos::monitor_mec::MonitorMec>();
  };
  ~AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)() override = default;

  bool Init() override;
  bool Proc(
      const std::shared_ptr<const os::v2x::device::TrafficLightReceiveData>&
          recv_data) override;

 private:
  void CallBack(const TrafficLightDataType& data);
  void OnPulseData(
      const std::shared_ptr<const os::v2x::device::CloudData>& data);
  void OnRadarStatisticData(
      const std::shared_ptr<const os::v2x::device::CloudData>& data);
  void WriteBackFromCommParam();
 private:
  std::unique_ptr<TrafficLightDevice> device_;
  std::unique_ptr<std::thread> task_;
  os::v2x::device::traffic_light::Config conf_;

  std::shared_ptr<airos::monitor_mec::MonitorMec> mec_monitor_ = nullptr;
  std::shared_ptr<airos::middleware::AirMiddlewareReader<os::v2x::device::CloudData>>
      pulse_reader_;
  std::shared_ptr<airos::middleware::AirMiddlewareReader<os::v2x::device::CloudData>>
      statistic_reader_;
};

REGISTER_AIROS_COMPONENT_CLASS(
    TrafficLightComponent, os::v2x::device::TrafficLightReceiveData);

}  // namespace device
}  // namespace v2x
}  // namespace os
