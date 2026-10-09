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

#include "lidar_perception_component.h"

#include <mutex>

#include <opencv2/opencv.hpp>

#include "base/common/log.h"
#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/plugin/algorithm_loader/dynamic_modules.h"
#include "middleware/runtime/src/air_middleware_rate.h"

namespace airos {
namespace perception {
namespace lidar {

static std::once_flag flag;

LIDAR_DETECTION_COMPONENT::~LIDAR_DETECTION_COMPONENT() {
  running_.exchange(false);
  if (proc_thread_ != nullptr && proc_thread_->joinable()) {
    proc_thread_->join();
  }
}

bool LIDAR_DETECTION_COMPONENT::InitConfig() {
  CHECK(LoadConfig<PerceptionLidarParam>(&lidar_param_))
      << "Read config failed: ";
  return true;
}

bool LIDAR_DETECTION_COMPONENT::InitLidarDriver() {
  using namespace airos::middleware::device_service;
  std::call_once(flag, [this]() {
    CHECK(LidarService::get_instance().InitLidar(lidar_param_.lidar_name()))
        << "Init lidar failed ";
    running_.store(true);
  });
  return true;
}

bool LIDAR_DETECTION_COMPONENT::InitAlgorithmPipeline() {
  lidar_obstacle_pipeline_.reset(
      BaseLidarPerceptionRegisterer::GetInstanceByName(
          lidar_param_.alg_name()));
  CHECK(lidar_obstacle_pipeline_ != nullptr);
  LidarPerceptionParam sdk_param;
  sdk_param.alg_path   = lidar_param_.alg_path();
  sdk_param.conf_path  = lidar_param_.conf_path();
  sdk_param.lidar_name = lidar_param_.lidar_name();
  CHECK(lidar_obstacle_pipeline_->Init(sdk_param) != false)
      << "init lidar obstacle pipeline failed";
  return true;
}

void LIDAR_DETECTION_COMPONENT::SetMsgHeader(
    uint64_t timestamp_us, PerceptionObstacles& message) {
  double publish_time = airos::base::TimeUtil::GetCurrentTime();
  auto* header        = message.mutable_header();
  header->set_timestamp_sec(publish_time);
  header->set_module_name("airos_lidar");
  header->set_sequence_num(seq_num_);
  header->set_frame_id(lidar_param_.lidar_name());
  header->set_camera_timestamp(static_cast<uint64_t>(timestamp_us * 1e3));
  header->set_lidar_timestamp(static_cast<uint64_t>(timestamp_us * 1e3));
  header->set_radar_timestamp(0);
}

bool LIDAR_DETECTION_COMPONENT::Init() {
  airos::base::DynamicModulesManager::GetInstance().Init("modules/lib/");
  if (InitConfig() == false) {
    LOG_ERROR << "InitConfig() failed.";
    return false;
  }

  if (InitLidarDriver() == false) {
    LOG_ERROR << "InitLidarDriver failed.";
    return false;
  }

  if (running_)
    proc_thread_.reset(new std::thread(
        std::bind(&LIDAR_DETECTION_COMPONENT::ProcThread, this)));
  return true;
}

void LIDAR_DETECTION_COMPONENT::ProcThread() {
  using namespace airos::middleware::device_service;
  std::string thread_name = lidar_param_.lidar_name() + "/ProcThread";
  thread_name.resize(15, '\0');
  CHECK_EQ(pthread_setname_np(pthread_self(), thread_name.c_str()), 0);

  airos::middleware::AirMiddlewareRate perception_rate(
      lidar_param_.frequency());

  if (InitAlgorithmPipeline() == false) {
    LOG_ERROR << "InitAlgorithmPipeline failed.";
    abort();
  }

  while (running_.load()) {
    auto lidar_data = LidarService::get_instance().GetLidarData();
    if (lidar_data == nullptr) {
      LOG_ERROR << "lidar get empty.";
      usleep(10000);
      continue;
    }

    std::shared_ptr<PerceptionObstacles> out_message(new PerceptionObstacles());

    if (lidar_obstacle_pipeline_->Perception(lidar_data, out_message.get()) ==
        false) {
      LOG_ERROR << "Perception fail.";
      continue;
    }

    SetMsgHeader(lidar_data->measurement_time(), *out_message);

    std::string output_obstacles_channel_name_ =
        "/airos/lidar/obstacles_" + lidar_param_.lidar_name();
    if (Send(output_obstacles_channel_name_, out_message) == false) {
      LOG_ERROR << "send_lidar_obstacles_msg failed. channel_name: "
                << output_obstacles_channel_name_;
    }
    seq_num_++;
    perception_rate.Sleep();
  }
}

}  // namespace lidar
}  // namespace perception
}  // namespace airos
