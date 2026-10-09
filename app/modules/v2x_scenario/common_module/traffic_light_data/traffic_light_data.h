/*********************************************************************************
 * @file		traffic_light_data.h
 * @brief		traffic_light_data
 * @details	traffic_light_data
 * @author		ChangXuhui
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
#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"

namespace airos {
namespace app {

typedef std::unordered_map<int, v2xpb::asn::SpatPhaseState> SpatPhasesMap;

class TrafficLightData {
 public:
  /**
   * @brief 单例, 访问TrafficLightData的入口
   * @return
   */
  static TrafficLightData& GetInstance() {
    static TrafficLightData instance;
    return instance;
  }
  
  /**
   * @brief 获取信号灯的数据
   * @details 使用该数据时，必须加锁!!!
   * @details 调用LockTrafficLightDataMap() 及 UnlockTrafficLightDataMap()
   * @return 红绿灯数据map <id , v2xpb::asn::SpatPhaseState>
   */
  const std::unordered_map<std::string, SpatPhasesMap>&
  GetTrafficLightDataMap() const {
    return traffic_light_data_map_;
  }
  /**
   * @brief 加锁
   */
  void LockTrafficLightDataMap() { traffic_light_mutex_.lock(); }
  /**
   * @brief 解锁
   */
  void UnlockTrafficLightDataMap() { traffic_light_mutex_.unlock(); }
  /**
   * @brief  通过Spat消息更新红绿灯数据
   * @param frame
   * @return
   */
  bool UpdateBySpatMsg(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

 private:
  TrafficLightData();  // 私有构造函数，防止外部创建对象
  /**
   * @brief 维护红绿灯数据
   */
  void TrafficLightMaintenance();

 private:
  std::unordered_map<std::string, SpatPhasesMap>
      traffic_light_data_map_;              // 信号灯数据
  mutable std::mutex traffic_light_mutex_;  // 信号灯数据的互斥锁
  std::shared_ptr<std::thread> traffic_light_maintenance_{nullptr};
  double updata_time_;  // 红绿灯数据的更新时间
};
}  // namespace app
}  // namespace airos
