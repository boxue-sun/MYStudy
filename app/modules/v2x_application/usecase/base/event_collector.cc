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

#include "app/modules/v2x_application/usecase/base/event_collector.h"

#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"

namespace airos {
namespace perception {
namespace usecase {

void EventCollector::Init() {}

bool EventCollector::UpdateProto(
    const std::shared_ptr<const airos::perception::PerceptionObstacles>&
        obstacles,
    std::shared_ptr<airos::usecase::EventOutputResult> proto) {
  std::lock_guard<std::mutex> lk(mutex_);
  proto->mutable_header()->CopyFrom(obstacles->header());
  // proto->mutable_header()->set_module_name(version_name_);
  proto->set_perception_error_code(obstacles->error_code());

  for (int j = 0; j < obstacles->perception_obstacle_size(); ++j) {
    // auto obj = obstacles->perception_obstacle(j);
    // proto->add_perception_obstacle(obj);

    auto obj = proto->add_perception_obstacle();
    obj->CopyFrom(obstacles->perception_obstacle(j));
  }

  // 处理所有事件，add 进输出的proto
  for (const auto& evt : all_events_) {
    auto event = proto->add_events();
    switch (evt.event_t) {
      // 车道拥塞事件
      case airos::usecase::EventInformation::LANE_CONGESTION: {
        event->set_event_type(evt.event_t);
        event->set_id(evt.event_id);
        event->set_stop_flag(evt.stop_flag);
        event->set_timestamp(evt.cur_time);
        event->mutable_location_point()->set_x(evt.center.x);
        event->mutable_location_point()->set_y(evt.center.y);
        event->mutable_location_point()->set_z(evt.center.z);
        event->set_start_time(evt.start_time);

        if (!evt.lane_id.empty()) {
          for (const auto& id : evt.lane_id) {
            event->add_lane_id(id);
          }
        }
        if (!evt.objs.empty()) {
          for (const auto& id : evt.objs) {
            event->add_obstacle_id(id);
          }
        }
        auto congestion_info = event->mutable_congestion_info();
        congestion_info->set_congestion_coeff(evt.congestion_coeff);
        airos::usecase::CongestionInfo::CongestionLevelType congestion_t =
            airos::usecase::CongestionInfo::NONE;
        congestion_t = evt.congestion_t;
        congestion_info->set_congestion_level(congestion_t);
        break;
      }

      default:
        break;
    }
  }
  return true;
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
