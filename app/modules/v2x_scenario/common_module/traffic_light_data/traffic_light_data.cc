/*********************************************************************************
 * @file		traffic_light_data.cc
 * @brief		traffic_light_data
 * @details	traffic_light_data
 * @author	ChangXuhui
 * @date		2024/4/22
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/22  ChangXuhui      1.0            ————                 ————
 *
 * @endverbatim
 ********************************************************************************/

#include "traffic_light_data.h"

#include "base/common/log.h"
#include "base/common/time_util.h"

namespace airos {
namespace app {

TrafficLightData::TrafficLightData() {
  traffic_light_maintenance_ = std::make_shared<std::thread>(
      std::bind(&TrafficLightData::TrafficLightMaintenance, this));
}

bool TrafficLightData::UpdateBySpatMsg(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  // 通过spat更新
  if (frame->has_spatframe()) {
    for (int i = 0; i < frame->spatframe().intersections_size(); ++i) {
      auto intersection = frame->spatframe().intersections(i);
      std::string id = std::to_string(intersection.node_region()) + "_" +
                       std::to_string(intersection.node_id());
      SpatPhasesMap spat_phases_map;
      for (int j = 0; j < intersection.phases_size(); ++j) {
        auto phase = intersection.phases(j);
        for (int k = 0; k < phase.phase_state_size(); ++k) {
          auto phase_state = phase.phase_state(k);
          if (0 == phase_state.timing_start()) {
            int phase_id = phase.id();
            spat_phases_map.insert({phase_id, phase_state});
            break;
          }
        }
      }

      LockTrafficLightDataMap();
      traffic_light_data_map_.clear();
      traffic_light_data_map_.insert({id, spat_phases_map});
      UnlockTrafficLightDataMap();
    }
  }
  return true;
}

void TrafficLightData::TrafficLightMaintenance() {
  while (true) {
    double time_now = airos::base::TimeUtil::GetCurrentTime();
    // 超过1s未更新,删除旧的红绿灯数据
    if (time_now - updata_time_ > 1.0) {
      LockTrafficLightDataMap();
      traffic_light_data_map_.clear();
      UnlockTrafficLightDataMap();
    }
    // 500ms 检查一次
    std::chrono::microseconds duration(500000);
    std::this_thread::sleep_for(duration);
  }
}
}  // namespace app
}  // namespace airos
