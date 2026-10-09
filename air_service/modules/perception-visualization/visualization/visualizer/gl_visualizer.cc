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

#include "air_service/modules/perception-visualization/visualization/visualizer/gl_visualizer.h"

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace visualization {

GLFusionVisualizer::GLFusionVisualizer()
    : name_("GLFusionVisualizer") {}

bool GLFusionVisualizer::Init(const VisualizerInitOptions& options) {
  viewer_ = std::shared_ptr<GLFWFusionViewer>(new GLFWFusionViewer());
  if (viewer_ == nullptr) {
    LOG_ERROR << "Failed to create opengl viewer";
    return false;
  }
  if (viewer_->initialize(options) == false) {
    LOG_ERROR << "Failed to initialize opengl viewer";
    return false;
  }
  LOG_ERROR << "Initialize GLFusionVisualizer successfully";
  return true;
}

bool GLFusionVisualizer::Render(
    const VisualizerOptions& options, const FrameContent& content) {
  viewer_->set_frame_content(const_cast<FrameContent*>(&content));
  viewer_->spin_once();
  return true;
}

REGISTER_VISUALIZER(GLFusionVisualizer);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
