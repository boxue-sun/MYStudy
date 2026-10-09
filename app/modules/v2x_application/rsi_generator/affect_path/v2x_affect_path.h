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

#include <cfloat>
#include <GeographicLib/UTMUPS.hpp>

#include "app/framework/proto/v2xpb-asn-map.pb.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/framework/proto/v2xpb-asn-rsi.pb.h"
#include "app/modules/v2x_application/proto/v2xpb-config-event-details.pb.h"

#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_application/usecase/common/segment2d.h"
#include "base/common/log.h"

namespace airos {
namespace app {
class RsiAffectPath {
 public:
  typedef std::shared_ptr<const v2xpb::rscu::config::RteEventDetail>
      RteEventDetailPtr;
  typedef std::shared_ptr<const v2xpb::rscu::config::SpecialEventDetail>
      SpecialEventDetailPtr;

  bool GetAffectPathBySpecialConf(
      const int mec_event_type,
      const airos::perception::usecase::Vec2d ev_point,
      const std::vector<SpecialEventDetailPtr> special_event_conf_list,
      const v2xpb::asn::Map& map,
      std::vector<v2xpb::asn::RsiReferencePath>* affect_paths);

  bool GetAffectPath(
      const RteEventDetailPtr ev_detail,
      const airos::perception::usecase::Vec2d ev_point,
      const v2xpb::asn::Map& map,
      std::vector<v2xpb::asn::RsiReferencePath>* affect_paths); 

 private:
   void GetAffecLinkPathByConf(
      const v2xpb::asn::Map& map, 
      const v2xpb::rscu::config::LinkPath& lk,
      std::vector<v2xpb::asn::RsiReferencePath>* affect_paths);
  /**
   * @brief 判断事件发生是否符合配置文件的特殊配置
   * @param mec_event_type 
   * @param ev_point 
   * @param special_event_conf 
   * @return 
   */
  bool IsSpecialEventMatched(const int mec_event_type,
                             const airos::perception::usecase::Vec2d ev_point,
                             const SpecialEventDetailPtr special_event_conf); 

  bool GetStopPoints(
      const v2xpb::asn::Map& map,
      std::vector<airos::perception::usecase::Vec2d>* points);

  bool IsInJunction(const airos::perception::usecase::Vec2d point);

  bool CalcAffectPath(
      const RteEventDetailPtr ev_detail,
      const v2xpb::asn::Map& map,
      std::vector<v2xpb::asn::RsiReferencePath>* affect_paths);

  bool CalcAffectPath(
      const airos::perception::usecase::Vec2d center_point,
      v2xpb::asn::RsiReferencePath* affect_path);

  bool GetNearstLink(
      const airos::perception::usecase::Vec2d point, const v2xpb::asn::Map& map,
      v2xpb::asn::MapLink* ret_link, int* idx);

  bool CalcAffectPath(
      const RteEventDetailPtr ev_detail,
      const airos::perception::usecase::Vec2d ev_point,
      const v2xpb::asn::MapLink& link, bool in_centor,
      v2xpb::asn::RsiReferencePath* affect_path);
  bool GetNearestPoint(
      const v2xpb::asn::MapLane& lane,
      const airos::perception::usecase::Vec2d ev_point,
      airos::perception::usecase::Vec2d* neatest_point, int* next_idx);

  /**
   * @brief 超速及类似事件特殊处理
   * @param ev_detail 
   * @param ev_point 
   * @param map 
   * @param affect_paths
   * @return 
   */
  bool CalcAffectPathOverSpeed(const RteEventDetailPtr ev_detail,
                               const airos::perception::usecase::Vec2d ev_point,
                               const v2xpb::asn::Map &map,
                               std::vector<v2xpb::asn::RsiReferencePath> *affect_paths);

  /**
   * @brief 行人闯入机动车道
   * @param ev_detail 
   * @param ev_point 
   * @param map 
   * @param affect_paths 
   * @return 
   */
  bool CalcAffectPathPedestrianIntrusion(const RteEventDetailPtr ev_detail,
                               const airos::perception::usecase::Vec2d ev_point,
                               const v2xpb::asn::Map &map,
                               std::vector<v2xpb::asn::RsiReferencePath> *affect_paths);
  /**
   * @brief 计算当前node通往某条道路的影响路径
   * @param ev_detail 事件的配置参数
   * @param map 当前Node的地图
   * @param nodeId 通往Node的Id
   * @param affect_paths 影响路径
   * @return
   */
  bool calculateExitDirectionLinkAffectedPath(
      const RteEventDetailPtr ev_detail,
      const v2xpb::asn::Map &map,
      const v2xpb::asn::MapNode::ID nodeId,
      std::vector<v2xpb::asn::RsiReferencePath> *affect_paths);
 
  /**
   * @brief 行人或者非机动车闯红灯
   * @param ev_detail 
   * @param ev_point 
   * @param map 
   * @param affect_paths 
   * @return 
   */
  bool CalcAffectPathPedestrianRunningRedLight(const RteEventDetailPtr ev_detail,
                               const airos::perception::usecase::Vec2d ev_point,
                               const v2xpb::asn::Map &map,
                               std::vector<v2xpb::asn::RsiReferencePath> *affect_paths); 
  /**
   * @brief 经纬度转UTM坐标
   * @param lat 
   * @param lon 
   * @param ponit 
   */
  void ConvertToUtm(double lat, double lon, airos::perception::usecase::Vec2d& ponit);

  /**
   * @brief 计算某条LINK的影响路径
   * @param link
   * @param affect_path
   * @return
   */
  bool CalcLinkAffectPath(
      const double impact_dist,
      const v2xpb::asn::MapLink& link,
      v2xpb::asn::RsiReferencePath* affect_path);

 private:
  int zone_;
  std::vector<airos::perception::usecase::Vec2d> stop_points_;
};

}  // namespace app
}  // namespace airos
