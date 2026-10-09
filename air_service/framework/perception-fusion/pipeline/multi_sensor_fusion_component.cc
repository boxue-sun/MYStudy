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

#include "multi_sensor_fusion_component.h"

#include <iostream>

#include "base/common/log.h"
#include "base/common/singleton.h"
#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"
#include "middleware/runtime/src/air_middleware_rate.h"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace perception {
namespace fusion {

bool SENSOR_FUSION_COMPONENT::Init() {
  airos::base::DynamicModulesManager::GetInstance().Init("modules/lib/");
  if (InitConfig() == false) {
    LOG_ERROR << "InitConfig failed.";
    return false;
  }

  if (InitFusion() == false) {
    LOG_ERROR << "InitFusion failed.";
    return false;
  }

  if (InitNode() == false) {
    LOG_ERROR << "InitNode failed.";
    return false;
  }
  fusion_input_list_.resize(fusion_base_conf_.input_size());

#ifdef MSF_VIZ
  InitVisualization();
#endif

  main_loop_.reset(
      new std::thread(std::bind(&SENSOR_FUSION_COMPONENT::MainLoop, this)));

  initialized_ = true;
  return true;
}

bool SENSOR_FUSION_COMPONENT::InitConfig() {
  CHECK(LoadConfig<PerceptionFusionParam>(&fusion_base_conf_))
      << "Read config failed: ";
  return true;
}

bool SENSOR_FUSION_COMPONENT::InitFusion() {
  air_fusion_.reset(BasePerceptionFusionRegisterer::GetInstanceByName(
      fusion_base_conf_.alg_name()));
  if (air_fusion_ == nullptr) return false;

  if (air_fusion_->Init(fusion_base_conf_) == false) {
    LOG_ERROR << "fusion init failed.";
    return false;
  }
  return true;
}

bool SENSOR_FUSION_COMPONENT::InitNode() {
  const std::string& node_name = fusion_base_conf_.node_name();
  node_ = std::make_shared<middleware::AirMiddlewareNode>(node_name);

  if (node_ == nullptr) {
    return false;
  }

  for (int i = 0; i < fusion_base_conf_.input_size(); ++i) {
    auto reader = node_->CreateReader<PerceptionObstacles>(
        fusion_base_conf_.input(i).channel_name(),
        std::bind(
            &SENSOR_FUSION_COMPONENT::InternalCallback, this, i,
            std::placeholders::_1));
    if (reader == nullptr) {
      return false;
    }
    reader_list_.push_back(reader);
  }

  const std::string& perception_channel =
      fusion_base_conf_.perception_channel();
  perception_writer_ =
      node_->CreateWriter<PerceptionObstacles>(perception_channel);
  if (perception_writer_ == nullptr) {
    return false;
  }

  return true;
}

#ifdef MSF_VIZ
using airos::base::Singleton;
bool SENSOR_FUSION_COMPONENT::InitVisualization() {
  LOG_INFO << "Enable visualization.";
  msf_visual_kernal_ = std::make_shared<msf::MSFVisualKernal>();
  auto fusion_params_file =
      fusion_base_conf_.alg_path() + "/conf/fusion_params.pt";
  if (!airos::base::FileUtil::Exists(fusion_params_file)) {
    return false;
  }
  airos::perception::msf::FusionParams fusion_params;
  CHECK(
      airos::base::ParseProtobufFromFile(fusion_params_file, &fusion_params) ==
      true);
  double chisquare_critic = fusion_params.score_params().chi_square_critical();
  msf_visual_kernal_->Init(fusion_base_conf_.viz_mode(), chisquare_critic);

  std::string air_msf_file =
      fusion_base_conf_.alg_path() + "/conf/msf_config.pt";
  if (!airos::base::FileUtil::Exists(air_msf_file)) return false;
  if (airos::base::ParseProtobufFromFile<airos::perception::msf::MsfConfig>(
          air_msf_file, &air_msf_config_) == false) {
    return false;
  }
  return true;
}
#endif

void SENSOR_FUSION_COMPONENT::InternalCallback(
    int sequence,
    const std::shared_ptr<const PerceptionObstacles>& callback_msg) {
  if (!initialized_ || is_exit_) {
    return;
  }

  {
    std::unique_lock<std::mutex> lck(loop_mutex_);
    if (callback_msg->error_code() == PerceptionErrorCode::ERROR_NONE) {
      fusion_input_list_[sequence] = callback_msg;
    }
  }
  new_msg_.notify_one();
  return;
}

void SENSOR_FUSION_COMPONENT::MainLoop() {
  double output_frequency = fusion_base_conf_.output_frequency();
  airos::middleware::AirMiddlewareRate rate(output_frequency);
  while (!is_exit_) {
    std::vector<std::shared_ptr<const PerceptionObstacles>> fusion_input_list;
    {
      std::unique_lock<std::mutex> lck(loop_mutex_);
      new_msg_.wait(lck);
      fusion_input_list = fusion_input_list_;
    }
    MsfProc(fusion_input_list);
    rate.Sleep();
  }
}

void SENSOR_FUSION_COMPONENT::MsfProc(
    std::vector<std::shared_ptr<const PerceptionObstacles>>&
        fusion_input_list) {
  double start_time  = airos::base::TimeUtil::GetCurrentTime();
  auto fusion_output = std::make_shared<PerceptionObstacles>();
  air_fusion_->Process(fusion_input_list, fusion_output.get());

  for (size_t i = 0; i < fusion_input_list.size(); ++i) {
    auto& message = fusion_input_list.at(i);
    if (message != nullptr && message->unobserved_obstacle().size() > 0) {
      for (const auto& object : message->unobserved_obstacle()) {
        fusion_output->add_unobserved_obstacle()->CopyFrom(object);
      }
    }
  }

#ifdef MSF_VIZ
  msf_visual_kernal_->UpdateObstacles(
      fusion_input_list, fusion_output, air_msf_config_);
#endif

  if (CheckTimestamp(fusion_output)) {  // TODO fgq
    perception_writer_->Write(fusion_output);
  }

  LOG_INFO << "MsfProc() time delay: "
           << (airos::base::TimeUtil::GetCurrentTime() - start_time) * 1000
           << "ms, with " << fusion_output->perception_obstacle_size()
           << " objects";
  seq_num_++;
}

bool SENSOR_FUSION_COMPONENT::CheckTimestamp(
    std::shared_ptr<PerceptionObstacles> output_msg) {
  auto header = output_msg->header();

  static uint64_t pre_camera_timestamp = 0;

  if (header.camera_timestamp() <= pre_camera_timestamp) {
    LOG_ERROR << std::setprecision(16) << "Sensor timestamp error: "
              << header.camera_timestamp() - pre_camera_timestamp
              << ", pre: " << pre_camera_timestamp
              << ", cur: " << header.camera_timestamp();
    return false;
  }

  pre_camera_timestamp = header.camera_timestamp();
  return true;
}

SENSOR_FUSION_COMPONENT::~SENSOR_FUSION_COMPONENT() {
  is_exit_ = true;
  new_msg_.notify_one();
  if (main_loop_->joinable()) {
    main_loop_->join();
  }
}

}  // namespace fusion
}  // namespace perception
}  // namespace airos
