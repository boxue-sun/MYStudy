/*********************************************************************************
 * @file		mec_adapter_ssm.cpp
 * @brief		
 * @details		
 * @author		ChangXuhui
 * @date		2024/11/05 
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/11/05   ChangXuhui       1.0       ————          Create this file   							   
 * @endverbatim
 ********************************************************************************/

#include "mec_adapter_ssm.h"

#include <sys/time.h>

#include <vector>
#include <thread>

#include "base/common/math_util.h"
#include "base/common/time_util.h"
#include "base/env/env.h"
#include "base/common/log.h"

namespace airos {
namespace app {

bool MECAdapterSsm::Init(const airos::app::ApplicationCallBack& send_cb) {
  send_ = send_cb;

  // 初始化SSM消息
  pb_ssm_ = std::make_shared<v2xpb::asn::MessageFrame>();
  auto extpb = pb_ssm_->mutable_extframe();
  extpb->set_messageid(12);
  auto valuepb = extpb->mutable_messagevalue();
  valuepb->set_typresent(v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_SensorSharingMsg);

  auto ssm = valuepb->mutable_sensorsharingmsg();
  ssm_count_ = (ssm_count_ >= 127) ? 0 : (ssm_count_ + 1);
  ssm->set_msg_cnt(ssm_count_);
  std::string idstr("12345678");
  ssm->set_id(idstr);
  ssm->set_equipmenttype(v2xpb::asn::EquipmentType::rsu);
  ssm->set_secmark(get_mill_second_minute());

  auto sensor_pos_llh = ssm->mutable_sensorpos()->mutable_llh();
  sensor_pos_llh->set_latitude(39.7786272);
  sensor_pos_llh->set_longitude(116.5621358);
  return true;
}

bool MECAdapterSsm::Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>&
              recv_data) {
  if (!recv_data) {
    APP_LOG_ERROR << "Recv data EventOutputResult failed";
    return false;
  }

  if(recv_data->perception_obstacle().empty())
  {
    APP_LOG_WARN << "Recv data perception_obstacle is empty";
    return false;
  }

  // 统计丢包率
  ++recv_total_package_num;
  recv_total_obj_num += recv_data->perception_obstacle().size();
  APP_LOG_INFO << "sumarry: Expected recv total package num: "
        << recv_data->header().total_package_num()
        << ", Actual recv package num: " << recv_total_package_num
        << ", Packet loss rate: " << std::fixed
        << (recv_data->header().total_package_num() -
            recv_total_package_num) *
               1.0 / recv_data->header().total_package_num();

  APP_LOG_INFO << "sumarry: Expected recv total obj num: "
        << recv_data->header().total_obj_num()
        << ", Actual recv obj num: " << recv_total_obj_num
        << ", Obj loss rate: " << std::fixed
        << (recv_data->header().total_obj_num() - recv_total_obj_num) * 1.0 /
               recv_data->header().total_obj_num();

  double time_now = airos::base::TimeUtil::GetCurrentTime();
  APP_LOG_INFO << "summary: Send timestamp: " << std::fixed
        << recv_data->header().timestamp_sec()
        << " s, recv timestamp: " << time_now << " s, delay: "
        << time_now - recv_data->header().timestamp_sec() << " s";
  
  uint32_t pb_ptc_cnt = recv_data->perception_obstacle().size();

  auto ssm = pb_ssm_->mutable_extframe()->mutable_messagevalue()->mutable_sensorsharingmsg();
  ssm->clear_participants();
  ssm->clear_obstacles();

  for (uint32_t i = 0; i < pb_ptc_cnt; ++i) {
    auto obstacle = recv_data->perception_obstacle()[i];
    if(!obstacle.has_type()) {
      continue;
    }

    auto type = obstacle.type();
    // 区分obstacles 和 participants
    if (type == airos::perception::PerceptionObstacle::PEDESTRIAN ||
        type == airos::perception::PerceptionObstacle::BICYCLE ||
        type == airos::perception::PerceptionObstacle::VEHICLE) {
        auto participant = ssm->mutable_participants()->add_list();
        auto ptc = participant->mutable_ptc();
        switch (type)
        {
        case airos::perception::PerceptionObstacle::PEDESTRIAN:
          ptc->set_ptc_type(v2xpb::asn::ParticipantType::PT_PEDESTRIAN);
          break;
        case airos::perception::PerceptionObstacle::BICYCLE:
          ptc->set_ptc_type(v2xpb::asn::ParticipantType::PT_NON_MOTOR);
          break;
        case airos::perception::PerceptionObstacle::VEHICLE:
          ptc->set_ptc_type(v2xpb::asn::ParticipantType::PT_MOTOR);
          break;
        default:
          break;
        }
        
        ptc->set_ptc_id(obstacle.id());
        ptc->set_source(v2xpb::asn::SourceType::ST_VIDEO);

        v2xpb::asn::Position *position = ptc->mutable_pos();
        v2xpb::asn::Position::XYZ *xyz = position->mutable_xyz();

        xyz->set_x(obstacle.position().x());
        xyz->set_y(obstacle.position().y());
        xyz->set_z(obstacle.position().z());
        xyz->set_zone(obstacle.position().zone());
        ptc->set_speed(
            airos::base::MathUtil::convertSpeedF2I(obstacle.velocity().x()));
        ptc->set_heading(
            airos::base::MathUtil::getValidDegAngle(obstacle.theta()) / 0.0125);

        v2xpb::asn::VehicleSize *size = ptc->mutable_size();
        size->set_length(airos::base::MathUtil::convertM2CM(obstacle.length()));
        size->set_width(airos::base::MathUtil::convertM2CM(obstacle.width()));
        size->set_height(
            airos::base::MathUtil::convertHeightF2I(obstacle.height()));
        ptc->set_sec_mark(get_mill_second_minute());
    } else {
        auto obs = ssm->mutable_obstacles()->add_list();
        if (obstacle.has_sub_type()) {
          switch (obstacle.sub_type()) {
          case airos::perception::SubType::TRAFFICCONE:
            obs->set_obstype(v2xpb::asn::ObstacleType::ObstacleType_trafficcone);
            break;
          default:
            obs->set_obstype(v2xpb::asn::ObstacleType::ObstacleType_unknown);
            break;
          }
        }

        obs->set_obsid(obstacle.id());
        obs->set_source(v2xpb::asn::SourceType::ST_VIDEO);

        auto position = obs->mutable_pos();
        auto xyz = position->mutable_xyz();

        xyz->set_x(obstacle.position().x());
        xyz->set_y(obstacle.position().y());
        xyz->set_z(obstacle.position().z());
        xyz->set_zone(obstacle.position().zone());
        obs->set_speed(
            airos::base::MathUtil::convertSpeedF2I(obstacle.velocity().x()));
        obs->set_heading(
            airos::base::MathUtil::getValidDegAngle(obstacle.theta()) / 0.0125);

        auto size = obs->mutable_size();
        size->set_length(airos::base::MathUtil::convertM2CM(obstacle.length()));
        size->set_width(airos::base::MathUtil::convertM2CM(obstacle.width()));
        size->set_height(
            airos::base::MathUtil::convertHeightF2I(obstacle.height()));
        obs->set_secmark(get_mill_second_minute());
    }
  }

  auto message_pb = std::make_shared<airos::app::ApplicationData>();
  message_pb->mutable_road_side_frame()->operator=(*pb_ssm_);
  // APP_LOG_ERROR << message_pb->DebugString();
  send_(message_pb);
  return true;
}

int64_t MECAdapterSsm::get_mill_second_minute() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0) {
    return -1;
  }
  struct tm* t     = nullptr;
  time_t startTime = time(0);

  struct tm buf = {};
  localtime_r(&startTime, &buf);
  t = &buf;

  if (t == nullptr) {
    return -1;
  }
  return (t->tm_sec * 1000 + tv.tv_usec / 1000);
}

}  // namespace app
}  // namespace airos
