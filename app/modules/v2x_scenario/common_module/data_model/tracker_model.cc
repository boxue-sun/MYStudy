/*********************************************************************************
 * @file		tracker_model.cc
 * @brief		tracker_model
 * @details		tracker_model
 * @author		ChangXuhui
 * @date		2024/4/1
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/1  ChangXuhui      1.0            ————                 ———— 
 *                                                             
 * @endverbatim
 ********************************************************************************/

#include "tracker_model.h"

#include <cfloat>

#include "base/common/log.h"

namespace airos {
namespace app {

TrackerModel::TrackerModel()
    : local_maps_(LocalMaps::GetInstance().GetLocalMap()) {
  stop_points_.clear();
  if (!GetStopPoints(&stop_points_)) {
    APP_LOG_ERROR << "No stop point";
  }
  zone_ = local_maps_.nodes(0).position().xyz().zone();
}

void TrackerModel::UpdateTracker(const PositionModel& pos) {
  ResetTrackerData();
  auto node = local_maps_.nodes(0);
  if (pos.GetPositionXYZ().zone != zone_) {
    APP_LOG_INFO << "Vehilce is in zone " << pos.GetPositionXYZ().zone
             << ", map in zone " << zone_ << ", different match fail";
    return;
  }

  airos::perception::usecase::Vec2d point;
  point.SetX(pos.GetPositionXYZ().x);
  point.SetY(pos.GetPositionXYZ().y);
  // 判断是否进入路口中心区域
  if (IsInJunction(point)) {
    APP_LOG_INFO << "The vehicle is located in the central area of the intersection.";
    node_id_ = node.id();
    is_at_across_ = true;
    return;
  }
  double heading = pos.GetHeading();
  MatchVehicleInMap(point, heading);
}

bool TrackerModel::GetStopPoints(
    std::vector<airos::perception::usecase::Vec2d>* points) {
  auto node = local_maps_.nodes(0);
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

bool TrackerModel::IsInJunction(const airos::perception::usecase::Vec2d point) {
  airos::perception::usecase::Polygon2d polygon(stop_points_);
  return polygon.IsPointIn(point);
}

void TrackerModel::ResetTrackerData() {
  node_id_.set_region(-1);
  node_id_.set_id(-1);
  upstream_node_id_.set_region(-1);
  upstream_node_id_.set_id(-1);
  lane_id_ = -1;
  dist_to_node_ = 0;
  is_at_across_ = false;
  is_allowed_maneuver_straight_ = false;
  is_allowed_maneuver_left_ = false;
  is_allowed_maneuver_right_ = false;
  is_allowed_maneuver_u_turn_ = false;  
}

bool TrackerModel::MatchVehicleInMap(
    const airos::perception::usecase::Vec2d point, double heading) {
  double min_len = DBL_MAX;

  double link_idx = -1;  // link的序号
  double lane_idx = -1;  // lane的序号
  double seg_idx = -1;   // lane上点的序号

  bool is_find = false;
  auto& node = local_maps_.nodes(0);
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

        // APP_LOG_INFO << line.HeadingNorth();
        // APP_LOG_INFO << heading;
        // // 判断角度是否满足要求
        // if (std::fabs(line.HeadingNorth() - heading) > 45) {
        //   continue;
        // }

        double len = line.DistanceTo(point);
        if (len < min_len && len <= (lane.width() / 2)) {
          min_len = len;
          link_idx = i;
          lane_idx = j;
          seg_idx = k + 1;
          is_find = true;
        }
      }
    }
  }

  if (is_find) {
    node_id_ = node.id();
    auto& link = node.links(link_idx);
    upstream_node_id_ = link.upstream_node_id();
    auto lane = link.lanes(lane_idx);
    lane_id_ = lane.id();
    airos::perception::usecase::Vec2d pos(lane.positions(seg_idx).xyz().x(),
                                          lane.positions(seg_idx).xyz().y());
    airos::perception::usecase::Segment2d line(point, pos);

    dist_to_node_ += line.Length();
    for (int i = seg_idx; i < lane.positions_size() - 2; ++i) {
      airos::perception::usecase::Vec2d s(link.positions(i).xyz().x(),
                                          link.positions(i).xyz().y());
      airos::perception::usecase::Vec2d e(lane.positions(i + 1).xyz().x(),
                                          lane.positions(i + 1).xyz().y());

      airos::perception::usecase::Segment2d l(point, pos);
      dist_to_node_ += l.Length();
    }

    for (int i = 0; i < lane.maneuvers_size(); ++i) {
      switch (lane.maneuvers(i))
      {
      case v2xpb::asn::MapLane::STRAIGHT:
        is_allowed_maneuver_straight_ = true;
        break;
      case v2xpb::asn::MapLane::LEFT:
        is_allowed_maneuver_left_ = true;
        break;
      case v2xpb::asn::MapLane::RIGHT:
        is_allowed_maneuver_right_ = true;
        break;
      case v2xpb::asn::MapLane::U_TURN:
        is_allowed_maneuver_u_turn_ = true;
        break;
      default:
        break;
      }
    }

    std::string node_id_str =
        std::to_string(node_id_.id()) + "_" + std::to_string(node_id_.region());

    std::string upstream_node_id_str =
        std::to_string(upstream_node_id_.id()) + "_" +
        std::to_string(upstream_node_id_.region());

    APP_LOG_INFO << "Match successful node: " << node_id_str
             << " ,upstream node: " << upstream_node_id_str
             << ", lane id: " << lane_id_ << ", dist to node: " << dist_to_node_;
  } else {
    APP_LOG_INFO << "Match fail";
  }
  return is_find;
}

}  // namespace app
}  // namespace airos
