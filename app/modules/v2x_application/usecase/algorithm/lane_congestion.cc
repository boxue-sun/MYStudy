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

#include "app/modules/v2x_application/usecase/algorithm/lane_congestion.h"

#include <algorithm>
#include <string>
#include <unordered_set>

#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "base/common/log.h"
#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace usecase {

bool LaneCongestion::Init(const std::shared_ptr<BaseParams> &conf) {
  APP_LOG_INFO << "Init lane_congestion";
  auto map_info = base::Singleton<MapInfo>::get_instance();
  map_info->GetStopoint2Light(&stopoint_to_light_);
  map_info->GetLocalLanes(&local_lanes_);

  try {
    peroid_window_size_ = conf->GetVal("peroid_window_size").Cast<int>();
    observe_sec_        = conf->GetVal("observe_sec").Cast<int>();
    polygon_id_         = conf->GetVal("polygon_id").Cast<int>();
    debug_              = conf->GetVal("debug").Cast<bool>();
    output_peroid_      = conf->GetVal("output_peroid").Cast<int>();
    congestion_coeff_a_ = conf->GetVal("congestion_coeff_a").Cast<double>();
    congestion_coeff_b_ = conf->GetVal("congestion_coeff_b").Cast<double>();
    congestion_coeff_c_ = conf->GetVal("congestion_coeff_c").Cast<double>();
    congestion_car_num_limit_ =
        conf->GetVal("congestion_car_num_limit").Cast<int>();
  } catch (...) {
    APP_LOG_ERROR << "Output: LaneCongestion default param.";
  }

  for (auto iter = stopoint_to_light_.begin(); iter != stopoint_to_light_.end();
       ++iter) {
    Point2d cur_point;
    cur_point.SetX(iter->second.first.X());
    cur_point.SetY(iter->second.first.Y());

    for (auto &lane : local_lanes_) {
      if (lane.second.polygon->IsPointIn(cur_point)) {
        APP_LOG_INFO << "stoppoint_id = " << iter->first
                 << " lane_id = " << lane.second.lane_id;
        lane_merge_map_[iter->first] = lane.second.lane_id;
        lane_in_map_[iter->first]    = lane.second;
        double heading;
        if (map_info->GetLaneHeading(
                cur_point, lane.second.lane_id, &heading)) {
          MakeArea(lane.second.lane_id, cur_point, heading, iter->first);
        } else {
          APP_LOG_ERROR << "lane_id = " << lane.second.lane_id
                    << " make area error";
        }
        break;
      }
    }
  }

  auto polygon_collector = base::Singleton<PolygonCollector>::get_instance();
  polygon_collector->AddPolygon("lane_congestion", lane_congestion_polygons_);

  ShowStopoint();
  return true;
}

void LaneCongestion::Proc(const std::shared_ptr<FrameData> &data) {
  CalculateLightCycle();
  if (peroid_length_deq_.empty()) {
    APP_LOG_ERROR << "lamp information is not complete to calculate cycle length";
    return;
  }

  GenQueuePidSet(data);

  for (auto &obj : data->Data()) {
    if (!obj.IsExists() || !obj.IsCar() || obj.PerceptionId() == -1) {
      continue;
    }

    auto id    = obj.Id();
    id_pid[id] = obj.PerceptionId();

    if (queue_id_set_.find(obj.Id()) == queue_id_set_.end()) {
      if (debug_) {
        APP_LOG_INFO << "<congestion zombie> not int queue pid = "
                 << obj.PerceptionId() << " id = " << obj.Id();
      }
      // obj不在此刻的排队车辆中
      if (obj_state_.find(id) == obj_state_.end()) {
        continue;
      }
      // obj之前在排队 现在非排队 可能是排队结束或者噪声 todo 噪声处理
    } else {
      if (debug_) {
        APP_LOG_INFO << "<congestion zombie> in queue id = " << obj.PerceptionId();
      }
      // obj此刻排队中 刚出现排队
      if (obj_state_.find(id) == obj_state_.end()) {
        obj_state_[id].start_time = data->TimeStamp();
      } else {
        if (obj_state_[id].start_time < 0) {
          obj_state_[id].start_time = data->TimeStamp();
        }
        obj_state_[id].duration = data->TimeStamp() - obj_state_[id].start_time;
      }
    }
    if (obj_state_.find(id) != obj_state_.end()) {
      obj_state_[id].pos = obj.GetInfo("pos").back().Cast<Tensor>();
    }
    // check obj cross stop line
    if (CrossStopLine(obj)) {
      // 将当前obj分配到对应的车道统计拥堵时间
      if (lane_state_[obj_state_[id].stopoint_id].find(id) ==
          lane_state_[obj_state_[id].stopoint_id].end()) {
        lane_state_[obj_state_[id].stopoint_id].insert(id);
      }

      // 如果当前障碍物穿越了停止线 更新duration
      if (obj_state_[id].cross_stop_line && obj_state_[id].duration < 0 &&
          obj_state_[id].start_time > 0) {
        obj_state_[id].duration = data->TimeStamp() - obj_state_[id].start_time;
      }
    }
  }

  // check obj disappear
  auto track_ids = data->TrackIds();
  for (auto it = obj_state_.begin(); it != obj_state_.end(); ++it) {
    if (!track_ids.count(it->first)) {
      if (!obj_state_[it->first].cross_stop_line  // obj没有穿越停止线
          && !obj_state_[it->first].disappear) {  // obj id跳变了
        if (lane_state_[obj_state_[it->first].stopoint_id].find(it->first) ==
            lane_state_[obj_state_[it->first].stopoint_id].end()) {
          lane_state_[obj_state_[it->first].stopoint_id].insert(it->first);
        }

        obj_state_[it->first].disappear = true;
        if (obj_state_[it->first].start_time > 0) {
          obj_state_[it->first].duration =
              data->TimeStamp() - obj_state_[it->first].start_time;
        }
      }
    }
  }
  UpdateLane(data);
  ShowObjStopDuration();
  if (debug_) {
    for (auto &iter : lane_state_) {
      std::string pid_str = "";
      for (auto &id : iter.second) {
        int pid = -1;
        if (id_pid.count(id)) {
          pid = id_pid[id];
        }
        pid_str += std::to_string(pid);
        pid_str += " ";
      }
      APP_LOG_INFO << "open_source : "
               << " stopoint id = " << iter.first << " " << pid_str;
    }
  }
}

void LaneCongestion::ShowObjStopDuration() {
  if (debug_) {
    for (auto &iter : obj_state_) {
      double duration     = iter.second.duration;
      std::string pos_str = std::to_string(static_cast<int>(duration));
      if (id_pid.count(iter.first)) {
        APP_LOG_INFO << "ShowObjStopDuration: obj = " << id_pid[iter.first]
                 << " duration = " << pos_str;
      }
    }
  }

  std::vector<std::pair<Tensor, std::string>> obj_dura;
  auto polygon_collector = base::Singleton<PolygonCollector>::get_instance();
  for (auto &iter : obj_state_) {
    double duration     = iter.second.duration;
    auto pos            = iter.second.pos;
    std::string pos_str = std::to_string(static_cast<int>(duration));
    obj_dura.push_back({pos, pos_str});
  }
  polygon_collector->AddTextMessage("lane_congestion_obj_dura", obj_dura);
}

void LaneCongestion::GenQueuePidSet(const std::shared_ptr<FrameData> &data) {
  auto map_info = base::Singleton<MapInfo>::get_instance();
  queue_id_set_.clear();
  std::unordered_map<int64_t, std::vector<int64_t>>
      obj_in_lane;  // stopoint_id -- obj_id
  std::unordered_map<int64_t, Point2d> obj_pos;
  for (auto &obj : data->Data()) {
    if (!obj.IsStop()) {
      continue;
    }
    if (obj.PerceptionId() == -1) {
      continue;
    }
    auto pos = obj.GetInfo("pos").back().Cast<Tensor>();
    Point2d pt(pos.x, pos.y);
    obj_pos[obj.Id()] = pt;
    for (auto &lane_info : lane_in_map_) {
      if (lane_info.second.polygon->IsPointIn(pt)) {
        // obj in lane
        obj_in_lane[lane_info.first].push_back(obj.Id());
      }
    }
  }

  for (auto iter : obj_in_lane) {
    auto &stopoint_id = iter.first;
    if (!stopoint_to_light_.count(stopoint_id)) {
      continue;
    }
    auto stopoint_pt = stopoint_to_light_[stopoint_id].first;
    auto &obj_vec    = iter.second;
    std::sort(obj_vec.begin(), obj_vec.end(), [&](int obj_l, int obj_r) {
      return map_info->GetDistance(obj_pos[obj_l], stopoint_pt) <
             map_info->GetDistance(obj_pos[obj_r], stopoint_pt);
    });
    if (debug_) {
      APP_LOG_INFO << "open_source : GenQueuePidSet "
               << " stopoint id = " << iter.first << " ";
      for (auto &id : iter.second) {
        int pid = -1;
        if (id_pid.count(id)) {
          pid = id_pid[id];
        }
        APP_LOG_INFO << " " << pid << " ";
      }
    }
    for (int i = 1, size = obj_vec.size(); i < size; i++) {
      int obj_l = obj_vec[i - 1];
      int obj_r = obj_vec[i];
      if (map_info->GetDistance(obj_pos[obj_l], obj_pos[obj_r]) < queue_dis_) {
        // tow queue car
        queue_id_set_.insert(obj_l);
        queue_id_set_.insert(obj_r);
        lane_state_[stopoint_id].insert(obj_l);
        lane_state_[stopoint_id].insert(obj_r);
      }
    }
  }
}

void LaneCongestion::ShowStopoint() {
  std::map<std::string, Tensor> stopoints;
  auto polygon_collector = base::Singleton<PolygonCollector>::get_instance();
  for (auto &iter : stopoint_to_light_) {
    auto stop_id = iter.first;
    auto pt      = iter.second.first;
    Tensor point;
    point.x                            = pt.X();
    point.y                            = pt.Y();
    stopoints[std::to_string(stop_id)] = point;
  }
  polygon_collector->AddSignalPoint("lane_congestion", stopoints);
}

void LaneCongestion::Normalize(Vec2d *point) {
  double length = std::sqrt(point->X() * point->X() + point->Y() * point->Y());
  if (length != 0) {
    point->SetX(point->X() / length);
    point->SetY(point->Y() / length);
  }
}

void LaneCongestion::MakeArea(
    std::string lane_id, const Point2d point, const double &heading,
    const int stopoint_id) {
  double width = LANE_WIDTH_;

  double c_x = point.X();
  double c_y = point.Y();
  Vec2d lane_direc;

  lane_direc.SetX(cos(heading));
  lane_direc.SetY(sin(heading));

  Normalize(&lane_direc);
  Vec2d vertical_direc;
  vertical_direc.SetX(0 - lane_direc.Y());
  vertical_direc.SetY(lane_direc.X());

  // // shift stop line
  c_x = c_x + lane_direc.X() * stop_line_shift_;
  c_y = c_y + lane_direc.Y() * stop_line_shift_;

  double in_x = c_x - lane_direc.X() * std::abs(region_gap_ * 0.01);
  double in_y = c_y - lane_direc.Y() * std::abs(region_gap_ * 0.01);

  double out_x = c_x + lane_direc.X() * std::abs(region_gap_);
  double out_y = c_y + lane_direc.Y() * std::abs(region_gap_);

  Vec2d pt_1, pt_2, pt_3, pt_4;

  std::vector<Vec2d> poly_out;
  pt_1.SetX(out_x + vertical_direc.X() * width / 2 * 1.5);
  pt_1.SetY(out_y + vertical_direc.Y() * width / 2 * 1.5);
  poly_out.push_back(pt_1);

  pt_2.SetX(pt_1.X() + lane_direc.X() * out_region_length_);
  pt_2.SetY(pt_1.Y() + lane_direc.Y() * out_region_length_);
  poly_out.push_back(pt_2);

  pt_3.SetX(pt_2.X() - vertical_direc.X() * width * 1.5);
  pt_3.SetY(pt_2.Y() - vertical_direc.Y() * width * 1.5);
  poly_out.push_back(pt_3);

  pt_4.SetX(pt_3.X() - lane_direc.X() * out_region_length_);
  pt_4.SetY(pt_3.Y() - lane_direc.Y() * out_region_length_);
  poly_out.push_back(pt_4);

  Polygon2d math_poly_out(poly_out);

  std::vector<Vec2d> poly_in;
  pt_1.SetX(in_x + vertical_direc.X() * width / 2.0 * 0.8);
  pt_1.SetY(in_y + vertical_direc.Y() * width / 2.0 * 0.8);
  poly_in.push_back(pt_1);

  pt_2.SetX(pt_1.X() - lane_direc.X() * in_region_length_);
  pt_2.SetY(pt_1.Y() - lane_direc.Y() * in_region_length_);
  poly_in.push_back(pt_2);

  pt_3.SetX(pt_2.X() - vertical_direc.X() * width * 0.8);
  pt_3.SetY(pt_2.Y() - vertical_direc.Y() * width * 0.8);
  poly_in.push_back(pt_3);

  pt_4.SetX(pt_3.X() + lane_direc.X() * in_region_length_);
  pt_4.SetY(pt_3.Y() + lane_direc.Y() * in_region_length_);
  poly_in.push_back(pt_4);

  Polygon2d math_poly_in(poly_in);
  stopoint_to_region_[stopoint_id] =
      std::make_pair(lane_direc, std::make_pair(math_poly_in, math_poly_out));

  std::vector<airos::perception::usecase::Vec2d> vec2d;
  for (size_t i = 0; i < poly_in.size(); ++i) {
    airos::perception::usecase::Vec2d point2d;
    point2d.SetX(poly_in[i].X());
    point2d.SetY(poly_in[i].Y());
    vec2d.push_back(point2d);
  }
  lane_congestion_polygons_.push_back(vec2d);

  vec2d.clear();
  for (size_t i = 0; i < poly_out.size(); ++i) {
    airos::perception::usecase::Vec2d point2d;
    point2d.SetX(poly_out[i].X());
    point2d.SetY(poly_out[i].Y());
    vec2d.push_back(point2d);
  }
  lane_congestion_polygons_.push_back(vec2d);
}

void LaneCongestion::CalculateLightCycle() {
  auto phase2light_ = MapInfo::GetRealLight();
  for (const auto &phase_light : phase2light_) {
    peroid_lamp_duration_[phase_light.first][phase_light.second.state] =
        phase_light.second.lamp_duration;

    if (peroid_lamp_duration_[phase_light.first].find("2") !=
            peroid_lamp_duration_[phase_light.first].end() &&
        peroid_lamp_duration_[phase_light.first].find("5") !=
            peroid_lamp_duration_[phase_light.first].end() &&
        peroid_lamp_duration_[phase_light.first].find("6") !=
            peroid_lamp_duration_[phase_light.first].end()) {
      while (static_cast<int>(peroid_length_deq_[phase_light.first].size()) >=
             peroid_window_size_) {
        peroid_length_deq_[phase_light.first].pop_front();
      }
      peroid_length_deq_[phase_light.first].push_back(
          peroid_lamp_duration_[phase_light.first]["2"] +
          peroid_lamp_duration_[phase_light.first]["5"] +
          peroid_lamp_duration_[phase_light.first]["6"]);

      peroid_lamp_duration_.clear();
      if (debug_) {
        APP_LOG_INFO << "output: phase id: " << phase_light.first << " "
                 << peroid_length_deq_[phase_light.first].back();
      }
      return;
    }
  }
}

void LaneCongestion::UpdateLane(const std::shared_ptr<FrameData> &data) {
  double light_sum_time       = 0.0;
  double light_cycle_duration = 0.0;
  if (peroid_length_deq_.empty()) {
    return;
  }
  auto &peroid_que = peroid_length_deq_.begin()->second;
  if (peroid_que.size() == 0) {
    return;
  }
  std::for_each(
      peroid_que.begin(), peroid_que.end(), [&light_sum_time](int duration) {
        light_sum_time += duration;
      });
  light_cycle_duration = light_sum_time / peroid_que.size();

  uint64_t timestamp_sec = data->TimeStamp();
  for (auto &lane_info : stopoint_to_light_) {
    int stopoint_id                  = lane_info.first;
    double max_vehicle_stay_duration = 0.0;
    double start_time                = 0.0;
    if (lane_state_.find(stopoint_id) != lane_state_.end()) {
      for (auto it = lane_state_[stopoint_id].begin();
           it != lane_state_[stopoint_id].end();) {
        APP_LOG_INFO << "output congestion: obj_state_[*it] "
                 << std::to_string(obj_state_[*it].start_time) << " "
                 << obj_state_[*it].duration << " "
                 << obj_state_[*it].cross_stop_line << " "
                 << obj_state_[*it].disappear << " "
                 << obj_state_[*it].disappear << " " << id_pid[*it];
        // Object超过观察时间
        if (abs(timestamp_sec - obj_state_[*it].start_time) > observe_sec_) {
          // Object消失或者穿过停止线，则删除
          if (obj_state_[*it].disappear || obj_state_[*it].cross_stop_line) {
            if (debug_) {
              APP_LOG_INFO << "<congestion car> remove dis or csl id = " << *it
                       << " pid = " << id_pid[*it]
                       << " stopoint_id = " << stopoint_id;
            }
            int remove_id = *it;
            it            = lane_state_[stopoint_id].erase(it);
            // 所有车道上都删除之后，再从存储了所有Object的map中删除
            bool rm_flag = true;
            for (auto iter = lane_state_.begin(); iter != lane_state_.end();
                 iter++) {
              if (iter->second.find(remove_id) != iter->second.end()) {
                rm_flag = false;
                break;
              }
            }
            if (rm_flag) {
              obj_state_.erase(remove_id);
              id_pid.erase(remove_id);
            }

            if (lane_state_[stopoint_id].find(remove_id) ==
                lane_state_[stopoint_id].end()) {
              if (debug_) {
                APP_LOG_INFO << "<congestion car> remove success in lane_state_";
              }
            }
            if (obj_state_.find(remove_id) == obj_state_.end()) {
              if (debug_) {
                APP_LOG_INFO << "<congestion car> remove success in obj_state_";
              }
            }
            continue;
          } else {
            // Object未消失且未穿过停止线
            max_vehicle_stay_duration = 360;
            start_time = timestamp_sec - max_vehicle_stay_duration;
            if (debug_) {
              APP_LOG_INFO << "<congestion car> id = " << *it
                       << " pid = " << id_pid[*it]
                       << " stopoint_id = " << stopoint_id;
            }
            break;
          }
        } else {
          // Object未超过观察时间
          if (obj_state_[*it].disappear || obj_state_[*it].cross_stop_line) {
            // Object消失或者穿过停止线,用duration计算停留时间
            if (max_vehicle_stay_duration < obj_state_[*it].duration) {
              max_vehicle_stay_duration = obj_state_[*it].duration;
              start_time                = obj_state_[*it].start_time;
            }
          } else {
            // Object已经不在当前车道
            if (lane_in_map_.find(stopoint_id) != lane_in_map_.end()) {
              auto lane = lane_in_map_[stopoint_id];
              Point2d pt(obj_state_[*it].pos.x, obj_state_[*it].pos.y);
              if (!lane.polygon->IsPointIn(pt)) {
                ++it;
                continue;
              }
            }
            // Object还在排队中
            if (max_vehicle_stay_duration <
                abs(timestamp_sec - obj_state_[*it].start_time)) {
              max_vehicle_stay_duration =
                  abs(timestamp_sec - obj_state_[*it].start_time);
              start_time               = obj_state_[*it].start_time;
              obj_state_[*it].duration = max_vehicle_stay_duration;
            }
          }
        }
        ++it;
      }
    }

    if (debug_) {
      APP_LOG_INFO << "output congestion: " << stopoint_id << " "
               << lane_merge_map_[stopoint_id] << " "
               << lane_state_[stopoint_id].size() << " "
               << max_vehicle_stay_duration << " " << light_cycle_duration
               << " " << max_vehicle_stay_duration / light_cycle_duration << " "
               << timestamp_sec;
    }

    if (timestamp_sec % output_peroid_ == 0) {
      EventCollector::EventInfo event;
      event.event_t = airos::usecase::EventInformation::LANE_CONGESTION;

      Tensor tensor;
      tensor.x     = stopoint_to_light_[stopoint_id].first.X();
      tensor.y     = stopoint_to_light_[stopoint_id].first.Y();
      event.pos    = tensor;
      event.center = event.pos;

      event.start_time = start_time < 0.01 ? data->TimeStamp() : start_time;

      event.cur_time  = data->TimeStamp();
      event.stop_flag = false;
      event.lane_id.push_back(lane_merge_map_[stopoint_id]);
      event.congestion_coeff = max_vehicle_stay_duration / light_cycle_duration;

      std::string lane_id = lane_merge_map_[stopoint_id];
      if (lane_last_event_map_.find(lane_id) == lane_last_event_map_.end()) {
        lane_last_event_map_[lane_id] = LastEventState();
      }

      std::vector<int64_t> objs;
      int car_count = 0;
      if (lane_in_map_.find(stopoint_id) != lane_in_map_.end()) {
        for (auto &obj : data->Data()) {
          if (!obj.IsExists() || !obj.IsCar() || obj.PerceptionId() == -1) {
            continue;
          }

          Tensor obj_pos = obj.GetInfo("pos").back().Cast<Tensor>();
          airos::perception::usecase::Vec2d point;
          point.SetX(obj_pos.x);
          point.SetY(obj_pos.y);

          const auto &tmp_lane = lane_in_map_[stopoint_id];
          if (tmp_lane.polygon->IsPointIn(point)) {
            ++car_count;
            objs.push_back(obj.PerceptionId());
          }
        }
      }
      if (debug_) {
        APP_LOG_INFO << "<congestion> car_count = " << car_count;
      }
      event.objs                              = std::move(objs);
      lane_last_event_map_[lane_id].car_count = car_count;
      // 判断是否需要上报
      if (event.congestion_coeff < congestion_coeff_b_) {
        // 当前为轻度/畅通状态
        if (lane_last_event_map_[lane_id].congestion_ceoff <
            congestion_coeff_b_) {
          // 如果上一次是轻度/畅通,更新事件id，不报事件
          if (debug_) {
            APP_LOG_INFO << "<congestion ceoff < 1.5, dont send>";
          }
          lane_last_event_map_[lane_id].congestion_ceoff =
              event.congestion_coeff;
        } else {
          // 如果上一次是中度/重度状态，获取旧事件，更新事件信息，事件消失
          if (debug_) {
            APP_LOG_INFO << "<congestion> lane_id = " << lane_id
                     << " coeff = " << event.congestion_coeff
                     << " --DISAPPEAR-- ";
          }
          event.event_id = lane_last_event_map_[lane_id].event_id;
          lane_last_event_map_[lane_id].congestion_ceoff =
              event.congestion_coeff;
          lane_last_event_map_[lane_id].end_time = data->TimeStamp();
          if (debug_) {
            APP_LOG_INFO << "<congestion> event_id = " << event.event_id;
          }
        }
      } else {
        // 当前为中度/重度状态
        if (lane_last_event_map_[lane_id].congestion_ceoff <
            congestion_coeff_b_) {
          // 如果上一次是轻度/畅通, 上报事件
          if (car_count < congestion_car_num_limit_) {
            continue;
          }

          lane_last_event_map_[lane_id].congestion_ceoff =
              event.congestion_coeff;
          event.event_id                         = global_id_++;
          lane_last_event_map_[lane_id].event_id = event.event_id;
          event.start_time                       = data->TimeStamp();

          lane_last_event_map_[lane_id].start_time = data->TimeStamp();
          if (event.congestion_coeff < congestion_coeff_a_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_NONE;
          } else if (
              event.congestion_coeff >= congestion_coeff_a_ &&
              event.congestion_coeff < congestion_coeff_b_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_LIGHT;
          } else if (
              event.congestion_coeff >= congestion_coeff_b_ &&
              event.congestion_coeff < congestion_coeff_c_) {
            event.congestion_t = airos::usecase::CongestionInfo::CONGESTION_MID;
          } else if (event.congestion_coeff >= congestion_coeff_c_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_HEAVY;
          }
          data->Collect(std::move(event));
        } else {
          // 如果上一次是中度/重度状态，获取旧事件，更新事件信息，上报事件
          if (debug_) {
            APP_LOG_INFO << "<congestion> car_count = "
                     << lane_last_event_map_[lane_id].car_count;
          }
          if (lane_last_event_map_[lane_id].car_count <
              congestion_car_num_limit_) {
            continue;
          }
          event.event_id   = lane_last_event_map_[lane_id].event_id;
          event.start_time = lane_last_event_map_[lane_id].start_time;
          lane_last_event_map_[lane_id].congestion_ceoff =
              event.congestion_coeff;
          if (event.congestion_coeff < congestion_coeff_a_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_NONE;
          } else if (
              event.congestion_coeff >= congestion_coeff_a_ &&
              event.congestion_coeff < congestion_coeff_b_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_LIGHT;
          } else if (
              event.congestion_coeff >= congestion_coeff_b_ &&
              event.congestion_coeff < congestion_coeff_c_) {
            event.congestion_t = airos::usecase::CongestionInfo::CONGESTION_MID;
          } else if (event.congestion_coeff >= congestion_coeff_c_) {
            event.congestion_t =
                airos::usecase::CongestionInfo::CONGESTION_HEAVY;
          }
          data->Collect(std::move(event));
        }
      }
    }
  }
}

bool LaneCongestion::CrossStopLine(const TrackObj &obj) {
  int64_t id  = obj.Id();
  bool is_in  = false;
  bool is_out = false;

  auto pos = obj.GetInfo("pos").back().Cast<Tensor>();
  Vec2d cur_pt;
  cur_pt.SetX(pos.x);
  cur_pt.SetY(pos.y);

  for (auto iter = stopoint_to_region_.begin();
       iter != stopoint_to_region_.end(); ++iter) {
    if (iter->second.second.first.IsPointIn(cur_pt)) {
      is_in = true;
    }
    if (iter->second.second.second.IsPointIn(cur_pt)) {
      is_out = true;
    }
    if (is_in || is_out) {
      obj_state_[id].stopoint_id = iter->first;
      break;
    }
  }
  obj_state_[id].in_flag  = is_in;
  obj_state_[id].out_flag = is_out;
  obj_state_[id].ever_in  = obj_state_[id].ever_in || is_in;
  obj_state_[id].ever_out = obj_state_[id].ever_out || is_out;

  if (obj_state_[id].ever_in && obj_state_[id].ever_out &&
      !obj_state_[id].in_flag && !obj_state_[id].out_flag) {
    // obj已经穿越了in和out区域 且现在没有在in或者out区域
    obj_state_[id].cross_stop_line = true;
  }
  return obj_state_[id].ever_in || obj_state_[id].ever_out;
}

USECASE_REGISTER_FACTORY(BaseAlgorithmModule, LaneCongestion, "lane_congestion")

}  // namespace usecase
}  // namespace perception
}  // namespace airos
