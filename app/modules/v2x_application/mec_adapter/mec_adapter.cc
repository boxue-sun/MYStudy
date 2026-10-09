/*********************************************************************************
 * @file		mec_adapter.cpp
 * @brief		
 * @details		
 * @author		ChangXuhui
 * @date		2024/02/27 
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/02/27   ChangXuhui       1.0       ————          Create this file   							   
 * @endverbatim
 ********************************************************************************/

#include "mec_adapter.h"

#include <sys/time.h>

#include <vector>
#include <thread>

#include "base/common/math_util.h"
#include "base/common/time_util.h"
#include "base/env/env.h"
#include "base/common/log.h"

namespace airos {
namespace app {

bool MECAdapter::Init(const airos::app::ApplicationCallBack& send_cb) {
  send_ = send_cb;
  return true;
}

bool MECAdapter::Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>&
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
  APP_LOG_INFO << "summary: Expected recv total package num: "
        << recv_data->header().total_package_num()
        << ", Actual recv package num: " << recv_total_package_num
        << ", Packet loss rate: " << std::fixed
        << (recv_data->header().total_package_num() -
            recv_total_package_num) *
               1.0 / recv_data->header().total_package_num();

  APP_LOG_INFO << "summary: Expected recv total obj num: "
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
  bool need_split = pb_ptc_cnt > MAX_PTC;

  for (uint32_t i = 0; i < pb_ptc_cnt; i += MAX_PTC) {  // 每16个包发送一次
    auto asn_pb = std::make_shared<v2xpb::asn::MessageFrame>();
    auto rsm = asn_pb->mutable_rsmframe();
    // 设置 Rsm 的字段值
    rsm_count_ = (rsm_count_ >= 127) ? 1 : (rsm_count_ + 1);
    rsm->set_msg_cnt(rsm_count_);
    rsm->set_id("12345678");

    // 创建 Position 对象并设置字段值,(后面看看从地图获取)
    v2xpb::asn::Position *position = rsm->mutable_refpos();
    v2xpb::asn::Position::LLH *llh = position->mutable_llh();
    llh->set_latitude(37.123456);
    llh->set_longitude(122.654321);
    llh->set_elevation(10.4);
    // 创建 ParticipantData
    // 对象并设置字段值，除经纬度和海拔不需要单位转换外，其余参数需要按ASN编码处理单位
    uint32_t send_ptc_size = 0;
    if (need_split) {
      if (i + MAX_PTC + 1 < pb_ptc_cnt) {
        send_ptc_size = MAX_PTC + i;
      } else {
        send_ptc_size = pb_ptc_cnt;
        // 丢包率时延统计
        rsm->set_end_flag(true);
        rsm->set_total_obj_num(recv_data->header().total_obj_num());
        rsm->set_message_timestamp(recv_data->header().timestamp_sec());
      }
    } else {
      send_ptc_size = pb_ptc_cnt;
      // 丢包率时延统计
      rsm->set_end_flag(true);
      rsm->set_total_obj_num(recv_data->header().total_obj_num());
      rsm->set_message_timestamp(recv_data->header().timestamp_sec());
    }


    for (uint32_t j = i; j < send_ptc_size; ++j) {
      auto obstacle = recv_data->perception_obstacle()[j];
      v2xpb::asn::ParticipantData *participant = rsm->add_participants();
      participant->set_ptc_id(obstacle.id());

      if (obstacle.has_type()) {
        APP_LOG_WARN << "[source-type]" << obstacle.type();
        switch (obstacle.type()) {
          case airos::perception::PerceptionObstacle_Type_UNKNOWN:
            participant->set_ptc_type(v2xpb::asn::ParticipantType::PT_UNKNOWN);
            break;
          case airos::perception::PerceptionObstacle_Type_VEHICLE:
            participant->set_ptc_type(v2xpb::asn::ParticipantType::PT_MOTOR);
            break;
          case airos::perception::PerceptionObstacle_Type_BICYCLE:
            participant->set_ptc_type(
                v2xpb::asn::ParticipantType::PT_NON_MOTOR);
            break;
          case airos::perception::PerceptionObstacle_Type_PEDESTRIAN:
            participant->set_ptc_type(
                v2xpb::asn::ParticipantType::PT_PEDESTRIAN);
            break;
          // case os::v2x::device::MecPtcType::MEC_PTC_RSU:
          //   participant->set_ptc_type(v2xpb::asn::ParticipantType::PT_RSU);
          //   break;
          default:
            APP_LOG_WARN << "not asn source type:" << obstacle.type();
            participant->set_ptc_type(v2xpb::asn::ParticipantType::PT_UNKNOWN);
            break;
        }
      }    
      // source 先固定v2xpb::asn::SourceType::ST_VIDEO
      participant->set_source(v2xpb::asn::SourceType::ST_VIDEO);

      v2xpb::asn::Position *position = participant->mutable_pos();
      v2xpb::asn::Position::XYZ *xyz = position->mutable_xyz();
      
      xyz->set_x(obstacle.position().x());
      xyz->set_y(obstacle.position().y());
      xyz->set_z(obstacle.position().z());
      xyz->set_zone(obstacle.position().zone());

      participant->set_speed(
          airos::base::MathUtil::convertSpeedF2I(obstacle.velocity().x()));
      participant->set_heading(
          airos::base::MathUtil::getValidDegAngle(obstacle.theta()) / 0.0125);

      v2xpb::asn::VehicleSize *size = participant->mutable_size();
      size->set_length(airos::base::MathUtil::convertM2CM(obstacle.length()));
      size->set_width(airos::base::MathUtil::convertM2CM(obstacle.width()));
      size->set_height(
          airos::base::MathUtil::convertHeightF2I(obstacle.height()));

        if (obstacle.has_vehicle_type()) {
            v2xpb::asn::VehicleClassification *vehicleClassification = participant->mutable_vehicle_class();
            switch (obstacle.vehicle_type()) {
                case airos::perception::VEHICLE_TYPE_AMBULANCE:
                    vehicleClassification->set_classification(airos::app::ClassificationEnum::CLASSFICIATION_TYPE_AMBULANCE);
                    break;
                case airos::perception::VEHICLE_TYPE_FIRE_TRUCK:
                    vehicleClassification->set_classification(airos::app::ClassificationEnum::CLASSFICIATION_TYPE_FIRE_TRUCK);
                    break;
                case airos::perception::VEHICLE_TYPE_POLICE_CAR:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_POLICE_CAR);
                    break;
                case airos::perception::VEHICLE_TYPE_SEDAN:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_SEDAN);
                    break;
                case airos::perception::VEHICLE_TYPE_VAN:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_VAN);
                    break;
                case airos::perception::VEHICLE_TYPE_TRUCK:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_TRUCK);
                    break;
                case airos::perception::VEHICLE_TYPE_BUS:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_BUS);
                    break;
                default:
                    APP_LOG_WARN << "not asn source type:" << obstacle.type();
                    break;
            }
        }
        if (obstacle.has_non_vehicle_type()) {
            v2xpb::asn::VehicleClassification *vehicleClassification = participant->mutable_vehicle_class();
            switch (obstacle.non_vehicle_type()) {
                case airos::perception::NON_VEHICLE_TYPE_BICYCLE:
                    vehicleClassification->set_classification(airos::app::ClassificationEnum::CLASSFICIATION_TYPE_BICCYCLE);
                    break;
                case airos::perception::NON_VEHICLE_TYPE_MOTORCYCLE:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_MOTORCYCLE);
                    break;
                case airos::perception::NON_VEHICLE_TYPE_TRICYCLE:
                    vehicleClassification->set_classification(
                            airos::app::ClassificationEnum::CLASSFICIATION_TYPE_TRIKE);
                    break;
                default:
                    APP_LOG_WARN << "not asn source type:" << obstacle.type();
                    break;
            }
        }

      participant->set_sec_mark(get_mill_second_minute()); 
    }

    auto message_pb = std::make_shared<airos::app::ApplicationData>();
    message_pb->mutable_road_side_frame()->operator=(*asn_pb);
    // APP_LOG_INFO << "generated:" << message_pb->DebugString();

    // 控制发送间隔
    static auto last_send_time = std::chrono::steady_clock::now();
    const std::chrono::microseconds min_interval(1200); // 最小写入间隔1.2ms
  
    auto current_time = std::chrono::steady_clock::now();
    auto time_since_last = current_time - last_send_time;
  
    if (time_since_last < min_interval) {
      // 如果距离上次发送时间不足1ms,则等待剩余时间
      std::this_thread::sleep_for(min_interval - time_since_last);
    }
    send_(message_pb);
    last_send_time = std::chrono::steady_clock::now();

    // send_(message_pb);
    // std::chrono::microseconds duration(1000);
    // std::this_thread::sleep_for(duration);
  }
  return true;
}

int64_t MECAdapter::get_mill_second_minute() {
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
