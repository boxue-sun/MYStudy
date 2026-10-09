/*********************************************************************************
 * @file		cooperation_intersection_cross.h
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

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "app/framework/interface/app_base.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/modules/v2x_scenario/common_module/data_model/position_model.h"
#include "app/modules/v2x_scenario/common_module/remote_vehicles/remote_vehicles.h"

namespace airos {
namespace app {

enum CICReqProcStatus {
  CICReqProcStatus_unknown = 0,
  CICReqProcStatus_request = 1,                       // 请求
  CICReqProcStatus_reject = 2,                        // 拒绝
  CICReqProcStatus_accept = 3,                        // 同意
  CICReqProcStatus_cooperation = 5,                   // 需要协作
  CICReqProcStatus_guiding_intersection_center = 6,   // 引导到路口中心点
  CICReqProcStatus_guiding_through_intersection = 7,  // 引导通过路口
  CICReqProcStatus_arrived_intersection_center = 8,   // 已经到达路口中心点
  CICReqProcStatus_cooperation_at_intersection_center = 9, // 在路口中心点等待协作
  CICReqProcStatus_execute = 9,    // 执行阶段
  CICReqProcStatus_cancel = 10,    // 取消
  CICReqProcStatus_complete = 11,  // 完成
  CICReqProcStatus_end = 12        // 结束
};

enum CooperationVehReqStatus {
  CooperationVehReqStatus_unknow = 0,
  CooperationVehReqStatus_no_req = 1,   // 未请求
  CooperationVehReqStatus_waiting = 2,  // 等待回复
  CooperationVehReqStatus_accept = 3,   // 协作车同意
  CooperationVehReqStatus_reject = 4    // 协作车拒绝
};

struct CICReqLane {
  v2xpb::asn::MapNode::ID node_id;
  v2xpb::asn::MapNode::ID upstream_node_id; 
  int lane_id;  // 车道ID
};

struct CICReqLink {
  v2xpb::asn::MapNode::ID node_id;
  v2xpb::asn::MapNode::ID upstream_node_id;
};

struct CooperationVehStatus {
  bool is_v2x_vehicle;              // 是否是网联车
  CooperationVehReqStatus req_status;  // 协作车状态
  double req_time;                     // 请求时间
};

struct CICReqProcData {
  std::string id;             // 申请车辆的ID
  int32_t req_id;             // reqID
  double update_time;         // 更新时间
  PositionModel current_pos;  // 申请车辆的位置
  CICReqProcStatus status;    // 请求当前的状态
  std::string reject_reason;  // 拒绝原因

  // 车辆申请时所在车道
  CICReqLane host_req_lane;    // 主车申请时, 所在车道
  CICReqLane target_req_lane;  // 主车申请时，左转的目标车道
  CICReqLink ref_link;         // 左转受影响的对向车道

  std::unordered_map<std::string, CooperationVehStatus> cooperation_veh_map;  // 协作车辆集合

  bool is_stop_at_cross_centor;  // 是否在路口中心，等待继续引导
  // TODO
};

class CooperationIntersectionCross {
 public:
  CooperationIntersectionCross()
      : remote_vehicles_(RemoteVehicles::GetInstance()){};

  virtual ~CooperationIntersectionCross(){};

  bool Init(const airos::app::ApplicationCallBack& send_cb,
            const std::string& app_conf_path);
  /**
   * @brief 处理收到的V2X消息
   * @param frame
   * @return
   */
  bool ProcV2xMsgRecv(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

 private:
  /**
   * @brief 处理vir协作式变道请求
   * @param frame
   */
  void ProcVirReq(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

  /**
   * @brief 周期处理请求信息
   */
  void CicReqProcMaintenance();

  /**
   * @brief 处理请求状态阶段的数据
   * @param cic_req_proc_data 
   */
  void ProcCICStatusRequest(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 处理拒绝的流程(主车不符合引导条件)
   * @param cic_req_proc_data 
   */
  void ProcCICStatusReject(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 处理同意的的流程(主车符合引导条件)
   * @param cic_req_proc_data 
   */
  void ProcCICStatusAccept(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 协作流程(有网连车，进入协作流程)
   * @param cic_req_proc_data 
   */
  void ProcCICReqStatusCooperation(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 引导路口中心点等待区域
   * @param cic_req_proc_data 
   */
  void ProcGuidingIntersectionCenter(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 引导通过路口
   * @param cic_req_proc_data 
   */
  void ProcGuidingThroughIntersection(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 到达路口中心点，等待区域
   * @param cic_req_proc_data 
   */
  void ProcCICReqProcStatusArrivedIntersectionCenter(CICReqProcData &cic_req_proc_data);
 
  /**
   * @brief 在路口中心点，协作流程
   * @param cic_req_proc_data 
   */
  void ProcCICReqProcStatusCooperationAtIntersectionCenter(
      CICReqProcData &cic_req_proc_data);

  /**
   * @brief 取消当前引导
   * @param cic_req_proc_data 
   */
  void ProcCICReqProcStatuscancel(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 完成引导
   * @param cic_req_proc_data 
   */
  void ProcCICReqProcStatusComplete(CICReqProcData &cic_req_proc_data);

  /**
   * @brief 引导 
   * @param cic_req_proc_data
   * @param guidance_point_list 引导的坐标
   */
  void ProcGuiding(
      CICReqProcData& cic_req_proc_data,
      const std::vector<v2xpb::asn::Position>& guidance_point_list);

  /**
   * @brief 发送rsc消息
   * @param vehicle_coordination
  */
  void TransmitRSC(std::vector<v2xpb::asn::VehicleCoordination> &vehicle_coordination_list);

  /**
   * @brief 获取时间戳
   * @return
   */
  int64_t get_mill_second_minute();

  /**
   * @brief 获取车道上离停止线最近的车辆ID
   * @param node_id
   * @param upstream_node_id
   * @param lane_id
   * @param std::string 车辆的ID, " "表示不存在
   */
  std::string GetNearestToNodeVehicleInLane(
      const v2xpb::asn::MapNode::ID& node_id,
      const v2xpb::asn::MapNode::ID& upstream_node_id, 
      const int& lane_id);

 private:
  std::string mec_id_;
  airos::app::ApplicationCallBack sender_;
  std::unordered_map<int, CICReqProcData> cic_req_proc_data_map_;
  mutable std::mutex cic_req_proc_data_map_mutex_;
  std::shared_ptr<std::thread> cic_req_proc_maintenance_{nullptr};
  RemoteVehicles& remote_vehicles_;
  int8_t rsc_count_;

  // 路口中心点的坐标，用于rsc的参考坐标 
  double cross_lat_ = 0.0;
  double cross_lon_ = 0.0;
  PositionModel cross_centor_;
};

}  // namespace app
}  // namespace airos
