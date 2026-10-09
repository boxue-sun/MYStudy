/*********************************************************************************
 * @file		mec_adapter_ssm.h
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

#pragma once

#include <memory>
#include <string>

#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/framework/interface/app_base.h"
#include "app/modules/v2x_application/common/producer_consumer_queue.h"

namespace airos {
namespace app {

class MECAdapterSsm {
 public:
  MECAdapterSsm(){};
  ~MECAdapterSsm(){};

  bool Init(const airos::app::ApplicationCallBack& send_cb);
  bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>&
                recv_data);
  int64_t get_mill_second_minute();

 private:
  airos::app::ApplicationCallBack send_;
  std::shared_ptr<v2xpb::asn::MessageFrame> pb_ssm_{nullptr};
  
  int ssm_count_ = 0;
  // 收到package的数量
  uint64_t recv_total_package_num = 0;
  // 收到目标的数量
  uint64_t recv_total_obj_num = 0;
};

}  // namespace app
}  // namespace airos
