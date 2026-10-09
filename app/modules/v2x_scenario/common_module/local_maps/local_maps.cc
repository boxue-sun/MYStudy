/*********************************************************************************
 * @file		remote_vehicles.cc
 * @brief		remote_vehicles
 * @details		remote_vehicles
 * @author		ChangXuhui
 * @date		2024/4/7
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/7  ChangXuhui      1.0            ————                 ————
 *
 * @endverbatim
 ********************************************************************************/
#include "local_maps.h"

#include <GeographicLib/UTMUPS.hpp>
#include <fstream>

#include "app/modules/v2x_application/usecase/common/polygon2d.h"
#include "app/modules/v2x_scenario/common/app_flag.h"
#include "base/common/log.h"
#include "base/work_param/configer_om_work_param.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"

namespace airos {
namespace app {

LocalMaps::LocalMaps() {
  message_frame_ = std::make_shared<v2xpb::asn::MessageFrame>();
  // std::string xml_map_file = "app/conf/airos_v2x_scenario/" + std::string(FLAGS_v2x_scenario_rsu_xml_map_file);
  // APP_LOG_INFO << "Map xml path: " << xml_map_file;

  std::string xml_map_file;
    
  // 地图的路径统一从/home/airos/common_config 目录下获取
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();
  if (!omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile.empty()) {
    xml_map_file = "/home/airos/common_config/" +
                    omWorkParamConfiger.mecDeviceWorkParam.xmlMapFile;
  } else {
    APP_LOG_ERROR << "XmlMapFile is empty";
    return;
  }
  APP_LOG_INFO << "Map xml path: " << xml_map_file;
  GetRsuMap(xml_map_file);
}

bool LocalMaps::GetRsuMap(const std::string& rsu_map_path) {
  std::string asn_version(FLAGS_v2x_scenario_asn_message_version);
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

  std::fstream fs(rsu_map_path.data(), std::fstream::in);
  if (!fs.good()) {
    APP_LOG_ERROR << "Xml file open fail:" << rsu_map_path;
    return false;
  }
  
  std::stringstream ss;
  ss << fs.rdbuf();
  fs.close();
  std::string content(ss.str());

  if (content.empty()) {
    APP_LOG_ERROR<< "Xml map data null";
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
    APP_LOG_ERROR << "Asn to pb false";
    return false;
  }
  APP_LOG_INFO << "Asn to pb true";

  if (message_frame_) {
    message_frame_->ParsePartialFromString(str_pb);
  } else {
    APP_LOG_WARN << "Map pb null";
    return false;
  }

  if (message_frame_->payload_case() !=
      v2xpb::asn::MessageFrame::PayloadCase::kMapFrame) {
    APP_LOG_ERROR << "Map pb get failed.";
    return false;
  }
  APP_LOG_INFO << message_frame_->DebugString();
  local_map_ = message_frame_->mapframe();
  if (local_map_.nodes_size() == 0) {
    APP_LOG_WARN << "Map node is empty";
    return false;
  }
  return true;
}

bool LocalMaps::GetLane(const v2xpb::asn::MapNode::ID& node_id,
                        const v2xpb::asn::MapNode::ID& upstream_node_id,
                        int const& lane_id, 
                        v2xpb::asn::MapLane& lane) {
  auto node = local_map_.nodes(0);
  if ((node.id().region() == node_id.region()) &&
      (node.id().id() == node_id.id())) {
    for (int i = 0; i < node.links_size(); ++i) {
      auto& link = node.links(i);
      if ((link.upstream_node_id().region() == upstream_node_id.region()) &&
          (link.upstream_node_id().id() == upstream_node_id.id())) {
        for (int j = 0; j < link.lanes_size(); ++j) {
          if (lane_id == link.lanes(j).id()) {
            lane = link.lanes(j);
            return true;
          }
        }
      }
    }
  }
  APP_LOG_ERROR << "Get lane, node region id: " << node_id.region()
            << " id: " << node_id.id()
            << " upstream node region id: " << upstream_node_id.region()
            << " id: " << upstream_node_id.id()
            << " lane id: " << lane_id
            << ", error!";

  return false;
}

bool LocalMaps::GetOnCommingLink(const v2xpb::asn::MapNode::ID& node_id,
                        const v2xpb::asn::MapNode::ID& upstream_node_id,
                        int const& lane_id, 
                        v2xpb::asn::MapLink& on_comming_link) {

  v2xpb::asn::MapLane current_lane;
  if (!GetLane(node_id, upstream_node_id, lane_id, current_lane)) {
    APP_LOG_ERROR << "Get host lane " << lane_id << " error";
    return false;
  }

  airos::perception::usecase::Vec2d s(
      current_lane.positions(current_lane.positions_size() - 2).xyz().x(),
      current_lane.positions(current_lane.positions_size() - 2).xyz().y());
  airos::perception::usecase::Vec2d e(
      current_lane.positions(current_lane.positions_size() - 1).xyz().x(),
      current_lane.positions(current_lane.positions_size() - 1).xyz().y());
  airos::perception::usecase::Segment2d line1(s, e);
  double current_lane_angle = line1.HeadingNorth();
  APP_LOG_INFO << "Current lane angle: " << current_lane_angle;

  auto node = local_map_.nodes(0);
  if ((node.id().region() == node_id.region()) &&
      (node.id().id() == node_id.id())) {
    for (int i = 0; i < node.links_size(); ++i) {
      auto& link = node.links(i);

      // 排除当前的link
      if ((link.upstream_node_id().region() == upstream_node_id.region()) &&
          (link.upstream_node_id().id() == upstream_node_id.id())) {
        continue;
      }

      // 使用第一条车道计算角度进行判断
      auto lane = link.lanes(0);
      airos::perception::usecase::Vec2d s(
          lane.positions(lane.positions_size() - 2).xyz().x(),
          lane.positions(lane.positions_size() - 2).xyz().y());
      airos::perception::usecase::Vec2d e(
          lane.positions(lane.positions_size() - 1).xyz().x(),
          lane.positions(lane.positions_size() - 1).xyz().y());
      airos::perception::usecase::Segment2d line2(s, e);
      double lane_angle = line2.HeadingNorth();

      double diff_angle = std::fabs(lane_angle - current_lane_angle);
      if (diff_angle > 150.0 && diff_angle < 210) {
        APP_LOG_INFO << "Find on comming link, angle diff: " << diff_angle;
        on_comming_link = link;
        return true;
      }
    }
  }
  return false;
}

bool LocalMaps::GetExitLinkStopLinePointByPredict(
    const v2xpb::asn::MapNode::ID& node_id,
    const v2xpb::asn::MapNode::ID& upstream_node_id,
    v2xpb::asn::Position& point) {

  v2xpb::asn::Position point_A;
  v2xpb::asn::Position point_B;
  v2xpb::asn::Position point_out;
  double angle = 0.0;

  // 计算出入口link上的相关点
  auto node = local_map_.nodes(0);
  if ((node.id().region() == node_id.region()) &&
      (node.id().id() == node_id.id())) {
    point_A = node.position();
    
    for (int i = 0; i < node.links_size(); ++i) {
      auto& link = node.links(i);
      if ((link.upstream_node_id().region() == upstream_node_id.region()) &&
          (link.upstream_node_id().id() == upstream_node_id.id())) {
        if (0 == link.lanes_size()) {
          return false;
        }
        // int j = link.lanes_size() / 2;
        auto lane = link.lanes(0);

        airos::perception::usecase::Vec2d s(
            lane.positions(lane.positions_size() - 2).xyz().x(),
            lane.positions(lane.positions_size() - 2).xyz().y());
        airos::perception::usecase::Vec2d e(
            lane.positions(lane.positions_size() - 1).xyz().x(),
            lane.positions(lane.positions_size() - 1).xyz().y());
        airos::perception::usecase::Segment2d line(s, e);
        angle = line.HeadingNorth();
        point_B = lane.positions(lane.positions_size() - 1);
      }
    }
  }

  // APP_LOG_INFO << std::fixed << "point B: " << point_B.llh().latitude() << " , " << point_B.llh().longitude();
  // APP_LOG_INFO << std::fixed << "point A: " << point_A.llh().latitude() << " , " << point_A.llh().longitude();
  // APP_LOG_INFO << std::fixed << "Angle: " << angle;

  // 将角度转换为与X轴的夹角
  angle = angle > 180 ? angle - 180 : angle;
  angle = angle > 90 ? 180 - angle : 90 - angle;

  // 计算出口link上停止线上的点
  const double PI = 3.14159265358979323846;
  double radians = angle * PI / 180.0;
  double x, y;

  // 特殊情况处理
  if (90 == angle || 270 == angle) {
    // 垂直线，斜率无限大
    x = 2 * point_A.xyz().x() - point_B.xyz().x();
    y = point_B.xyz().y();
  } else if (0 == angle || 180 == angle) {
    // 水平线，斜率为0
    x = point_B.xyz().x();
    y = 2 * point_A.xyz().y() - point_B.xyz().y();
  } else {
    // 一般情况
    // 计算直线的斜率
    double m = std::tan(radians);
    // 直线方程 y = mx + c 经过点A的截距c
    double c = point_A.xyz().y() - m * point_A.xyz().x();
    // 求B点关于直线的对称点
    double d = (point_B.xyz().x() + (point_B.xyz().y() - c) * m) / (1 + m * m);
    x = 2 * d - point_B.xyz().x();
    y = 2 * d * m - point_B.xyz().y() + 2 * c;
  }

  auto point_xyz = point.mutable_xyz();
  point_xyz->set_x(x);
  point_xyz->set_y(y);
  point_xyz->set_z(0);
  point_xyz->set_zone(point_A.xyz().zone());

  // double lat, lon;
  // GeographicLib::UTMUPS::Reverse(point_A.xyz().zone(), true, x, y, lat, lon);
  // APP_LOG_INFO << std::fixed << "lat: " << lat << ", lon: " << lon;  
  return true;
}

}  // namespace app
}  // namespace airos
