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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "air_service/modules/perception-visualization/visualization/base/frame_content.h"
#include "base/plugin/registerer.h"

namespace airos {
namespace perception {
namespace visualization {

class BaseGLViewport {
 public:
  BaseGLViewport()
      : frame_content_(nullptr)
      , x_(0)
      , y_(0)
      , width_(640)
      , height_(480) {}

  BaseGLViewport(int x, int y, int width, int height)
      : x_(x)
      , y_(y)
      , width_(width)
      , height_(height) {}

  virtual ~BaseGLViewport() = default;

  BaseGLViewport(const BaseGLViewport&)            = delete;
  BaseGLViewport& operator=(const BaseGLViewport&) = delete;

  void render();

  void set_camera_names(const std::vector<std::string>& camera_names) {
    camera_names_.clear();
    camera_names_.assign(camera_names.begin(), camera_names.end());
  }

  void set_main_sensor_name(const std::string& sensor_name) {
    main_sensor_name_ = sensor_name;
  }

  void set_viewport(int x, int y, int width, int height) {
    x_      = x;
    y_      = y;
    width_  = width;
    height_ = height;
  }

  void set_frame_content(FrameContent* frame_content) {
    frame_content_ = frame_content;
  }

  int get_x() const {
    return x_;
  }

  int get_y() const {
    return y_;
  }

  int get_width() const {
    return width_;
  }

  int get_height() const {
    return height_;
  }

 protected:
  virtual void draw() = 0;

 protected:
  FrameContent* frame_content_;
  std::vector<std::string> camera_names_;

  std::string main_sensor_name_;

 private:
  int x_;
  int y_;
  int width_;
  int height_;
};

PERCEPTION_REGISTER_REGISTERER(BaseGLViewport);
#define REGISTER_GLVIEWPORT(name) \
  PERCEPTION_REGISTER_CLASS(BaseGLViewport, name)

}  // namespace visualization
}  // namespace perception
}  // namespace airos
