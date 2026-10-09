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
#include "vehicle_model.h"

#include <GeographicLib/UTMUPS.hpp>

namespace airos {
namespace app {
VehicleModel::VehicleModel() {
  tracker_ptr_ = std::make_shared<TrackerModel>();
}

void VehicleModel::UpdatePosition(const PositionModel& pos) {
  ParticipantModel::UpdatePosition(pos);
  tracker_ptr_->UpdateTracker(pos);
}

// TODO
}  // namespace app
}  // namespace airos