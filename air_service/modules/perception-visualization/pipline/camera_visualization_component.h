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

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "air_service/modules/perception-visualization/pipline/camera_perception_viz_message.h"
#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/visualizer/gl_visualizer.h"
#include "base/common/msg_queue.h"
#include "middleware/runtime/src/air_middleware_component.h"

namespace airos {
namespace perception {
namespace visualization {

#define CAMERA_VIZ_COMPONENT \
  AIROS_COMPONENT_CLASS_NAME(CameraVisualizationComponent)

class CAMERA_VIZ_COMPONENT
    : public airos::middleware::ComponentAdapter<CameraPerceptionVizMessage> {
 public:
  CAMERA_VIZ_COMPONENT()
      : msg_queue(10){};
  ~CAMERA_VIZ_COMPONENT();
  CAMERA_VIZ_COMPONENT(const CAMERA_VIZ_COMPONENT&)            = delete;
  CAMERA_VIZ_COMPONENT& operator=(const CAMERA_VIZ_COMPONENT&) = delete;

  bool Init() override;
  bool Proc(const std::shared_ptr<CameraPerceptionVizMessage const>& message)
      override;

 private:
  void Render();

 private:
  std::string main_camera_name_;

  std::unique_ptr<visualization::GLFusionVisualizer> visualizer_;
  visualization::VisualizerInitOptions viz_init_options_;
  visualization::VisualizerOptions viz_options_;

  std::shared_ptr<visualization::FrameContent> frame_content_;

  std::thread render_thread_;
  std::atomic<bool> running_{false};

  airos::base::common::MsgQueue<std::shared_ptr<visualization::CameraContent>>
      msg_queue;
};

REGISTER_AIROS_COMPONENT_CLASS(
    CameraVisualizationComponent, CameraPerceptionVizMessage);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
