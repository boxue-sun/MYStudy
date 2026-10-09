/*********************************************************************************
 * @file		remote_vehicles.h
 * @brief		remote_vehicles
 * @details		remote_vehicles
 * @author		ChangXuhui
 * @date		2024/4/3
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/3  ChangXuhui      1.0            ————                 ————
 *
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/modules/v2x_scenario/common_module/data_model/vehicle_model.h"

namespace airos {
namespace app {

class RemoteVehicles {
 public:
  /**
   * @brief 单例, 访问RemoteVehicles的入口
   * @return
   */
  static RemoteVehicles& GetInstance() {
    static RemoteVehicles instance;
    return instance;
  }

  /**
   * @brief 获取周边车辆的数据
   * @details 使用该数据时，必须加锁!!!
   * @details 调用LockRemoteVehiclesMap() 及 UnlockRemoteVehiclesMap()
   * @return 车辆map <id , VehicleModel>
   */
  const std::unordered_map<std::string, VehicleModel>& GetRemoteVehiclesMap()
      const;
  /**
   * @brief 加锁
   */
  void LockRemoteVehiclesMap() { remote_vehicles_mutex_.lock(); }
  /**
   * @brief 解锁
   */
  void UnlockRemoteVehiclesMap() { remote_vehicles_mutex_.unlock(); }

  /**
   * @brief  通过感知数据更新周边车辆数据
   * @details 通过感知数据(MEC下发的RSM、SSM)
   * @param frame
   * @return
   */
  bool UpdateByPerception(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);
  /**
   * @brief  通过V2X网联数据更新周边车辆数据
   * @details 通过V2X消息(MEC收到的的BSM)
   * @param frame
   * @return
   */
  bool UpdateByV2XConnected(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

 private:
  RemoteVehicles();  // 私有构造函数，防止外部创建对象
  /**
   * @brief 维护周边车辆数据，删除过期的车辆数据
  */
  void RemoteVehicleMaintenance();

 private:
  // 周边车辆数据
  std::unordered_map<std::string, VehicleModel> remote_vehicles_map_;
  mutable std::mutex remote_vehicles_mutex_; // 车辆数据的互斥锁 
  std::shared_ptr<std::thread> vehicle_maintenance_{nullptr};
};
}  // namespace app
}  // namespace airos
