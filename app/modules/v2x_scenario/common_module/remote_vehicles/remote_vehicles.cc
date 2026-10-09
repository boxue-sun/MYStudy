/*********************************************************************************
 * @file		remote_vehicles.cc
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

#include "remote_vehicles.h"

#include <functional>

#include "app/modules/v2x_scenario/common_module/local_maps/local_maps.h"
#include "base/common/log.h"
#include "base/common/math_util.h"
#include "base/common/time_util.h"

namespace airos {
namespace app {

RemoteVehicles::RemoteVehicles() {
  auto local_maps = LocalMaps::GetInstance().GetLocalMap();
  vehicle_maintenance_ = std::make_shared<std::thread>(std::bind(&RemoteVehicles::RemoteVehicleMaintenance, this));
}

const std::unordered_map<std::string, VehicleModel>&
RemoteVehicles::GetRemoteVehiclesMap() const {
  return remote_vehicles_map_;
}

bool RemoteVehicles::UpdateByPerception(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
    // 通过rsm更新
    if (frame->has_rsmframe()) {
      // 使用索引遍历 repeated 字段
      for (int i = 0; i < frame->rsmframe().participants_size(); ++i) {
        const auto& ptc = frame->rsmframe().participants(i);
        APP_LOG_INFO << "Start proc ptc:  " << ptc.id();
        if (ptc.ptc_type() != v2xpb::asn::ParticipantType::PT_MOTOR) {
          APP_LOG_INFO << "Ptc type is not motor, ignore";
          continue;
        }

        // 车辆位置
        PositionXYZ pos_xyz;
        pos_xyz.x = ptc.pos().xyz().x();
        pos_xyz.y = ptc.pos().xyz().y();
        pos_xyz.z = ptc.pos().xyz().z();
        pos_xyz.zone = ptc.pos().xyz().zone();

        PositionModel current_position;
        current_position.SetPosition(pos_xyz);
        current_position.SetSpeed(
            airos::base::MathUtil::convertSpeedI2F(ptc.speed()));
        current_position.SetHeading(
            airos::base::MathUtil::convertRadHeadingI2F(ptc.heading()));

        // 车辆尺寸
        ParticipantSize veh_size;
        veh_size.height = ptc.size().height();
        veh_size.width = ptc.size().width();
        veh_size.length = ptc.size().length();

        LockRemoteVehiclesMap();
        auto iter = remote_vehicles_map_.find(ptc.id());
        if (iter == remote_vehicles_map_.end()) {
          // 设置更新车辆的基础信息
          VehicleModel vehicle;
          vehicle.SetVehicleID(ptc.id());
          vehicle.SetVehicleLicenseNum(ptc.plate_no());
          vehicle.SetIsV2XVehicleFlag(false);
          vehicle.SetParticipantId(ptc.id());
          vehicle.SetParticipantType(ParticipantType::motor);

          vehicle.SetParticipantSize(veh_size);
          vehicle.UpdatePosition(current_position);
          remote_vehicles_map_.insert({vehicle.GetVehicleID(), vehicle});
        }
        else
        {
          auto vehicle = iter->second;
          // 更新车辆位置
          vehicle.UpdatePosition(current_position);
        }
        UnlockRemoteVehiclesMap();
      }
    }
  // TODO
  return true;
}

bool RemoteVehicles::UpdateByV2XConnected(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  // 通过BSM更新
  if (frame->has_bsmframe()) {
    auto &bsm =  frame->bsmframe();
    std::string id(bsm.id().begin(), bsm.id().end());
    APP_LOG_INFO << "Start proc bsm:  " << id;
    // 车辆位置
    PositionXYZ pos_xyz;
    pos_xyz.x = bsm.pos().xyz().x();
    pos_xyz.y = bsm.pos().xyz().y();
    pos_xyz.z = bsm.pos().xyz().z();
    pos_xyz.zone = bsm.pos().xyz().zone();

    PositionModel current_position;
    current_position.SetPosition(pos_xyz);
    current_position.SetSpeed(
        airos::base::MathUtil::convertSpeedI2F(bsm.speed()));
    current_position.SetHeading(
        airos::base::MathUtil::convertRadHeadingI2F(bsm.heading()));

    // 车辆尺寸
    ParticipantSize veh_size;
    if (bsm.has_size())
    {
      veh_size.height = bsm.size().height();
      veh_size.width = bsm.size().width();
      veh_size.length = bsm.size().length(); 
    }

    int vehtype = bsm.vehicle_class().classification();
    LockRemoteVehiclesMap();
        auto iter = remote_vehicles_map_.find(id);
    if (iter == remote_vehicles_map_.end()) {
      // 设置更新车辆的基础信息
      VehicleModel vehicle;
      vehicle.SetVehicleID(id);
      if (bsm.has_plate_no())
      {
        std::string plate_no(bsm.plate_no().begin(), bsm.plate_no().end());
        vehicle.SetVehicleLicenseNum(plate_no);
      }
      vehicle.SetIsV2XVehicleFlag(true);
      vehicle.SetParticipantId(id);
      vehicle.SetParticipantType(ParticipantType::motor);

      vehicle.SetParticipantSize(veh_size);
      vehicle.SetVehicleType(vehtype);
      vehicle.UpdatePosition(current_position);
      remote_vehicles_map_.insert({vehicle.GetVehicleID(), vehicle});
      APP_LOG_INFO << "Add new veh by bsm, id: " << id;
    } else {
      auto &vehicle = iter->second;
      // 更新车辆位置
      vehicle.UpdatePosition(current_position);
      APP_LOG_INFO << "Update veh by bsm, id: " << id;
    }
    UnlockRemoteVehiclesMap();
  }
  // TODO
  return true;
}

void RemoteVehicles::RemoteVehicleMaintenance() {
  while (true) {
    double time_now = airos::base::TimeUtil::GetCurrentTime();

    LockRemoteVehiclesMap();
    auto iter = remote_vehicles_map_.begin();
    for (; iter != remote_vehicles_map_.end();) {
      if (time_now - iter->second.GeUpdateTime() > 1.0) {
        APP_LOG_INFO << "Remote vehicle: " << iter->first
                 << " obsolete, And remote it from remote vehicles map";
        iter = remote_vehicles_map_.erase(iter);
      } else {
        ++iter;
      }
    }
    // APP_LOG_INFO << "Remote vehicles map size: "<< remote_vehicles_map_.size();
    UnlockRemoteVehiclesMap();
    // 100ms 检查一次
    std::chrono::microseconds duration(100000);
    std::this_thread::sleep_for(duration);
  }
}
}  // namespace app
}  // namespace airos
