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

#include "app/modules/v2x_application/usecase_adapter/usecase_adapter.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "app/modules/v2x_application/usecase/common/util.h"
#include "base/common/singleton.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace usecase {

bool UsecaseAdapter::Init(const std::string& conf_path) {
  // init config manager
  YAML::Node config_yaml_node;
  YAML::Node param_yaml_node;

  auto config_manager         = base::Singleton<ConfigManager>::get_instance();
  std::string usecase_configs = conf_path + "/usecase_configs.yaml";

  if (!config_manager->Init(usecase_configs)) {
    std::cout << "Init ConfigManager Failed!" << std::endl;
    return false;
  }

  auto param = Factory<BaseParams>::Instance().GetShared("usecase");
  if (!param) {
    return false;
  }
  config_manager->LoadFor(param);

  is_usecase_ = param->GetVal("active").Cast<bool>();

  if (!airos::base::ParseProtobufFromFile(
          param->GetVal("lane_param_path").Cast<std::string>(),
          &mini_map_params_)) {
    APP_LOG_ERROR << "Parse Protobuf From File Failed";
    return false;
  }

  // init Runner
  events_runner_.reset(new Runner());
  std::string events_path     = conf_path + "/usecase_events.yaml";
  YAML::Node events_yaml_node = YAML::LoadFile(events_path);
  if (is_usecase_) {
    events_pipeline_ = std::make_shared<std::vector<std::string>>();
    if (events_yaml_node["events_pipeline"].IsSequence()) {
      for (size_t i = 0; i < events_yaml_node["events_pipeline"].size(); ++i) {
        events_pipeline_->push_back(
            events_yaml_node["events_pipeline"][i].as<std::string>());
      }
    }
    events_runner_->SetPipeline(events_pipeline_);
  }
  events_runner_->Init();

#ifdef USE_VIZ
  usecase_visualizer_.reset(new UsecaseVisualizer());
  usecase_visualizer_->Init();
#endif

  APP_LOG_INFO << "UseCase Init Success!";
  return true;
}

bool UsecaseAdapter::Proc(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        perception_obstacles) {
  if (!perception_obstacles) {
    return true;
  }
  const double start_time = GetCurrentTimeMicroseconds();
  timer_.Reset();

  std::shared_ptr<airos::usecase::EventOutputResult> output = nullptr;

  // proc priority cases
  output = events_runner_->Run(perception_obstacles);
  PostProc(output, start_time);
  {
    std::lock_guard<std::mutex> guard(g_usecase_mtx);
    g_usecase_for_perception.Clear();
    g_usecase_for_perception.operator=(*output);
    g_uescase_for_rsi.Clear();
    g_uescase_for_rsi.operator=(*output);
    g_usecase_condition.notify_all();
  }
  APP_LOG_INFO << " cost time(ms): " << float(timer_.DurationMicrosec()) / 1000.0
           << " , obstacle count: "
           << (perception_obstacles->perception_obstacle_size()) << std::endl;

  // clean events
  events_runner_->ClearEvents();

#ifdef USE_VIZ
  {
    std::lock_guard<std::mutex> _lock(usecase_visualizer_->mutex_);
    usecase_visualizer_->event_output_result_que_.push(output);
  }
#endif

  return true;
}

void UsecaseAdapter::PostProc(
    std::shared_ptr<airos::usecase::EventOutputResult> output_event,
    const double& us) {
  // set time
  if (!output_event->mutable_header()->has_radar_timestamp()) {
    output_event->mutable_header()->set_radar_timestamp(
        static_cast<uint64_t>(us * 1e3));
  }
  using namespace std::chrono;
  auto now       = high_resolution_clock::now();
  auto micros    = duration_cast<microseconds>(now.time_since_epoch());
  double seconds = static_cast<double>(micros.count()) *
                   microseconds::period::num / microseconds::period::den;
  output_event->mutable_header()->set_timestamp_sec(seconds);
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
