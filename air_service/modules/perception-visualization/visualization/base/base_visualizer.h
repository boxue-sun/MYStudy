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

#include <string>
#include <vector>

#include "GL/glew.h"
#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "base/plugin/registerer.h"

namespace airos {
namespace perception {
namespace visualization {

struct VisualizerInitOptions {
  // std::string root_dir;
  // std::string conf_file;
  // std::string conf_model_name;
  // std::string screen_output_dir;
};

struct VisualizerOptions {};

class BaseVisualizer {
 public:
  BaseVisualizer() = default;

  virtual ~BaseVisualizer() = default;

  virtual bool Init(
      const VisualizerInitOptions& options = VisualizerInitOptions()) = 0;

  // @brief: render from given frame content
  // @param [in]: options
  // @param [in]: content
  virtual bool Render(
      const VisualizerOptions& options, const FrameContent& content) = 0;

  virtual std::string Name() const = 0;

  virtual void SetCallback(const std::function<void()>& call_back_func) {}

  BaseVisualizer(const BaseVisualizer&)            = delete;
  BaseVisualizer& operator=(const BaseVisualizer&) = delete;
};  // class BaseVisualizer

PERCEPTION_REGISTER_REGISTERER(BaseVisualizer);
#define REGISTER_VISUALIZER(name) \
  PERCEPTION_REGISTER_CLASS(BaseVisualizer, name)

}  // namespace visualization
}  // namespace perception
}  // namespace airos
