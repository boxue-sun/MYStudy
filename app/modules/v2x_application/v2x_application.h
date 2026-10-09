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

#include "app/framework/interface/app_base.h"
#include "app/framework/interface/app_factory.h"
#include "app/modules/v2x_application/cloud_adapter/cloud_adapter.h"
#include "app/modules/v2x_application/common/producer_consumer_queue.h"
#include "app/modules/v2x_application/monitor/monitor.h"
#include "app/modules/v2x_application/perception_adapter/perception_adapter.h"
#include "app/modules/v2x_application/rsi_generator/rsi_generator.h"
#include "app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h"
#include "app/modules/v2x_application/usecase_adapter/usecase_adapter.h"
#include "app/modules/v2x_application/v2x_message_reporter/v2x_message_reporter.h"
#include "app/modules/v2x_application/mec_adapter/mec_adapter.h"
#include "app/modules/v2x_application/map_generator/map_generator.h"
#include "app/modules/v2x_application/mec_adapter_ssm/mec_adapter_ssm.h"

namespace airos {
namespace app {

class V2xApplication : public airos::app::AirOSApplication {
 public:
  V2xApplication(const ApplicationCallBack& cb)
      : AirOSApplication(cb) {}

  ~V2xApplication();

  bool Init(const AppliactionConfig& conf) override;
  void Start() override;

 private:
  ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>
      cloud_sequence_;
  std::shared_ptr<CloudAdapter> cloud_adapter_;
  std::shared_ptr<airos::perception::usecase::UsecaseAdapter> usecase_adapter_;
  std::shared_ptr<PerceptionAdapter> perception_adapter_;
  std::shared_ptr<RsiGenerator> rsi_generator_;
  bool is_rsi_generator_init_;
  std::shared_ptr<TrafficLightAdapter> traffic_light_adapter_;
  bool is_traffic_light_adapter_init_;
  std::shared_ptr<MECAdapter> mec_adapter_;
  bool is_mec_adapter_init_;
  std::shared_ptr<MECAdapterSsm> mec_adapter_ssm_;
  std::shared_ptr<MapGenerator> map_generator_;
  bool is_map_generator_init_;
  
  std::shared_ptr<V2xMessageReporter> v2x_message_reporter_;
  std::shared_ptr<airos::monitor::Monitor> monitor_;

  std::shared_ptr<std::thread> thread_perception_adapter_{nullptr};
  std::shared_ptr<std::thread> thread_rsi_generator_{nullptr};

  std::atomic<bool> b_perception_adapter_{true};
  std::atomic<bool> b_rsi_generator_{true};
  void OnTrafficLightData(
      const std::shared_ptr<
          const ::airos::trafficlight::TrafficLightServiceData>& data);

  void OnMECData(
      const std::shared_ptr<
          const airos::usecase::EventOutputResult>& data);
  void StartThreadPerceptionAdapter();
  void StartThreadRsiGenerator();
};

}  // namespace app
}  // namespace airos
