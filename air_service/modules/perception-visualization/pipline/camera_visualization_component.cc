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

#include "air_service/modules/perception-visualization/pipline/camera_visualization_component.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/format.hpp>

#include "air_service/modules/perception-visualization/config/viz_config_manager.h"
#include "base/common/log.h"
#include "base/common/singleton.h"
namespace airos {
namespace perception {
namespace visualization {

bool CAMERA_VIZ_COMPONENT::Init() {
  auto viz_config_manager = base::Singleton<VizConfigManager>::get_instance();
  main_camera_name_ =
      viz_config_manager->GetVizConfig().camera_config().main_camera_id();
  frame_content_.reset(new visualization::FrameContent);
  running_.store(true);

  render_thread_ = std::thread(std::bind(&CAMERA_VIZ_COMPONENT::Render, this));
  msg_queue.enable();
  return true;
}

bool CAMERA_VIZ_COMPONENT::Proc(
    const std::shared_ptr<CameraPerceptionVizMessage const>& message) {
  std::shared_ptr<visualization::CameraContent> camera_content(
      new visualization::CameraContent);
  camera_content->set_timestamp(message->msg_timestamp_);
  camera_content->set_sensor_name(message->camera_name_);
  camera_content->pose_camera_to_world_ = message->pose_camera_to_world_;
  camera_content->intrinsic_params_     = message->intrinsic_params_;
  camera_content->image_data_ptr_       = message->image_blob_;
  camera_content->camera_objects_       = message->camera_objects_;
  LOG_DEBUG << "camera_objects : " << camera_content->camera_objects_.size()
            << ", pose :" << camera_content->pose_camera_to_world_;
  msg_queue.push(std::move(camera_content));
  return true;
}

void CAMERA_VIZ_COMPONENT::Render() {
  visualizer_.reset(new visualization::GLFusionVisualizer);
  if (!visualizer_->Init(viz_init_options_)) {
    LOG_ERROR << "Failed to init visualizer.";
    return;
  }
  while (running_.load()) {
    std::shared_ptr<visualization::CameraContent> camera_content;
    if (msg_queue.pop(
            &camera_content, airos::base::common::PopPolicy::nonblock) == 0) {
      frame_content_->AddCameraContent(camera_content);
      if (camera_content->sensor_name() == main_camera_name_) {
        frame_content_->SyncSensorContentsByTimestamp(
            camera_content->timestamp());
      }
    }
    visualizer_->Render(viz_options_, *frame_content_);
  }
}

CAMERA_VIZ_COMPONENT::~CAMERA_VIZ_COMPONENT() {
  running_.exchange(false);
  if (render_thread_.joinable()) {
    render_thread_.join();
  }
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
