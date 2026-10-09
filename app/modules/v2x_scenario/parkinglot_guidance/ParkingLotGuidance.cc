/*********************************************************************************
 * @file		ParkingLotGuidance.cpp
 * @brief		ParkingLotGuidance belongs to ncs_4layer
 * @details		ParkingLotGuidance belongs to ncs_4layer
 * @author		Changxuhui
 * @date		2022/1/24
 * @copyright	Copyright (c) 2022 Gohigh V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/1/24   ChangXuhui       1.0       ————             Create this file
 * 													   
 * @endverbatim
 ********************************************************************************/

#include "ParkingLotGuidance.h"
#include "base/common/log.h"
#include "base/common/network/time_stamp.h"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace app {
using namespace v2xpb::asn;
ParkingLotGuidance::ParkingLotGuidance()
{
    // TODO Auto-generated constructor stub
}

ParkingLotGuidance::~ParkingLotGuidance()
{
    // TODO Auto-generated constructor stub
}

bool ParkingLotGuidance::Init(const airos::app::ApplicationCallBack& send_cb, const std::string& conf_path)
{

	APP_LOG_INFO << "parkinglot_guidance_configs init";
	send_ = send_cb;
	std::string parkinglot_guidance_configs = conf_path + "/parkinglot_guidance.yaml";

	YAML::Node root_node = YAML::LoadFile(parkinglot_guidance_configs);
	if (!root_node.IsMap()) {
		APP_LOG_INFO << "parkinglot_guidance_configs Init ConfigManager Failed!";
		return false;
	}    

    PAMTrasmitPeriod_ = root_node["PAMTrasmitPeriod"].as<double>();
    GuidancePeriod_ = root_node["GuidancePeriod"].as<double>();
    RealTimeGuidance_ = root_node["RealTimeGuidance"].as<bool>();
    if(root_node["PAMParkingMapFiles"])
    {
    	const YAML::Node& list = root_node["PAMParkingMapFiles"];
        for(const auto& tempfile : list)
        {
            PAMParkingMapFiles_.push_back(conf_path + "/" + tempfile.as<std::string>());
        }
    }
    else
    {
        APP_LOG_ERROR << "PAMParkingMapFiles not found";
        return false;
    }

    m_Task.reset(new std::thread([&](){ InitEventLoop(); }));

    // 地图加载到内存
    PAMMapProcess::readPAMParkingMap(PAMParkingMapFiles_, pamParkingMapList_);
    // 地图处理转换
    pamParkingMapTransform();

    // 初始化PAMMatcher_和PAMParkingNavigate_
    PAMMatcher_.reset(new PAMMatcher(pamNodes_, pamNodesIdSeq_));
    PAMParkingNavigate_.reset(new PAMParkingNavigate(pamNodes_,
                                                     pamNodesIdSeq_,
                                                     pamNodesMovents_,
                                                     matrix_,
                                                     parkingEntrance_,
                                                     parkingExit_));

    m_Eventloop->addTimer(std::bind(&ParkingLotGuidance::pamRequestProcess, this), GuidancePeriod_, true);

    m_Eventloop->addTimer(std::bind(&ParkingLotGuidance::pamTransmit, this), PAMTrasmitPeriod_, true);
    // pam_process_thread_ = std::make_shared<std::thread>(
    //     std::bind(&ParkingLotGuidance::pamRequestProcess, this));

    // pam_transmit_thread_ = std::make_shared<std::thread>(
    //     std::bind(&ParkingLotGuidance::pamTransmit, this));   

#if 0 // 定位匹配测试
    PositionModelExt refPos;
    refPos.setLatitude(32.0410983);
    refPos.setLongitude(112.1268034);
    refPos.setHeading(180);

    printf("[ParkingLotGuidance]: start matching ...\n");
    pammodule::MatchLocation_t currentPosition;
    
    // 如果匹配不成功，未进入停车场，默认从node开始引导，默认从入口开始
    if (!PAMMatcher_->PAMMatching(refPos, currentPosition))
    {
        currentPosition.arrive = false;
        currentPosition.nodeId = parkingEntrance_[0];
        currentPosition.upNodeId = -1;

    }
    printf("[ParkingLotGuidance]: matching successful, current Pos: nodeID: %ld  upNodeID: %ld arriver %d\n",
           currentPosition.nodeId,
           currentPosition.upNodeId,
           currentPosition.arrive);

    pammodule::NavigateResult_t NavigateResult;
    int slotid = 0;
    if(PAMParkingNavigate_->enterNavigate(refPos, currentPosition, slotid, NavigateResult))
    {
        printf("[ParkingLotGuidance]: already find Nearest slot: %ld, dist: %f\n",
               NavigateResult.aimParkingSlot->slotID,
               NavigateResult.dist);
        printf("[ParkingLotGuidance]: The path: CurrentPos");
        for (auto tempPath : NavigateResult.path)
        {
            printf("-->%lu",tempPath);
        }
        printf("\n");
    }
#endif
// 离场测试
#if 0
    pammodule::ExitNavigateResult_t ExitNavigateResult;
    PAMParkingNavigate_->exitNavigate(106, ExitNavigateResult);
#endif

// 测试消息发送
#if 0
    // 入场
    pammodule::ParkingVirRequest_t tempParkingVirRequest;
    tempParkingVirRequest.bsmPostionFlag = true;
    tempParkingVirRequest.postion.setLatitude(32.0410983);
    tempParkingVirRequest.postion.setLongitude(112.1268034);
    tempParkingVirRequest.parkingReuest.ParkingRequest_enter = true;
    tempParkingVirRequest.reqID = 10;
    tempParkingVirRequest.path.clear();
    parkingVirRequestList_.insert({"京A3252", tempParkingVirRequest});

    // 离场
    pammodule::ParkingVirRequest_t tempParkingVirRequestExit;
    tempParkingVirRequestExit.bsmPostionFlag = false;
    tempParkingVirRequestExit.expectedParkingSlotID = 11337;
    tempParkingVirRequestExit.parkingReuest.ParkingRequest_enter = false;
    tempParkingVirRequestExit.parkingReuest.ParkingRequest_exit = true;
    tempParkingVirRequestExit.reqID = 12;
    tempParkingVirRequestExit.path.clear();
    parkingVirRequestList_.insert({"京C3255", tempParkingVirRequestExit});

#endif 
    return true;
}

void ParkingLotGuidance::InitEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    m_Eventloop->loop();
}

// 地图处理转换
void ParkingLotGuidance::pamParkingMapTransform()
{
    if(pamParkingMapList_.empty())
    {
        APP_LOG_ERROR << "pamParkingMapList_ is empty";
        printf("[ParkingLotGuidance]: pamParkingMapList_ is empty\n");
        return;
    }

    printf("[ParkingLotGuidance]: Start processing map data\n");

    // 对地图进行处理，处理成需要的数据存储pamNodes_ 、 pamNodesIdSeq_、 matrix_、 parkingEntrance_、 parkingExit_、 slotsMap_
    PAMMapProcess::transformToPamNode(pamParkingMapList_, pamNodes_, parkingEntrance_, parkingExit_, slotsMap_);
    PAMMapProcess::transformToPamNodeIdSeq(pamNodes_, pamNodesIdSeq_);
    PAMMapProcess::transformToAdjacencyMatrix(pamNodes_, pamNodesIdSeq_, matrix_);
    PAMMapProcess::transformToPamNodeMovents(pamNodes_, pamNodesMovents_);


#if 1   // 打印出每个节点通向的节点列表
    printf("[PMAParkingMap] The pamNodes Movent to list: \n");
    for (auto iter : pamNodesMovents_)
    {
        std::cout << iter.first << ":\t";
        for (auto iterMovent : iter.second)
        {
            std::cout << iterMovent << "\t";
        }
        std::cout << std::endl;
    }
#endif

#if 1   // 求出的打印邻接矩阵数据
    printf("[PMAParkingMap] Adjacency matrix information: \n");
    for(int i = 0; i < matrix_.num_vertexes; ++i)
    {
        for(int j = 0; j < matrix_.num_vertexes; ++j)
        {
            std::cout << "\t" << matrix_.arc[i][j];
        }
        std::cout <<"\n";
    }
    std::cout << std::endl;
#endif

#if 1  // 打印出入口
    printf("[PMAParkingMap] Parking enter node ID: \n");
    for (auto id : parkingEntrance_)
    {
        std::cout << id << "\t";
    }
    std::cout << std::endl;

    printf("[PMAParkingMap] Parking exit node ID: \n");
    for (auto id : parkingExit_)
    {
        std::cout << id << "\t";
    }
    std::cout << std::endl;
#endif
}

void ParkingLotGuidance::ProcV2xMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{
    m_Eventloop->runInLoop([this, frame]()
    {
        if(frame && frame->has_bsmframe())
        {
            pamBsmMessage(frame);            
        }

        if(frame && frame->has_extframe() && frame->extframe().messagevalue().typresent() == 8)
        {
            pamVirRequest(frame);
        }
    });
}

void ParkingLotGuidance::ProcV2xUsecaseMsg(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame)
{
    if(frame->perception_obstacle().empty())
    {
        return;
    }

    m_Eventloop->runInLoop([this, frame]()
    {
        processPtcs(frame); 
    });

}


void ParkingLotGuidance::processPtcs(const std::shared_ptr<const airos::usecase::EventOutputResult>& frame)
{
    uint32_t count = frame->perception_obstacle().size();
    for (uint32_t i = 0; i < count; ++i) 
    {
        auto obstacle = frame->perception_obstacle()[i];
        if(obstacle.type() != airos::perception::PerceptionObstacle::VEHICLE || !obstacle.has_plate_num())
        {
            continue;
        }

        //速度大于0.1km/h，若已占有某车位，则将该车位状态改为可用
        if(obstacle.has_ptc_speed() && obstacle.ptc_speed() > 0.1)
        {
            for(auto& temp : slotsMap_)
            {
                if(temp.second->platenum == obstacle.plate_num())
                {
                    temp.second->status = pammodule::SlotStatus_available;
                    temp.second->platenum = "";
                    APP_LOG_INFO << "slot become available " << temp.second->slotID << ", vechicle leave: " << obstacle.plate_num();
                }
            }
            continue;
        }

        for(auto nmfp : pamParkingMapList_)
        {
            if(!nmfp || !nmfp->has_extframe() || nmfp->extframe().messagevalue().typresent() != 3)
            {
                continue;
            }

            for (int i = 0; i < nmfp->extframe().messagevalue().pamdata().pamnodes_size(); ++i)
            {
                auto nmfpPamNode = nmfp->extframe().messagevalue().pamdata().pamnodes(i);
                for (int j = 0; j < nmfpPamNode.indrives_size(); ++j)
                {
                    auto nmfpInDrive = nmfpPamNode.indrives(j);
                    for (int n = 0; n < nmfpInDrive.parkingslots_size(); ++n)
                    {
                        auto nmfpParkingSlot = nmfpInDrive.parkingslots(n);
                        if (!nmfpParkingSlot.has_position() || !nmfpParkingSlot.position().topright().has_llh() || !nmfpParkingSlot.position().bottomleft().has_llh()) 
                        {
                            continue;
                        }
                        double minlon = std::min(nmfpParkingSlot.position().topright().llh().longitude(), nmfpParkingSlot.position().bottomleft().llh().longitude());
                        double maxlon = std::max(nmfpParkingSlot.position().topright().llh().longitude(), nmfpParkingSlot.position().bottomleft().llh().longitude());
                        double minlat = std::min(nmfpParkingSlot.position().topright().llh().latitude(), nmfpParkingSlot.position().bottomleft().llh().latitude());
                        double maxlat = std::max(nmfpParkingSlot.position().topright().llh().latitude(), nmfpParkingSlot.position().bottomleft().llh().latitude());
                        //在车位内
                        if(minlon < obstacle.position_gcs().lon() && obstacle.position_gcs().lon() < maxlon
                            && minlat < obstacle.position_gcs().lat() && obstacle.position_gcs().lat() < maxlat)
                        {
                            auto iter = slotsMap_.find(nmfpParkingSlot.slotid());
                            if(iter != slotsMap_.cend())
                            {
                                iter->second->status = pammodule::SlotStatus_occupied;
                                iter->second->platenum = obstacle.plate_num();
                                APP_LOG_INFO << "slot occupied " << nmfpParkingSlot.slotid() << " by vechicle: " << obstacle.plate_num();
                            }
                        }

                    }
                }
            }
        }
    }
}

void ParkingLotGuidance::pamVirRequest(const std::shared_ptr<const v2xpb::asn::MessageFrame> nmfp)
{

    auto tempVir = &nmfp->extframe().messagevalue().vehintentionandrequest();

    // 遍历VIR中的事件请求, 看是否又符合的事件请求
    for (int i = 0; i < tempVir->intandreq().reqs_size(); ++i)
    {
        auto tempReq = tempVir->intandreq().reqs(i);
        //  如果是parking申请的信息
        if (tempReq.has_info() && tempReq.info().has_parking())
        {
            // 获取申请车辆的ID
            string vechicleID = tempVir->id();
            auto iter = parkingVirRequestList_.find(vechicleID);
            
            bool has_enter = false;
            bool has_exit = false;
            for(int tempparktype = 0; tempparktype < tempReq.info().parking().req_size(); ++tempparktype)
            {
                if(tempReq.info().parking().req(tempparktype) == 0)
                {
                    has_enter = true;
                }
                else if(tempReq.info().parking().req(tempparktype) == 1)
                {
                    has_exit = true;
                }
                else
                {

                }
            }
            // 如果车辆不是首次申请
            if (iter != parkingVirRequestList_.end())
            {
                // 申请车辆的reqID与之前相同(相同的req申请内容相同),不更新申请的map直接返回
                printf("iter->second.reqID: %ld,tempReq.reqID() : %ld \n", iter->second.reqID, tempReq.reqid());
                if (iter->second.reqID == tempReq.reqid())
                {
                    return;
                }


                // 申请内容相同，直接返回。 标准中相同事件所发出的reqID相同,但是我们的obu发出VIR reqId是每条消息都在递增。因此加此判断
                if ((iter->second.parkingReuest.ParkingRequest_enter == has_exit) &&
                    iter->second.parkingReuest.ParkingRequest_exit == has_exit)
                {
                    return;
                }

                // 申请的内容与之前不相同, 在map中删除
                parkingVirRequestList_.erase(iter);
            }

            pammodule::ParkingVirRequest_t tempParkingVirRequest;
            tempParkingVirRequest.reqID = tempReq.reqid();
            tempParkingVirRequest.parkingReuest.ParkingRequest_enter = has_enter;
            tempParkingVirRequest.parkingReuest.ParkingRequest_exit = has_exit;
            
            // 设置默认值
            tempParkingVirRequest.expectedParkingSlotID = -1;   
            tempParkingVirRequest.path.clear();             // 清空路径规划
            tempParkingVirRequest.recvTime = 0;
            tempParkingVirRequest.bsmPostionFlag = false;   // 没有通过bsm获取当前位置

            // 离场请求必须包当前车位ID
            if (tempParkingVirRequest.parkingReuest.ParkingRequest_exit)
            {
                if (tempReq.info().parking().has_expectedparkingslotid())
                {
                    tempParkingVirRequest.expectedParkingSlotID = tempReq.info().parking().expectedparkingslotid();
                }
                else
                {
                    printf("[ParkingLotGuidance][error]: The vir req of ParkingRequest_exit not include expectedParkingSlotID\n");
                    APP_LOG_ERROR << "The vir req of ParkingRequest_exit not include expectedParkingSlotID";
                    return;
                }
            }

            string reqType = tempParkingVirRequest.parkingReuest.ParkingRequest_enter == 1 ?  "Enter" : "Exit";
            printf("[ParkingLotGuidance]: add req of ParkingRequest in Reqlist, vechicleID: %s , reqType: %s \n",
                    vechicleID.c_str(), reqType.c_str());
            APP_LOG_INFO << "add req of ParkingRequest in Reqlist, vechicleID: " << vechicleID << ", reqType: " << reqType;
            parkingVirRequestList_.insert({vechicleID, tempParkingVirRequest});
            break;
        }
    }
    
}

void ParkingLotGuidance::pamBsmMessage(const std::shared_ptr<const v2xpb::asn::MessageFrame> nmfp)
{
    auto& tempBsm = nmfp->bsmframe();
    string vechicleID = tempBsm.id();
    
    auto iter = parkingVirRequestList_.find(vechicleID); 
    // 如果收到的bsm,没有请求过parking。直接忽略
    if(iter == parkingVirRequestList_.end())
    {
        return;
    }

    printf("[ParkingLotGuidance]: update postion from bsm, vechicleID: %s\n", vechicleID.c_str());
    APP_LOG_INFO << "update postion from bsm, vechicleID: " << vechicleID;

    // 更新车位请求，车辆的位置
    iter->second.postion.setLatitude(tempBsm.pos().llh().latitude());
    iter->second.postion.setLongitude(tempBsm.pos().llh().longitude());
    iter->second.postion.setHeading(tempBsm.heading());
    iter->second.bsmPostionFlag = true;
    iter->second.recvTime = afl::util::TimeStamp::now(true).microSeconds();
    return;
}

void ParkingLotGuidance::pamRequestProcess()
{

    for (auto &parkingVirRequest : parkingVirRequestList_)
    {
        // 需要实时引导 或者 首次引导。   进行需求处理
        if (RealTimeGuidance_ || parkingVirRequest.second.path.empty())
        {
            // 保存上一次的规划结果
            long oldAimSlotId = -1;
            if (parkingVirRequest.second.path.empty() && parkingVirRequest.second.aimParkingSlot != nullptr)
            {
                oldAimSlotId = parkingVirRequest.second.aimParkingSlot->slotID;
            }

            // 进场请求处理
            if (parkingVirRequest.second.parkingReuest.ParkingRequest_enter)
            {
                // 未收到该车的bsm更新位置信息，不进行处理，直接返回
                if (!parkingVirRequest.second.bsmPostionFlag)
                {
                    continue;
                }

                pammodule::MatchLocation_t currentPosition;
                // 如果匹配不成功，未进入停车场，默认从入口开始引导（默认只有一个入口）
                if (!PAMMatcher_->PAMMatching(parkingVirRequest.second.postion, currentPosition))
                {
                    currentPosition.arrive = true;
                    currentPosition.nodeId = parkingEntrance_[0];

                    printf("[ParkingLotGuidance]: vechicleID: %s match error in the map. default use enter position \n", 
                        parkingVirRequest.first.c_str());
                    APP_LOG_INFO << "vechicleID: " << parkingVirRequest.first << " match error in the map. default use enter position";
                }
                else
                {
                    printf("[ParkingLotGuidance]: vechicleID: %s match successful\n", parkingVirRequest.first.c_str());
                    APP_LOG_INFO << "vechicleID: " << parkingVirRequest.first << " match successful";
                }

                printf("[ParkingLotGuidance]:vechicleID: %s matching result, current Pos: nodeID: %ld  upNodeID: %ld arriver %d\n",
                    parkingVirRequest.first.c_str(),
                    currentPosition.nodeId,
                    currentPosition.upNodeId,
                    currentPosition.arrive);
                APP_LOG_INFO << "vechicleID: " << parkingVirRequest.first << " matching result, current Pos: nodeID: " << currentPosition.nodeId << " upNodeID: " << currentPosition.upNodeId << " arriver " << currentPosition.arrive;

                // 寻找最近车位,输出路径
                pammodule::NavigateResult_t NavigateResult;
                if (PAMParkingNavigate_->enterNavigate(parkingVirRequest.second.postion,
                                                    currentPosition,
                                                    oldAimSlotId,
                                                    NavigateResult))
                {
                    printf("[ParkingLotGuidance]: already find vechicleID: %s Nearest slot: %ld, dist: %f\n",
                        parkingVirRequest.first.c_str(),
                        NavigateResult.aimParkingSlot->slotID,
                        NavigateResult.dist);
                    APP_LOG_INFO <<"already find vechicleID: " <<  parkingVirRequest.first << " Nearest slot: " << NavigateResult.aimParkingSlot->slotID << " dist: " << NavigateResult.dist;

                    printf("[ParkingLotGuidance]: The path: CurrentPos");
                    for (auto tempPath : NavigateResult.path)
                    {
                        printf("-->%lu", tempPath);
                    }
                    printf("\n");

                    // 将车位设置成占用状态
                    auto iter = slotsMap_.find(NavigateResult.aimParkingSlot->slotID);
                    if (iter != slotsMap_.cend())
                    {
                        for (auto &nmfpParkingSlot : iter->second->nmfpParkingSlot)
                        {
                            nmfpParkingSlot.set_parkinglock(ParkingLock_locked);
                        }
                        iter->second->parkingLock = pammodule::ParkingLock_locked;
                    }

                    // 如果规划了新的车位, 将原来车位的坐标修改为放弃锁定状态
                    if (NavigateResult.aimParkingSlot->slotID != oldAimSlotId)
                    {
                        auto iter = slotsMap_.find(oldAimSlotId);
                        if (iter != slotsMap_.cend())
                        {
                            for (auto &nmfpParkingSlot : iter->second->nmfpParkingSlot)
                            {
                                nmfpParkingSlot.set_parkinglock(ParkingLock_unlocked);
                            }
                            iter->second->parkingLock = pammodule::ParkingLock_unlocked;
                        }
                    }
                }
                else
                {
                    printf("[ParkingLotGuidance][Fail]: find vechicleID: %s Nearest slot Fail\n",
                        parkingVirRequest.first.c_str());
                    continue;
                }

                parkingVirRequest.second.aimParkingSlot = NavigateResult.aimParkingSlot;
                parkingVirRequest.second.path = NavigateResult.path;
            }
            // 离场请求处理
            else if (parkingVirRequest.second.parkingReuest.ParkingRequest_exit)
            {
                // 没有当前所在的车位ID, 直接返回
                if (parkingVirRequest.second.expectedParkingSlotID < 0)
                {
                    continue;
                }

                pammodule::ExitNavigateResult_t ExitNavigateResult;
                PAMParkingNavigate_->exitNavigate(parkingVirRequest.second.expectedParkingSlotID, ExitNavigateResult);
                parkingVirRequest.second.path = ExitNavigateResult.path;
            }
        }
    }
}

void ParkingLotGuidance::pamTransmit()
{
    // 去除parkingVirRequestList_没有处理好的信息。只发送已经规划好引导路径的车辆的引导信息
    std::unordered_map<string, pammodule::ParkingVirRequest_t> tempParkingVirRequestList;
    tempParkingVirRequestList.clear();
    for (auto iter = parkingVirRequestList_.cbegin(); iter != parkingVirRequestList_.cend(); ++iter)
    {
        if (!iter->second.path.empty())
        {
            tempParkingVirRequestList.insert({iter->first, iter->second});
        }
    }

    // 填充nmfp消息中parkingAreaGuidance信息, 可能有多个nmfp消息，添加相同的parkingAreaGuidance信息
    for(auto& nmfp : pamParkingMapList_)
    {
        // 清空nmfp之前的parkingAreaGuidance
        nmfp->mutable_extframe()->mutable_messagevalue()->mutable_pamdata()->clear_parkingareaguidance();
        // 填充parkingAreaGuidance消息
        for (size_t i = 0; i < tempParkingVirRequestList.size(); ++i)
        {
            auto iter = tempParkingVirRequestList.cbegin();

            auto tempGuidance = nmfp->mutable_extframe()->mutable_messagevalue()->mutable_pamdata()->add_parkingareaguidance();

            // 填充车辆ID
            tempGuidance->set_vehid(iter->first);

            // 填充路径引导信息
            if (!iter->second.path.empty())
            {
                for (int j = 0; j < iter->second.path.size(); ++j)
                {
                    tempGuidance->add_drivepathlist(iter->second.path[j]);
                }
            }

            // 如果是入场请求, 填充目标车位
            if (iter->second.parkingReuest.ParkingRequest_enter)
            {
                tempGuidance->set_targetparkingslot(iter->second.aimParkingSlot->slotID);
            }
            ++iter;
        }

        auto message_pb = std::make_shared<airos::app::ApplicationData>();
        message_pb->mutable_road_side_frame()->operator=(*nmfp);
        APP_LOG_ERROR << message_pb->DebugString();
        send_(message_pb);
    }
}

} }
