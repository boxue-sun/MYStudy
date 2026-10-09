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

#include <cmath>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"

#include "app/modules/v2x_application/usecase/common/tensor.h"
#include "app/modules/v2x_application/visualization/polygon_collect.h"
#include "app/modules/v2x_application/visualization/window_wrapper.h"

namespace airos {
namespace perception {
namespace usecase {

class UsecaseVisualizer {
 public:
  UsecaseVisualizer() {}

  ~UsecaseVisualizer() {
    is_exit_ = true;
    disp_thread_->join();
  }

  void Init();

 public:
  std::queue<std::shared_ptr<airos::usecase::EventOutputResult>>
      event_output_result_que_;
  std::mutex mutex_;

 private:
  void DispThread();

 private:
  std::shared_ptr<airos::usecase::EventOutputResult> event_output_result_;
  WindowWrapper window_wrapper_;

  std::unique_ptr<std::thread> disp_thread_;
  bool is_exit_ = false;

  // std::shared_ptr<PolygonCollector> polygon_collector_;

  void DrawPoly(
      const std::string& nm,
      std::map<
          std::string,
          std::vector<std::vector<airos::perception::usecase::Vec2d>>>
          polygons);
  void DrawPoint(
      const std::string& nm, std::map<std::string, std::vector<Tensor>> points);
  void DrawPoint(
      const std::string& nm,
      std::map<std::string, std::map<std::string, Tensor>> points);
  void DrawPolyWithID(
      const std::string& nm,
      std::map<
          std::string,
          std::vector<std::vector<airos::perception::usecase::Vec2d>>>
          polygons);
  void DrawHightLightObj(
      const std::string& nm, std::map<std::string, std::vector<int64_t>> objs);
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
