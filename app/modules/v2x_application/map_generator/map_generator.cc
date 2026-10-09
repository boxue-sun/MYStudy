/*********************************************************************************
 * @file		map_generator.cpp
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

#include "map_generator.h"

#include <sys/time.h>

#include <fstream>

#include "app/modules/v2x_application/common/app_flag.h"
#include "base/common/log.h"
#include "base/work_param/configer_om_work_param.h"

namespace airos {
namespace app {

bool MapGenerator::Init(const airos::app::ApplicationCallBack& send_cb,
                        const std::string& app_conf_path) {
  send_ = send_cb;

  // 地图的路径统一从/home/airos/common_config 目录下获取
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();
  if (!omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile.empty()) {
    xml_map_file_ = "/home/airos/common_config/" +
                    omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile;
  } else {
    APP_LOG_ERROR << "XmlMapFile is empty";
    return false;
  }

  xml_map_send_rate_ = FLAGS_xml_map_send_rate;
  msg_cnt_ = 0;

  std::string asn_version(FLAGS_asn_message_version);
  if (asn_version.compare("4layer") == 0) {
    asn_type_ = EnAsnType::CASE_53_2020;
  } else if (asn_version.compare("new_4layer") == 0) {
    asn_type_ = EnAsnType::YDT_3709_2020;
  } else if (asn_version.compare("new_4layer_ext") == 0) {
    asn_type_ = EnAsnType::YDT_3709_2020_EXT;
  } else {
    // pass
  }

  return true;
}

void MapGenerator::ThreadMap() {
  APP_LOG_INFO << "xml map:" << xml_map_send_rate_ << " " << xml_map_file_;

  std::fstream fs(xml_map_file_.data(), std::fstream::in);
  if (!fs.good()) {
    APP_LOG_WARN << "xml file open false:" << xml_map_file_;
    return;
  }
  std::stringstream ss;
  ss << fs.rdbuf();
  fs.close();
  std::string content(ss.str());

  if (content.empty()) {
    APP_LOG_WARN << "xml map data null";
    return;
  }

  std::string str_asn("");
  // 先使用智路的地图进行解析，不成功再使用asn xer的地图格式
  if (message_frame_map_xml2uper_adapter(content, &str_asn, asn_type_) > 0 ) {
    APP_LOG_INFO << " map xml to uper: true, use airos xml2uper";
  } else if (message_frame_xer2uper_adapter(content, &str_asn, asn_type_) > 0) {
    APP_LOG_INFO << " map xml to uper: true, use xer2uper";
  } else {
    APP_LOG_WARN << " map xml to uper: false";
    return;
  }

  std::string str_pb("");
  if (0 >= message_frame_uper2pbstr_adapter(str_asn, &str_pb, asn_type_)) {
    APP_LOG_WARN << "asn to pb false";
    return;
  }
  APP_LOG_INFO << "asn to pb true";

  pb_map_ = std::make_shared<v2xpb::asn::MessageFrame>();
  if (pb_map_) {
    pb_map_->ParsePartialFromString(str_pb);
  } else {
    APP_LOG_WARN << "map pb null";
    return;
  }

  sptr_map_.reset(new std::thread([&] {
    while (true) {
      if (pb_map_ && pb_map_->payload_case() ==
                         v2xpb::asn::MessageFrame::PayloadCase::kMapFrame) {
        pb_map_->mutable_mapframe()->set_message_count(msg_cnt_);
        msg_cnt_ = (msg_cnt_ + 1) % 128;
        APP_LOG_INFO << "map msgcnt:" << pb_map_->mapframe().message_count();
      }
      auto message_pb = std::make_shared<airos::app::ApplicationData>();
      message_pb->mutable_road_side_frame()->operator=(*pb_map_);

      // APP_LOG_INFO << message_pb->DebugString();
      send_(message_pb);
      std::this_thread::sleep_for(
          std::chrono::milliseconds(xml_map_send_rate_));
    }
  }));
}
}  // namespace app
}  // namespace airos
