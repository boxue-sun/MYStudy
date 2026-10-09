/*********************************************************************************
 * @file		tracker_model.h
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
#pragma once
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_application/usecase/common/segment2d.h"
#include "app/modules/v2x_scenario/common_module/data_model/position_model.h"
#include "app/modules/v2x_scenario/common_module/local_maps/local_maps.h"

namespace airos {
namespace app {

class TrackerModel {
 public:
  TrackerModel();
  /**
   * @brief 所在节点的ID
   * @return v2xpb::asn::MapNode::ID 大于0有效
   */
  v2xpb::asn::MapNode::ID GetRefNodeId() const { return node_id_; }
  /**
   * @brief 所在Link的上游节点ID
   * @return v2xpb::asn::MapNode::ID 大于0有效
   */
  v2xpb::asn::MapNode::ID GetRefLinkUpStreamNodeId() const {
    return upstream_node_id_;
  }
  /**
   * @brief 所在车道的ID
   * @return int32_t 大于0有效
   */
  int32_t GetRefLaneId() const { return lane_id_; }

  /**
   * @brief 过去到停止线的距离
   * @return
   */
  double GetDistToNode() const { return dist_to_node_; }

  /**
   * @brief 返回是否在路口
   * @return bool true: 在路口   false: 不在路口
   */
  bool IsAtAcross() const { return is_at_across_; }

  /**
   * @brief 当前车道是否允许直行
   * @return
   */
  bool IsCurrentLaneAllowedStraight() const {
    return is_allowed_maneuver_straight_;
  }
  /**
   * @brief 当前车道是否允许左转
   * @return
   */
  bool IsCurrentLaneAllowedLeft() const { return is_allowed_maneuver_left_; }
  /**
   * @brief 当前车道是否允许右转
   * @return
   */
  bool IsCurrentLaneAllowedRight() const { return is_allowed_maneuver_right_; }
  /**
   * @brief 当前车道是否允许掉头
   * @return
   */
  bool IsCurrentLaneAllowedUTurn() const { return is_allowed_maneuver_u_turn_; }

  /**
   * @brief 更新定位匹配情况
   * @param pos 
   */
  void UpdateTracker(const PositionModel& pos);

 private:
  /**
   * @brief 获取停止线上的点
   * @param points 
   * @return 
   */
  bool GetStopPoints(std::vector<airos::perception::usecase::Vec2d>* points);

  /**
   * @brief 判断是否位于路口内部
   * @param point
   * @return
   */
  bool IsInJunction(const airos::perception::usecase::Vec2d point);

  /**
   * @brief 重置定位匹配数据
   */
  void ResetTrackerData();

  /**
   * @brief 在地图中匹配车道所在的位置
   * @return true: 匹配成功  false: 匹配失败
   */
  bool MatchVehicleInMap(const airos::perception::usecase::Vec2d point,
                         const double heading);

 private:
  v2xpb::asn::MapNode::ID node_id_;  // 当前节点的ID, 大于0有效
  v2xpb::asn::MapNode::ID upstream_node_id_;  // link 上游节点的ID, 大于0有效
  int32_t lane_id_ = -1;                      // 车道ID, -1 无效
  double dist_to_node_ = 0;                   // 距离停止线的距离
  bool is_at_across_ = false;                 // 是否进入路口
  std::vector<airos::perception::usecase::Vec2d> stop_points_;  // 停止线上的点
  const v2xpb::asn::Map& local_maps_;                           // map
  int zone_;

  bool is_allowed_maneuver_straight_ = false;  // 当前车到允许直行
  bool is_allowed_maneuver_left_ = false;      // 当前车道允许左转
  bool is_allowed_maneuver_right_ = false;     // 当前车道允许右转
  bool is_allowed_maneuver_u_turn_ = false;    // 当前车道允许掉头
};
}  // namespace app
}  // namespace airos
