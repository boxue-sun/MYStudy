/*********************************************************************************
 * @file		mec_adapter.h
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

#pragma once

#include <memory>
#include <string>

#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/framework/interface/app_base.h"
#include "app/modules/v2x_application/common/producer_consumer_queue.h"

namespace airos {
namespace app {

enum ClassificationEnum {
    CLASSFICIATION_TYPE_AMBULANCE = 65,
    CLASSFICIATION_TYPE_FIRE_TRUCK = 62,
    CLASSFICIATION_TYPE_POLICE_CAR = 66,
    CLASSFICIATION_TYPE_SEDAN = 10,
    CLASSFICIATION_TYPE_VAN = 26,
    CLASSFICIATION_TYPE_TRUCK = 20,
    CLASSFICIATION_TYPE_BUS = 56,
    CLASSFICIATION_TYPE_BICCYCLE = 85,
    CLASSFICIATION_TYPE_MOTORCYCLE = 40,
    CLASSFICIATION_TYPE_TRIKE = 47,
};

class MECAdapter {
 public:
  MECAdapter(){};
  ~MECAdapter(){};

  bool Init(const airos::app::ApplicationCallBack& send_cb);
  bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>&
                recv_data);
  int64_t get_mill_second_minute();

 private:
  airos::app::ApplicationCallBack send_;
  
  int rsm_count_ = 0;
  // max ptc 16
  static const uint32_t MAX_PTC = 16;

  // 收到package的数量
  uint64_t recv_total_package_num = 0;
  // 收到目标的数量
  uint64_t recv_total_obj_num = 0;
};

}  // namespace app
}  // namespace airos
