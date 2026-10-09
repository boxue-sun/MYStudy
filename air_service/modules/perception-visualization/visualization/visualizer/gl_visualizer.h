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

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "air_service/modules/perception-visualization/visualization/base/base_visualizer.h"
#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "air_service/modules/perception-visualization/visualization/visualizer/glfw_viewer.h"

namespace airos {
namespace perception {
namespace visualization {

class GLFusionVisualizer : public BaseVisualizer {
 public:
  GLFusionVisualizer();
  virtual ~GLFusionVisualizer() = default;

  GLFusionVisualizer(const GLFusionVisualizer&)            = delete;
  GLFusionVisualizer& operator=(const GLFusionVisualizer&) = delete;

  bool Init(const VisualizerInitOptions& options) override;

  std::string Name() const override {
    return name_;
  }

  bool Render(
      const VisualizerOptions& options, const FrameContent& content) override;

 private:
  std::shared_ptr<GLFWFusionViewer> viewer_;
  std::string name_;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
