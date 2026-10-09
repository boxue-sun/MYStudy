
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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "air_service/framework/proto/airos_traffic_light.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/modules/v2x_application/proto/roi_lane.pb.h"
#include "app/modules/v2x_application/proto/roi_points.pb.h"

#include "app/modules/v2x_application/usecase/algorithm/params.h"
#include "app/modules/v2x_application/usecase/base/config_manager.h"
#include "app/modules/v2x_application/usecase/common/factory.hpp"
#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_application/usecase/common/util.h"
#include "app/modules/v2x_application/usecase/common/vec2d.h"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace perception {
namespace usecase {

using Vec2d     = airos::perception::usecase::Vec2d;
using Polygon2d = airos::perception::usecase::Polygon2d;
using Point2d   = Vec2d;

// 保存红绿灯灯态
struct RealLightInfo {
  std::string state;
  int count_down;
  int lamp_duration;
};

struct Lane {
  std::string lane_id;
  std::shared_ptr<Polygon2d> polygon;
  std::vector<Vec2d> center;
};

void SetTrafficLightState(
    const std::shared_ptr<const ::airos::trafficlight::TrafficLightServiceData>
        &light_states);

class MapInfo {
 public:
  MapInfo();
  bool Init();
  bool GetStopoint2Light(
      std::map<int, std::pair<Point2d, std::map<std::string, int>>> *out) {
    out->clear();
    *out = stopoint_to_light_;
    return have_xml_;
  }
  bool GetLightLinks(std::map<int, std::vector<int>> *out) {
    out->clear();
    *out = light_links_;
    return have_xml_;
  }

  bool GetLocalLanes(std::unordered_map<std::string, Lane> *out) {
    out->clear();
    *out = local_lanes_;
    return have_lane_;
  }

  double GetDistance(
      const double x1, const double y1, const double x2, const double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  }

  double GetDistance(const Point2d &pt1, const Point2d &pt2) {
    return GetDistance(pt1.X(), pt1.Y(), pt2.X(), pt2.Y());
  }

  double GetCrossProduct(const Vec2d &vec1, const Vec2d &vec2) {
    return vec1.X() * vec2.Y() - vec1.Y() * vec2.X();
  }

  double GetDotProduct(const Vec2d &vec1, const Vec2d &vec2) {
    return vec1.X() * vec2.X() + vec1.Y() * vec2.Y();
  }

  bool GetLaneHeading(
      const Point2d &point, const std::string &lane_id, double *heading);

  static std::map<int, RealLightInfo> GetRealLight() {
    std::lock_guard<std::mutex> g(g_real_light_lock);
    return g_real_light_state;
  }
  static std::atomic<bool> g_real_light_flag;
  static std::map<int, RealLightInfo> g_real_light_state;
  static std::mutex g_real_light_lock;

 private:
  int ParseRsuMapXml(
      const char *fname =
          "/home/airos/os/app/conf/airos_v2x_application/rsu_map.xml");
  float LONANDLATCOE_ = 1;
  std::shared_ptr<Utils> tools_;
  std::map<int, std::pair<Point2d, std::map<std::string, int>>>
      stopoint_to_light_;
  std::map<int, std::vector<int>> light_links_;
  int stopoint_count_ = 0;
  bool have_xml_      = true;

  // for lane pt
  bool InitLaneParam(std::string lane_param_path);
  airos::usecase::roi::LaneParam lane_params_;
  std::string lane_param_path_ = "";
  bool have_lane_              = true;
  std::unordered_map<std::string, Lane> local_lanes_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
