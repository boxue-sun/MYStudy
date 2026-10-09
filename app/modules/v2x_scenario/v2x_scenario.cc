/*********************************************************************************
 * @file		v2x_second_stage_scenario.cpp
 * @brief		v2x_second_stage_scenario
 * @details		v2x_second_stage_scenario
 * @author		ChangXuhui
 * @date		2024/3/27
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/3/27  ChangXuhui      1.0          ————               二阶段场景开发
 *                                                             
 *
 * @endverbatim
 ********************************************************************************/

#include "v2x_scenario.h"

#include "app/framework/interface/app_middleware.h"
#include "base/common/log.h"
#include "common/app_flag.h"

namespace airos {
namespace app {
bool V2xScenario::Init(const AppliactionConfig& conf) {

  APP_LOG_ERROR << "Path: " << conf.app_config_path();
  // 协作式交叉路口场景
  cooperation_intersection_cross_ =
      std::make_shared<CooperationIntersectionCross>();
 if (FLAGS_enable_send) {
    cooperation_intersection_cross_->Init(sender_, conf.app_config_path());
  }

  cooperation_rampin_ =
      std::make_shared<CooperationRampIn>();
  if (FLAGS_enable_send) {
    cooperation_rampin_->Init(sender_, conf.app_config_path());
  }

  cooperation_lane_change_ =
      std::make_shared<CooperationLaneChange>();
  if (FLAGS_enable_send) {
    cooperation_lane_change_->Init(sender_, conf.app_config_path());
  }

  cooperation_service_announcement_ = std::make_shared<CooperationServiceAnnouncement>();
  if (FLAGS_enable_send) {
    cooperation_service_announcement_->Init(sender_, conf.app_config_path());
  }

  parkinglot_duidance_ = std::make_shared<ParkingLotGuidance>();
  if(FLAGS_enable_send) {
    parkinglot_duidance_->Init(sender_, conf.app_config_path());
  }
  if (FLAGS_enable_send) {
      RegisterOSMessage<v2xpb::asn::MessageFrame>(
              OSMessageType::V2X_MESSAGE_SEND,
              std::bind(&V2xScenario::ProcV2XMsgSend, this, std::placeholders::_1));

      RegisterOSMessage<v2xpb::asn::MessageFrame>(
              OSMessageType::V2X_MESSAGE_RECEIVED,
              std::bind(&V2xScenario::ProcV2XMsgRecv, this, std::placeholders::_1));
      RegisterOSMessage<airos::usecase::EventOutputResult>(
          OSMessageType::MEC_DATA,
          std::bind(&V2xScenario::ProcV2XUsecaseMsg, this, std::placeholders::_1));  
  }
  return true;
}

void V2xScenario::ProcV2XMsgSend(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  remote_vehicles_.UpdateByPerception(frame);
  traffic_light_data_.UpdateBySpatMsg(frame);
  // TODO
  return;
}

void V2xScenario::ProcV2XMsgRecv(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  remote_vehicles_.UpdateByV2XConnected(frame);
  cooperation_intersection_cross_->ProcV2xMsgRecv(frame);
  cooperation_rampin_->ProcV2xMsgRecv(frame);
  cooperation_lane_change_->ProcV2xMsgRecv(frame);
  parkinglot_duidance_->ProcV2xMsgRecv(frame);

  // TODO
  return;
}

void V2xScenario::ProcV2XUsecaseMsg(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame)
{
  parkinglot_duidance_->ProcV2xUsecaseMsg(frame);
}

void V2xScenario::Start() {
  APP_LOG_INFO << "v2x scenario start";
}

AIROS_APPLICATION_REG_FACTORY(V2xScenario, "v2xscenario")
}  // namespace app
}  // namespace airos
