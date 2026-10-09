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

#include "v2x_affect_path.h"

namespace airos {
namespace app {

bool RsiAffectPath::GetAffectPathBySpecialConf(
    const int mec_event_type, const airos::perception::usecase::Vec2d ev_point,
    const std::vector<SpecialEventDetailPtr> special_event_conf_list,
    const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  APP_LOG_INFO << "Get affect path by special conf";
  zone_ = map.nodes(0).position().xyz().zone();

  for (auto const special_event_conf : special_event_conf_list) {
    // 判断是否在触发区域
    if (IsSpecialEventMatched(mec_event_type, ev_point, special_event_conf)) {
      // 填充路径

      // 按照配置点进行填充
      if (special_event_conf->has_affected_area()) {
        APP_LOG_INFO << "Fill affects the path by conf affected area point";
        auto affected_area = special_event_conf->affected_area();

        for (int i = 0; i < affected_area.position_list_size(); ++i) {
          auto position_list = affected_area.position_list(i);
          if (position_list.position_size() < 2) {
            continue;
          }
          v2xpb::asn::RsiReferencePath info;
          for (int j = 0; j < position_list.position_size(); ++j) {
            auto position = position_list.position(j);

            airos::perception::usecase::Vec2d s;
            ConvertToUtm(position.latitude(), position.longitude(), s);

            auto point = info.add_points();
            point->mutable_xyz()->set_x(s.X());
            point->mutable_xyz()->set_y(s.Y());
            point->mutable_xyz()->set_z(0.0);
            point->mutable_xyz()->set_zone(zone_);
          }
          info.set_radius(position_list.radius());
          affect_paths->push_back(info);
        }
      }

      // 按照配置的link进行填充
      if (special_event_conf->has_affected_path()) {
        APP_LOG_INFO << "Fill affects the path by conf affected path";
        auto affected_path = special_event_conf->affected_path();

        for (int i = 0; i < affected_path.link_path_size(); ++i) {
          auto link = affected_path.link_path(i);
          GetAffecLinkPathByConf(map, link, affect_paths);
        }
      }
    };
  }
  return true;
}

void RsiAffectPath::GetAffecLinkPathByConf(
    const v2xpb::asn::Map& map, const v2xpb::rscu::config::LinkPath& lk,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  auto node = map.nodes(0);

  // 判断node是否相同
  if ((lk.node_id().region() != node.id().region()) ||
      (lk.node_id().id() != node.id().id())) {
    return;
  }

  for (int i = 0; i < node.links_size(); ++i) {
    auto link = node.links(i);
    // 判断link是否相同
    if ((lk.upstream_node_id().region() == link.upstream_node_id().region()) &&
        (lk.upstream_node_id().id() == link.upstream_node_id().id())) {
      // 4 表示所有转向, 路径为当前的LINK
      if (4 == lk.maneuver()) {
        v2xpb::asn::RsiReferencePath info;
        CalcLinkAffectPath(lk.length(), link, &info);
        affect_paths->emplace_back(info);
        continue;
      }

      // 寻找符合转向要求的lane
      for (int j = 0; j < link.lanes_size(); ++j) {
        auto lane = link.lanes(i);
        bool is_find = false;

        auto maneuver_conf =
            static_cast<v2xpb::asn::MapLane::AllowedManeuver>(lk.maneuver());
        for (auto k = 0; k < lane.maneuvers_size(); ++k) {
          if (maneuver_conf == lane.maneuvers(k)) {
            is_find = true;
            break;
          }
        }

        // 将lane上对应点的坐标填上去
        if (is_find) {
          v2xpb::asn::RsiReferencePath info;
          double start_x = 0.0;
          double start_y = 0.0;
          auto stop_pos_idx = lane.positions_size() - 1;
          double affect_path_len = 0;
          start_x = lane.positions(stop_pos_idx).xyz().x();
          start_y = lane.positions(stop_pos_idx).xyz().y();
          for (int i = stop_pos_idx; i > 0; --i) {
            airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                                link.positions(i).xyz().y());
            airos::perception::usecase::Vec2d e(
                lane.positions(i - 1).xyz().x(),
                lane.positions(i - 1).xyz().y());
            airos::perception::usecase::Segment2d line(s, e);
            affect_path_len += line.Length();
            APP_LOG_INFO << "current index: " << i - 1
                     << ", current affect path len: " << affect_path_len;
            if (affect_path_len >= lk.length() || i - 1 == 0) {
              auto point_end = info.add_points();
              point_end->mutable_xyz()->set_x(e.X());
              point_end->mutable_xyz()->set_y(e.Y());
              point_end->mutable_xyz()->set_z(0.0);
              point_end->mutable_xyz()->set_zone(zone_);
              break;
            }
          }

          auto point_start = info.add_points();
          point_start->mutable_xyz()->set_x(start_x);
          point_start->mutable_xyz()->set_y(start_y);
          point_start->mutable_xyz()->set_z(0.0);
          point_start->mutable_xyz()->set_zone(zone_);
          info.set_radius(lane.width());
          affect_paths->emplace_back(info);
        }
      }
    }
  }
}

/**
 * @brief 判断事件发生是否符合配置文件的特殊配置
 * @param mec_event_type
 * @param ev_point
 * @param special_event_conf
 * @return
 */
bool RsiAffectPath::IsSpecialEventMatched(
    const int mec_event_type, const airos::perception::usecase::Vec2d ev_point,
    const SpecialEventDetailPtr special_event_conf) {
  APP_LOG_INFO << "Start match event in special conf";
  bool istTypeFind = false;
  for (int i = 0; i < special_event_conf->event_type_mec_list().event_type_mec_size(); ++i) {
    auto event_type_mec_conf =
        special_event_conf->event_type_mec_list().event_type_mec(i);
    if (static_cast<int>(event_type_mec_conf) == mec_event_type) {
      APP_LOG_INFO << "Find event type in special conf";
      istTypeFind = true;
    }
  }

  // 判断事件是否发生在配置的ROI区域内部
  if (istTypeFind) {
    std::vector<airos::perception::usecase::Vec2d> points;
    for (int i = 0;
         i < special_event_conf->region_of_interest().position_size(); ++i) {
      auto position = special_event_conf->region_of_interest().position(i);
      airos::perception::usecase::Vec2d point;
      APP_LOG_INFO <<  std::fixed << "Pos lat:  " << position.latitude() << ", lon: " << position.longitude();
      ConvertToUtm(position.latitude(), position.longitude(), point);
      APP_LOG_INFO << point.X() << " , " << point.Y();
      points.push_back(point);
    }

    APP_LOG_INFO << "point list: " << points.size();
    airos::perception::usecase::Polygon2d polygon(points);
    if (polygon.IsPointIn(ev_point)) {
      APP_LOG_INFO << "Event match in conf roi";
      return true;
    }
  }
  return false;
}

bool RsiAffectPath::GetAffectPath(
    const RteEventDetailPtr ev_detail,
    const airos::perception::usecase::Vec2d ev_point,
    const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  if (map.nodes_size() <= 0) {
    APP_LOG_WARN << "map node size <= 0";
    return false;
  }
  zone_ = map.nodes(0).position().xyz().zone();
  stop_points_.clear();
  if (!GetStopPoints(map, &stop_points_)) {
    APP_LOG_WARN << "no stop point";
    return false;
  }

  // 超速事件特殊处理
  if (901 == ev_detail->event_type_id()) {
    APP_LOG_INFO << "The speeding incident occurred on the road and will be "
                "handled with special measures. "
             << "event_type_id: " << ev_detail->event_type_id();
    // 超速事件，特殊处理
    return CalcAffectPathOverSpeed(ev_detail, ev_point, map, affect_paths);
  }

  // 紧急车辆做特殊处理
  if (418 == ev_detail->event_type_id()) {
    APP_LOG_INFO << "The emergency vehicle on the road and will be handled with "
                "special measures. "
             << "event_type_id: " << ev_detail->event_type_id();
    // 紧急车辆，特殊处理（处理逻辑与超速事件相同）
    return CalcAffectPathOverSpeed(ev_detail, ev_point, map, affect_paths);
  }

  // 行人闯入机动车道特殊处理
  if (405 == ev_detail->event_type_id()) {
    APP_LOG_INFO << "The pedestrian entering the motor vehicle lane and will be "
                "handled with special measures. "
             << "event_type_id: " << ev_detail->event_type_id();
    // 行人闯入机动车道，特殊处理
    return CalcAffectPathPedestrianIntrusion(ev_detail, ev_point, map,
                                             affect_paths);
  }

  // 信号灯故障 特殊处理
  if (410 == ev_detail->event_type_id()) {
    APP_LOG_INFO << "traffic light malfunction and will be handled with special "
                "measures, "
             << "event_type_id: " << ev_detail->event_type_id();
    // 计算路口中心和所有link的影响路径
    return CalcAffectPath(ev_detail, map, affect_paths);
  }

  // 非机动车闯红灯和行人闯红灯事件特殊处理
  if (416 == ev_detail->event_type_id() || 415 == ev_detail->event_type_id()) {
    APP_LOG_INFO << "Incidents of non-motorized vehicles or pedestrians running "
                "red lights and will be handled with special measures,  "
             << "event_type_id: " << ev_detail->event_type_id();
    return CalcAffectPathPedestrianRunningRedLight(ev_detail, ev_point, map,
                                                   affect_paths);
  }

  if (IsInJunction(ev_point)) {
    APP_LOG_INFO << "calc in junction affect path";
    // 计算路口中心和所有link的影响路径
    return CalcAffectPath(ev_detail, map, affect_paths);
  }
  //  计算事件点所在link的影响路径
  v2xpb::asn::MapLink link;
  int lane_idx = 0;
  if (!GetNearstLink(ev_point, map, &link, &lane_idx)) {
    APP_LOG_WARN << "can't find nearst link";
    return false;
  }
  v2xpb::asn::RsiReferencePath info;
  if (!CalcAffectPath(ev_detail, ev_point, link, false, &info)) {
    APP_LOG_WARN << "calc affect path failed";
    return false;
  }
  affect_paths->emplace_back(info);
  return affect_paths->size() > 0;
}

bool RsiAffectPath::GetStopPoints(
    const v2xpb::asn::Map& map,
    std::vector<airos::perception::usecase::Vec2d>* points) {
  auto node = map.nodes(0);
  // get all stop point
  for (int i = 0; i < node.links_size(); ++i) {
    auto link = node.links(i);
    for (int j = 0; j < link.lanes_size(); ++j) {
      auto lane = link.lanes(j);
      int stop_point_idx = lane.positions_size() - 1;
      if (stop_point_idx < 0) {
        continue;
      }
      auto stop_point = lane.positions(stop_point_idx);
      points->emplace_back(stop_point.xyz().x(), stop_point.xyz().y());
    }
  }
  return points->size() > 0;
}

bool RsiAffectPath::IsInJunction(
    const airos::perception::usecase::Vec2d point) {
  airos::perception::usecase::Polygon2d polygon(stop_points_);
  return polygon.IsPointIn(point);
}

bool RsiAffectPath::CalcAffectPath(
    const RteEventDetailPtr ev_detail, const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  auto node = map.nodes(0);
  auto center_pos = node.position().xyz();

// 事件范围，不包含路口中心的区域
#if 0
    v2xpb::asn::RsiReferencePath info;
    if (!CalcAffectPath({center_pos.x(), center_pos.y()}, &info)) {
      return false;
    }
    affect_paths->emplace_back(info);
#endif

  for (int i = 0; i < node.links_size(); ++i) {
    auto link = node.links(i);
    airos::perception::usecase::Vec2d ev_point(-1, -1);
    v2xpb::asn::RsiReferencePath info;
    if (!CalcAffectPath(ev_detail, ev_point, link, true, &info)) {
      continue;
    }
    affect_paths->emplace_back(info);
  }
  return affect_paths->size() > 0;
}

bool RsiAffectPath::CalcAffectPath(
    const airos::perception::usecase::Vec2d center_point,
    v2xpb::asn::RsiReferencePath* affect_path) {
  double max_len = 0;
  for (unsigned int i = 0; i < stop_points_.size(); ++i) {
    airos::perception::usecase::Segment2d line(center_point, stop_points_[i]);
    if (line.Length() > max_len) {
      max_len = line.Length();
    }
  }
  auto point_start = affect_path->add_points();
  point_start->mutable_xyz()->set_x(center_point.X());
  point_start->mutable_xyz()->set_y(center_point.Y());
  point_start->mutable_xyz()->set_z(0.0);
  point_start->mutable_xyz()->set_zone(zone_);
  auto point_end = affect_path->add_points();
  point_end->mutable_xyz()->set_x(center_point.X());
  point_end->mutable_xyz()->set_y(center_point.Y());
  point_end->mutable_xyz()->set_z(0.0);
  point_end->mutable_xyz()->set_zone(zone_);
  affect_path->set_radius(max_len);
  return true;
}

bool RsiAffectPath::GetNearstLink(const airos::perception::usecase::Vec2d point,
                                  const v2xpb::asn::Map& map,
                                  v2xpb::asn::MapLink* ret_link, int* idx) {
  // calc min distance to point
  // return link pointer
  double min_len = DBL_MAX;
  bool is_find = false;
  auto& node = map.nodes(0);
  for (int i = 0; i < node.links_size(); ++i) {
    auto& link = node.links(i);
    for (int j = 0; j < link.lanes_size(); ++j) {
      auto& lane = link.lanes(j);
      for (int k = 0; k < lane.positions_size() - 1; ++k) {
        airos::perception::usecase::Vec2d s(lane.positions(k).xyz().x(),
                                            lane.positions(k).xyz().y());
        airos::perception::usecase::Vec2d e(lane.positions(k + 1).xyz().x(),
                                            lane.positions(k + 1).xyz().y());
        airos::perception::usecase::Segment2d line(s, e);
        double len = line.DistanceTo(point);
        if (len < min_len && len <= (lane.width() / 2)) {
          min_len = len;
          ret_link->CopyFrom(link);
          *idx = j;
          is_find = true;
        }
      }
    }
  }
  return is_find;
}

bool RsiAffectPath::CalcAffectPath(
    const RteEventDetailPtr ev_detail,
    const airos::perception::usecase::Vec2d ev_point,
    const v2xpb::asn::MapLink& link, bool in_centor,
    v2xpb::asn::RsiReferencePath* affect_path) {
  APP_LOG_INFO << "current link name " << link.name() << ", lane_size "
           << link.lanes_size() << ", link width " << link.width();
  // get affect path lane
  int affect_lane_idx = -1;
  double affect_path_radius = 0.0;

  affect_lane_idx = (link.lanes_size() - 1) / 2;
  affect_path_radius = link.width() / 2;

  auto& lane = link.lanes(affect_lane_idx);
  // calc start point
  double start_x = 0.0;
  double start_y = 0.0;
  auto stop_pos_idx = lane.positions_size() - 1;
  int next_idx = lane.positions_size() - 1;
  if (ev_point.X() < 0 && ev_point.Y() < 0) {
    APP_LOG_INFO << "calc affect path startwith stop point";
    start_x = lane.positions(stop_pos_idx).xyz().x();
    start_y = lane.positions(stop_pos_idx).xyz().y();
  } else {
    APP_LOG_INFO << "calc affect path startwith ev point";
    airos::perception::usecase::Vec2d neatest_point;
    GetNearestPoint(lane, ev_point, &neatest_point, &next_idx);
    start_x = neatest_point.X();
    start_y = neatest_point.Y();
  }
  APP_LOG_INFO << "nearly index: " << next_idx << ", stop index: " << stop_pos_idx;

  // calc 100m pointer
  double affect_path_len = 0;
  airos::perception::usecase::Vec2d s(start_x, start_y);
  airos::perception::usecase::Vec2d e(lane.positions(next_idx).xyz().x(),
                                      lane.positions(next_idx).xyz().y());
  airos::perception::usecase::Segment2d line(s, e);
  affect_path_len += line.Length();
  for (int i = next_idx; i > 0; --i) {
    airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                        link.positions(i).xyz().y());
    airos::perception::usecase::Vec2d e(lane.positions(i - 1).xyz().x(),
                                        lane.positions(i - 1).xyz().y());
    airos::perception::usecase::Segment2d line(s, e);
    affect_path_len += line.Length();
    APP_LOG_INFO << "current index: " << i - 1
             << ", current affect_path_len: " << affect_path_len;
    if (affect_path_len >= ev_detail->impact_dist() || i - 1 == 0) {
      auto point_end = affect_path->add_points();
      point_end->mutable_xyz()->set_x(e.X());
      point_end->mutable_xyz()->set_y(e.Y());
      point_end->mutable_xyz()->set_z(0.0);
      point_end->mutable_xyz()->set_zone(zone_);
      break;
    }
  }

  APP_LOG_INFO << "current index: " << next_idx
           << ", current affect_path_len: " << affect_path_len
           << ", required impact dist by conf: " << ev_detail->impact_dist();

  // 防止一个点发不出去
  if (0 == next_idx) {
    auto point_next = affect_path->add_points();
    point_next->mutable_xyz()->set_x(lane.positions(next_idx).xyz().x());
    point_next->mutable_xyz()->set_y(lane.positions(next_idx).xyz().y());
    point_next->mutable_xyz()->set_z(0.0);
    point_next->mutable_xyz()->set_zone(zone_);
  }

  auto point_start = affect_path->add_points();
  point_start->mutable_xyz()->set_x(start_x);
  point_start->mutable_xyz()->set_y(start_y);
  point_start->mutable_xyz()->set_z(0.0);
  point_start->mutable_xyz()->set_zone(zone_);

  affect_path->set_radius(affect_path_radius);
  return true;
}

bool RsiAffectPath::GetNearestPoint(
    const v2xpb::asn::MapLane& lane,
    const airos::perception::usecase::Vec2d ev_point,
    airos::perception::usecase::Vec2d* neatest_point, int* next_idx) {
  double min_len = DBL_MAX;
  for (int i = lane.positions_size() - 1; i > 0; --i) {
    airos::perception::usecase::Vec2d s(lane.positions(i).xyz().x(),
                                        lane.positions(i).xyz().y());
    airos::perception::usecase::Vec2d e(lane.positions(i - 1).xyz().x(),
                                        lane.positions(i - 1).xyz().y());
    airos::perception::usecase::Segment2d line(s, e);
    airos::perception::usecase::Vec2d tmp;
    double len = line.DistanceTo(ev_point, &tmp);
    if (len < min_len) {
      neatest_point->SetX(tmp.X());
      neatest_point->SetY(tmp.Y());
      min_len = len;
      *next_idx = i - 1;
    }
  }
  return true;
}

/**
 * @brief 超速及类似事件特殊处理
 * @param ev_detail
 * @param ev_point
 * @param map
 * @param affect_paths
 * @return
 */
bool RsiAffectPath::CalcAffectPathOverSpeed(
    const RteEventDetailPtr ev_detail,
    const airos::perception::usecase::Vec2d ev_point,
    const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  // 发生在路口
  if (IsInJunction(ev_point)) {
    APP_LOG_INFO << "calc in junction affect path";
    // 计算路口中心和所有link的影响路径
    return CalcAffectPath(ev_detail, map, affect_paths);
  }

  // 发生在道路上
  v2xpb::asn::MapLink link;
  int lane_idx = 0;
  if (!GetNearstLink(ev_point, map, &link, &lane_idx)) {
    APP_LOG_WARN << "can't find nearst link";
    return false;
  }
  APP_LOG_INFO << "current link name " << link.name() << ", lane_size "
           << link.lanes_size() << ", link width " << link.width();
  // get affect path lane
  int affect_lane_idx = -1;
  double affect_path_radius = 0.0;

  affect_lane_idx = (link.lanes_size() - 1) / 2;
  affect_path_radius = link.width() / 2;

  auto& lane = link.lanes(affect_lane_idx);
  // calc start point
  double start_x = 0.0;
  double start_y = 0.0;
  auto stop_pos_idx = lane.positions_size() - 1;
  int next_idx = lane.positions_size() - 1;

  v2xpb::asn::RsiReferencePath affect_path;
  APP_LOG_INFO << "calc affect path startwith ev point";
  airos::perception::usecase::Vec2d neatest_point;
  GetNearestPoint(lane, ev_point, &neatest_point, &next_idx);
  APP_LOG_INFO << "nearly index: " << next_idx << ", stop index: " << stop_pos_idx;
  next_idx += 1;
  start_x = neatest_point.X();
  start_y = neatest_point.Y();

  auto point_start = affect_path.add_points();
  point_start->mutable_xyz()->set_x(start_x);
  point_start->mutable_xyz()->set_y(start_y);
  point_start->mutable_xyz()->set_z(0.0);
  point_start->mutable_xyz()->set_zone(zone_);
  affect_path.set_radius(affect_path_radius);

  if (next_idx >= stop_pos_idx) {
    auto point_stop = affect_path.add_points();
    point_stop->mutable_xyz()->set_x(lane.positions(stop_pos_idx).xyz().x());
    point_stop->mutable_xyz()->set_y(lane.positions(stop_pos_idx).xyz().y());
    point_stop->mutable_xyz()->set_z(0.0);
    point_stop->mutable_xyz()->set_zone(zone_);
    affect_path.set_radius(affect_path_radius);
    affect_paths->emplace_back(affect_path);
    return true;
  }

  // calc sd m pointer
  double affect_path_len = 0;
  airos::perception::usecase::Vec2d s(start_x, start_y);
  airos::perception::usecase::Vec2d e(lane.positions(next_idx).xyz().x(),
                                      lane.positions(next_idx).xyz().y());
  airos::perception::usecase::Segment2d line(s, e);
  affect_path_len += line.Length();
  for (int i = next_idx; i < stop_pos_idx; ++i) {
    airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                        link.positions(i).xyz().y());
    airos::perception::usecase::Vec2d e(lane.positions(i + 1).xyz().x(),
                                        lane.positions(i + 1).xyz().y());
    airos::perception::usecase::Segment2d line(s, e);
    affect_path_len += line.Length();
    APP_LOG_INFO << "current index: " << i + 1
             << ", current affect_path_len: " << affect_path_len;

    if (affect_path_len >= ev_detail->impact_dist() || i + 1 == stop_pos_idx) {
      next_idx = i + 1;
      auto point_end = affect_path.add_points();
      point_end->mutable_xyz()->set_x(e.X());
      point_end->mutable_xyz()->set_y(e.Y());
      point_end->mutable_xyz()->set_z(0.0);
      point_end->mutable_xyz()->set_zone(zone_);
      break;
    }
  }
  affect_path.set_radius(affect_path_radius);
  affect_paths->emplace_back(affect_path);

  APP_LOG_INFO << "current index: " << next_idx
           << ", current affect_path_len: " << affect_path_len
           << ", required impact dist by conf: " << ev_detail->impact_dist();
  // 距离路口小于SD 米, 影响其他车到其他进入路口的车道
  if (affect_path_len <= ev_detail->impact_dist()) {
    auto node = map.nodes(0);
    for (int i = 0; i < node.links_size(); ++i) {
      auto other_link = node.links(i);
      if ((other_link.upstream_node_id().region() ==
           link.upstream_node_id().region()) &&
          (other_link.upstream_node_id().id() ==
           link.upstream_node_id().id())) {
        APP_LOG_INFO << "Ingore current link, link name: " << other_link.name();
        continue;
      }
      v2xpb::asn::RsiReferencePath info;
      if (!CalcAffectPath(ev_detail, ev_point, other_link, true, &info)) {
        continue;
      }
      affect_paths->emplace_back(info);
    }
  }
  return true;
};

/**
 * @brief 行人闯入机动车道
 * @param ev_detail
 * @param ev_point
 * @param map
 * @param affect_paths
 * @return
 */
bool RsiAffectPath::CalcAffectPathPedestrianIntrusion(
    const RteEventDetailPtr ev_detail,
    const airos::perception::usecase::Vec2d ev_point,
    const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  // 发生在路口
  if (IsInJunction(ev_point)) {
    APP_LOG_INFO << "Incident occurred at the intersection";
    // 计算路口中心和所有link的影响路径
    return CalcAffectPath(ev_detail, map, affect_paths);
  }

  // 发生在道路上
  v2xpb::asn::MapLink link;
  int lane_idx = 0;
  if (!GetNearstLink(ev_point, map, &link, &lane_idx)) {
    APP_LOG_WARN << "can't find nearst link";
    return false;
  }
  APP_LOG_INFO << "current link name " << link.name() << ", lane_size "
           << link.lanes_size() << ", link width " << link.width();

  APP_LOG_INFO << "Calculate the affected path on the current link";
  v2xpb::asn::RsiReferencePath info;
  if (!CalcAffectPath(ev_detail, ev_point, link, false, &info)) {
    APP_LOG_WARN << "calc affect path in current link failed";
  } else {
    affect_paths->emplace_back(info);
  }

  APP_LOG_INFO << "Calculate the affected path of the link leading to \
    the opposite direction of the current link towards the exit direction.";

  calculateExitDirectionLinkAffectedPath(ev_detail, map,
                                         link.upstream_node_id(), affect_paths);

  if (affect_paths->empty()) {
    return false;
  }
  return true;
};

/**
 * @brief 计算当前node通往某条道路的影响路径
 * @param ev_detail 事件的配置参数
 * @param map 当前Node的地图
 * @param nodeId 通往Node的Id
 * @param affect_paths 影响路径
 * @return
 */
bool RsiAffectPath::calculateExitDirectionLinkAffectedPath(
    const RteEventDetailPtr ev_detail, const v2xpb::asn::Map& map,
    const v2xpb::asn::MapNode::ID nodeId,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  auto node = map.nodes(0);
  for (int i = 0; i < node.links_size(); ++i) {
    auto link = node.links(i);
    if ((link.upstream_node_id().region() == nodeId.region()) &&
        (link.upstream_node_id().id() == nodeId.id())) {
      APP_LOG_INFO << "Ingore link, link name: " << link.name();
      continue;
    }
    v2xpb::asn::RsiReferencePath info;
    for (int j = 0; j < link.lanes_size(); ++j) {
      auto& lane = link.lanes(j);
      bool isfind = false;
      for (int k = 0; k < lane.connections_size(); ++k) {
        APP_LOG_INFO << "search connections in link " << link.name() << ", lane id " << lane.id();
        if (lane.connections(k).remote_node_id().region() == nodeId.region() &&
            lane.connections(k).remote_node_id().id() == nodeId.id()) {
          isfind = true;
          APP_LOG_INFO << "find in link " << link.name() << ", lane id " << lane.id();
          break;
        }
      }
      if (isfind) {
        if (0 == lane.positions_size()) {
          APP_LOG_INFO << "No point in line, ingore";
          continue;
        }
        double start_x = 0.0;
        double start_y = 0.0;
        auto stop_pos_idx = lane.positions_size() - 1;
        double affect_path_len = 0;
        start_x = lane.positions(stop_pos_idx).xyz().x();
        start_y = lane.positions(stop_pos_idx).xyz().y();
        for (int i = stop_pos_idx; i > 0; --i) {
          airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                              link.positions(i).xyz().y());
          airos::perception::usecase::Vec2d e(lane.positions(i - 1).xyz().x(),
                                              lane.positions(i - 1).xyz().y());
          airos::perception::usecase::Segment2d line(s, e);
          affect_path_len += line.Length();
          APP_LOG_INFO << "current index: " << i - 1
                   << ", current affect_path_len: " << affect_path_len;
          if (affect_path_len >= ev_detail->impact_dist() || i - 1 == 0) {
            auto point_end = info.add_points();
            point_end->mutable_xyz()->set_x(e.X());
            point_end->mutable_xyz()->set_y(e.Y());
            point_end->mutable_xyz()->set_z(0.0);
            point_end->mutable_xyz()->set_zone(zone_);
            break;
          }
        }
        auto point_start = info.add_points();
        point_start->mutable_xyz()->set_x(start_x);
        point_start->mutable_xyz()->set_y(start_y);
        point_start->mutable_xyz()->set_z(0.0);
        point_start->mutable_xyz()->set_zone(zone_);
        info.set_radius(lane.width());
        affect_paths->emplace_back(info);
      }
    }
  }
  return true;
}

/**
 * @brief 行人或者非机动车闯红灯
 * @param ev_detail
 * @param ev_point
 * @param map
 * @param affect_paths
 * @return
 */
bool RsiAffectPath::CalcAffectPathPedestrianRunningRedLight(
    const RteEventDetailPtr ev_detail,
    const airos::perception::usecase::Vec2d ev_point,
    const v2xpb::asn::Map& map,
    std::vector<v2xpb::asn::RsiReferencePath>* affect_paths) {
  // 闯红灯事件, 只能发生在路口里面
  if (!IsInJunction(ev_point)) {
    APP_LOG_INFO << "Incident occurred not at the intersection, ignore";
    return false;
  }

  v2xpb::asn::MapLink min_link;
  double min_dist = DBL_MAX;
  // 寻找事件发生的位置, 离哪个Link上停止线的垂直距离最近
  auto node = map.nodes(0);
  for (int i = 0; i < node.links_size(); ++i) {
    auto link = node.links(i);
    double dist = DBL_MAX;
    if (0 == link.lanes_size()) {
      continue;
    }
    // 如果只有一个车道，直接计算事件发生位置，到车道最后一个点的距离
    else if (1 == link.lanes_size()) {
      auto lane = link.lanes(0);
      airos::perception::usecase::Vec2d s(
          lane.positions(lane.positions_size() - 1).xyz().x(),
          lane.positions(lane.positions_size() - 1).xyz().y());

      airos::perception::usecase::Segment2d line(s, ev_point);
      dist = line.Length();

    }
    // 计算点到link停止线的垂直距离
    else if (link.lanes_size() > 1) {
      auto lane1 = link.lanes(0);
      auto lane2 = link.lanes(link.lanes_size() - 1);

      airos::perception::usecase::Vec2d s(
          lane1.positions(lane1.positions_size() - 1).xyz().x(),
          lane1.positions(lane1.positions_size() - 1).xyz().y());

      airos::perception::usecase::Vec2d e(
          lane2.positions(lane2.positions_size() - 1).xyz().x(),
          lane2.positions(lane2.positions_size() - 1).xyz().y());

      airos::perception::usecase::Segment2d line(s, e);
      dist = line.PerpendicularDistanceTo(ev_point);
    }
    
    APP_LOG_INFO << "current link name " << link.name() << "dist:  " << dist;
    
    if (dist < min_dist) {
      min_dist = dist;
      min_link = link;
    }
  }

  if (min_dist < DBL_MAX) {
    APP_LOG_INFO << "Find near link, dist to link stopline  " << min_dist;
  } else {
    APP_LOG_INFO << "Not find near link";
    return false;
  }

  APP_LOG_INFO << "current link name " << min_link.name() << ", lane_size "
           << min_link.lanes_size() << ", link width " << min_link.width();

  APP_LOG_INFO << "upstream_node_id: regionId "
           << min_link.upstream_node_id().region() << ", Id "
           << min_link.upstream_node_id().id();

  airos::perception::usecase::Vec2d ev_point_temp(-1, -1);
  APP_LOG_INFO << "Calculate the affected path on the nearly link";

  v2xpb::asn::RsiReferencePath info;
  if (!CalcAffectPath(ev_detail, ev_point_temp, min_link, false, &info)) {
    APP_LOG_WARN << "calc affect path failed";
  } else {
    affect_paths->emplace_back(info);
  }

  APP_LOG_INFO << "Calculate the affected path of the link leading to \
    the opposite direction of the current link towards the exit direction.";

  calculateExitDirectionLinkAffectedPath(
      ev_detail, map, min_link.upstream_node_id(), affect_paths);

  if (affect_paths->empty()) {
    return false;
  }
  return true;
};

/**
 * @brief 经纬度转UTM坐标
 * @param lat
 * @param lon
 * @param ponit
 */
void RsiAffectPath::ConvertToUtm(double lat, double lon,
                                 airos::perception::usecase::Vec2d& point) {
  // 经纬度坐标转UTM坐标
  int zone;
  bool dummy;
  double x, y;
  GeographicLib::UTMUPS::Forward(lat, lon, zone, dummy, x, y);

  point.SetX(x);
  point.SetY(y);
}

/**
 * @brief 计算某条LINK的影响路径
 * @param link
 * @param affect_path
 * @return
 */
bool RsiAffectPath::CalcLinkAffectPath(
    const double impact_dist, const v2xpb::asn::MapLink& link,
    v2xpb::asn::RsiReferencePath* affect_path) {
  APP_LOG_INFO << "current link name " << link.name() << ", lane_size "
           << link.lanes_size() << ", link width " << link.width();
  // get affect path lane
  int affect_lane_idx = -1;
  double affect_path_radius = 0.0;

  affect_lane_idx = (link.lanes_size() - 1) / 2;
  affect_path_radius = link.width() / 2;

  auto& lane = link.lanes(affect_lane_idx);
  // calc start point
  double start_x = 0.0;
  double start_y = 0.0;
  auto stop_pos_idx = lane.positions_size() - 1;
  int next_idx = lane.positions_size() - 1;

  start_x = lane.positions(stop_pos_idx).xyz().x();
  start_y = lane.positions(stop_pos_idx).xyz().y();

  APP_LOG_INFO << "nearly index: " << next_idx << ", stop index: " << stop_pos_idx;

  // calc 100m pointer
  double affect_path_len = 0;
  airos::perception::usecase::Vec2d s(start_x, start_y);
  airos::perception::usecase::Vec2d e(lane.positions(next_idx).xyz().x(),
                                      lane.positions(next_idx).xyz().y());
  airos::perception::usecase::Segment2d line(s, e);
  affect_path_len += line.Length();
  for (int i = next_idx; i > 0; --i) {
    airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                        link.positions(i).xyz().y());
    airos::perception::usecase::Vec2d e(lane.positions(i - 1).xyz().x(),
                                        lane.positions(i - 1).xyz().y());
    airos::perception::usecase::Segment2d line(s, e);
    affect_path_len += line.Length();
    APP_LOG_INFO << "current index: " << i - 1
             << ", current affect_path_len: " << affect_path_len;
    if (affect_path_len >= impact_dist || i - 1 == 0) {
      auto point_end = affect_path->add_points();
      point_end->mutable_xyz()->set_x(e.X());
      point_end->mutable_xyz()->set_y(e.Y());
      point_end->mutable_xyz()->set_z(0.0);
      point_end->mutable_xyz()->set_zone(zone_);
      break;
    }
  }

  APP_LOG_INFO << "current index: " << next_idx
           << ", current affect_path_len: " << affect_path_len
           << ", required impact dist by conf: " << impact_dist;

  // 防止一个点发不出去
  if (0 == next_idx) {
    auto point_next = affect_path->add_points();
    point_next->mutable_xyz()->set_x(lane.positions(next_idx).xyz().x());
    point_next->mutable_xyz()->set_y(lane.positions(next_idx).xyz().y());
    point_next->mutable_xyz()->set_z(0.0);
    point_next->mutable_xyz()->set_zone(zone_);
  }

  auto point_start = affect_path->add_points();
  point_start->mutable_xyz()->set_x(start_x);
  point_start->mutable_xyz()->set_y(start_y);
  point_start->mutable_xyz()->set_z(0.0);
  point_start->mutable_xyz()->set_zone(zone_);

  affect_path->set_radius(affect_path_radius);
  return true;
}
}  // namespace app
}  // namespace airos
