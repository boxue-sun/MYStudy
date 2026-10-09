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

#include <sys/time.h>

#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "app/modules/v2x_application/proto/roi_lane.pb.h"
#include "app/modules/v2x_application/proto/roi_points.pb.h"

#include "app/modules/v2x_application/usecase/base/runner.h"
#include "app/modules/v2x_application/usecase/common/timer.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"

#ifdef USE_VIZ
#include "app/modules/v2x_application/visualization/visualization_internal.h"
#endif

namespace airos {
namespace perception {
namespace usecase {

std::mutex g_usecase_mtx;
std::condition_variable g_usecase_condition;
airos::usecase::EventOutputResult g_usecase_for_perception;
airos::usecase::EventOutputResult g_uescase_for_rsi;

class UsecaseAdapter {
 public:
  UsecaseAdapter()
      : seq_num_(0) {}
  ~UsecaseAdapter() = default;

  bool Init(const std::string& conf_path);
  bool Proc(const std::shared_ptr<const airos::perception::PerceptionObstacles>&
                perception_obstacles);

 private:
  void PostProc(
      std::shared_ptr<airos::usecase::EventOutputResult> output_event,
      const double& us);
  double GetCurrentTimeMicroseconds() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000 + tv.tv_usec;
  }

 private:
  bool is_usecase_ = true;
  uint32_t seq_num_;
  airos::usecase::roi::LaneParam mini_map_params_;
  std::string channel_v2x_usecase_;
  // for Runner
  std::unique_ptr<Runner> events_runner_;
  std::shared_ptr<std::vector<std::string>> events_pipeline_;
  Timer timer_;

#ifdef USE_VIZ
  std::shared_ptr<UsecaseVisualizer> usecase_visualizer_;
#endif
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
