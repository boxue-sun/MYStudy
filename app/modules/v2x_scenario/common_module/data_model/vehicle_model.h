/*********************************************************************************
 * @file		position_model.h
 * @brief		position_model
 * @details		position_model
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

#include <cstdint>
#include <string>

#include "app/modules/v2x_scenario/common_module/data_model/participant_model.h"
#include "app/modules/v2x_scenario/common_module/data_model/tracker_model.h"

namespace airos {

namespace app {

class VehicleModel : public ParticipantModel {
 public:
  VehicleModel();
  /**
   * @brief 过去车辆ID
   * @return 
   */
  std::string GetVehicleID() const { return vehicle_id_; };
  /**
   * @brief 获取车牌号
   * @return 
   */
  std::string GetVehicleLicenseNum() const { return license_num_; };
  /**
   * @brief 获取是否是V2X网联车
   * @return true 是网联车  false 是感知车辆
   */
  bool GetIsV2XVehicleFlag() const { return is_v2x_vehicle_; };

  /**
   * @brief 获取位置匹配跟踪数据
   * @return  std::shared_ptr<TrackerModel>
   */
  std::shared_ptr<TrackerModel> GetTracker() const { return tracker_ptr_; }

  /**
   * @brief 获取车辆类型
   * @return int
   */
  int GetVehicleType() const { return vehicle_type_; };

  /**
   * @brief 设置车辆Id
   * @param vehicle_id 
   */
  void SetVehicleID(std::string vehicle_id) { vehicle_id_ = vehicle_id; };
  /**
   * @brief 设置车牌号
   * @param license_num 
   */
  void SetVehicleLicenseNum(std::string license_num) { license_num_ = license_num; };
  /**
   * @brief 设置是否是V2X网联车的标志
   * @param is_v2x_vehicle true: 是网联车 false: 感知车辆
   */
  void SetIsV2XVehicleFlag(bool is_v2x_vehicle) {
    is_v2x_vehicle_ = is_v2x_vehicle;
  };

  /**
   * @brief 更新位置数据
   * @param pos
   */
  void UpdatePosition(const PositionModel& pos);

  void SetVehicleType(int type) {
	  vehicle_type_ = type;
  }

 private:
  std::string vehicle_id_;   // 车辆Id
  std::string license_num_;  // 车牌号
  bool is_v2x_vehicle_;      // 是否是v2x网联车（通过bsm更新)
  std::shared_ptr<TrackerModel> tracker_ptr_;  // 位置匹配跟踪数据
  int vehicle_type_;                            // 车辆类型
};
}  // namespace app
}  // namespace airos
