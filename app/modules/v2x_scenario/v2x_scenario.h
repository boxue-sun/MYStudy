/*********************************************************************************
 * @file		v2x_second_stage_scenario.h
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

#include "app/framework/interface/app_base.h"
#include "app/framework/interface/app_factory.h"
#include "app/modules/v2x_scenario/common_module/local_maps/local_maps.h"
#include "app/modules/v2x_scenario/common_module/remote_vehicles/remote_vehicles.h"
#include "app/modules/v2x_scenario/common_module/traffic_light_data/traffic_light_data.h"
#include "app/modules/v2x_scenario/cooperation_intersection_cross/cooperation_intersection_cross.h"
#include "app/modules/v2x_scenario/cooperation_lanechange/cooperation_lanechange.h"
#include "app/modules/v2x_scenario/cooperation_rampin/cooperation_rampin.h"
#include "app/modules/v2x_scenario/cooperation_service_announcement/cooperation_service_announcement.h"
#include "app/modules/v2x_scenario/parkinglot_guidance/ParkingLotGuidance.h"
#include "air_service/framework/proto/airos_usecase.pb.h"

namespace airos {
namespace app {

class V2xScenario : public airos::app::AirOSApplication {
 public:
  V2xScenario(const ApplicationCallBack& cb)
      : AirOSApplication(cb),
        remote_vehicles_(RemoteVehicles::GetInstance()),
        traffic_light_data_(TrafficLightData::GetInstance()) {}

  ~V2xScenario() {}

  bool Init(const AppliactionConfig& conf) override;
  void Start() override;

 private:
  /**
   * @brief 处理MEC收到的V2X消息
   * @param frame std::shared_ptr<const v2xpb::asn::MessageFrame>
   */
  void ProcV2XMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);
  /**
   * @brief 处理MEC发出的V2X消息
   * @param frame const std::shared_ptr<const v2xpb::asn::MessageFrame>
   */
  void ProcV2XMsgSend(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

  void ProcV2XUsecaseMsg(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame);

 private:
  RemoteVehicles& remote_vehicles_;
  TrafficLightData& traffic_light_data_;
  std::shared_ptr<CooperationIntersectionCross> cooperation_intersection_cross_;
  std::shared_ptr<CooperationRampIn> cooperation_rampin_;
  std::shared_ptr<CooperationLaneChange> cooperation_lane_change_;
  std::shared_ptr<CooperationServiceAnnouncement> cooperation_service_announcement_;
  std::shared_ptr<ParkingLotGuidance> parkinglot_duidance_;


};
}  // namespace app
}  // namespace airos
