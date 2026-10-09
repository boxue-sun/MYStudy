/*********************************************************************************
 * @file		ParkingLotGuidance.h
 * @brief		ParkingLotGuidance belongs to ncs_4layer
 * @details		ParkingLotGuidance belongs to ncs_4layer
 * @author		Changxuhui
 * @date		2022/1/24 
 * @copyright	Copyright (c) 2021 Gohigh V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/1/24   ChangXuhui       1.0       ————          Create this file   
 * 													   
 * @endverbatim
 ********************************************************************************/

#ifndef PARKING_LOT_GUIDANCE_H_
#define PARKING_LOT_GUIDANCE_H_

#include <mutex>
#include <thread>
#include "Dijkstra.h"
#include "PAMMapProcess.h"
#include "PAMMatcher.h"
#include "PAMModule.h"
#include "PAMParkingNavigate.h"
#include "app/framework/interface/app_base.h"
#include "base/common/network/event_loop.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
namespace airos {
namespace app {
using namespace std;
class ParkingLotGuidance 
{
public:
    ParkingLotGuidance();
    virtual ~ParkingLotGuidance();

	bool Init(const airos::app::ApplicationCallBack& send_cb, const std::string& conf_path);
    void ProcV2xMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);
    void ProcV2xUsecaseMsg(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame);
private:
    void InitEventLoop();
    void pamParkingMapTransform();                   // pam地图处理转换，生成需要的数据
    void pamVirRequest(const std::shared_ptr<const v2xpb::asn::MessageFrame> nmfp); // 处理车辆的VIR请求
    void pamBsmMessage(const std::shared_ptr<const v2xpb::asn::MessageFrame> nmfp); // 处理车辆的Bsm消息
    void processPtcs(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame); //判断感知车辆占用车位情况
    void pamRequestProcess();                        // 处理请求，规划路径
    void pamTransmit();                              // pam Transmit

private:
    std::vector<std::shared_ptr<v2xpb::asn::MessageFrame>> pamParkingMapList_; // 存放map的list
    pammodule::PamNodes pamNodes_;                    // 存放每个节点node数据
    pammodule::PamNodesIdSeq pamNodesIdSeq_;          // 节点ID与顺序对应关系的map
    pammodule::PamNodesMovents pamNodesMovents_;      // 每个节点ID通向的节点ID list
    MGraph_t matrix_;                                 // 节点的邻接矩阵
    std::vector<long> parkingEntrance_;               // 停车场出口
    std::vector<long> parkingExit_;                   // 停车场入口

    std::unique_ptr<PAMMatcher> PAMMatcher_;                 //  PAMMatcher_ 模块用于匹配在地图上的位置
    std::unique_ptr<PAMParkingNavigate> PAMParkingNavigate_; // PAMParkingNavigate_ 模块输出目标车位，及路径

    std::unordered_map<string, pammodule::ParkingVirRequest_t> parkingVirRequestList_; // 车辆ID和对应的引导信息
    std::unordered_map<long, pammodule::PamParkingSlotPtr> slotsMap_;                  // <车位ID,存放车位的指针>便于同步修改车位的状态

    airos::app::ApplicationCallBack send_;
    std::vector<std::string> PAMParkingMapFiles_;
    double PAMTrasmitPeriod_ = 1.0;
    double GuidancePeriod_ = 1.0;
    bool RealTimeGuidance_ = false;
    std::shared_ptr<std::thread> pam_process_thread_{nullptr};
    std::shared_ptr<std::thread> pam_transmit_thread_{nullptr};
    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::unique_ptr<std::thread>    m_Task;

};

} } 

#endif  // PARKING_LOT_GUIDANCE_H_
