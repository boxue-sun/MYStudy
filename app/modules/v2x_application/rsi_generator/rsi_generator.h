/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#pragma once

#include <string>
#include <thread>

#include "air_service/framework/proto/airos_usecase.pb.h"
#include "app/framework/interface/app_base.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/modules/v2x_application/proto/v2xpb-config-event-details.pb.h"
#include "app/modules/v2x_application/proto/v2xpb-rsi-config.pb.h"
#include "app/modules/v2x_application/rsi_generator/affect_path/v2x_affect_path.h"

namespace airos {
namespace app {

class RsiGenerator {
 public:
  typedef std::shared_ptr<const v2xpb::rscu::config::RteEventDetail>
      RteEventDetailPtr;
  
  typedef std::shared_ptr<const v2xpb::rscu::config::SpecialEventDetail>
      SpecialEventDetailPtr;

  RsiGenerator(){};

  virtual ~RsiGenerator(){};

  bool Init(
      const airos::app::ApplicationCallBack& send_cb,
      const std::string& app_conf_path);
  bool Proc(const std::shared_ptr<const airos::usecase::EventOutputResult>&
                event_ptr);

 private:
  void GenerateRsiMsg(v2xpb::asn::Rsi* rsi_pb, int msg_cnt);
  bool GetRsuMap(const std::string& rsu_map);

  /**
   * @brief 发送配置文件中配置的RSI
  */
  void RsiConfTransmit();

  /**
   * @brief 判断当前时间是否在时间段内
   * @param start_time 开始时间
   * @param end_time 结束时间
   * @return bool true: 当前时间在时间段内; false: 当前时间不在事件段内
  */
  bool isWithinTimeRange(std::string start_time, std::string end_time);

 private:
  std::string rscu_sn_;
  static int msg_cnt_;
  std::shared_ptr<v2xpb::asn::MessageFrame> asn_map_data_;
  std::shared_ptr<v2xpb::asn::MessageFrame> asn_pb_data_;
  int zone_;
  double cross_lat_        = 0.0;
  double cross_lon_        = 0.0;
  std::string city_string_ = "beij#";  // length 5 byte
  int rsu_intersection_id_ = 10;
  std::shared_ptr<RsiAffectPath> affect_path_;
  v2xpb::rscu::config::EventConfig conf_;
  std::map<int, RteEventDetailPtr> ev_map_;
  std::vector<SpecialEventDetailPtr> special_event_list_;
  airos::app::ApplicationCallBack sender_;

  v2xpb::rsi::config::RSIConfig rsi_conf_;  // 配置文件中配置的RSI事件
  std::shared_ptr<std::thread> rsi_conf_transmit_{nullptr};  // 单独启一个线程, 发送配置文件配置的Rsi
};

}  // namespace app
}  // namespace airos
