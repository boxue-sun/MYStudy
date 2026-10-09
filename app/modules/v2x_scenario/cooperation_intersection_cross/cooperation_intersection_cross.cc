/*********************************************************************************
 * @file		cooperation_intersection_cross.cpp
 * @brief		cooperation_intersection_cross
 * @details		cooperation_intersection_cross
 * @author		ChangXuhui
 * @date		2024/4/19
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/19  ChangXuhui      1.0          ————               二阶段场景开发
 *
 *
 * @endverbatim
 ********************************************************************************/

#include "cooperation_intersection_cross.h"

#include "app/modules/v2x_scenario/common_module/traffic_light_data/traffic_light_data.h"
#include "app/modules/v2x_scenario/common_module/local_maps/local_maps.h"
#include "base/common/log.h"
#include "base/common/time_util.h"

namespace airos {
namespace app {
bool CooperationIntersectionCross::Init(
    const airos::app::ApplicationCallBack& send_cb,
    const std::string& app_conf_path) {
    mec_id_ = "12345678";
    sender_ = send_cb;
    rsc_count_ = 1;
    auto local_maps = LocalMaps::GetInstance().GetLocalMap();

    auto nodes = local_maps.nodes(0);
    if (nodes.has_position() && nodes.position().has_llh()) {
      auto pos = nodes.position().llh();
      PositionLLH pos_llh;
      pos_llh.latitude = pos.latitude();
      pos_llh.longitude = pos.longitude();
      cross_centor_.SetPosition(pos_llh);
    } else {
      APP_LOG_ERROR << "Get ref pos from map error";
    }

    cic_req_proc_maintenance_ = std::make_shared<std::thread>(
        std::bind(&CooperationIntersectionCross::CicReqProcMaintenance, this));
    // TODO
    return true;
}

bool CooperationIntersectionCross::ProcV2xMsgRecv(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  // 处理vir消息
  if (frame->has_extframe() &&
      frame->extframe().messagevalue().has_vehintentionandrequest()) {
    ProcVirReq(frame);
  }
  return true;
}

void CooperationIntersectionCross::ProcVirReq(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  auto& vir = frame->extframe().messagevalue().vehintentionandrequest();
  APP_LOG_INFO << "Start proc vir msg, vir id: " << vir.id();
  cic_req_proc_data_map_mutex_.lock();
  // 先处理resp消息
  for (int i = 0; i < vir.intandreq().resps_size(); ++i) {
    if (!vir.intandreq().resps(i).has_resprsu() ||
        vir.intandreq().resps(i).resprsu() != mec_id_) {
      APP_LOG_INFO << "rsp resp rsu id is not same mec id: " << mec_id_ << ", ignore";
      continue;
    }

    auto resp = vir.intandreq().resps(i);

    auto iter = cic_req_proc_data_map_.find(resp.respid());
    if (iter == cic_req_proc_data_map_.end()) {
      APP_LOG_INFO << "Resp id " << resp.respid() << ", not in cic_req_proc_data_map_, ignore";
      continue;
    }

    auto iter_cooveh =  iter->second.cooperation_veh_map.find(vir.id());
    if(iter_cooveh == iter->second.cooperation_veh_map.end()){
      APP_LOG_INFO << "Not find vehicle: " << vir.id() << "in cooperation_veh_map, ignore";
      continue;
    }

    switch (resp.respstatus())
    {
    case v2xpb::asn::ReqStatus_accept:
      iter_cooveh->second.req_status = CooperationVehReqStatus_accept;
      APP_LOG_INFO << "Vehicle: " << vir.id() << " accept cooperation";
      break;

    case v2xpb::asn::ReqStatus_reject:
      iter_cooveh->second.req_status = CooperationVehReqStatus_reject;
      APP_LOG_INFO << "Vehicle: " << vir.id() << " reject Cooperation";
      break;
    default:
      break;
    }
  }

  for (int i = 0; i < vir.intandreq().reqs_size(); ++i)
  {
    if (!vir.intandreq().reqs(i).has_targetrsu()) {
      APP_LOG_INFO << "Req target rsu id is not exit";
      continue;
    }

    if (vir.intandreq().reqs(i).targetrsu() != mec_id_) {
      APP_LOG_INFO << "Req target rsu id: " << vir.intandreq().reqs(i).targetrsu()
                   << " is not same mec id: " << mec_id_
                   << ", ignore";
      continue;
    }

    auto& current_behavior = vir.intandreq().currentbehavior();
    auto iter = std::find(current_behavior.begin(), current_behavior.end(),
                          v2xpb::asn::DriveBehavior_intersectionTurnLeft);
    if (iter == current_behavior.end()) {
      APP_LOG_INFO << "Current behavior is not intersection rurn left, ignore";
      continue;
    }

    auto req = vir.intandreq().reqs(i);
    // 申请状态是请求
    if (v2xpb::asn::ReqStatus_request == req.status()) {
      // 将申请数据, 放到处理的map中
      CICReqProcData req_proc_data;
      req_proc_data.req_id = req.reqid();
      req_proc_data.id = vir.id();

      if (vir.intandreq().has_currentpos()) {
        auto currentpos = vir.intandreq().currentpos();
        PositionLLH pos;
        pos.altitude = currentpos.pos().llh().elevation();
        pos.longitude = currentpos.pos().llh().longitude();
        pos.latitude = currentpos.pos().llh().latitude();
        req_proc_data.current_pos.SetPosition(pos);
      }

      req_proc_data.status = CICReqProcStatus_request;
      auto iter = cic_req_proc_data_map_.find(req_proc_data.req_id);
      if (iter != cic_req_proc_data_map_.end()) {
        APP_LOG_INFO << "Update req id: " << req_proc_data.req_id
                 << ", in cic_req_proc_data_map_";
        iter->second.current_pos = req_proc_data.current_pos;
      } else {
        APP_LOG_INFO << "Add req id: " << req_proc_data.req_id
                 << ", in cic_req_proc_data_map_";
        cic_req_proc_data_map_.insert({req_proc_data.req_id, req_proc_data});
      }
    } 
    
    // 申请状态为其他
    else {
      auto iter = cic_req_proc_data_map_.find(req.reqid());
      if (iter == cic_req_proc_data_map_.end()) {
        APP_LOG_INFO << "Not find req id: " << req.reqid()
                 << ", in cic_req_proc_data_map_, ignore";
      }
      switch (req.status()) {
        case v2xpb::asn::ReqStatus_cancel:
          iter->second.status = CICReqProcStatus_cancel;
          APP_LOG_INFO << "Req id: " << req.reqid() << "req status is cancel";
          break;
        case v2xpb::asn::ReqStatus_complete:
          iter->second.status = CICReqProcStatus_complete;
          APP_LOG_INFO << "Req id: " << req.reqid() << "req status is complete";
          break;
        default:
          break;
      }
    }
  }
  cic_req_proc_data_map_mutex_.unlock();
}

void CooperationIntersectionCross::CicReqProcMaintenance() {
  while (true) {
    double time_now = airos::base::TimeUtil::GetCurrentTime();
    
    cic_req_proc_data_map_mutex_.lock();
    remote_vehicles_.GetInstance().LockRemoteVehiclesMap();
    for (auto iter = cic_req_proc_data_map_.begin();
         iter != cic_req_proc_data_map_.end();) {
      APP_LOG_INFO << "Proc remote vehicles " << iter->second.id << ", req id " 
        << iter->second.req_id << ", status " << iter->second.status;
      auto& CICReqProcData = iter->second;

      // 如果为结束状态，从申请列表中移除。否则进入处理流程
      if (CICReqProcStatus_end == CICReqProcData.status) {
        APP_LOG_INFO << "Req status is CICReqProcStatus_end, remote it from req map";
        cic_req_proc_data_map_.erase(iter++);

      } else {
        switch (CICReqProcData.status) {
          case CICReqProcStatus_request:
            ProcCICStatusRequest(CICReqProcData);
            break;
          case CICReqProcStatus_reject:
            ProcCICStatusReject(CICReqProcData);
            break;
          case CICReqProcStatus_accept:
            ProcCICStatusAccept(CICReqProcData);
            break;
          case CICReqProcStatus_cooperation:
            ProcCICReqStatusCooperation(CICReqProcData);
            break;
          case CICReqProcStatus_guiding_intersection_center:
            ProcGuidingIntersectionCenter(CICReqProcData);
            break;
          case CICReqProcStatus_guiding_through_intersection:
            ProcGuidingThroughIntersection(CICReqProcData);
            break;
          case CICReqProcStatus_arrived_intersection_center:
            ProcCICReqProcStatusArrivedIntersectionCenter(CICReqProcData);
            break;
          case CICReqProcStatus_cooperation_at_intersection_center:
            ProcCICReqProcStatusCooperationAtIntersectionCenter(CICReqProcData);
            break;
          case CICReqProcStatus_cancel:
            ProcCICReqProcStatuscancel(CICReqProcData);
            break;
          case CICReqProcStatus_complete:
            ProcCICReqProcStatusComplete(CICReqProcData);
            break;
          default:
            break;
        }
        ++iter;
      }
    }
    remote_vehicles_.GetInstance().UnlockRemoteVehiclesMap();
    cic_req_proc_data_map_mutex_.unlock();
    
    // 100ms 检查一次
    std::chrono::microseconds duration(100000);
    std::this_thread::sleep_for(duration);
  }
}

void CooperationIntersectionCross::ProcCICStatusRequest(
    CICReqProcData& cic_req_proc_data) {
    APP_LOG_INFO << "Proc vehicle request";
    auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
    // 位置匹配，暂时使用BSM中的数据
    auto iter = remote_vehicles_map.find(cic_req_proc_data.id);
    if (iter == remote_vehicles_map.end()) {
      APP_LOG_WARN << "Not find vehicle " << cic_req_proc_data.id
               << ", in remote vehicle map, ignore";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason =
          "Current location is not within the guidance area";
      return; 
    }

    // 判断位置是够满足引导要求（距离路口20米内, 左转车道, 停止线之前第一辆车）
    auto vehicle = iter->second;

    auto node_id = vehicle.GetTracker()->GetRefNodeId();
    auto up_stream_node_id = vehicle.GetTracker()->GetRefLinkUpStreamNodeId();
    auto lane_id = vehicle.GetTracker()->GetRefLaneId();
    APP_LOG_INFO << "Node id: " << node_id.region() << ", id: " << node_id.id();
    APP_LOG_INFO << "UpStream node id: " << up_stream_node_id.region()
             << ", id: " << up_stream_node_id.id();
    APP_LOG_INFO << "Lane id: " << lane_id;
     
    // 填充主车申请时, 所在的车道，方便后续计算
    cic_req_proc_data.host_req_lane.lane_id = lane_id;
    cic_req_proc_data.host_req_lane.node_id = node_id;
    cic_req_proc_data.host_req_lane.upstream_node_id = up_stream_node_id;

    // 是否在路口中心区域, 
    if (vehicle.GetTracker()->IsAtAcross()) {
      APP_LOG_WARN << "Vehicle is in across, reject";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason = "Current location is not within the guidance area";
      return;
    }

    // 当前车道是否允许左转
    if (!vehicle.GetTracker()->IsCurrentLaneAllowedLeft()) {
      APP_LOG_WARN << "Current lane is not allowed turn left, reject";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason =
          "Current location is not within the guidance area";
      return;
    }

    double current_dist_to_node = vehicle.GetTracker()->GetDistToNode();
    APP_LOG_INFO << "Current dist to node: " << current_dist_to_node;

    // 判断是否距离路口20m内
    if (current_dist_to_node > 100) {
      APP_LOG_WARN << "Current dist to node is " << current_dist_to_node << ", more than 100m, reject";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason =
          "Current location is not within the guidance area";
      return;
    }

    // 判断是否是左转车道第一辆车
    for (auto veh = remote_vehicles_map.begin();
         veh != remote_vehicles_map.end(); ++veh) {
      
      // 去除本车
      if (iter->first == veh->first) {
        continue;
      }

      // 过滤出同车道的车进行处理
      if ((iter->second.GetTracker()->GetRefNodeId().region() ==
           veh->second.GetTracker()->GetRefNodeId().region()) &&
          (iter->second.GetTracker()->GetRefNodeId().id() ==
           veh->second.GetTracker()->GetRefNodeId().id()) &&
          (iter->second.GetTracker()->GetRefLinkUpStreamNodeId().region() ==
           veh->second.GetTracker()->GetRefLinkUpStreamNodeId().region()) &&
          (iter->second.GetTracker()->GetRefNodeId().id() ==
           veh->second.GetTracker()->GetRefNodeId().id()) &&
          (iter->second.GetTracker()->GetRefLaneId() ==
           veh->second.GetTracker()->GetRefLaneId())) {

        // 判断是否离路口最近
        if (current_dist_to_node > veh->second.GetTracker()->GetDistToNode()) {
          APP_LOG_WARN << "Not the first vehicle in current lane, veh: "
                   << veh->first << "dist to node: "
                   << veh->second.GetTracker()->GetDistToNode() 
                   << ", reject";
          cic_req_proc_data.status = CICReqProcStatus_reject;
          cic_req_proc_data.reject_reason =
              "Not the first vehicle in current lane";
          return;
        }
      }
    }

    // 从地图中过去左转的phaseID
    v2xpb::asn::MapLane lane;
    if (!LocalMaps::GetInstance().GetLane(node_id, 
                                         up_stream_node_id,
                                         lane_id,
                                         lane)) {
      APP_LOG_ERROR << "Get Lane " << lane_id << " error";
      return;
    }

    // 筛选出左转的phaseId
    int ref_phase_id = -1;
    for (int i = 0; i < lane.connections_size(); ++i) {
      for (int j = 0; j < lane.connections(i).maneuvers_size(); ++j) {
        auto maneuver = lane.connections(j).maneuvers(j);
        if (maneuver == v2xpb::asn::MapLane::LEFT) {
          ref_phase_id = lane.connections(i).phase_id();

          // 填充目标的车道, 方便后续使用
          cic_req_proc_data.target_req_lane.lane_id = lane.connections(i).lane_id();
          cic_req_proc_data.target_req_lane.node_id = lane.connections(i).remote_node_id();
          cic_req_proc_data.target_req_lane.upstream_node_id = node_id;
        }
      }
    }

    APP_LOG_INFO << "Get turn left phase id " << ref_phase_id;
    if (ref_phase_id < 0) {
      APP_LOG_ERROR << "Ref phase id is empty";
      return;
    }

# if 0  // 暂时配出红绿灯的影响 
    // 判断左转phaseId对应的灯色是够可通行, 获取红绿灯的数据
    auto& traffic_light_data = TrafficLightData::GetInstance();
    traffic_light_data.LockTrafficLightDataMap();
    auto& traffic_light_map = traffic_light_data.GetTrafficLightDataMap();

    // 判断红绿灯所在的路口与当前的路口是否相同
    std::string id =
        std::to_string(node_id.region()) + "_" + std::to_string(node_id.id());
    auto iter_traffic_light_map = traffic_light_map.find(id);
    if (iter_traffic_light_map == traffic_light_map.end()) {
      APP_LOG_ERROR << "Not find node " << id << " data, in traffic light data";
      
      traffic_light_data.UnlockTrafficLightDataMap();
      return;
    }

    // 查找左转的phaseId
    auto traffic_light_phases = iter_traffic_light_map->second;
    auto iter_phase = traffic_light_phases.find(ref_phase_id);
    if (iter_phase == traffic_light_phases.end()) {
      APP_LOG_ERROR << "Not find phase id " << ref_phase_id;

      traffic_light_data.UnlockTrafficLightDataMap();
      return;
    }

    // 左转不是绿灯，不满足条件
    if (iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_FLASHING_GREEN &&
        iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PERMISSIVE_GREEN &&
        iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PROTECTED_GREEN) {
      APP_LOG_WARN << "The current traffic light color does not meet the requirements for passage, reject";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason = "The current traffic light color does not meet the requirements for passage";
      
      traffic_light_data.UnlockTrafficLightDataMap();
      return;
    }

    // 绿灯还有3s, 不再进行引导
    if (iter_phase->second.timing_end() < 3) {
      APP_LOG_WARN << "The green light for left turn is about to end, reject";
      cic_req_proc_data.status = CICReqProcStatus_reject;
      cic_req_proc_data.reject_reason =
          "The green light for left turn is about to end, unable to pass "
          "through the intersection ahead.";
      
      traffic_light_data.UnlockTrafficLightDataMap();
      return;
    }
    traffic_light_data.UnlockTrafficLightDataMap();
#endif
    // 进入同意状态
    cic_req_proc_data.status = CICReqProcStatus_accept;
    APP_LOG_INFO << "Change status to CICReqProcStatus_accept";
    return;
}

void CooperationIntersectionCross::ProcCICStatusReject(
    CICReqProcData& cic_req_proc_data) {
  // 拒绝流程
  v2xpb::asn::VehicleCoordination vehicle_coordination;
  vehicle_coordination.set_vehid(cic_req_proc_data.id);
  vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
  vehicle_coordination.set_status(v2xpb::asn::ReqStatus::ReqStatus_reject);

  auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();
  driver_suggestion->set_suggestion(
      v2xpb::asn::DriveBehavior::DriveBehavior_intersectionTurnLeft);
  vehicle_coordination.set_rejectreason(v2xpb::asn::Reason::Reason_engaged);
  APP_LOG_INFO << "Send reject to vehicle: " << cic_req_proc_data.id
           << ", req id: " << cic_req_proc_data.req_id;

  std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;
  vehicle_coordination_list.emplace_back(vehicle_coordination);
  TransmitRSC(vehicle_coordination_list);
  // 发送拒绝后, 设置状态为结束
  cic_req_proc_data.status = CICReqProcStatus::CICReqProcStatus_end;
  return;
}

void CooperationIntersectionCross::ProcCICReqProcStatuscancel(
    CICReqProcData& cic_req_proc_data) {
  // 取消当前的流程
  cic_req_proc_data.status = CICReqProcStatus::CICReqProcStatus_complete;
  return;
}

void CooperationIntersectionCross::ProcCICReqProcStatusComplete(
    CICReqProcData& cic_req_proc_data) {
  // 引导完成
  std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;

  v2xpb::asn::VehicleCoordination vehicle_coordination;
  auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();  
  // 给申请车辆发送完成
  vehicle_coordination.set_vehid(cic_req_proc_data.id);
  vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
  vehicle_coordination.set_status(v2xpb::asn::ReqStatus::ReqStatus_complete);

  driver_suggestion->set_suggestion(
      v2xpb::asn::DriveBehavior::DriveBehavior_intersectionTurnLeft);
  APP_LOG_INFO << "Send compete to req vehicle: " << cic_req_proc_data.id
           << ", req id: " << cic_req_proc_data.req_id;
  vehicle_coordination_list.emplace_back(vehicle_coordination);

  // 给协作车辆发送完成消息
  for (auto iter = cic_req_proc_data.cooperation_veh_map.begin();
       iter != cic_req_proc_data.cooperation_veh_map.end(); ++iter) {
    v2xpb::asn::VehicleCoordination vehicle_coordination_coo;
    vehicle_coordination_coo.set_vehid(iter->first);
    vehicle_coordination_coo.set_reqid(cic_req_proc_data.req_id);
    vehicle_coordination_coo.set_status(v2xpb::asn::ReqStatus::ReqStatus_complete);

    APP_LOG_INFO << "Send compete to cooperation vehicle: " << iter->first
             << ", req id: " << cic_req_proc_data.req_id;
    vehicle_coordination_list.emplace_back(vehicle_coordination_coo);
  }

  TransmitRSC(vehicle_coordination_list);
  // 发送引导完成后, 设置状态为结束
  cic_req_proc_data.status = CICReqProcStatus::CICReqProcStatus_end;
  return;
}

void CooperationIntersectionCross::ProcCICStatusAccept(
    CICReqProcData& cic_req_proc_data) {
  // 判断对向车道是否有车, 如果对向车道没有车, 直接引导通过路口。如果对向车道有车，进入协作模式

  // 获取对向的link
  v2xpb::asn::MapLink on_comming_link;
  if (!LocalMaps::GetInstance().GetOnCommingLink(
          cic_req_proc_data.host_req_lane.node_id,
          cic_req_proc_data.host_req_lane.upstream_node_id,
          cic_req_proc_data.host_req_lane.lane_id, on_comming_link)) {
    APP_LOG_ERROR << "Get on comming link error";
    return;
  }

  APP_LOG_INFO << "On comming link upstream region id : "
           << on_comming_link.upstream_node_id().region()
           << "id : " << on_comming_link.upstream_node_id().id();

  // 先判断对向车道是否是红灯，如果是红灯，直接引导通过路口
  // 先筛选出直行的phaseId
  int ref_phase_id = -1;
  for (int k = 0; k < on_comming_link.lanes_size(); ++k) {
    auto lane = on_comming_link.lanes(k);

    for (int i = 0; i < lane.connections_size(); ++i) {
      for (int j = 0; j < lane.connections(i).maneuvers_size(); ++j) {
        auto maneuver = lane.connections(j).maneuvers(j);
        if (maneuver == v2xpb::asn::MapLane::STRAIGHT) {
          ref_phase_id = lane.connections(i).phase_id();
          break;
        }
      }
      if (-1 != ref_phase_id) {
        break;
      }
    }
  }

  APP_LOG_INFO << "Get on comming stright phase id " << ref_phase_id;
  if (ref_phase_id < 0) {
    APP_LOG_ERROR << "Ref phase id is empty";
    return;
  }
#if 0
  // 判断直行phaseId对应的灯色是够可通行, 获取红绿灯的数据
  auto& traffic_light_data = TrafficLightData::GetInstance();
  traffic_light_data.LockTrafficLightDataMap();
  auto& traffic_light_map = traffic_light_data.GetTrafficLightDataMap();

  // 判断红绿灯所在的路口与当前的路口是否相同
  std::string id =
      std::to_string(cic_req_proc_data.host_req_lane.node_id.region()) + "_" +
      std::to_string(cic_req_proc_data.host_req_lane.node_id.id());

  auto iter_traffic_light_map = traffic_light_map.find(id);
  if (iter_traffic_light_map == traffic_light_map.end()) {
    APP_LOG_ERROR << "Not find node " << id << " data, in traffic light data";
    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }

  // 查找直行的phaseId
  auto traffic_light_phases = iter_traffic_light_map->second;
  auto iter_phase = traffic_light_phases.find(ref_phase_id);
  if (iter_phase == traffic_light_phases.end()) {
    APP_LOG_ERROR << "Not find phase id " << ref_phase_id;
  
    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }

  // 对向车道直行不是绿灯，引导直接通过路口
  if (iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_FLASHING_GREEN &&
      iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PERMISSIVE_GREEN &&
      iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PROTECTED_GREEN) {
    APP_LOG_INFO << "The comming current traffic light color does not meet the requirements for passage, guiding through intersection";
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;      

    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }
  traffic_light_data.UnlockTrafficLightDataMap();
# endif
  // 对向是绿灯，获取每条车道的第一辆车，判断是否有碰撞风险
  for (int k = 0; k < on_comming_link.lanes_size(); ++k) {
    auto lane = on_comming_link.lanes(k);
    std::string veh_id = GetNearestToNodeVehicleInLane(
        cic_req_proc_data.host_req_lane.node_id,
        cic_req_proc_data.host_req_lane.upstream_node_id, 
        lane.id());

    auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
    auto iter = remote_vehicles_map.find(veh_id);
    if (iter == remote_vehicles_map.end()) {
      APP_LOG_WARN << "Not find vehicle " << veh_id
               << "in remote vehicle map, ignore";
      continue;
    }

    // 进入路口的TTC超过5s, 视为没有危险的车
    double ttc = iter->second.GetTracker()->GetDistToNode() /
                 iter->second.GetPosition().GetSpeed();
    if (ttc > 5) {
      APP_LOG_INFO << "Veh to node TTC is " << ttc << ", more than 5s ignore";
    }

    CooperationVehStatus veh_status;
    veh_status.is_v2x_vehicle = iter->second.GetIsV2XVehicleFlag();
    veh_status.req_status = CooperationVehReqStatus_no_req;
    cic_req_proc_data.cooperation_veh_map.insert({veh_id, veh_status});
  }

  // 如果不存在协作车辆, 引导车辆通过路口
  if (cic_req_proc_data.cooperation_veh_map.empty()) {
    APP_LOG_INFO << "No cooperation veh, guiding through intersection";
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;
    return;
  }

  // 如果存在非网连车辆, 引导车辆进入路口中心等待区等待
  for (auto iter = cic_req_proc_data.cooperation_veh_map.begin();
       iter != cic_req_proc_data.cooperation_veh_map.end(); iter++) {
    if (false == iter->second.is_v2x_vehicle) {
      APP_LOG_INFO << "Cooperation veh has not v2x vehicle, guiding intersection center";
      cic_req_proc_data.status = CICReqProcStatus_guiding_intersection_center;
      return;
    }
  }

  // 如果全部是网联车，进入协作模式
  APP_LOG_INFO << "Cooperation veh is all v2x vehicle, guiding intersection center";
  cic_req_proc_data.status = CICReqProcStatus_cooperation;
  return;
}

void CooperationIntersectionCross::ProcCICReqStatusCooperation(
    CICReqProcData& cic_req_proc_data) {
  if (cic_req_proc_data.cooperation_veh_map.empty()) {
    APP_LOG_INFO << "Cooperation veh is empty, set req status is "
                "CICReqProcStatus_accept";
    cic_req_proc_data.status = CICReqProcStatus_accept;
  }
  bool all_cooveh_accept = true;
  
  std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;
  for (auto iter = cic_req_proc_data.cooperation_veh_map.begin();
       iter != cic_req_proc_data.cooperation_veh_map.end(); ++iter) {
        switch (iter->second.req_status)
        {
        case CooperationVehReqStatus_reject:
          // 如果是拒绝状态，引导车辆进入路口中心点等待
          cic_req_proc_data.status = CICReqProcStatus_guiding_intersection_center;
          APP_LOG_INFO << "Cooperation veh reject, guiding intersection center,Cooperation  veh id: " 
                   << iter->first;
          return;
        case CooperationVehReqStatus_waiting:
          // 如果等待状态，超过1s钟，引导车辆进入路口中心点等待
          if(airos::base::TimeUtil::GetCurrentTime() - iter->second.req_time > 1.0) {
          cic_req_proc_data.status = CICReqProcStatus_guiding_intersection_center;
          APP_LOG_INFO << "Cooperation veh time out, guiding intersection center,Cooperation  veh id: "
                   << iter->first;
          return;
          }
        case CooperationVehReqStatus_accept:
          continue;
        default:
          break;
        }

        all_cooveh_accept = false;

        // 发送协作请求
        v2xpb::asn::VehicleCoordination vehicle_coordination;
        vehicle_coordination.set_vehid(iter->first);
        vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
        vehicle_coordination.set_status(
            v2xpb::asn::ReqStatus::ReqStatus_request);

        auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();
        driver_suggestion->set_suggestion(
            v2xpb::asn::DriveBehavior::DriveBehavior_stop);

        auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
        auto iter_v = remote_vehicles_map.find(iter->first);
        if (iter_v == remote_vehicles_map.end()) {
          APP_LOG_WARN << "Not find vehicle " << iter->first
                   << "in remote vehicle map, ignore";
          continue;
        }

        auto vehicle = iter_v->second;
        std::vector<v2xpb::asn::Position> guidance_point_list;

        v2xpb::asn::Position pos_start;
        auto pos_start_llh = pos_start.mutable_llh();
        pos_start_llh->set_latitude(vehicle.GetPosition().GetPositionLLH().latitude);
        pos_start_llh->set_longitude(vehicle.GetPosition().GetPositionLLH().longitude);
        guidance_point_list.push_back(pos_start);

        v2xpb::asn::MapLane lane;
        if (LocalMaps::GetInstance().GetLane(
                vehicle.GetTracker()->GetRefNodeId(),
                vehicle.GetTracker()->GetRefLinkUpStreamNodeId(),
                vehicle.GetTracker()->GetRefLaneId(), lane)) {
          guidance_point_list.push_back(lane.positions(lane.positions_size() - 1));
        }

        auto path_guidance = vehicle_coordination.mutable_pathguidance();
        for (const auto& point : guidance_point_list) {
          auto path_planning_point = path_guidance->add_list();
          path_planning_point->mutable_pos()->operator=(point);
        }
        vehicle_coordination_list.emplace_back(vehicle_coordination);

        iter->second.req_time = airos::base::TimeUtil::GetCurrentTime();
        iter->second.req_status = CooperationVehReqStatus::CooperationVehReqStatus_waiting;
  }

  if (all_cooveh_accept) {
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;
    APP_LOG_INFO << "All cooperation veh accept, guiding through cross";
    return;
  }

  TransmitRSC(vehicle_coordination_list);
}

void CooperationIntersectionCross::ProcCICReqProcStatusCooperationAtIntersectionCenter(
    CICReqProcData& cic_req_proc_data) {
  if (cic_req_proc_data.cooperation_veh_map.empty()) {
    APP_LOG_INFO << "Cooperation veh is empty, set req status is "
                "CICReqProcStatus_arrived_intersection_center";
    cic_req_proc_data.status = CICReqProcStatus_arrived_intersection_center;
  }
  bool all_cooveh_accept = true;
  
  std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;
  for (auto iter = cic_req_proc_data.cooperation_veh_map.begin();
       iter != cic_req_proc_data.cooperation_veh_map.end(); ++iter) {
        switch (iter->second.req_status)
        {
        case CooperationVehReqStatus_reject:
          // 如果是拒绝状态，引导车辆进入路口中心点等待
          cic_req_proc_data.status = CICReqProcStatus_arrived_intersection_center;
          APP_LOG_INFO << "Cooperation veh reject, id: " << iter->first;
          return;

        case CooperationVehReqStatus_waiting:
          // 如果等待状态，超过1s钟，引导车辆进入路口中心点等待
          if(airos::base::TimeUtil::GetCurrentTime() - iter->second.req_time > 1.0) {
          cic_req_proc_data.status = CICReqProcStatus_arrived_intersection_center;
          APP_LOG_INFO << "Cooperation veh time out, id: "
                   << iter->first;
          return;
          }
        case CooperationVehReqStatus_accept:
          continue;
        default:
          break;
        }

        all_cooveh_accept = false;

        // 发送协作请求
        v2xpb::asn::VehicleCoordination vehicle_coordination;
        vehicle_coordination.set_vehid(iter->first);
        vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
        vehicle_coordination.set_status(
            v2xpb::asn::ReqStatus::ReqStatus_request);

        auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();
        driver_suggestion->set_suggestion(
            v2xpb::asn::DriveBehavior::DriveBehavior_stop);

        auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
        auto iter_v = remote_vehicles_map.find(iter->first);
        if (iter_v == remote_vehicles_map.end()) {
          APP_LOG_WARN << "Not find vehicle " << iter->first
                   << "in remote vehicle map, ignore";
          continue;
        }

        auto vehicle = iter_v->second;
        std::vector<v2xpb::asn::Position> guidance_point_list;

        v2xpb::asn::Position pos_start;
        auto pos_start_llh = pos_start.mutable_llh();
        pos_start_llh->set_latitude(vehicle.GetPosition().GetPositionLLH().latitude);
        pos_start_llh->set_longitude(vehicle.GetPosition().GetPositionLLH().longitude);
        guidance_point_list.push_back(pos_start);

        v2xpb::asn::MapLane lane;
        if (LocalMaps::GetInstance().GetLane(
                vehicle.GetTracker()->GetRefNodeId(),
                vehicle.GetTracker()->GetRefLinkUpStreamNodeId(),
                vehicle.GetTracker()->GetRefLaneId(), lane)) {
          guidance_point_list.push_back(lane.positions(lane.positions_size() - 1));
        }

        auto path_guidance = vehicle_coordination.mutable_pathguidance();
        for (const auto& point : guidance_point_list) {
          auto path_planning_point = path_guidance->add_list();
          path_planning_point->mutable_pos()->operator=(point);
        }
        vehicle_coordination_list.emplace_back(vehicle_coordination);

        iter->second.req_time = airos::base::TimeUtil::GetCurrentTime();
        iter->second.req_status = CooperationVehReqStatus::CooperationVehReqStatus_waiting;
  }

  if (all_cooveh_accept) {
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;
    APP_LOG_INFO << "All cooperation veh accept, guiding through cross";
    return;
  }

  TransmitRSC(vehicle_coordination_list);
}

void CooperationIntersectionCross::ProcCICReqProcStatusArrivedIntersectionCenter(
    CICReqProcData& cic_req_proc_data) {
  // 到达停止线附近，判断是否可以继续通行
  
  cic_req_proc_data.cooperation_veh_map.clear();
  // 获取对向的link
  v2xpb::asn::MapLink on_comming_link;
  if (!LocalMaps::GetInstance().GetOnCommingLink(
          cic_req_proc_data.host_req_lane.node_id,
          cic_req_proc_data.host_req_lane.upstream_node_id,
          cic_req_proc_data.host_req_lane.lane_id, on_comming_link)) {
    APP_LOG_ERROR << "Get on comming link error";
    return;
  }

  APP_LOG_INFO << "On comming link upstream region id : "
           << on_comming_link.upstream_node_id().region()
           << "id : " << on_comming_link.upstream_node_id().id();

  // 先判断对向车道是否是红灯，如果是红灯，直接引导通过路口
  // 先筛选出直行的phaseId
  int ref_phase_id = -1;
  for (int k = 0; k < on_comming_link.lanes_size(); ++k) {
    auto lane = on_comming_link.lanes(k);

    for (int i = 0; i < lane.connections_size(); ++i) {
      for (int j = 0; j < lane.connections(i).maneuvers_size(); ++j) {
        auto maneuver = lane.connections(j).maneuvers(j);
        if (maneuver == v2xpb::asn::MapLane::STRAIGHT) {
          ref_phase_id = lane.connections(i).phase_id();
          break;
        }
      }
      if (-1 != ref_phase_id) {
        break;
      }
    }
  }

  APP_LOG_INFO << "Get on comming stright phase id " << ref_phase_id;
  if (ref_phase_id < 0) {
    APP_LOG_ERROR << "Ref phase id is empty";
    return;
  }
#if 0
  // 判断直行phaseId对应的灯色是够可通行, 获取红绿灯的数据
  auto& traffic_light_data = TrafficLightData::GetInstance();
  traffic_light_data.LockTrafficLightDataMap();
  auto& traffic_light_map = traffic_light_data.GetTrafficLightDataMap();

  // 判断红绿灯所在的路口与当前的路口是否相同
  std::string id =
      std::to_string(cic_req_proc_data.host_req_lane.node_id.region()) + "_" +
      std::to_string(cic_req_proc_data.host_req_lane.node_id.id());

  auto iter_traffic_light_map = traffic_light_map.find(id);
  if (iter_traffic_light_map == traffic_light_map.end()) {
    APP_LOG_ERROR << "Not find node " << id << " data, in traffic light data";
    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }

  // 查找直行的phaseId
  auto traffic_light_phases = iter_traffic_light_map->second;
  auto iter_phase = traffic_light_phases.find(ref_phase_id);
  if (iter_phase == traffic_light_phases.end()) {
    APP_LOG_ERROR << "Not find phase id " << ref_phase_id;
  
    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }

  // 对向车道直行不是绿灯，引导直接通过路口
  if (iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_FLASHING_GREEN &&
      iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PERMISSIVE_GREEN &&
      iter_phase->second.color() != v2xpb::asn::SpatPhaseColor::COLOR_PROTECTED_GREEN) {
    APP_LOG_INFO << "The comming current traffic light color does not meet the requirements for passage, guiding through intersection";
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;      

    traffic_light_data.UnlockTrafficLightDataMap();
    return;
  }
  traffic_light_data.UnlockTrafficLightDataMap();
# endif
  // 对向是绿灯，获取每条车道的第一辆车，判断是否有碰撞风险
  for (int k = 0; k < on_comming_link.lanes_size(); ++k) {
    auto lane = on_comming_link.lanes(k);
    std::string veh_id = GetNearestToNodeVehicleInLane(
        cic_req_proc_data.host_req_lane.node_id,
        cic_req_proc_data.host_req_lane.upstream_node_id, 
        lane.id());

    auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
    auto iter = remote_vehicles_map.find(veh_id);
    if (iter == remote_vehicles_map.end()) {
      APP_LOG_WARN << "Not find vehicle " << veh_id
               << "in remote vehicle map, ignore";
      continue;
    }

    // 进入路口的TTC超过5s, 视为没有危险的车
    double ttc = iter->second.GetTracker()->GetDistToNode() /
                 iter->second.GetPosition().GetSpeed();
    if (ttc > 5) {
      APP_LOG_INFO << "Veh to node TTC is " << ttc << ", more than 5s ignore";
    }

    CooperationVehStatus veh_status;
    veh_status.is_v2x_vehicle = iter->second.GetIsV2XVehicleFlag();
    veh_status.req_status = CooperationVehReqStatus_no_req;
    cic_req_proc_data.cooperation_veh_map.insert({veh_id, veh_status});
  }

  // 如果不存在协作车辆, 引导车辆通过路口
  if (cic_req_proc_data.cooperation_veh_map.empty()) {
    APP_LOG_INFO << "No cooperation veh, guiding through intersection";
    cic_req_proc_data.status = CICReqProcStatus_guiding_through_intersection;
    return;
  }

  // 如果存在非网连车辆, 继续在路口中心区域等待
  for (auto iter = cic_req_proc_data.cooperation_veh_map.begin();
       iter != cic_req_proc_data.cooperation_veh_map.end(); iter++) {
    if (false == iter->second.is_v2x_vehicle) {
      APP_LOG_INFO << "Cooperation veh has not v2x vehicle, stop waiting";

      // 停车等待
      v2xpb::asn::VehicleCoordination vehicle_coordination;
      vehicle_coordination.set_vehid(cic_req_proc_data.id);
      vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
      vehicle_coordination.set_status(v2xpb::asn::ReqStatus::ReqStatus_execute);

      auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();
      driver_suggestion->set_suggestion(
          v2xpb::asn::DriveBehavior::DriveBehavior_stop);
      APP_LOG_INFO << "Send stop wait to vehicle: " << cic_req_proc_data.id
               << ", req id: " << cic_req_proc_data.req_id;

      std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;
      vehicle_coordination_list.emplace_back(vehicle_coordination);
      TransmitRSC(vehicle_coordination_list);
      return;
    }
  }

  // 如果全部是网联车，进入协作模式
  APP_LOG_INFO << "Cooperation veh is all v2x vehicle, set status "
              "CICReqProcStatus_cooperation_at_intersection_center";
  cic_req_proc_data.status =
      CICReqProcStatus_cooperation_at_intersection_center;
  return;
}

void CooperationIntersectionCross::ProcGuidingThroughIntersection(
    CICReqProcData& cic_req_proc_data) {
  APP_LOG_INFO << "Guidance veh " << cic_req_proc_data.id << ", through intersection";
  // 引导通过路口

  auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
  // 位置匹配，暂时使用BSM中的数据
  auto iter = remote_vehicles_map.find(cic_req_proc_data.id);
  if (iter == remote_vehicles_map.end()) {
    APP_LOG_WARN << "Not find vehicle " << cic_req_proc_data.id
             << "in remote vehicle map, ignore";
    return;
  }
  auto vehicle = iter->second;

  // 没有匹配在地图上, 认为已经通过了路口, 完成引导
  if (vehicle.GetTracker()->GetRefLaneId() <= 0 &&
      !vehicle.GetTracker()->IsAtAcross()) {
    cic_req_proc_data.status = CICReqProcStatus_complete;
    APP_LOG_INFO << "Veh not match in map, through intersection, set status complete";
    return;
  }

  std::vector<v2xpb::asn::Position> guidance_point_list;

  v2xpb::asn::Position pos_start;
  auto pos_start_llh = pos_start.mutable_llh();
  pos_start_llh->set_latitude(iter->second.GetPosition().GetPositionLLH().latitude);
  pos_start_llh->set_longitude(iter->second.GetPosition().GetPositionLLH().longitude);
  guidance_point_list.push_back(pos_start);

  // 如果没有进入路口，加上停止线上的点
  if (!vehicle.GetTracker()->IsAtAcross()) {
    APP_LOG_INFO << "Vehicle not in across";
    v2xpb::asn::MapLane lane;
    if (LocalMaps::GetInstance().GetLane(
            cic_req_proc_data.host_req_lane.node_id,
            cic_req_proc_data.host_req_lane.upstream_node_id,
            cic_req_proc_data.host_req_lane.lane_id,
            lane)) {
      guidance_point_list.push_back(lane.positions(lane.positions_size() - 1));
    } else {
      APP_LOG_WARN << "Get Lane " << cic_req_proc_data.host_req_lane.lane_id
               << " error";
    }
  }

  // // 添加路口中心
  // v2xpb::asn::Position pos_centor_pos;
  // auto pos_centor_pos_llh = pos_centor_pos.mutable_llh();
  // pos_centor_pos_llh->set_latitude(cross_centor_.GetPositionLLH().latitude);
  // pos_centor_pos_llh->set_longitude(cross_centor_.GetPositionLLH().longitude);
  // guidance_point_list.push_back(pos_centor_pos);

  v2xpb::asn::MapLane lane;
  v2xpb::asn::Position point;

  if (LocalMaps::GetInstance().GetLane(
          cic_req_proc_data.target_req_lane.node_id,
          cic_req_proc_data.target_req_lane.upstream_node_id,
          cic_req_proc_data.target_req_lane.lane_id, lane)) {
    guidance_point_list.push_back(lane.positions(lane.positions_size() - 1));
  } else if (LocalMaps::GetInstance().GetExitLinkStopLinePointByPredict(
                 cic_req_proc_data.target_req_lane.upstream_node_id,
                 cic_req_proc_data.target_req_lane.node_id, point)) {
    guidance_point_list.push_back(point);
  }

  ProcGuiding(cic_req_proc_data, guidance_point_list);
  return;
}

void CooperationIntersectionCross::ProcGuidingIntersectionCenter(
    CICReqProcData& cic_req_proc_data) {
  APP_LOG_INFO << "Guidance veh " << cic_req_proc_data.id << "intersection center";
  // 引导车辆进入路口中心点

  auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();
  // 位置匹配，使用BSM中的数据
  auto iter = remote_vehicles_map.find(cic_req_proc_data.id);
  if (iter == remote_vehicles_map.end()) {
    APP_LOG_WARN << "Not find vehicle " << cic_req_proc_data.id
             << "in remote vehicle map, ignore";
    cic_req_proc_data.status = CICReqProcStatus_end;
    return;
  }
  auto vehicle = iter->second;

  // 没有匹配在地图上
  if (vehicle.GetTracker()->GetRefLaneId() <= 0 ||
      !vehicle.GetTracker()->IsAtAcross()) {
    cic_req_proc_data.status = CICReqProcStatus_end;
    APP_LOG_INFO << "Veh not match in map, set status CICReqProcStatus_end";
  }

  // 判断车辆是否到达路口中心点附近
  airos::perception::usecase::Vec2d s(vehicle.GetPosition().GetPositionXYZ().x,
                                      vehicle.GetPosition().GetPositionXYZ().y);

  airos::perception::usecase::Vec2d e(cross_centor_.GetPositionXYZ().x,
                                      cross_centor_.GetPositionXYZ().y);
  airos::perception::usecase::Segment2d line(s, e);
  if (line.Length() < 2) {
    APP_LOG_INFO << "Vehicle dist to across centor is " << line.Length()
             << ", arrived cross centor";
    cic_req_proc_data.status = CICReqProcStatus_arrived_intersection_center;
    cic_req_proc_data.is_stop_at_cross_centor = true;
    return;
  }

  std::vector<v2xpb::asn::Position> guidance_point_list;

  v2xpb::asn::Position pos_start;
  auto pos_start_llh = pos_start.mutable_llh();
  pos_start_llh->set_latitude(iter->second.GetPosition().GetPositionLLH().latitude);
  pos_start_llh->set_longitude(iter->second.GetPosition().GetPositionLLH().longitude);
  guidance_point_list.push_back(pos_start);

  // 如果没有进入路口，加上停止线上的点
  if (!vehicle.GetTracker()->IsAtAcross()) {
    APP_LOG_INFO << "Vehicle not in across";
    v2xpb::asn::MapLane lane;
    if (LocalMaps::GetInstance().GetLane(
            cic_req_proc_data.host_req_lane.node_id,
            cic_req_proc_data.host_req_lane.upstream_node_id,
            cic_req_proc_data.host_req_lane.lane_id,
            lane)) {
      guidance_point_list.push_back(lane.positions(lane.positions_size() - 1));
    } else {
      APP_LOG_WARN << "Get Lane " << cic_req_proc_data.host_req_lane.lane_id
               << " error";
    }
  }

  v2xpb::asn::Position pos_end;
  auto pos_end_llh = pos_end.mutable_llh();
  pos_end_llh->set_latitude(cross_centor_.GetPositionLLH().latitude);
  pos_end_llh->set_longitude(cross_centor_.GetPositionLLH().longitude);
  guidance_point_list.push_back(pos_end);

  ProcGuiding(cic_req_proc_data, guidance_point_list);
  return;
}


void CooperationIntersectionCross::ProcGuiding(CICReqProcData& cic_req_proc_data,
                 const std::vector<v2xpb::asn::Position>& guidance_point_list) {
  v2xpb::asn::VehicleCoordination vehicle_coordination;
  vehicle_coordination.set_vehid(cic_req_proc_data.id);
  vehicle_coordination.set_reqid(cic_req_proc_data.req_id);
  vehicle_coordination.set_status(v2xpb::asn::ReqStatus::ReqStatus_accept);

  auto driver_suggestion = vehicle_coordination.mutable_drivesuggestion();
  driver_suggestion->set_suggestion(
      v2xpb::asn::DriveBehavior::DriveBehavior_intersectionTurnLeft);

  auto path_guidance = vehicle_coordination.mutable_pathguidance();
  for (const auto& point : guidance_point_list) {
    auto path_planning_point = path_guidance->add_list();
    path_planning_point->mutable_pos()->operator = (point);
  }

  std::vector<v2xpb::asn::VehicleCoordination> vehicle_coordination_list;
  vehicle_coordination_list.emplace_back(vehicle_coordination);
  TransmitRSC(vehicle_coordination_list);
  return;
}

void CooperationIntersectionCross::TransmitRSC(
    std::vector<v2xpb::asn::VehicleCoordination> &vehicle_coordination_list) {
  auto asn_pb = std::make_shared<v2xpb::asn::MessageFrame>();
  
  auto msgext = asn_pb->mutable_extframe();
  msgext->set_messageid(11);
  auto msgvalue = msgext->mutable_messagevalue();

  msgvalue->set_typresent(v2xpb::asn::MessageFrameExt__value::
                              MessageFrameExt__value_PR_RoadsideCoordination);
  auto rsc = msgvalue->mutable_roadsidecoordination();

  rsc_count_ = (rsc_count_ >= 127) ? 1 : (rsc_count_ + 1);
  rsc->set_msgcnt(rsc_count_);

  rsc->set_id(mec_id_);
  rsc->set_secmark(get_mill_second_minute());
  auto ref_llh = rsc->mutable_refpos()->mutable_llh();

  // 设置参考坐标
  ref_llh->set_latitude(cross_centor_.GetPositionLLH().latitude);
  ref_llh->set_longitude(cross_centor_.GetPositionLLH().longitude);

  ref_llh->set_latitude(40.7786272);  // 暂时使用绝对坐标，方便排查问题
  ref_llh->set_longitude(cross_centor_.GetPositionLLH().longitude);

  for (auto vehicle_coordination : vehicle_coordination_list) {
    auto veh_coordination = rsc->add_coordinates();
    veh_coordination->operator=(vehicle_coordination);
  }

  auto message_pb = std::make_shared<airos::app::ApplicationData>();
  message_pb->mutable_road_side_frame()->operator=(*asn_pb);
  sender_(message_pb);
  APP_LOG_INFO << "Send rsc msg";
  // APP_LOG_INFO << message_pb->DebugString();
}

int64_t CooperationIntersectionCross::get_mill_second_minute() {
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

std::string CooperationIntersectionCross::GetNearestToNodeVehicleInLane(
    const v2xpb::asn::MapNode::ID& node_id,
    const v2xpb::asn::MapNode::ID& upstream_node_id, const int& lane_id) {
  
  double minDistance = std::numeric_limits<double>::max();  // 初始化最小距离为最大值
  std::string nearest_veh_id = "";
  auto remote_vehicles_map = remote_vehicles_.GetRemoteVehiclesMap();

  // 判断是否是左转车道第一辆车
  for (auto veh = remote_vehicles_map.begin(); veh != remote_vehicles_map.end();
       ++veh) {
    // 过滤出同车道的车进行处理
    if ((node_id.region() == veh->second.GetTracker()->GetRefNodeId().region()) &&
        (node_id.id() == veh->second.GetTracker()->GetRefNodeId().id()) &&
        (upstream_node_id.region() == veh->second.GetTracker()->GetRefLinkUpStreamNodeId().region()) &&
        (upstream_node_id.id() == veh->second.GetTracker()->GetRefNodeId().id()) &&
        (lane_id == veh->second.GetTracker()->GetRefLaneId())) {
      // 寻找距离路口最近的车
      if (minDistance < veh->second.GetTracker()->GetDistToNode()) {
        minDistance = veh->second.GetTracker()->GetDistToNode();
        nearest_veh_id = veh->first;
      }
    }
  }
  return nearest_veh_id;
}

}  // namespace app
}  // namespace airos
