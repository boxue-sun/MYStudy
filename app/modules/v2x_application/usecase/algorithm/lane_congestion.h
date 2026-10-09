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

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/modules/v2x_application/usecase/base/async_alg.h"
#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_application/usecase/common/tensor.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"
#include "app/modules/v2x_application/usecase/map/map_info.h"
#include "app/modules/v2x_application/visualization/polygon_collect.h"

namespace airos {
namespace perception {
namespace usecase {

class MapInfo;
class PolygonCollector;

class LaneCongestion : public AsyncAlgorithmModule {
  using Vec2d   = airos::perception::usecase::Vec2d;
  using Polygon = airos::perception::usecase::Polygon2d;

  struct LastEventState {  // 上一次某个路口的事件状态
    int64_t event_id;
    int car_count           = 0;
    double start_time       = 0.0;
    double end_time         = 0.0;
    double congestion_ceoff = 0.0;
  };

  struct State {
    Tensor pos;
    bool in_flag         = false;
    bool out_flag        = false;
    bool ever_in         = false;
    bool ever_out        = false;
    bool cross_stop_line = false;  // 穿越了停止线
    bool disappear       = false;  // id消失了
    int stopoint_id      = -1;
    double start_time    = -1;  //
    double duration      = -1;  // 停车时间
  };

 public:
  bool Init(const std::shared_ptr<BaseParams>& conf) override;
  void Proc(const std::shared_ptr<FrameData>& data) override;

 private:
  void CalculateLightCycle();
  void GenLane2RegionMap();
  void GenLane();
  void MakeArea(
      std::string lane_id, const Point2d point, const double& heading,
      const int stopoint_id);
  void Normalize(Vec2d* point);
  bool CrossStopLine(const TrackObj& obj);
  void UpdateLane(const std::shared_ptr<FrameData>& data);
  void ShowStopoint();
  void GenQueuePidSet(const std::shared_ptr<FrameData>& data);
  void ShowObjStopDuration();

 private:
  std::map<int, int64_t> id_pid;
  std::map<int, std::vector<int>> light_links_;
  std::map<int, std::pair<Vec2d, std::map<std::string, int>>>
      stopoint_to_light_;
  std::unordered_map<std::string, Lane> local_lanes_;  // lane_id --- lane info
  std::map<int, Lane> lane_in_map_;  // stopoint_id -- lane info
  std::unordered_map<int, std::string>
      lane_merge_map_;  // stopoint_id -- merged_lane_id
  std::unordered_map<int, std::unordered_set<int64_t>>
      lane_state_;  // stopoint_id -- <obj_id_>
  std::unordered_map<int64_t, State> obj_state_;
  std::unordered_map<int, std::unordered_map<std::string, int>>
      peroid_lamp_duration_;  // phase --- <light_status(green...) --- duration>
  std::unordered_map<int, std::deque<int>>
      peroid_length_deq_;  // phase --- <duration>
  std::map<int, std::pair<Vec2d, std::pair<Polygon, Polygon>>>
      stopoint_to_region_;  // stopoint_count --- <lane_direc --- <poly_in,
                            // poly_out>>
  std::map<std::string, LastEventState> lane_last_event_map_;
  std::unordered_set<int64_t> queue_id_set_;  // 存储queue车辆

  int peroid_window_size_       = 3;
  int observe_sec_              = 300;
  float LANE_WIDTH_             = 3.8;
  double in_region_length_      = 8.0;
  float stop_line_shift_        = 0.1;
  float region_gap_             = 0.2;
  double out_region_length_     = 3.0;
  double congestion_coeff_a_    = 0.8;
  double congestion_coeff_b_    = 1.1;
  double congestion_coeff_c_    = 1.8;
  int congestion_car_num_limit_ = 2;
  float queue_dis_              = 10;
  int temp_id_                  = -1;
  int polygon_id_               = -1;
  int output_peroid_            = 60;
  bool debug_                   = false;

 private:
  std::vector<std::vector<Vec2d>> lane_congestion_polygons_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
