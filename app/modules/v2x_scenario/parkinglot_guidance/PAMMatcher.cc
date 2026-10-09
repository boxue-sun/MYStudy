/*********************************************************************************
 * @file		PAMMatcher.cpp
 * @brief		PAMMatcher belongs to ncs_4layer
 * @details		PAMMatcher belongs to ncsrt_4layer
 * @author		Changxuhui
 * @date		2022/2/23 
 * @copyright	Copyright (c) 2021 Gohigh V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/2/23   ChangXuhui       1.0       ————          Create this file  
 * 													   
 * @endverbatim
 ********************************************************************************/

#include "PAMMatcher.h"

namespace airos {
namespace app {
PAMMatcher::PAMMatcher(pammodule::PamNodes &pamNodes,pammodule::PamNodesIdSeq &pamNodesIdSeq)
                    : pamNodes_(pamNodes), pamNodesIdSeq_(pamNodesIdSeq)
{
}

PAMMatcher::~PAMMatcher()
{
}

bool PAMMatcher::PAMMatching(const PositionModelExt &refPos, pammodule::MatchLocation_t &matchLocation)
{
    matchLocation.nodeId = -1;
    matchLocation.upNodeId = -1;
    matchLocation.arrive = false;

    CoarseScale tempCoarseScale(refPos);

    // 先在node上进行匹配
    for (const auto &tempPamNode : pamNodes_)
    {
        // 如果在node上匹配成功
        if (tempPamNode.nodeCoarseScale->OverLap(tempCoarseScale))
        {
            printf("[PAMMatcher]:  Match succeeded on node, nodeID : %ld\n", tempPamNode.id);
            printf("size:%d\n", tempPamNode.inDrives.size());
            for (const auto &tempInDrives : tempPamNode.inDrives)
            {
                // 在link上匹配成功
                printf("111111111111111:%lld\n", tempInDrives.upstreamPAMNodeId);
                if (tempInDrives.LinkCoarseScale->OverLap(tempCoarseScale))
                {
                    printf("[PAMMatcher]:  Match succeeded on Link, upnodeID : %ld\n", tempInDrives.upstreamPAMNodeId);
                    for (const auto &tempPathSegment : tempInDrives.PathSegments)
                    {
                        // 在PathSegments上进行匹配
                        if (tempPathSegment->m_CoarseScale->OverLap(tempCoarseScale))
                        {
                            if (abs(tempPathSegment->getHeading() - refPos.getHeading()) < 35)
                            {
                                printf("[PAMMatcher]:  Match succeeded on PathSegment, upnodeID : %ld\n", tempInDrives.upstreamPAMNodeId);
                                double tempDist = distFromNode(refPos, tempPamNode.id);
                                // 如果计算距离错误,继续匹配下一段。
                                if (tempDist < 0)
                                {
                                    continue;
                                }

                                // 如果距离node的距离小于4m,表示已经到达当前node
                                if (tempDist < 4)
                                {
                                    double tempAzimuth = AngleOfConnectWithNode(refPos, tempPamNode.id);
                                    // 如果计算距离错误,继续匹配下一段。
                                    if(tempAzimuth < 0)
                                    {
                                        continue;
                                    }
                                    // 如果车头的方向与当前位置与node节点连线的方向夹角大于90度
                                    // 说明车子已经开过当前节点,继续匹配
                                    if ((tempDist != 0) && (base::MathUtil::getInAngle(tempAzimuth, refPos.getHeading()) > 90))
                                    {
                                        printf("[PAMMatcher]: angle %f \n", base::MathUtil::getInAngle(tempAzimuth, refPos.getHeading()));
                                        continue;
                                    }
                                    printf("[PAMMatcher]: Approaching node  nodeID: %ld \n", tempPamNode.id);
                                    matchLocation.nodeId = tempPamNode.id;
                                    matchLocation.upNodeId = tempInDrives.upstreamPAMNodeId;
                                    matchLocation.arrive = true;
                                    return true;
                                }
                                else
                                {
                                    printf("[PAMMatcher]:Not Approaching node,  nodeID: %ld \n", tempPamNode.id);
                                    matchLocation.nodeId = tempPamNode.id;
                                    matchLocation.upNodeId = tempInDrives.upstreamPAMNodeId;
                                    matchLocation.arrive = false;
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

double  PAMMatcher::distFromNode(const PositionModelExt &refPos, long NodeId)
{
    auto iter = pamNodesIdSeq_.find(NodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        return -1;
    }

    PositionModelExt tempNodePos;
    tempNodePos.setLatitude(pamNodes_[iter->second].refPos.lat); 
    tempNodePos.setLongitude(pamNodes_[iter->second].refPos.Long);
    return base::MathUtil::getDistance(refPos.getLatitude(), refPos.getLongitude(), tempNodePos.getLatitude(), tempNodePos.getLongitude());
}

double PAMMatcher::AngleOfConnectWithNode(const PositionModelExt &refPos, long NodeId)
{
    auto iter = pamNodesIdSeq_.find(NodeId);
    if (iter == pamNodesIdSeq_.end())
    {
        return -1;
    }

    PositionModelExt tempNodePos;
    tempNodePos.setLatitude(pamNodes_[iter->second].refPos.lat);
    tempNodePos.setLongitude(pamNodes_[iter->second].refPos.Long);

    return base::MathUtil::getAzimuth(refPos.getLatitude(), refPos.getLongitude(), tempNodePos.getLatitude(), tempNodePos.getLongitude());
}


} }
