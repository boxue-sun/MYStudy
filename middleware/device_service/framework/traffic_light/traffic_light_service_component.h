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

#include "air_service/framework/proto/airos_traffic_light.pb.h"
#include "base/device_connect/proto/traffic_light_data.pb.h"

#include "middleware/runtime/src/air_middleware_component.h"

namespace os {
namespace v2x {
namespace service {

struct LightStatusRecord {
  ::airos::trafficlight::LightState status =
      ::airos::trafficlight::LightState::UNAVAILABLE;
  int countdown = 0;
};

class AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)
    : public airos::middleware::ComponentAdapter<
          os::v2x::device::TrafficLightBaseData> {
 public:
  AIROS_COMPONENT_CLASS_NAME(TrafficLightServiceComponent)() = default;
  virtual ~AIROS_COMPONENT_CLASS_NAME(
      TrafficLightServiceComponent)() override{};
  bool Init() override;
  bool Proc(const std::shared_ptr<const os::v2x::device::TrafficLightBaseData>&
                traffic_light_data_pb) override;

 private:
  bool LoadPhaseTable(std::string phase_to_direction_file);
  void TransformPhase(
      const std::shared_ptr<os::v2x::device::TrafficLightBaseData>&
          traffic_light_data_pb);
  bool TrafficLightDataPb2ServicePb(
      const std::shared_ptr<os::v2x::device::TrafficLightBaseData>&
          traffic_light_data_pb,
      std::shared_ptr<::airos::trafficlight::TrafficLightServiceData>&
          traffic_light_service_pb);
  bool IsLightUnChanged(
      int light_id, ::airos::trafficlight::LightState status,
      bool has_countdown);

  // Config for TrafficLightServiceData
  std::string city_code_{"beijing"};
  int32_t region_id_{111};
  int32_t cross_id_{22};

  // transfor phase
  std::map<int, uint8_t> map_direction_to_phase_;
  std::map<int, int> map_direction_to_round_;

  std::unordered_map<int, LightStatusRecord> light_duration_record_;
};

REGISTER_AIROS_COMPONENT_CLASS(
    TrafficLightServiceComponent, os::v2x::device::TrafficLightBaseData);

}  // namespace service
}  // namespace v2x
}  // namespace os
