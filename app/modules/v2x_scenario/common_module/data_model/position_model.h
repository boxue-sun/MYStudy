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

namespace airos {
namespace app {

struct PositionXYZ {
  double x;      // in meters.
  double y;      // in meters.
  double z;      // height in meters.
  int16_t zone;  // UTM zone
};
struct PositionLLH {
  double latitude;
  double longitude;
  double altitude;
};

class PositionModel {
 public:
  /**
   * @brief 获取UTM形式坐标
   * @return PositionXYZ
   */
  PositionXYZ GetPositionXYZ() const { return pos_xyz_; }
  /**
   * @brief 获取经纬度坐标
   * @return PositionLLH
   */
  PositionLLH GetPositionLLH() const { return pos_llh_; }
  /**
   * @brief 获取速度
   * @return 
   */
  double GetSpeed() const { return speed_; }
  /**
   * @brief 获取航向角
   * @return 
   */
  double GetHeading() const { return heading_; }

  /**
   * @brief 设置位置信息(通过XYZ形式更新)
   * @param pos PositionXYZ
   */
  void SetPosition(const PositionXYZ& pos);
  /**
   * @brief 设置位置信息(通过LLH形式更新)
   * @param pos PositionLLH
   */
  void SetPosition(const PositionLLH& pos);
  /**
   * @brief 设置速度
   * @param speed 
   */
  void SetSpeed(const double& speed);
  /**
   * @brief 设置航向角
   * @param heading
   */
  void SetHeading(const double& heading);
 private:
  PositionXYZ pos_xyz_;
  PositionLLH pos_llh_;
  double speed_;
  double heading_;
};
}  // namespace app
}  // namespace airos
