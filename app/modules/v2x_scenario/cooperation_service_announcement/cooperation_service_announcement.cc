/*********************************************************************************
 * @file		cooperation_service_announcement.cpp
 * @brief		cooperation_service_announcement
 * @details		cooperation_service_announcement
 * @author		ChangXuhui
 * @date		2024/6/26
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/6/26  ChangXuhui      1.0          ————               业务能力广播
 *
 *
 * @endverbatim
 ********************************************************************************/

#include "cooperation_service_announcement.h"

#include "base/work_param/configer_om_work_param.h"

namespace airos {
namespace app {

bool CooperationServiceAnnouncement::Init(
    const airos::app::ApplicationCallBack& send_cb,
    const std::string& app_conf_path) {
  APP_LOG_INFO << "CooperationServiceAnnouncement init";
  sender_ = send_cb;

  // 从工参表中获取路口的ID
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();
  int rsu_intersection_id = omWorkParamConfiger.mecDeviceWorkParam.crossID;
  if (rsu_intersection_id < 10) {
    sam_id_ = "beij#00" + std::to_string(rsu_intersection_id);
  } else if (rsu_intersection_id < 100) {
    sam_id_ = "beij#0" + std::to_string(rsu_intersection_id);
  } else {
    sam_id_ = "beij#" + std::to_string(rsu_intersection_id);
  }

  // 生成Sam消息
  GenerateSam();

  periodic_broadcast_ = std::make_shared<std::thread>(
      std::bind(&CooperationServiceAnnouncement::PeriodicBroadcast, this));
  return true;
}

void CooperationServiceAnnouncement::PeriodicBroadcast() {
  auto message_pb = std::make_shared<airos::app::ApplicationData>();
  message_pb->mutable_road_side_frame()->operator=(asn_msg_frame_);

  while (true) {
    message_pb->mutable_road_side_frame()
        ->mutable_extframe()
        ->mutable_messagevalue()
        ->mutable_serviceannouncementmessage()
        ->set_msgcnt(msg_cnt_++);

    msg_cnt_ = msg_cnt_ > 127 ? 0 : msg_cnt_;

    sender_(message_pb);

    // 1s 为周期发送
    std::chrono::microseconds duration(1000000);
    std::this_thread::sleep_for(duration);
  }
}

void CooperationServiceAnnouncement::GenerateSam() {
  auto msgext = asn_msg_frame_.mutable_extframe();
  msgext->set_messageid(28);
  auto msgvalue = msgext->mutable_messagevalue();

  msgvalue->set_typresent(
      v2xpb::asn::MessageFrameExt__value::
          MessageFrameExt__value_PR_ServiceAnnouncementMessage);
  auto sam = msgvalue->mutable_serviceannouncementmessage();
  sam->set_msgcnt(msg_cnt_);
  sam->set_id(sam_id_);
  sam->set_secmark(get_mill_second_minute());

  auto maneuverCooperation = sam->mutable_maneuvercooperation();
  maneuverCooperation->set_equipmenttype(v2xpb::asn::EquipmentType::rsu);

  maneuverCooperation->add_intcooperation(
      v2xpb::asn::IntentCooperation::rsuCoopLaneChange);
  maneuverCooperation->add_intcooperation(
      v2xpb::asn::IntentCooperation::rsuCoopLaneMerge);
  maneuverCooperation->add_intcooperation(
      v2xpb::asn::IntentCooperation::rsuCoopSignalIntersection);

  APP_LOG_INFO << asn_msg_frame_.DebugString();
}

int64_t CooperationServiceAnnouncement::get_mill_second_minute() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0) {
    return -1;
  }
  struct tm* t = nullptr;
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
