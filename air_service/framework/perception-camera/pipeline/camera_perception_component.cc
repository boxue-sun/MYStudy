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

#include "camera_perception_component.h"

#include <mutex>

#include <opencv2/opencv.hpp>

#include "base/common/log.h"
#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/plugin/algorithm_loader/dynamic_modules.h"
#include "middleware/runtime/src/air_middleware_rate.h"
#ifdef PERCEPTION_VIZ
#include "air_service/modules/perception-visualization/pipline/camera_perception_viz_message.h"
using airos::perception::visualization::CameraPerceptionVizMessage;
#endif
namespace airos {
namespace perception {
namespace camera {

static std::once_flag flag;

CAMERA_DETECTION_COMPONENT::~CAMERA_DETECTION_COMPONENT() {
  running_.exchange(false);
  if (proc_thread_ != nullptr && proc_thread_->joinable()) {
    proc_thread_->join();
  }
}

bool CAMERA_DETECTION_COMPONENT::InitConfig() {
  CHECK(LoadConfig<PerceptionParam>(&perception_param_))
      << "Read config failed: ";
  return true;
}

bool CAMERA_DETECTION_COMPONENT::InitCameraDriver() {
  using namespace airos::middleware::device_service;

  std::call_once(flag, []() {
    CameraService::get_instance().Init();
  });

  std::vector<std::string> all_cameras;
  CHECK(CameraService::get_instance().QueryAllCameras(all_cameras))
      << "QueryAllCameras failed ";
  for (auto& camera : all_cameras) {
    if (camera == perception_param_.camera_name()) running_.store(true);
  }

  if (running_) {
    CHECK(CameraService::get_instance().InitCamera(
        perception_param_.camera_name(), perception_param_.gpu_id()))
        << "Init Camera failed ";
  } else {
    LOG_ERROR << "Camera does not exist : " << perception_param_.camera_name()
              << ", this channel will stop!!";
  }
  return true;
}

bool CAMERA_DETECTION_COMPONENT::InitAlgorithmPipeline() {
  camera_obstacle_pipeline_.reset(
      BaseCameraPerceptionRegisterer::GetInstanceByName(
          perception_param_.alg_name()));
  CHECK(camera_obstacle_pipeline_ != nullptr);
  CameraPerceptionParam sdk_param;
  sdk_param.sdk_path    = perception_param_.alg_path();
  sdk_param.conf_path   = perception_param_.conf_path();
  sdk_param.camera_name = perception_param_.camera_name();
  sdk_param.gpu_id      = perception_param_.gpu_id();
  CHECK(camera_obstacle_pipeline_->Init(sdk_param) != false)
      << "init camera obstacle pipeline failed";
  return true;
}

void CAMERA_DETECTION_COMPONENT::SetMsgHeader(
    uint64_t timestamp_us, PerceptionObstacles& message) {
  double publish_time = airos::base::TimeUtil::GetCurrentTime();
  auto* header        = message.mutable_header();
  header->set_timestamp_sec(publish_time);
  header->set_module_name("airos_obstacle");
  header->set_sequence_num(seq_num_);
  header->set_frame_id(perception_param_.camera_name());
  header->set_lidar_timestamp(static_cast<uint64_t>(timestamp_us * 1e3));
  header->set_camera_timestamp(static_cast<uint64_t>(timestamp_us * 1e3));
  header->set_radar_timestamp(0);
}

bool CAMERA_DETECTION_COMPONENT::Init() {
  airos::base::DynamicModulesManager::GetInstance().Init("modules/lib/");
  if (InitConfig() == false) {
    LOG_ERROR << "InitConfig() failed.";
    return false;
  }

  if (InitCameraDriver() == false) {
    LOG_ERROR << "InitCameraDriver failed.";
    return false;
  }

  // running_.store(true);
  if (running_)
    proc_thread_.reset(new std::thread(
        std::bind(&CAMERA_DETECTION_COMPONENT::ProcThread, this)));
  return true;
}

void CAMERA_DETECTION_COMPONENT::ProcThread() {
  using namespace airos::middleware::device_service;
  std::string thread_name = perception_param_.camera_name() + "/ProcThread";
  thread_name.resize(15, '\0');
  CHECK_EQ(pthread_setname_np(pthread_self(), thread_name.c_str()), 0);

  airos::middleware::AirMiddlewareRate perception_rate(
      perception_param_.frequency());

  if (InitAlgorithmPipeline() == false) {
    LOG_ERROR << "InitAlgorithmPipeline failed.";
    abort();
  }

  while (running_.load()) {
    double start_time = airos::base::TimeUtil::GetCurrentTime();
    auto camera_data  = CameraService::get_instance().GetCameraData(
        perception_param_.camera_name());
    if (camera_data == nullptr) {
      LOG_ERROR << "camera get empty.";
      usleep(10000);
      continue;
    }
    double get_img_time = airos::base::TimeUtil::GetCurrentTime();
    LOG_INFO << perception_param_.camera_name() << " perception get image cost "
             << static_cast<int>((get_img_time - start_time) * 1000) << " ms";
    LOG_INFO << perception_param_.camera_name() << " driver cost "
             << static_cast<int>(get_img_time * 1000) -
                    camera_data->timestamp / 1000
             << " ms";

    std::shared_ptr<PerceptionObstacles> out_message(new PerceptionObstacles());
    std::shared_ptr<TrafficLightDetection> traffic_light_message(
        new TrafficLightDetection());

    if (camera_obstacle_pipeline_->Perception(
            *camera_data, out_message.get(), traffic_light_message.get()) ==
        false) {
      LOG_ERROR << "Perception fail.";
      continue;
    }

    SetMsgHeader(camera_data->timestamp, *out_message);

    double perception_time = airos::base::TimeUtil::GetCurrentTime();
    LOG_INFO << "perception cost "
             << static_cast<int>((perception_time - get_img_time) * 1000)
             << " ms";

    std::string output_obstacles_channel_name_ =
        "/airos/perception/obstacles_" + camera_data->camera_name;
    if (Send(output_obstacles_channel_name_, out_message) == false) {
      LOG_ERROR << "send_perception_obstacles_msg failed. channel_name: "
                << output_obstacles_channel_name_;
    }

    std::string output_tl_channel_name_ = "/airos/visual_trafficlight";
    if (Send(output_tl_channel_name_, traffic_light_message) == false) {
      LOG_ERROR << "send_traffic_light_msg failed. channel_name: "
                << output_tl_channel_name_;
    }

#ifdef PERCEPTION_VIZ
    CameraParam camera_param;
    if (CameraService::get_instance().GetCameraParam(
            perception_param_.camera_name(), &camera_param) == true) {
      Eigen::VectorXf matrix(9 + 5);
      for (size_t i = 0; i < camera_param.intrinsic_params.K.size(); ++i) {
        matrix(i) = camera_param.intrinsic_params.K[i];
      }
      for (size_t i = 0; i < camera_param.intrinsic_params.D.size(); ++i) {
        matrix(9 + i) = camera_param.intrinsic_params.D[i];
      }
      Eigen::Matrix3f camera_k_matrix;
      camera_k_matrix(0, 0) = matrix(0);
      camera_k_matrix(0, 1) = matrix(1);
      camera_k_matrix(0, 2) = matrix(2);
      camera_k_matrix(1, 0) = matrix(3);
      camera_k_matrix(1, 1) = matrix(4);
      camera_k_matrix(1, 2) = matrix(5);
      camera_k_matrix(2, 0) = matrix(6);
      camera_k_matrix(2, 1) = matrix(7);
      camera_k_matrix(2, 2) = matrix(8);
      std::shared_ptr<CameraPerceptionVizMessage> viz_msg(
          new CameraPerceptionVizMessage(
              camera_data->camera_name, camera_data->timestamp,
              camera_param.camera2world_pose.matrix(), camera_k_matrix,
              camera_data->image->blob(), out_message));
      Send("v2x/perception/viz", viz_msg);
    }
#endif
    seq_num_++;

    const double end_time     = airos::base::TimeUtil::GetCurrentTime();
    const double elapsed_time = end_time - start_time;
    LOG_INFO << "Global_elapsed_time " << perception_param_.camera_name()
             << ": " << elapsed_time * 1000 << " ms";
    perception_rate.Sleep();
  }
}

}  // namespace camera
}  // namespace perception
}  // namespace airos
