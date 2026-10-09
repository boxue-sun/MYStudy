/*********************************************************************************
 * @file		map_generator.h
 * @brief		
 * @details		
 * @author		ChangXuhui
 * @date		2024/03/07 
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/03/07   ChangXuhui       1.0       ————          Create this file   							   
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <thread>

#include "app/framework/interface/app_base.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/modules/v2x_application/common/app_flag.h"
#include "app/modules/v2x_application/common/producer_consumer_queue.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"

namespace airos {
namespace app {

class MapGenerator {
 public:
  MapGenerator(){};
  ~MapGenerator(){};

  bool Init(const airos::app::ApplicationCallBack& send_cb,
            const std::string& app_conf_path);
  void ThreadMap();

 private:
  airos::app::ApplicationCallBack send_;

  std::shared_ptr<std::thread> sptr_map_{nullptr};
  std::shared_ptr<v2xpb::asn::MessageFrame> pb_map_{nullptr};
  std::string xml_map_file_{""};
  int xml_map_send_rate_{1000};
  int msg_cnt_{0};
	EnAsnType asn_type_{EnAsnType::YDT_3709_2020};
    bool enable_print = false;
};

}  // namespace app
}  // namespace airos
