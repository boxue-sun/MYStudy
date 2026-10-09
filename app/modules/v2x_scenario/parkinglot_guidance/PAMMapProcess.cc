/*********************************************************************************
 * @file		PAMMapProcess.cpp
 * @brief		PAMMapProcess belongs to ncs_4layer
 * @details		PAMMapProcess belongs to ncs_4layer
 * @author		Changxuhui
 * @date		2022/2/15 
 * @copyright	Copyright (c) 2022 Gohigh V2X Division.
 * @verbatim
 *  
 *
 *  Change History:
 *  Date           Author      Version    ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2022/2/15   ChangXuhui       1.0       ————            Create this file
 * 													   
 * @endverbatim
 ********************************************************************************/

#include <fstream>
#include "PAMMapProcess.h"
#include "base/common/log.h"

namespace airos {
namespace app {

PAMMapProcess::PAMMapProcess()
{
}

PAMMapProcess::~PAMMapProcess()
{
}

void PAMMapProcess::readPAMParkingMap(const std::vector<std::string> &fileList,
                            std::vector<std::shared_ptr<v2xpb::asn::MessageFrame>> &pamParkingMapList)
{
    // 从配置文件中读取
    for (auto &fs : fileList)
    {
        auto nmfpPamMap = std::make_shared<v2xpb::asn::MessageFrame>();
        readPAMParkingMapFile(fs, nmfpPamMap);
        if(nmfpPamMap)
        {
            pamParkingMapList.push_back(nmfpPamMap);

#if 0 // 打印转换完程后的nmfp消息, 用于测试
            auto readable = nmfpPamMap.print<4096>();
            printf("%s\n", readable.c_str());
#endif
        }
        else
        {
            APP_LOG_ERROR << "read PAM parking map file " << fs << " error!";
            printf("read PAM parking map file %s error!\n", fs.c_str());
        }
    }
}

void PAMMapProcess::readPAMParkingMapFile(const std::string &file, std::shared_ptr<v2xpb::asn::MessageFrame> &nmfpPamMap)
{
    std::fstream m(file, std::ios::in);
    std::shared_ptr<v2xpb::asn::MessageFrame> retval = nullptr;

    if (!m.is_open())
    {
        APP_LOG_ERROR <<"open PAM parking map file error!";
        printf("open PAM parking map file %s error!\n", file.c_str());
        return;
    }

    std::ostringstream cxt;
    cxt << m.rdbuf();
    std::string content(cxt.str());
    try
    {
        std::string str_asn("");
        // 先使用智路的地图进行解析，不成功再使用asn xer的地图格式
        if(message_frame_map_xml2uper_adapter(content, &str_asn, EnAsnType::YDT_3709_2020_EXT) > 0 ) 
        {
            APP_LOG_INFO << "pam map xml to uper: true, use airos xml2uper";
        } 
        else if(message_frame_xer2uper_adapter(content, &str_asn, EnAsnType::YDT_3709_2020_EXT) > 0) 
        {
            APP_LOG_INFO << "pam map xml to uper: true, use xer2uper";
        }
        else 
        {
            APP_LOG_WARN << "pam map xml to uper: false";
            return;
        }

        for(int tempk = 0; tempk < str_asn.size(); tempk++)
        {
            printf("%02x ", str_asn.at(tempk));
        }
        printf("\n");
        std::string str_pb("");
        if(0 >= message_frame_uper2pbstr_adapter(str_asn, &str_pb, EnAsnType::YDT_3709_2020_EXT)) 
        {
            APP_LOG_WARN << "pam asn to pb false";
            return;
        }
        APP_LOG_INFO << "pam asn to pb true";

        nmfpPamMap->ParsePartialFromString(str_pb);
        APP_LOG_INFO << "pam pb:" << nmfpPamMap->DebugString();

    }
    catch (...)
    {
        APP_LOG_ERROR << "PAM parking map file  decode error!";
        printf("PAM parking map file %s decode error!\n", file.c_str());
        return;
    }

    APP_LOG_ERROR << "PAM parking map file decode success!";
    printf("PAM parking map file %s decode success!\n", file.c_str());
    return;
}

void PAMMapProcess::transformToPamNode(const std::vector<std::shared_ptr<v2xpb::asn::MessageFrame>> &pamParkingMapList,
                                       pammodule::PamNodes &pamNodeDate,
                                       std::vector<long> &parkingEntrance,
                                       std::vector<long> &parkingExit,
                                       std::unordered_map<long, pammodule::PamParkingSlotPtr> &slotsMap)
{
    for (auto nmfp : pamParkingMapList)
    {
        if(!nmfp || !nmfp->has_extframe() || nmfp->extframe().messagevalue().typresent() != 3)
        {
            APP_LOG_ERROR << "messageframe type error!";
            return;
        }

        // 遍历pamNodes, 将pamNode的数据存放私格式消息pamNodeDate中, 可选字段如果没有,设置默认值
        for (int i = 0; i < nmfp->extframe().messagevalue().pamdata().pamnodes_size(); ++i)
        {
            auto nmfpPamNode = nmfp->extframe().messagevalue().pamdata().pamnodes(i);
            pammodule::PamNode_t tempPamNode;
            tempPamNode.id = nmfpPamNode.id();
            tempPamNode.refPos.lat = nmfpPamNode.refpos().llh().latitude();
            tempPamNode.refPos.Long = nmfpPamNode.refpos().llh().longitude();
            tempPamNode.refPos.elevation = nmfpPamNode.refpos().llh().has_elevation() ? nmfpPamNode.refpos().llh().elevation() : 0;

            // 所在楼层
            tempPamNode.floor = nmfpPamNode.has_floor() ? nmfpPamNode.floor() : 1;

            // 节点的属性
            tempPamNode.attributes.PAMNodeAttributes_entrance = false;
            tempPamNode.attributes.PAMNodeAttributes_exit = false;
            tempPamNode.attributes.PAMNodeAttributes_toUpstair = false;
            tempPamNode.attributes.PAMNodeAttributes_toDownstair = false;
            tempPamNode.attributes.PAMNodeAttributes_etc = false;
            tempPamNode.attributes.PAMNodeAttributes_mtc = false;
            tempPamNode.attributes.PAMNodeAttributes_passAfterPayment = false;
            tempPamNode.attributes.PAMNodeAttributes_blocke = false;
            APP_LOG_INFO << "get node floor";
            for(int tempattr = 0; tempattr < nmfpPamNode.attributes_size(); ++tempattr)
            {
                switch(nmfpPamNode.attributes(tempattr))
                {
                case 0:
                    tempPamNode.attributes.PAMNodeAttributes_entrance = true;
                    break;
                case 1:
                    tempPamNode.attributes.PAMNodeAttributes_exit = true;
                    break;
                case 2:
                    tempPamNode.attributes.PAMNodeAttributes_toUpstair = true;
                    break;
                case 3:
                    tempPamNode.attributes.PAMNodeAttributes_toDownstair = true;
                    break;
                case 4:
                    tempPamNode.attributes.PAMNodeAttributes_etc = true;
                    break;
                case 5:
                    tempPamNode.attributes.PAMNodeAttributes_mtc = true;
                    break;
                case 6:
                    tempPamNode.attributes.PAMNodeAttributes_passAfterPayment = true;
                    break;
                case 7:
                    tempPamNode.attributes.PAMNodeAttributes_blocke = true;
                    break;
                default:
                    break;                
                }
            }


            // 当前的node为入口
            if (tempPamNode.attributes.PAMNodeAttributes_entrance)
            {
                parkingEntrance.emplace_back(tempPamNode.id);
            }
            // 当前的node为出口
            if (tempPamNode.attributes.PAMNodeAttributes_exit)
            {
                parkingExit.emplace_back(tempPamNode.id);
            }


            APP_LOG_INFO << "get node attributes";
            tempPamNode.inDrives.clear();
            if (nmfpPamNode.indrives_size() > 0)
            {
                for (int j = 0; j < nmfpPamNode.indrives_size(); ++j)
                {
                    auto nmfpInDrive = nmfpPamNode.indrives(j);
                    pammodule::PamDrive_t tempInDrive;
                    tempInDrive.upstreamPAMNodeId = nmfpInDrive.upstreampamnodeid();

                    // 可选的字段如果存在,直接赋值。不存在设置默认值
                    tempInDrive.driveID = nmfpInDrive.has_driveid() ? nmfpInDrive.driveid() : -1;
                    tempInDrive.twowaySepration = nmfpInDrive.has_twowaysepration() ? nmfpInDrive.twowaysepration() : false;
                    // 停车场限速，如果不存在。设置默认值40;
                    tempInDrive.speedLimit = nmfpInDrive.has_speedlimit() ? nmfpInDrive.speedlimit() : 40;
                    // 限高，如果不存在，设置默认值为3
                    tempInDrive.heightRestriction = nmfpInDrive.has_heightrestriction() ? nmfpInDrive.heightrestriction() : 3;
                    // 路的宽度，如果不存在, 设置默认值为3.5m
                    tempInDrive.driveWidth = nmfpInDrive.has_drivewidth() ? nmfpInDrive.drivewidth() : 3.5;
                    // 车道的数量，如果不存在，设置默认值1
                    tempInDrive.laneNum = nmfpInDrive.has_lanenum() ? nmfpInDrive.lanenum() : 1;

                    tempInDrive.points.clear();
                    for (int k = 0; k < nmfpInDrive.points_size(); ++k)
                    {
                        pammodule::PamRoadPoint_t tempPoint;
                        auto nmfpPoint = nmfpInDrive.points(k);

                        if (nmfpPoint.has_llh())
                        {
                            if (nmfpPoint.llh().has_latitude() && nmfpPoint.llh().has_longitude())
                            {
                                tempPoint.lat = nmfpPoint.llh().latitude();
                                tempPoint.Long = nmfpPoint.llh().longitude();
                                tempInDrive.points.push_back(tempPoint);
                            }
                        }
                    }

                    tempInDrive.movements.clear();

                    for (int m = 0; m < nmfpInDrive.movements_size(); ++m)
                    {
                        tempInDrive.movements.push_back(nmfpInDrive.movements(m));
                    }
                    

                    tempInDrive.parkingSlots.clear();

                    for (int n = 0; n < nmfpInDrive.parkingslots_size(); ++n)
                    {
                        auto nmfpParkingSlot = nmfpInDrive.parkingslots(n);

                        // 因为同一个车位可能挂在多个node上，现在slotmap中寻找
                        auto iter = slotsMap.find(nmfpParkingSlot.slotid());
                        if (iter != slotsMap.end())
                        {
                            iter->second->nmfpParkingSlot.emplace_back(nmfpParkingSlot);
                            tempInDrive.parkingSlots.emplace_back(iter->second);
                        }
                        else
                        {
                            pammodule::PamParkingSlotPtr tempParkingSlot = std::make_shared<pammodule::PamParkingSlot_t>();

                            tempParkingSlot->nmfpParkingSlot.emplace_back(nmfpParkingSlot);
                            tempParkingSlot->slotID = nmfpParkingSlot.slotid();

                            if (nmfpParkingSlot.has_position() 
                                && nmfpParkingSlot.position().topleft().has_llh() 
                                && nmfpParkingSlot.position().topright().has_llh()
                                && nmfpParkingSlot.position().bottomleft().has_llh())
                            {
                                tempParkingSlot->position.topLeft.lat = nmfpParkingSlot.position().topleft().llh().latitude();
                                tempParkingSlot->position.topLeft.Long = nmfpParkingSlot.position().topleft().llh().longitude();
                                
                                tempParkingSlot->position.topRight.lat = nmfpParkingSlot.position().topright().llh().latitude();
                                tempParkingSlot->position.topRight.Long = nmfpParkingSlot.position().topright().llh().longitude();
                            
                                tempParkingSlot->position.bottomLeft.lat = nmfpParkingSlot.position().bottomleft().llh().latitude();
                                tempParkingSlot->position.bottomLeft.Long = nmfpParkingSlot.position().bottomleft().llh().longitude();
                          
                            }
                            else
                            {
                                tempParkingSlot->position.topLeft.lat = -1;
                                tempParkingSlot->position.topLeft.Long = -1;
                                tempParkingSlot->position.topRight.lat = -1;
                                tempParkingSlot->position.topRight.Long = -1;
                                tempParkingSlot->position.bottomLeft.lat = -1;
                                tempParkingSlot->position.bottomLeft.Long = -1;
                            }

                            if (nmfpParkingSlot.has_sign())
                            {
                                tempParkingSlot->sign = nmfpParkingSlot.sign();                                                                   
                            }
                            else
                            {
                                tempParkingSlot->sign = "";
                            }

                            tempParkingSlot->ParkingType.ParkingType_unknown = false;
                            tempParkingSlot->ParkingType.ParkingType_ordinary = false;
                            tempParkingSlot->ParkingType.ParkingType_disabled = false;
                            tempParkingSlot->ParkingType.ParkingType_mini = false;
                            tempParkingSlot->ParkingType.ParkingType_attached = false;
                            tempParkingSlot->ParkingType.ParkingType_charging = false;
                            tempParkingSlot->ParkingType.ParkingType_stereo = false;
                            tempParkingSlot->ParkingType.ParkingType_lady = false;
                            tempParkingSlot->ParkingType.ParkingType_extended = false;
                            tempParkingSlot->ParkingType.ParkingType_privat = false;

                            switch(nmfpParkingSlot.parkingtype())
                            {
                            case 0:
                                tempParkingSlot->ParkingType.ParkingType_unknown = true;
                                break;
                            case 1:
                                tempParkingSlot->ParkingType.ParkingType_ordinary = true;
                                break;
                            case 2:
                                tempParkingSlot->ParkingType.ParkingType_disabled = true;
                                break;
                            case 3:
                                tempParkingSlot->ParkingType.ParkingType_mini = true;
                                break;
                            case 4:
                                tempParkingSlot->ParkingType.ParkingType_attached = true;
                                break;
                            case 5:
                                tempParkingSlot->ParkingType.ParkingType_charging = true;
                                break;
                            case 6:
                                tempParkingSlot->ParkingType.ParkingType_stereo = true;
                                break;
                            case 7:
                                tempParkingSlot->ParkingType.ParkingType_lady = true;
                                break;
                            case 8:
                                tempParkingSlot->ParkingType.ParkingType_extended = true;
                                break;
                            case 9:
                                tempParkingSlot->ParkingType.ParkingType_privat = true;
                                break;
                            default:
                                break;                
                            }
                            


                            tempParkingSlot->status = nmfpParkingSlot.status();
                            tempParkingSlot->parkingSpaceTheta = nmfpParkingSlot.parkingspacetheta();
                            tempParkingSlot->parkingLock = nmfpParkingSlot.parkinglock();
                            tempParkingSlot->platenum = "";
                            tempInDrive.parkingSlots.push_back(tempParkingSlot);

                            // 将车位信息的智能指着添加到slotmap中便于快速修改车位的状态
                            slotsMap[tempParkingSlot->slotID] = tempParkingSlot;
                        }
                    }
                    
                    tempPamNode.inDrives.push_back(tempInDrive);
                }
            }
            APP_LOG_INFO << "get node indrive";

            // 检测存入的nodeID有没有相同的。如果有相同的直接丢掉。没有相同的添加
            bool alreadyExit = false;
            for (auto node : pamNodeDate)
            {
                if (tempPamNode.id == node.id)
                {
                    alreadyExit = true;
                    break;
                }
            }
            if (!alreadyExit)
            {
                pamNodeDate.emplace_back(tempPamNode);
            }
        }
    }
    addCoarseScaleInPamNode(pamNodeDate);
}

void PAMMapProcess::addCoarseScaleInPamNode(pammodule::PamNodes &pamNodeDate)
{
    pammodule::PamNodesIdSeq tempPamNodeIdSeq;
    transformToPamNodeIdSeq(pamNodeDate, tempPamNodeIdSeq);

    for (auto &tempPamNode : pamNodeDate)
    {
        // 当前节点的坐标
        PositionModelExt tempNodePosition;
        tempNodePosition.setLongitude(tempPamNode.refPos.Long);
        tempNodePosition.setLatitude(tempPamNode.refPos.lat);
        tempPamNode.nodeCoarseScale.reset(new CoarseScale(tempNodePosition));
        
        // 该节点没有上游node,直接跳过
        if (tempPamNode.inDrives.size() == 0)
        {
            continue;
        }

        for (auto &tempInDrive : tempPamNode.inDrives)
        {
            // 存放上游节点到当前节点中点的坐标，用于计算两个节点之间的距离
            std::vector<PositionModelExt> tempPositionList;
            
            // 获取上游节点坐标
            auto iter = tempPamNodeIdSeq.find(tempInDrive.upstreamPAMNodeId);
            if (iter == tempPamNodeIdSeq.end())
            {
                continue;
            }
            int tempSeqUpId = iter->second;

            PositionModelExt tempUpPosition;
            tempUpPosition.setLatitude(pamNodeDate[tempSeqUpId].refPos.lat);
            tempUpPosition.setLongitude(pamNodeDate[tempSeqUpId].refPos.Long);
            tempPositionList.push_back(tempUpPosition);

            // 两个节点之间的坐标
            for(auto &tempPoint : tempInDrive.points)
            {
                PositionModelExt tempPointPosition;
                tempPointPosition.setLongitude(tempPoint.lat);
                tempPointPosition.setLatitude(tempPoint.Long);
                tempPositionList.push_back(tempPointPosition);
            }
            tempPositionList.push_back(tempNodePosition);
            
            // 生成匹配区域
            double tempLaneWidth = tempInDrive.driveWidth <= 0 ? 3.5 : tempInDrive.driveWidth;
            for (uint i = 0; i < tempPositionList.size() - 1; ++i)
            {
                tempInDrive.PathSegments.emplace_back(PathSegmentPtr(new PathSegment(tempPositionList[i],
                                                                                    tempPositionList[i + 1],
                                                                                    tempLaneWidth)));
            }

            tempInDrive.LinkCoarseScale.reset(new CoarseScale(tempInDrive.PathSegments, tempLaneWidth));
            *tempPamNode.nodeCoarseScale += *tempInDrive.LinkCoarseScale;
        }
    }
}

void PAMMapProcess::transformToPamNodeMovents(const pammodule::PamNodes &pamNodeDate,
                                                pammodule::PamNodesMovents &movents)
{
    for(auto &pamNode : pamNodeDate)
    {
        long tempPamNodeId = pamNode.id;
        for (auto inDrive : pamNode.inDrives)
        {
            movents[inDrive.upstreamPAMNodeId].push_back(tempPamNodeId);
        }
    }
}

void PAMMapProcess::transformToPamNodeIdSeq(const pammodule::PamNodes &pamNodeDate,
                                              pammodule::PamNodesIdSeq &pamNodeIdSeq)
{
    for (size_t i = 0; i < pamNodeDate.size(); ++i)
    {
        pamNodeIdSeq.insert(std::make_pair(pamNodeDate[i].id, i));
    }
}

void PAMMapProcess::transformToAdjacencyMatrix(const pammodule::PamNodes &pamNodeDate,
                                                 const pammodule::PamNodesIdSeq &pamNodeIdSeqconst,
                                                 MGraph_t &matrix)
{
    if (pamNodeIdSeqconst.size() != pamNodeDate.size())
    {
        return;
    }

    matrix.num_vertexes = pamNodeIdSeqconst.size();

    // node节点的数量超过默认设置的最大值
    if (matrix.num_vertexes > DIJISTRA_MAXVEX)
    {
        return;
    }

    // 初始化邻接矩阵
    for (int i = 0; i < matrix.num_vertexes; ++i)
    {
        for (int j = 0; j < matrix.num_vertexes; ++j)
        {
            matrix.arc[i][j] = DIJISTRA_INFINITY;
        }
    }

    // 计算两个节点之间的距离
    for(auto &tempPamNode : pamNodeDate)
    {
        // 该节点没有上游node,直接跳过
        if (tempPamNode.inDrives.size() == 0)
        {
            continue;
        }
        
        auto iterId = pamNodeIdSeqconst.find(tempPamNode.id);
        if (iterId == pamNodeIdSeqconst.end())
        {
            continue;
        }
        int tempSeqId = iterId->second;

        // 当前节点的坐标,用于计算距离
        PositionModelExt tempNodePosition;
        tempNodePosition.setLongitude(tempPamNode.refPos.Long);
        tempNodePosition.setLatitude(tempPamNode.refPos.lat);

        for (auto &tempInDrive : tempPamNode.inDrives)
        {
            // 存放上游节点到当前节点中点的坐标，用于计算两个节点之间的距离
            std::vector<PositionModelExt> tempPositionList;
            
            // 获取上游节点坐标
            auto iter = pamNodeIdSeqconst.find(tempInDrive.upstreamPAMNodeId);
            if (iter == pamNodeIdSeqconst.end())
            {
                continue;
            }
            int tempSeqUpId = iter->second;

            PositionModelExt tempUpPosition;
            tempUpPosition.setLatitude(pamNodeDate[iter->second].refPos.lat);
            tempUpPosition.setLongitude(pamNodeDate[iter->second].refPos.Long);
            tempPositionList.push_back(tempUpPosition);

            // 两个节点之间的坐标,用于计算距离
            for(auto &tempPoint : tempInDrive.points)
            {
                PositionModelExt tempPointPosition;
                tempPointPosition.setLongitude(tempPoint.lat);
                tempPointPosition.setLatitude(tempPoint.Long);
                tempPositionList.push_back(tempPointPosition);
            }

            tempPositionList.push_back(tempNodePosition);
            
            // 两个节点之间的距离, 填入邻接矩阵
            double tempDistance = 0.0;
            for (size_t i = 0; i < tempPositionList.size() - 1; ++i)
            {
                tempDistance += base::MathUtil::getDistance(tempPositionList[i].getLatitude(), tempPositionList[i].getLongitude(), tempPositionList[i+1].getLatitude(), tempPositionList[i+1].getLongitude());
            }
            matrix.arc[tempSeqUpId][tempSeqId] = static_cast<int>(tempDistance);
        }
    }
}
} }
