/*********************************************************************************
 * @file		PAMParkingNavigate.h
 * @brief		PAMParkingNavigate belongs to ncs_4layer
 * @details		PAMParkingNavigate belongs to ncs_4layer
 * @author		Changxuhui
 * @date		2022/2/21 
 * @copyright	Copyright (c) 2022 Gohigh V2X Division.
 * @verbatim
 *  
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/2/21   ChangXuhui       1.0       ————            Create this file
 * 													   
 * @endverbatim
 ********************************************************************************/
#include "PAMParkingNavigate.h"

namespace airos {
namespace app {

PAMParkingNavigate::PAMParkingNavigate(pammodule::PamNodes &pamNodes,
                                           pammodule::PamNodesIdSeq &pamNodesIdSeq,
                                           pammodule::PamNodesMovents &pamNodesMovents,
                                           MGraph_t &matrix,
                                           std::vector<long> &parkingEntrance,
                                           std::vector<long> &parkingExit)
        : pamNodes_(pamNodes), pamNodesIdSeq_(pamNodesIdSeq),
          pamNodesMovents_(pamNodesMovents), matrix_(matrix), 
          parkingEntrance_(parkingEntrance), parkingExit_(parkingExit)
{

}

PAMParkingNavigate::~PAMParkingNavigate()
{
 
}

bool PAMParkingNavigate::enterNavigate(const PositionModelExt &refPos,
                                       const pammodule::MatchLocation_t &matchLocation,
                                       const long &oldAimSlotId,
                                       pammodule::NavigateResult_t &NavigateResult)
{
    /**
     * 两种情况: 1. 在link上  2. 在node上
     * 1. 如果在link上, 先匹配当前link上的车位
     * 2. 如果是在node上，则先匹配该node，所连link的车位，取最短距离。如果不合适，寻找下一个距离最近的node
     */

    pammodule::ShortLot_t tempShortLot;
    
    // 如果当前在link上，在当前link上寻找合适的车位
    if (matchLocation.arrive == false)
    {
        printf("[PAMParkingNavigate]: start find slot OnCurrentLink\n");
        // 在当前link上寻找到合适车位(如果车位的坐标不存在,会直接返回false)
        if (shortLotOnCurrentLink(refPos, matchLocation, oldAimSlotId, tempShortLot))
        {
            printf("[PAMParkingNavigate]: success find slot OnCurrentLink\n");
            NavigateResult.aimParkingSlot = tempShortLot.aimParkingSlot;
            NavigateResult.nodeId = tempShortLot.moventId;
            NavigateResult.upNodeId = tempShortLot.nodeId;

            // 计算当前link到目标车位的距离。车位的坐标一定存在
            PositionModelExt slotPositon;
            slotPositon.setLatitude(tempShortLot.aimParkingSlot->position.topLeft.lat);
            slotPositon.setLongitude(tempShortLot.aimParkingSlot->position.topLeft.Long);
            NavigateResult.dist = base::MathUtil::getDistance(slotPositon.getLatitude(), slotPositon.getLongitude(), refPos.getLatitude(), refPos.getLongitude());
            return true;
        }
    }

    printf("[PAMParkingNavigate]: failed find slot OnCurrentLink\n");
    printf("[PAMParkingNavigate]: start find slot on node: %ld  move to link\n", matchLocation.nodeId);

    double distRefPosToNextNode = distCurrentPosToNode(refPos, matchLocation.nodeId); // 当前位置到下一个节点的距离
    if(distRefPosToNextNode < 0)
    {
        printf("[PAMParkingNavigate]: failed get dist from CurrentPos To Node\n");
        return false;
    }
    printf("[PAMParkingNavigate]: dist from CurrentPos To Node is %f\n", distRefPosToNextNode);
    
    double distNodeToSlot = INFINITY;
    // 在当前node上寻找去往的link上的最短车位
    if (shortLotOnNode(matchLocation.nodeId, oldAimSlotId, tempShortLot, distNodeToSlot))
    {
        printf("[PAMParkingNavigate]: successful find slot on node: %ld  move to node: %ld  link\n",
               NavigateResult.nodeId,
               tempShortLot.nodeId);
        NavigateResult.aimParkingSlot = tempShortLot.aimParkingSlot;
        NavigateResult.nodeId = tempShortLot.moventId;
        NavigateResult.upNodeId = tempShortLot.nodeId;
        NavigateResult.dist = distNodeToSlot + distRefPosToNextNode;
        NavigateResult.path.emplace_back(NavigateResult.upNodeId);
        return true;
    }

    printf("[PAMParkingNavigate]: failed find slot and find other node use Dijkstra\n");
    // 如果node直接连接的node没有合适的路径,采用Dijkstra算法，当寻找其他路径上的最短路径
    auto iter = pamNodesIdSeq_.find(matchLocation.nodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        return false;
    }

    int v = iter->second;
    std::vector<DijkstraResult_t> dijkstraResult;
    Dijkstra::DijkstraCalcu(matrix_, v, dijkstraResult);
    
    double minDistToAimSlot = INFINITY;
    for (const auto &distResult : dijkstraResult)
    {
        double dist = INFINITY;
        if (shortLotOnNode(pamNodes_[distResult.seq].id, oldAimSlotId, tempShortLot, dist))
        {
            dist += distResult.dist;
            if (dist < minDistToAimSlot)
            {
                minDistToAimSlot = dist;
                NavigateResult.aimParkingSlot = tempShortLot.aimParkingSlot;
                NavigateResult.nodeId = tempShortLot.moventId;
                NavigateResult.upNodeId = tempShortLot.nodeId;
                NavigateResult.dist = distRefPosToNextNode + dist;
                NavigateResult.path.clear();
                NavigateResult.path.emplace_back(matchLocation.nodeId);
                for (const auto &tempNode : distResult.path)
                {
                    NavigateResult.path.emplace_back(pamNodes_[tempNode].id);
                }
            }
        }
    }

    if (minDistToAimSlot == INFINITY)
    {
        printf("[PAMParkingNavigate]: failed find slot!\n");
        // 如果都没有找到合适的路径，返回false
        return false;
    }

    printf("[PAMParkingNavigate]: successful find slot!\n");
    return true;
}

bool PAMParkingNavigate::exitNavigate(const long currentSlotID, pammodule::ExitNavigateResult_t &ExitNavigateResult)
{
    // 出口的List 是空的
    if (parkingExit_.empty())
    {
        printf("[PAMParkingNavigate]: parkingExit_ is empty! (There is no exit on the map)\n");
        return false;
    }
    
    // 给nodeID一个负值，用于后续判断
    ExitNavigateResult.nodeId = -1;
    ExitNavigateResult.ExitNodeId = -1;

    // 搜索当前车位所在的node
    for (const auto &pamNode : pamNodes_)
    {
        for (const auto &inDrive : pamNode.inDrives)
        {
            for (const auto &parkingSlot : inDrive.parkingSlots)
            {
                // 已经搜索当前车位所在的node
                if (parkingSlot->slotID == currentSlotID)
                {
                    ExitNavigateResult.nodeId = pamNode.id;
                    ExitNavigateResult.upNodeId = inDrive.upstreamPAMNodeId;
                    ExitNavigateResult.currentSlot = parkingSlot;
                    break;
                }
            }
            // 已经搜索当前车位所在的node
            if (ExitNavigateResult.nodeId >= 0)
            {
                break;
            }
        }
        // 已经搜索当前车位所在的node
        if (ExitNavigateResult.nodeId >= 0)
        {
            break;
        }
    }

    if (ExitNavigateResult.nodeId < 0)
    {
        printf("[PAMParkingNavigate]: The current parking ID: %lu was not found on the map!\n", currentSlotID);
        return false;
    }

    // 如果当前车位所在的节点就是出口
    if(pamNodes_[ExitNavigateResult.nodeId].attributes.PAMNodeAttributes_exit)
    {
        ExitNavigateResult.path.emplace_back(ExitNavigateResult.nodeId);
        ExitNavigateResult.ExitNodeId = ExitNavigateResult.nodeId;
           
        // 打印，用于测试
        printf("[PAMParkingNavigate]: already find Nearest exit ID: %lu, The current node ID: %lu\n",
               ExitNavigateResult.ExitNodeId,
               ExitNavigateResult.nodeId);
        printf("[PAMParkingNavigate]: The path:  CurrentPos --> %lu \n", ExitNavigateResult.ExitNodeId);
        return true;
    }

    // 通过迪杰斯特拉算法，去往最近的出口
    auto iter = pamNodesIdSeq_.find(ExitNavigateResult.nodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        printf("[PAMParkingNavigate]: The current node ID: %lu was not found on pamNodesIdSeq_!\n", ExitNavigateResult.nodeId);
        return false;
    }

    int v = iter->second;
    std::vector<DijkstraResult_t> dijkstraResult;
    Dijkstra::DijkstraCalcu(matrix_, v, dijkstraResult);
    
    double minDistance  = INFINITY;
    int minSeq = -1;
    
    for (uint i = 0; i < dijkstraResult.size(); ++i)
    {
        auto iter = &dijkstraResult[i];
        if (pamNodes_[iter->seq].attributes.PAMNodeAttributes_exit)
        {
            if (iter->dist < minDistance)
            {
                minSeq = i;
                minDistance = iter->dist;
            }
        }
    }

    // 输出最短路径,注意将seq转换为nodeID
    if(minSeq >= 0)
    {
        auto dijkstraResultPtr = &dijkstraResult[minSeq];
        ExitNavigateResult.ExitNodeId = pamNodes_[dijkstraResultPtr->seq].id;
        
        ExitNavigateResult.path.emplace_back(ExitNavigateResult.nodeId);
        for (const auto &iter : dijkstraResultPtr->path)
        {
            ExitNavigateResult.path.emplace_back(pamNodes_[iter].id);
        }

        // 打印，用于测试
        printf("[PAMParkingNavigate]: already find Nearest exit ID: %lu, The current node ID: %lu\n",
               ExitNavigateResult.ExitNodeId,
               ExitNavigateResult.nodeId);
        printf("[PAMParkingNavigate]: The path:  CurrentPos ");
        for (const auto &nodeID : ExitNavigateResult.path)
        {
            printf("-->%lu", nodeID);
        }
        printf("\n");

        return true;
    }
    return false;
}

bool PAMParkingNavigate::shortLotOnCurrentLink(const PositionModelExt &refPos,
                                               const pammodule::MatchLocation_t &matchLocation,
                                               const long &oldAimSlotId,
                                               pammodule::ShortLot_t &shortLot)
{
    if (matchLocation.arrive == true)
    {
        return false;
    }

    auto iter = pamNodesIdSeq_.find(matchLocation.nodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        return false;
    }

    for (const auto &inDrive : pamNodes_[iter->second].inDrives)
    {
        if (inDrive.upstreamPAMNodeId == matchLocation.upNodeId)
        {
            for (const auto &slot : inDrive.parkingSlots)
            {
                // 如果车位状态不可用,直接跳过
                if (slot->status != pammodule::SlotStatus_available)
                {
                    continue;
                }
                
                // 如果车位被锁定, 且不是被本车锁定, 直接跳过。寻找下一个车位
                if (slot->slotID != oldAimSlotId && slot->parkingLock == pammodule::ParkingLock_locked)
                {
                    continue;
                }

                // 没有车位坐标
                if (slot->position.topLeft.lat < 0 || slot->position.topLeft.Long < 0)
                {
                    continue;
                }

                PositionModelExt tempSlotPos;
                tempSlotPos.setLatitude(slot->position.topLeft.lat);
                tempSlotPos.setLongitude(slot->position.topLeft.Long);
                // 计算目标车位与当前连线的方向
                double angle = base::MathUtil::getAzimuth(refPos.getLatitude(), refPos.getLongitude(), tempSlotPos.getLatitude(), tempSlotPos.getLongitude());
                // 车子与车位方向与车辆前进方向的夹角小于60度, 说明车位在车辆行进方向的前方。
                if (base::MathUtil::getInAngle(refPos.getHeading(), angle) < 60)
                {
                    shortLot.aimParkingSlot = slot;
                    shortLot.nodeId = matchLocation.upNodeId;
                    shortLot.moventId = matchLocation.nodeId;
                    return true;
                }
            }
        }
    }

    return false;
}

bool PAMParkingNavigate::shortLotOnNode(long nodeId,
                                        const long &oldAimSlotId,
                                        pammodule::ShortLot_t &shortLot,
                                        double &dist)
{
    std::vector<pammodule::ShortLot_t> tempShortLotList;

    auto iter = pamNodesMovents_.find(nodeId);
    if (iter != pamNodesMovents_.end())
    {
        // 对该节点通往其他节点的所有link进行搜索, 求每条link的最短车位
        for (auto tempMoventId : iter->second)
        {
            pammodule::ShortLot_t tempShortLot;
            // 求link上的最短车位
            if (shortLotOnLink(nodeId, tempMoventId, oldAimSlotId, tempShortLot))
            {
                tempShortLotList.emplace_back(tempShortLot);
            }
        }
    }

    // 未匹配到车位
    if (tempShortLotList.empty())
    {
        return false;
    }

    // 默认设置第一个,防止车位坐标不存在，计算不出距离
    shortLot = tempShortLotList[0];
    
    // 对所有link上的车位求最小。
    double mindistance = INFINITY;
    for (const auto &slot : tempShortLotList)
    {
        double tempDistance;
        tempDistance = distSlotFromNode(slot, nodeId);
        std::cout << tempDistance << std::endl;
        std::cout << "nodeId:   " << nodeId << "-->" << slot.aimParkingSlot->slotID << ":    "
                  << tempDistance << std::endl;

        if ((tempDistance > 0) && (tempDistance < mindistance))
        {
            shortLot = slot;
            mindistance = tempDistance;
        }
    }

    // 如果没有设置车位的坐标，求不出距离即mindistance == INFINITY, 默认设置节点通往车位的距离为 0
    dist = (mindistance == INFINITY) ? 0 : mindistance;
    return true;
}

bool PAMParkingNavigate::shortLotOnLink(const long nodeId,
                                        const long moventNodeId,
                                        const long &oldAimSlotId,
                                        pammodule::ShortLot_t &shortLot)
{
    auto iter = pamNodesIdSeq_.find(moventNodeId);

    if (iter == pamNodesIdSeq_.end())
    {
        return false;
    }

    for(const auto inDrive : pamNodes_[iter->second].inDrives)
    {
        if (inDrive.upstreamPAMNodeId == nodeId)
        {
            // 寻找可用的车位。 默认车位是按顺序排序的
            for (const auto &parkingSlot : inDrive.parkingSlots)
            {
                // 如果车位被锁定, 且不是被本车锁定, 直接跳过。寻找下一个车位
                if (parkingSlot->slotID != oldAimSlotId && parkingSlot->parkingLock == pammodule::ParkingLock_locked)
                {
                    continue;
                }

                if (parkingSlot->status == pammodule::SlotStatus_available)
                {
                    shortLot.aimParkingSlot = parkingSlot;
                    shortLot.moventId = moventNodeId;
                    shortLot.nodeId = nodeId;
                    return true;
                }
            }
        }
    }
    return false;
}

double PAMParkingNavigate::distSlotFromNode(const pammodule::ShortLot_t &shortLot, long NodeId)
{
    // 获取节点的坐标
    auto iter = pamNodesIdSeq_.find(NodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        printf("[PAMParkingNavigate] Not find NodeId: %ld!\n", NodeId);
        return -1;
    }

    PositionModelExt tempNodePos;
    tempNodePos.setLatitude(pamNodes_[iter->second].refPos.lat); 
    tempNodePos.setLongitude(pamNodes_[iter->second].refPos.Long);

    // 获取车位的坐标, 默认采用左上角,如果车位位置没有设置，返回-1
    if(shortLot.aimParkingSlot->position.topLeft.lat < 0 || shortLot.aimParkingSlot->position.topLeft.Long < 0)
    {
        printf("[PAMParkingNavigate] get aimParkingSlot topLeft position error: %ld!\n", NodeId);
        return -1;
    }

    PositionModelExt tempSlotPos;
    tempSlotPos.setLatitude(shortLot.aimParkingSlot->position.topLeft.lat);
    tempSlotPos.setLongitude(shortLot.aimParkingSlot->position.topLeft.Long);

    return base::MathUtil::getDistance(tempSlotPos.getLatitude(), tempSlotPos.getLongitude(), tempNodePos.getLatitude(), tempNodePos.getLongitude());
}

double PAMParkingNavigate::distCurrentPosToNode(const PositionModelExt &refPos, long NodeId)
{
    // 获取节点的坐标
    auto iter = pamNodesIdSeq_.find(NodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        printf("[PAMParkingNavigate] Not find NodeId: %ld!\n", NodeId);
        return -1;
    }

    PositionModelExt tempNodePos;
    tempNodePos.setLatitude(pamNodes_[iter->second].refPos.lat); 
    tempNodePos.setLongitude(pamNodes_[iter->second].refPos.Long);

    return base::MathUtil::getDistance(refPos.getLatitude(), refPos.getLongitude(), tempNodePos.getLatitude(), tempNodePos.getLongitude());
}

} }
