/*********************************************************************************
 * @file		cooperation_service_announcement.h
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

#pragma once

#include <thread>

#include "app/framework/interface/app_base.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"

namespace airos {
namespace app {

class CooperationServiceAnnouncement {
 public:
  CooperationServiceAnnouncement(){};

  virtual ~CooperationServiceAnnouncement(){};

  bool Init(const airos::app::ApplicationCallBack& send_cb,
            const std::string& app_conf_path);

 private:
  /**
   * @brief 业务能力周期广播
   * @return
   */
  void PeriodicBroadcast();
  /**
   * @brief 生成Sam消息
   * @return
   */
  void GenerateSam();
  /**
   * @brief 获取时间戳
   * @return
   */
  int64_t get_mill_second_minute();

 private:
  std::shared_ptr<std::thread> periodic_broadcast_{nullptr};
  airos::app::ApplicationCallBack sender_;
  v2xpb::asn::MessageFrame asn_msg_frame_;
  uint8_t msg_cnt_ = 0;
  std::string sam_id_;
};

}  // namespace app
}  // namespace airos
