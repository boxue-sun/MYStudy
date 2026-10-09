/*********************************************************************************
 * @file		position_model.cc
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
#include "position_model.h"

#include <GeographicLib/UTMUPS.hpp>
namespace airos {
namespace app {
void PositionModel::SetHeading(const double& heading) { heading_ = heading; }
void PositionModel::SetSpeed(const double& speed) { speed_ = speed; }

void PositionModel::SetPosition(const PositionXYZ& pos) {
  pos_xyz_ = pos;
  // UTM 转经纬度
  double lat, lon;
  GeographicLib::UTMUPS::Reverse(pos_xyz_.zone, true, pos_xyz_.x, pos_xyz_.y,
                                 lat, lon);
  pos_llh_.latitude = lat;
  pos_llh_.longitude = lon;
  pos_llh_.altitude = pos_xyz_.z;
}

void PositionModel::SetPosition(const PositionLLH& pos) {
  pos_llh_ = pos;
  // 经纬度坐标转UTM坐标
  int zone;
  bool dummy;
  double x, y;
  GeographicLib::UTMUPS::Forward(pos_llh_.latitude, pos_llh_.longitude, zone,
                                 dummy, x, y);
  pos_xyz_.x = x;
  pos_xyz_.y = y;
  pos_xyz_.z = pos_llh_.latitude;
  pos_xyz_.zone = zone;
}

// TODO
}  // namespace app
}  // namespace airos