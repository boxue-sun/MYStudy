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
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "air_service/framework/proto/airos_perception_obstacle.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"

#include "app/modules/v2x_application/usecase/common/tensor.h"

namespace airos {
namespace perception {
namespace usecase {

class EventCollector {
 public:
  struct EventInfo {
    airos::usecase::EventInformation::EventType event_t =
        airos::usecase::EventInformation::NONE;
    int64_t event_id;
    Tensor pos;
    Tensor center;
    Tensor speed;
    std::vector<Tensor> poly;
    double start_time = -1;
    double cur_time   = -1;
    std::vector<int64_t> objs;
    std::vector<float> duras;
    std::vector<std::string> ordered_camera;
    bool stop_flag = false;

    std::vector<std::string> lane_id;

    // for lane congestion
    double congestion_coeff = 0.0;
    airos::usecase::CongestionInfo::CongestionLevelType congestion_t =
        airos::usecase::CongestionInfo::NONE;
  };

  void Init();

  void AddEvent(EventInfo&& evet) {
    std::lock_guard<std::mutex> lk(mutex_);
    all_events_.push_back(std::forward<EventInfo>(evet));
  }
  std::vector<EventInfo> GetEvent() {
    std::lock_guard<std::mutex> lk(mutex_);
    return all_events_;
  }
  void Clear() {
    std::lock_guard<std::mutex> lk(mutex_);
    all_events_.clear();
  }

  // 所有感知障碍物和事件都add到参数proto中
  bool UpdateProto(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles,
      std::shared_ptr<airos::usecase::EventOutputResult> proto);

 private:
  std::mutex mutex_;
  std::vector<EventInfo> all_events_;  // 所有事件集合
  std::string version_name_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
