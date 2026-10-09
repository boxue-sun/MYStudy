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
#include "rsi_generator.h"

#include <fstream>
#include <sstream>

#include "app/modules/v2x_application/common/app_flag.h"
#include "base/common/log.h"
#include "base/env/env.h"
#include "base/io/protobuf_util.h"
#include "base/work_param/configer_om_work_param.h"
#include "gflags/gflags.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"

namespace airos {
namespace app {

int RsiGenerator::msg_cnt_ = 0;

bool RsiGenerator::GetRsuMap(const std::string& rsu_map) {
  std::string asn_version(FLAGS_asn_message_version);
  EnAsnType asn_type{EnAsnType::CASE_53_2020};
  if (asn_version.compare("4layer") == 0) {
    asn_type = EnAsnType::CASE_53_2020;
  } else if (asn_version.compare("new_4layer") == 0) {
    asn_type = EnAsnType::YDT_3709_2020;

  } else if (asn_version.compare("new_4layer_ext") == 0) {
    asn_type = EnAsnType::YDT_3709_2020_EXT;
    
  } else {
    // pass
  }

  std::fstream fs(rsu_map.data(), std::fstream::in);
  if (!fs.good()) {
    APP_LOG_WARN << "xml file open false:" << rsu_map;
    return false;
  }
  std::stringstream ss;
  ss << fs.rdbuf();
  fs.close();
  std::string content(ss.str());

  if (content.empty()) {
    APP_LOG_WARN << "xml map data null";
    return false;
  }

  std::string str_asn("");
  // 先使用智路的地图进行解析，不成功再使用asn xer的地图格式
  if (message_frame_map_xml2uper_adapter(content, &str_asn, asn_type) > 0 ) {
    APP_LOG_INFO << " map xml to uper: true, use airos xml2uper";
  } else if (message_frame_xer2uper_adapter(content, &str_asn, asn_type) > 0) {
    APP_LOG_INFO << " map xml to uper: true, use xer2uper";
  } else {
    APP_LOG_WARN << " map xml to uper: false";
    return false;
  }

  std::string str_pb("");
  if (0 >= message_frame_uper2pbstr_adapter(str_asn, &str_pb, asn_type)) {
    APP_LOG_WARN << "asn to pb false";
    return false;
  }
  APP_LOG_INFO << "asn to pb true";

  asn_map_data_ = std::make_shared<v2xpb::asn::MessageFrame>();
  if (asn_map_data_) {
    asn_map_data_->ParsePartialFromString(str_pb);
  } else {
    APP_LOG_WARN << "map pb null";
    return false;
  }
  if (asn_map_data_->payload_case() !=
      v2xpb::asn::MessageFrame::PayloadCase::kMapFrame) {
    APP_LOG_ERROR << "MAP PB get failed.";
    return false;
  }
  APP_LOG_INFO << asn_map_data_->DebugString();
  auto asn_map = asn_map_data_->mapframe();
  if (asn_map.nodes_size() == 0) {
    APP_LOG_WARN << "MAP WRONG.";
    return false;
  }
  auto nodes = asn_map.nodes(0);
  if (nodes.has_position() && nodes.position().has_llh()) {
    auto pos   = nodes.position().llh();
    cross_lat_ = pos.latitude();
    cross_lon_ = pos.longitude();
  }
  APP_LOG_WARN << "cross_lat : " << cross_lat_ << " cross_lon : " << cross_lon_;
  return true;
}

bool RsiGenerator::Init(
    const airos::app::ApplicationCallBack& send_cb,
    const std::string& app_conf_path) {
  rscu_sn_               = airos::base::Environment::GetDeviceSn();
  std::string event_file = app_conf_path + "/v2x_config_event_details.pb";
  if (!airos::base::ParseProtobufFromFile<v2xpb::rscu::config::EventConfig>(
          event_file, &conf_)) {
    APP_LOG_ERROR << "parse protobuf from file failed.";
    return false;
  } else {
    APP_LOG_INFO << "parse protobuf from file succeed.";
  }

  APP_LOG_INFO << "conf: " << conf_.DebugString();

  for (int ev_num = 0; ev_num < conf_.basic_event_config().event_detail_size(); ++ev_num) {
    auto ev_detail = conf_.basic_event_config().event_detail(ev_num);
    if (!ev_detail.has_event_type_mec()) {
      continue;
    }
    
    APP_LOG_INFO << "event mec type config: " << ev_detail.event_type_mec();
    RteEventDetailPtr rte_event_ptr(
        new v2xpb::rscu::config::RteEventDetail(ev_detail.rte_detail()));
    ev_map_[ev_detail.event_type_mec()] = rte_event_ptr;
  }

  for (int i = 0; i < conf_.special_event_config().special_event_detail_size(); ++i) {
    auto special_event_detail =
        conf_.special_event_config().special_event_detail(i);
    if (!special_event_detail.has_event_type_mec_list()) {
      continue;
    }

    SpecialEventDetailPtr special_event_detail_ptr(
        new v2xpb::rscu::config::SpecialEventDetail(special_event_detail));
    special_event_list_.push_back(special_event_detail_ptr);
  }

  affect_path_         = std::make_shared<RsiAffectPath>();
  city_string_         = FLAGS_city_string;
  sender_              = send_cb;
  
  std::string map_path;
  
  // map_path 和 rsu_intersection_id rsi_config_file从工参表中获取 
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();
  rsu_intersection_id_ = omWorkParamConfiger.mecDeviceWorkParam.crossID;

  if (!omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile.empty()) {
    map_path = "/home/airos/common_config/" +
                    omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile;
  } else {
    APP_LOG_ERROR << "XmlMapFile is empty";
    return false;
  }

  if (!GetRsuMap(map_path)) {
    APP_LOG_ERROR << "Get Rsu Map Failed." << map_path;
    return false;
  }
  zone_                = 31 + cross_lon_ / 6;

  if (!omWorkParamConfiger.mecDeviceWorkParam.rsiConfigFile.empty()) {
    std::string rsi_config_file = "/home/airos/common_config/" +
      omWorkParamConfiger.mecDeviceWorkParam.rsiConfigFile;

    if (!airos::base::ParseProtobufFromFile<v2xpb::rsi::config::RSIConfig>(
            rsi_config_file, &rsi_conf_)) {
      APP_LOG_ERROR << "parse protobuf from file failed. File path: "
                << rsi_config_file;
    } else {
      APP_LOG_INFO << "parse protobuf from file succeed. File path: "
               << rsi_config_file;

      APP_LOG_INFO << "conf: " << rsi_conf_.DebugString();

      // 单独起一个线程，发送配置文件中配置的rsi
      rsi_conf_transmit_ = std::make_shared<std::thread>(
          std::bind(&RsiGenerator::RsiConfTransmit, this));
    }
  }
  return true;
}

bool RsiGenerator::Proc(
    const std::shared_ptr<const airos::usecase::EventOutputResult>& event_ptr) {
  if (event_ptr->ByteSizeLong() == 0) {
    APP_LOG_ERROR << "v2xobstacles recv data is nullptr.";
    return false;
  }
  int rte_count = 0;
  // APP_LOG_INFO << event_ptr->DebugString();
  v2xpb::asn::Rsi* rsi_pb_data;
  for (int index = 0; index < event_ptr->events_size(); ++index) {
    auto event_info = event_ptr->events(index);
    int event_type_mec =  event_info.event_type_mec();
    APP_LOG_INFO << "Event Ocurred, event type mec: " << event_type_mec;

    if (ev_map_.find(event_type_mec) == ev_map_.end()) {
      APP_LOG_INFO << "UNDEFINED event type: " << event_type_mec;
      continue;
    }
    auto ev_detail = ev_map_[event_type_mec];
    if (!ev_detail->has_priority()) {
      APP_LOG_INFO << "CONFIG has no type id or description.";
      continue;
    }
    if (rte_count == 0) {
      asn_pb_data_ = std::make_shared<v2xpb::asn::MessageFrame>();
      rsi_pb_data  = asn_pb_data_->mutable_rsiframe();
    }
    auto v2x_rte = rsi_pb_data->add_rtes();
    v2x_rte->set_event_type(ev_detail->event_type_id());
    v2x_rte->set_priority(ev_detail->priority());
    v2x_rte->set_rte_id(msg_cnt_);
    v2x_rte->set_event_source(v2xpb::asn::RsiRte_Source_SRC_DETECTION);
    v2x_rte->set_event_radius(25);
    auto position     = v2x_rte->mutable_position();
    auto position_xyz = position->mutable_xyz();
    if (event_info.location_point().has_zone()) {
      position_xyz->set_zone(event_info.location_point().zone());
    } else {
      position_xyz->set_zone(zone_);
    }
    position_xyz->set_x(event_info.location_point().x());
    position_xyz->set_y(event_info.location_point().y());
    airos::perception::usecase::Vec2d vec2d(
        event_info.location_point().x(), event_info.location_point().y());
    std::vector<v2xpb::asn::RsiReferencePath> affect_paths;

    // 先判断配置的特殊事件
    affect_path_->GetAffectPathBySpecialConf(event_type_mec,
                                             vec2d,
                                             special_event_list_,
                                             asn_map_data_->mapframe(),
                                             &affect_paths);

    // 如果配置的特殊事件匹配失败, 再通过通用的方法进行配置
    if (affect_paths.empty()) {
      APP_LOG_INFO << "Get affect path by generic approach";
      affect_path_->GetAffectPath(ev_detail, vec2d, asn_map_data_->mapframe(),
                                  &affect_paths);
    }

    for (auto affect_path : affect_paths) {
      v2x_rte->add_ref_paths()->operator=(affect_path);
    }
    v2x_rte->set_description(ev_detail->description());
    rte_count++;
    if (rte_count == 8) {
      GenerateRsiMsg(rsi_pb_data, msg_cnt_);
      msg_cnt_++;
      if (msg_cnt_ > 127) {
        msg_cnt_ = 0;
      }
      APP_LOG_INFO << asn_pb_data_->DebugString();
      auto message_pb = std::make_shared<airos::app::ApplicationData>();
      message_pb->mutable_road_side_frame()->operator=(*asn_pb_data_);
      sender_(message_pb);
      rsi_pb_data = nullptr;
      rte_count   = 0;
    }
  }
  if (rte_count > 0) {
    GenerateRsiMsg(rsi_pb_data, msg_cnt_);
    msg_cnt_++;
    if (msg_cnt_ > 127) {
      msg_cnt_ = 0;
    }
    APP_LOG_INFO << asn_pb_data_->DebugString();
    auto message_pb = std::make_shared<airos::app::ApplicationData>();
    message_pb->mutable_road_side_frame()->operator=(*asn_pb_data_);
    sender_(message_pb);
    rsi_pb_data = nullptr;
    rte_count   = 0;
  }
  return true;
}

void RsiGenerator::GenerateRsiMsg(v2xpb::asn::Rsi* rsi_pb, int msg_cnt) {
  if (rsu_intersection_id_ < 10) {
    rsi_pb->set_rsi_id(
        city_string_ + "00" + std::to_string(rsu_intersection_id_));
  } else if (rsu_intersection_id_ < 100) {
    rsi_pb->set_rsi_id(
        city_string_ + "0" + std::to_string(rsu_intersection_id_));
  } else {
    rsi_pb->set_rsi_id(city_string_ + std::to_string(rsu_intersection_id_));
  }
  rsi_pb->set_message_count(msg_cnt);
  auto ref_llh = rsi_pb->mutable_position()->mutable_llh();
  ref_llh->set_latitude(cross_lat_);
  ref_llh->set_longitude(cross_lon_);
}

// 从配置文件中发送rsi数据
void RsiGenerator::RsiConfTransmit() {
  if (!rsi_conf_.send_enable()) {
    APP_LOG_WARN << "Send rsi enable is false, return";
    return;
  }

  int rsi_content_size = rsi_conf_.rsi_contents().rsi_content_size();
  APP_LOG_INFO << "Rsi content size is " << rsi_content_size;
  if (rsi_content_size < 1) {
    APP_LOG_WARN << "rsi content is empty!, return";
  }
  
  // 发送的周期：s --> us
  int32_t send_period = rsi_conf_.send_period() * 1000 * 1000;

  while (true) {
    v2xpb::asn::MessageFrame asn_msg_frame;
    auto rsi_pb_data = asn_msg_frame.mutable_rsiframe();
    for (int i = 0; i < rsi_content_size; ++i) {
      auto rsi_content = rsi_conf_.rsi_contents().rsi_content(i);
      if (rsi_content.rsi_msg_id() < 0) {
        continue;
      }

      // 判断是否在设置的时间段内
      if (rsi_content.is_time_set_valid() &&
          !isWithinTimeRange(rsi_content.start_time(), rsi_content.end_time())) {
        APP_LOG_INFO << "Rsi content not in time range, msg id: "
                 << rsi_content.rsi_msg_id() << ", ignore";
        continue;
      }

      // 打包成Rte发送
      if (rsi_content.pack_as_rte()) {
        auto rte = rsi_pb_data->add_rtes();
        rte->set_rte_id(rsi_content.rsi_msg_id());
        rte->set_event_type(rsi_content.alert_type());
        rte->set_event_source(v2xpb::asn::RsiRte::SRC_UNKNOWN);
        
        auto position = rte->mutable_position();
        auto position_llh = position->mutable_llh();
        position_llh->set_latitude(rsi_content.position().latitude());
        position_llh->set_longitude(rsi_content.position().longitude());
        position_llh->set_elevation(rsi_content.position().elevation());

        rte->set_event_radius(rsi_content.enent_radius());
        rte->set_priority(rsi_content.priority());
        rte->set_description(rsi_content.alert_desc());

        // 按照点位填充, 影响路径
        if (rsi_content.has_alert_paths() &&
            rsi_content.alert_paths().position_list_size() > 0) {
          for (int j = 0; j < rsi_content.alert_paths().position_list_size(); ++j) {
            auto position_list = rsi_content.alert_paths().position_list(j);
            auto ref_paths = rte->add_ref_paths();
            ref_paths->set_radius(position_list.radius());

            for (int k = 0; k < position_list.position_size(); ++k) {
              auto pos = position_list.position(k);
              auto point = ref_paths->add_points();
              auto point_llh = point->mutable_llh();
              point_llh->set_latitude(pos.latitude());
              point_llh->set_longitude(pos.longitude());
              point_llh->set_elevation(pos.elevation());
            }
          }
        }

        // 按照link来配置影响路径
        if (rsi_content.has_alert_links() &&
            rsi_content.alert_links().alert_link_size() > 0) {
          for (int j = 0; j < rsi_content.alert_links().alert_link_size(); ++j) {
            auto alert_link = rsi_content.alert_links().alert_link(j);
            auto ref_link = rte->add_ref_links();
            // 上游节点的ID
            auto upstream_node_id = ref_link->mutable_upstreamnode();
            upstream_node_id->set_region(alert_link.upstream_node_id().region());
            upstream_node_id->set_id(alert_link.upstream_node_id().id());

            // 下游节点ID
            auto downstream_node_id = ref_link->mutable_downstreamnode();
            downstream_node_id->set_region(alert_link.node_id().region());
            downstream_node_id->set_id(alert_link.node_id().id());

            // 所影响的车道
            for (int k = 0; k < alert_link.reference_lanes().lane_id_size(); ++k) {
              auto lane_id = alert_link.reference_lanes().lane_id(k);
              if (lane_id > 15) {
                APP_LOG_WARN << "Lane id: " << lane_id << "is more than 15, ignore";
                continue;
              }
              switch (lane_id) {
                case 0:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::ReferenceLane_reserved);
                  break;
                case 1:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane1);
                  break;
                case 2:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane2);
                  break;
                case 3:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane3);
                  break;
                case 4:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane4);
                  break;
                case 5:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane5);
                  break;
                case 6:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane6);
                  break;
                case 7:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane7);
                  break;
                case 8:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane8);
                  break;
                case 9:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane9);
                  break;
                case 10:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane10);
                  break;
                case 11:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane11);
                  break;
                case 12:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane12);
                  break;
                case 13:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane13);
                  break;
                case 14:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane14);
                  break;
                case 15:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane15);
                  break;
                default:
                  APP_LOG_WARN << "Lane id: " << lane_id
                           << "is out of range, ignore";
                  break;
              }
            }
          }
        }
      } else {
        // 打包成Rts发送
        auto rts = rsi_pb_data->add_rtss();
        rts->set_rts_id(rsi_content.rsi_msg_id());
        rts->set_sign_type(rsi_content.alert_type());
        
        auto position = rts->mutable_position();
        auto position_llh = position->mutable_llh();
        position_llh->set_latitude(rsi_content.position().latitude());
        position_llh->set_longitude(rsi_content.position().longitude());
        position_llh->set_elevation(rsi_content.position().elevation());

        rts->set_description(rsi_content.alert_desc());
        rts->set_priority(rsi_content.priority());

        // 按照点位填充, 影响路径
        if (rsi_content.has_alert_paths() &&
            rsi_content.alert_paths().position_list_size() > 0) {
          for (int j = 0; j < rsi_content.alert_paths().position_list_size(); ++j) {
            auto position_list = rsi_content.alert_paths().position_list(j);
            auto ref_paths = rts->add_ref_paths();
            ref_paths->set_radius(position_list.radius());

            for (int k = 0; k < position_list.position_size(); ++k) {
              auto pos = position_list.position(k);
              auto point = ref_paths->add_points();
              auto point_llh = point->mutable_llh();
              point_llh->set_latitude(pos.latitude());
              point_llh->set_longitude(pos.longitude());
              point_llh->set_elevation(pos.elevation());
            }
          }
        }

        // 按照link来配置影响路径
        if (rsi_content.has_alert_links() &&
            rsi_content.alert_links().alert_link_size() > 0) {
          for (int j = 0; j < rsi_content.alert_links().alert_link_size(); ++j) {
            auto alert_link = rsi_content.alert_links().alert_link(j);
            auto ref_link = rts->add_ref_links();
            // 上游节点的ID
            auto upstream_node_id = ref_link->mutable_upstreamnode();
            upstream_node_id->set_region(alert_link.upstream_node_id().region());
            upstream_node_id->set_id(alert_link.upstream_node_id().id());

            // 下游节点ID
            auto downstream_node_id = ref_link->mutable_downstreamnode();
            downstream_node_id->set_region(alert_link.node_id().region());
            downstream_node_id->set_id(alert_link.node_id().id());

            // 所影响的车道
            for (int k = 0; k < alert_link.reference_lanes().lane_id_size(); ++k) {
              auto lane_id = alert_link.reference_lanes().lane_id(k);
              if (lane_id > 15) {
                APP_LOG_WARN << "Lane id: " << lane_id << "is more than 15, ignore";
                continue;
              }
              switch (lane_id) {
                case 0:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::ReferenceLane_reserved);
                  break;
                case 1:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane1);
                  break;
                case 2:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane2);
                  break;
                case 3:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane3);
                  break;
                case 4:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane4);
                  break;
                case 5:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane5);
                  break;
                case 6:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane6);
                  break;
                case 7:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane7);
                  break;
                case 8:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane8);
                  break;
                case 9:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane9);
                  break;
                case 10:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane10);
                  break;
                case 11:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane11);
                  break;
                case 12:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane12);
                  break;
                case 13:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane13);
                  break;
                case 14:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane14);
                  break;
                case 15:
                  ref_link->add_referencelanes(
                      v2xpb::asn::ReferenceLane::lane15);
                  break;
                default:
                  APP_LOG_WARN << "Lane id: " << lane_id
                           << "is out of range, ignore";
                  break;
              }
            }
          }
        }
      }

      GenerateRsiMsg(rsi_pb_data, 10);
      APP_LOG_INFO << rsi_pb_data->DebugString();
      auto message_pb = std::make_shared<airos::app::ApplicationData>();
      message_pb->mutable_road_side_frame()->operator=(asn_msg_frame);
      sender_(message_pb);
    }

    // 控制发送的周期
    std::chrono::microseconds duration(send_period);
    std::this_thread::sleep_for(duration);
  }
}

bool RsiGenerator::isWithinTimeRange(std::string start_time,
                                     std::string end_time) {
  // 解析开始时间
  std::stringstream startTimeStream(start_time);

  int startHour, startMinute, startSecond;
  char delimiter;
  startTimeStream >> startHour >> delimiter >> startMinute >> delimiter >>
      startSecond;

  // 解析结束时间
  std::stringstream endTimeStream(end_time);
  int endHour, endMinute, endSecond;
  endTimeStream >> endHour >> delimiter >> endMinute >> delimiter >> endSecond;

  // 获取当前时间
  auto now = std::chrono::system_clock::now();
  std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
  std::tm* localTime = std::localtime(&currentTime);

  // 将开始时间转换为秒数
  int startTotalSeconds = startHour * 3600 + startMinute * 60 + startSecond;

  // 将结束时间转换为秒数
  int endTotalSeconds = endHour * 3600 + endMinute * 60 + endSecond;

  // 获取当前时间的秒数
  int currentTotalSeconds =
      localTime->tm_hour * 3600 + localTime->tm_min * 60 + localTime->tm_sec;

  // 检查当前时间是否在时间段内
  if (currentTotalSeconds >= startTotalSeconds &&
      currentTotalSeconds <= endTotalSeconds) {
    // 在时间段内
    return true;
  } else {
    // 不在时间段内
    return false;
  }
}
}  // namespace app
}  // namespace airos
