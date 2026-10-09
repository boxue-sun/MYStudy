/*********************************************************************************
 * @file		map_model.h
 * @brief		map_model
 * @details		map_model
 * @author		ChangXuhui
 * @date		2024/4/7
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/7  ChangXuhui      1.0            ————                 ———— 
 *                                                             
 * @endverbatim
 ********************************************************************************/
#pragma once
#include <memory>
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"

namespace airos {
namespace app {

class LocalMaps {
 public:
  /**
   * @brief 单例, 访问LocalMaps的接口
   * @return
   */
  static LocalMaps& GetInstance() {
    static LocalMaps instance;
    return instance;
  }

  ~LocalMaps() {}

  /**
   * @brief 获取地图数据
   * @return 
   */
  const v2xpb::asn::Map& GetLocalMap() const { return local_map_; }

  /**
   * @brief 获取对应的车道
   * @param node_id
   * @param upstream_node_id
   * @param lane_id
   * @param lane 返回对应车道的引用数据
   * @return bool true: 获取成功  false: 没有找到
   */
  bool GetLane(const v2xpb::asn::MapNode::ID& node_id,
               const v2xpb::asn::MapNode::ID& upstream_node_id,
               const int& lane_id, 
               v2xpb::asn::MapLane& lane);

  /**
   * @brief 获取对向的link
   * @param node_id
   * @param upstream_node_id
   * @param lane_id
   * @param link 获取对向的车道
   * @return bool true: 获取成功  false: 没有找到
   */
  bool GetOnCommingLink(const v2xpb::asn::MapNode::ID& node_id,
               const v2xpb::asn::MapNode::ID& upstream_node_id,
               const int& lane_id, 
               v2xpb::asn::MapLink& link);

  /**
   * @brief 根据node入口link,预测出出口link上停止线上的点
   * @param node_id
   * @param upstream_node_id
   * @param point 出口link上停止线上的点
   * @return bool true: 获取成功  false: 没有找到
   */
  bool GetExitLinkStopLinePointByPredict(
      const v2xpb::asn::MapNode::ID& node_id,
      const v2xpb::asn::MapNode::ID& upstream_node_id,
      v2xpb::asn::Position& point);

 private:
  LocalMaps();  // 私有构造函数，防止外部创建对象
  bool GetRsuMap(const std::string& rsu_map_path);

 private:
  std::shared_ptr<v2xpb::asn::MessageFrame> message_frame_;
  v2xpb::asn::Map local_map_;
};
}  // namespace app
}  // namespace airos
