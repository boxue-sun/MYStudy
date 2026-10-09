
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

#include "app/modules/v2x_application/usecase/map/map_info.h"

#include <map>
#include <string>
#include <vector>

#include "base/common/singleton.h"
#include "base/io/protobuf_util.h"
#include "pugixml.hpp"

namespace airos {
namespace perception {
namespace usecase {

using airos::perception::usecase::Vec2d;

double Str2Double(const std::string &str) {
  std::stringstream ss(str);
  double data;
  ss >> data;

  return data;
}
int Str2Int(const std::string &str) {
  std::stringstream ss(str);
  int data;
  ss >> data;

  return data;
}

std::atomic<bool> MapInfo::g_real_light_flag(false);
std::map<int, RealLightInfo> MapInfo::g_real_light_state;
std::mutex MapInfo::g_real_light_lock;

void SetTrafficLightState(
    const std::shared_ptr<const ::airos::trafficlight::TrafficLightServiceData>
        &light_states) {
  MapInfo::g_real_light_flag = true;
  std::lock_guard<std::mutex> g(MapInfo::g_real_light_lock);
  const auto &traffic_light = light_states->traffic_light();
  for (auto &info : traffic_light.phase()) {
    if (info.has_light_id() && info.has_light_status() &&
        info.has_count_down() && info.step_info_list().size() > 0) {
      if (info.step_info_list(0).has_duration()) {
        int cur_id     = info.light_id();
        auto state     = info.light_status();
        auto state_str = std::to_string(static_cast<int>(state));
        MapInfo::g_real_light_state[cur_id].state      = state_str;
        MapInfo::g_real_light_state[cur_id].count_down = info.count_down();
        MapInfo::g_real_light_state[cur_id].lamp_duration =
            info.step_info_list(0).duration();
      }
    } else {
      APP_LOG_WARN << "debug light info miss "
                  "phase_id/lamp_stats/count_down/lamp_duration";
    }
  }
}

MapInfo::MapInfo() {
  Init();
}

bool MapInfo::Init() {
  tools_.reset(new Utils());
  if (0 > ParseRsuMapXml()) {
    have_xml_ = false;
    APP_LOG_ERROR << "failed to parse rsu map xml";
    return false;
  }

  auto mgr   = base::Singleton<ConfigManager>::get_instance();
  auto param = Factory<BaseParams>::Instance().GetShared("usecase");
  mgr->LoadFor(param);
  lane_param_path_ = param->GetVal("lane_param_path").Cast<std::string>();

  if (!InitLaneParam(lane_param_path_)) {
    have_lane_ = false;
    APP_LOG_ERROR << "failed to parse lane param pt";
    return false;
  }
  return true;
}

bool MapInfo::InitLaneParam(std::string lane_param_path) {
  // todo初始化车道参数，存储车道polygon，车道中心点，
  if (!airos::base::ParseProtobufFromFile(lane_param_path, &lane_params_)) {
    APP_LOG_ERROR << "read roi polygon : Parse Protobuf From File Failed";
    return false;
  }
  for (auto &lane_roi : lane_params_.lane_roi_param()) {
    std::vector<Vec2d> lane_polygon;
    std::vector<Vec2d> lane_center;
    Lane new_lane;
    new_lane.lane_id = lane_roi.lane_id();
    for (auto &point : lane_roi.polygon()) {
      Vec2d pt(point.x(), point.y());
      lane_polygon.push_back(pt);
    }
    for (auto &point : lane_roi.center()) {
      Vec2d pt(point.x(), point.y());
      lane_center.push_back(pt);
    }
    Polygon2d polygon(lane_polygon);
    new_lane.polygon               = std::make_shared<Polygon2d>(polygon);
    new_lane.center                = lane_center;
    local_lanes_[new_lane.lane_id] = new_lane;
  }
  return true;
}

// todo 某一位置车道朝向获取
bool MapInfo::GetLaneHeading(
    const Point2d &point, const std::string &lane_id, double *heading) {
  if (local_lanes_.find(lane_id) == local_lanes_.end()) {
    return false;
  }
  auto polygon = local_lanes_[lane_id].polygon;
  if (!polygon->IsPointIn(point)) {
    return false;
  }
  auto &center = local_lanes_[lane_id].center;

  auto get_sign = [](double x) -> int {
    if (fabs(x) < 1e-6) {
      return 0;
    } else {
      return x > 0 ? 1 : -1;
    }
  };

  double min_dis = 1e9;
  int min_index  = -1;
  for (int i = 0, size = center.size(); i < size - 1; i++) {
    Point2d a(center[i].X(), center[i].Y());
    Point2d b(center[i + 1].X(), center[i + 1].Y());

    Vec2d ap(point.X() - a.X(), point.Y() - a.Y());
    Vec2d ab(b.X() - a.X(), b.Y() - a.Y());
    Vec2d bp(point.X() - b.X(), point.Y() - b.Y());
    double dis = min_dis;
    if (a.X() == b.X() && a.Y() == b.Y()) {
      dis = GetDistance(a, b);
    } else if (get_sign(GetDotProduct(ap, ab)) < 0) {
      dis = GetDistance(point, a);
    } else if (get_sign(GetDotProduct(ab, bp)) > 0) {
      dis = GetDistance(point, b);
    } else {
      dis = GetCrossProduct(ab, ap) / GetDistance(a, b);
    }
    if (min_dis > dis) {
      min_dis   = dis;
      min_index = i;
    }
  }
  if (min_index == -1) {
    return false;
  }
  Vec2d nearest_center(
      center[min_index + 1].X() - center[min_index].X(),
      center[min_index + 1].Y() - center[min_index].Y());
  *heading = atan2(nearest_center.Y(), nearest_center.X());
  return true;
}

int MapInfo::ParseRsuMapXml(const char *fname) {
  pugi::xml_document xml_doc;
  if (!xml_doc.load_file(fname)) {
    APP_LOG_INFO << "open xml file fasle:" << fname;
    return -1;
  }
  APP_LOG_INFO << "open xml file true:" << fname;
  if (xml_doc.child("nodes").empty()) {
    APP_LOG_INFO << "xml no nodes";
    return -1;
  }
  if (std::distance(
          xml_doc.child("nodes").begin(), xml_doc.child("nodes").end()) == 0) {
    APP_LOG_INFO << "xml nodes no node";
    return -1;
  }
  bool bool_have_optional_ = false;
  size_t nodes_size        = std::distance(
      xml_doc.child("nodes").begin(), xml_doc.child("nodes").end());
  pugi::xml_node iter_nodes = xml_doc.child("nodes").first_child();
  for (size_t i = 0; i < nodes_size; i++) {
    bool_have_optional_ = false;
    if (!iter_nodes.child("links").empty() &&
        std::distance(
            iter_nodes.child("links").begin(),
            iter_nodes.child("links").end()) > 0) {
      bool_have_optional_ = true;
    }
    if (bool_have_optional_) {
      size_t links_size = std::distance(
          iter_nodes.child("links").begin(), iter_nodes.child("links").end());
      pugi::xml_node iter_links = iter_nodes.child("links").first_child();
      for (size_t index_links = 0; index_links < links_size; index_links++) {
        size_t lanes_size        = iter_links.child("lanes").empty()
                                       ? 0
                                       : std::distance(
                                      iter_links.child("lanes").begin(),
                                      iter_links.child("lanes").end());
        pugi::xml_node iter_lane = iter_links.child("lanes").first_child();
        for (size_t index_lanes = 0; index_lanes < lanes_size; index_lanes++) {
          // point
          bool_have_optional_ = false;
          if (!iter_lane.child("points").empty() &&
              std::distance(
                  iter_lane.child("points").begin(),
                  iter_lane.child("points").end()) > 1) {
            bool_have_optional_ = true;
          }
          if (bool_have_optional_) {
            size_t points_size = std::distance(
                iter_lane.child("points").begin(),
                iter_lane.child("points").end());
            pugi::xml_node iter_points =
                iter_lane.child("points").first_child();
            for (size_t index_points = 0; index_points < points_size;
                 index_points++) {
              if (index_points == points_size - 1) {
                double latitude = 0;
                if (!iter_points.child("geo").child("latitude").empty()) {
                  latitude = Str2Double(iter_points.child("geo")
                                            .child("latitude")
                                            .child_value()) *
                             LONANDLATCOE_;
                }
                double longitude = 0;
                if (!iter_points.child("geo").child("longitude").empty()) {
                  longitude = Str2Double(iter_points.child("geo")
                                             .child("longitude")
                                             .child_value()) *
                              LONANDLATCOE_;
                }

                double cur_x, cur_y;
                tools_->LatLonToUTMXY(latitude, longitude, &cur_x, &cur_y);

                Vec2d pt;
                pt.SetX(cur_x);
                pt.SetY(cur_y);

                stopoint_to_light_[stopoint_count_++].first = pt;
                light_links_[index_links].push_back(stopoint_count_ - 1);
              }

              iter_points = iter_points.next_sibling();
            }  // points
          }

          // phase_id
          bool_have_optional_ = false;
          if (!iter_lane.child("movements").empty() &&
              std::distance(
                  iter_lane.child("movements").begin(),
                  iter_lane.child("movements").end()) > 0) {
            bool_have_optional_ = true;
          }
          if (bool_have_optional_) {
            std::string light_type;
            size_t conn_size = std::distance(
                iter_lane.child("movements").begin(),
                iter_lane.child("movements").end());
            pugi::xml_node xml_conn =
                iter_lane.child("movements").first_child();
            for (size_t m = 0; m < conn_size; m++) {
              light_type          = "None";
              bool_have_optional_ = false;
              bool_have_optional_ =
                  xml_conn.child("connectingLane").empty() ? false : true;
              if (bool_have_optional_) {
                bool_have_optional_ = false;
                bool_have_optional_ =
                    xml_conn.child("connectingLane").child("maneuver").empty()
                        ? false
                        : true;
                if (bool_have_optional_) {
                  double maneuver;
                  maneuver = Str2Double(xml_conn.child("connectingLane")
                                            .child("maneuver")
                                            .child_value());
                  if (maneuver == 2000) {
                    light_type = "right";
                  }
                  if (maneuver == 8000) {
                    light_type = "straight";
                  }
                  if (maneuver == 4000) {
                    light_type = "left";
                  }
                  if (maneuver == 1000) {
                    light_type = "u-turn";
                  }
                }
              }

              bool_have_optional_ = false;
              bool_have_optional_ =
                  xml_conn.child("phase_id").empty() ? false : true;
              if (bool_have_optional_) {
                int phase_id =
                    Str2Int(xml_conn.child("phase_id").child_value());
                stopoint_to_light_[stopoint_count_ - 1].second[light_type] =
                    phase_id;
              }
              xml_conn = xml_conn.next_sibling();
            }
          }
          iter_lane = iter_lane.next_sibling();
        }
        iter_links = iter_links.next_sibling();
      }
    }
    iter_nodes = iter_nodes.next_sibling();
  }
  return 1;
}

}  // namespace usecase
}  // namespace perception
}  // namespace airos
