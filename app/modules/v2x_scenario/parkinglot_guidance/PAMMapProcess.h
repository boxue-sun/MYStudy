/*********************************************************************************
 * @file		PAMMapProcess.h
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
#ifndef PARKING_LOT_GUIDANCE_PAMMAPPROCESS_H
#define PARKING_LOT_GUIDANCE_PAMMAPPROCESS_H

#include "base/common/math_util.h"
#include "app/modules/v2x_scenario/parkinglot_guidance/PositionModelExt.h"
#include "Dijkstra.h"
#include "PAMModule.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"

namespace airos {
namespace app {

class PAMMapProcess
{
public:
    PAMMapProcess();
    virtual ~PAMMapProcess();
    
    /**
     * @brief   从xml文件中读取pamMap数据,转换成nMessageFramePtr格式
     * @param fileList  存放xml文件地址的列表
     * @param pamNodeDate   返回转换成nMessageFramePtr格式数据的列表
     */
    static void readPAMParkingMap(const std::vector<std::string> &fileList,
                                  std::vector<std::shared_ptr<v2xpb::asn::MessageFrame>> &pamParkingMapList);

    /**
     * @brief   对nMessageFramePtr格式的pam进行处理，将节点存放到私有定义的PamNodes中,并获取出入口节点。
     * @param nmfp  pam消息的nMessageFramePtr指针list。
     * @param pamNodeDate   返回将nmfp处理好的pam节点相关的数据(私有消息格式,见PAMModule.h)
     * @param parkingEntrance  返回入口nodeID
     * @param parkingExit      返回出口nodeID
     * @param slotsMap         返回存放车位智能指针的map(用于快速修改车位的状态)
     */
    static void transformToPamNode(const std::vector<std::shared_ptr<v2xpb::asn::MessageFrame>> &pamParkingMapList,
                                   pammodule::PamNodes &pamNodeDate,
                                   std::vector<long> &parkingEntrance,
                                   std::vector<long> &parkingExit,
                                   std::unordered_map<long, pammodule::PamParkingSlotPtr> &slotsMap);

    /**
     * @brief   对pam节点相关的数据进行处理，得到seq与nodeId对应关系的map。方便后续进行处理
     * @param pamNodeDate      pam节点相关的数据，(私有消息格式,见PAMModule.h)。
     * @param pamNodeIdSeq     返回处理好的seq与nodeId对应关系的map(私有消息格式,见PAMModule.h)
     */

    static void transformToPamNodeIdSeq(const pammodule::PamNodes &pamNodeDate, pammodule::PamNodesIdSeq &pamNodeIdSeq);

    /**
     * @brief   求节点与节点之间的邻接矩阵
     * @param pamNodeDate   pam节点相关的数据，(私有消息格式,见PAMModule.h)。
     * @param pamNodeIdSeq  seq与nodeId对应关系的map(私有消息格式,见PAMModule.h)
     * @param matrix  返回处理好的节点间的邻接矩阵
     */
    static void transformToAdjacencyMatrix(const pammodule::PamNodes &pamNodeDate,
                                           const pammodule::PamNodesIdSeq &pamNodeIdSeqconst,
                                           MGraph_t &matrix);

    /**
     * @brief   求出每个节点通往其他的节点
     * @param pamNodeDate   pam节点相关的数据，(私有消息格式,见PAMModule.h)。
     * @param movents  节点通往其他节点的map，(私有消息格式,见PAMModule.h)。
     */
    static void transformToPamNodeMovents(const pammodule::PamNodes &pamNodeDate, pammodule::PamNodesMovents &movents);

private:
    /**
     * @brief   从单个xml文件中读取pamMap数据,转换成nMessageFramePtr格式
     * @param file   存放xml文件地址
     * @param nmfpPamMap 转换成nMessageFramePtr格式的pamMap地图数据
     */
    static void readPAMParkingMapFile(const std::string &file, std::shared_ptr<v2xpb::asn::MessageFrame> &nmfpPamMap);

    /**
     * @brief   补充完善pam节点中CoarseScale等关于路径匹配相关的参数，用于定位匹配
     * @param pamNodeDate   pam节点相关的数据(私有消息格式,见PAMModule.h)。
     */
    static void addCoarseScaleInPamNode(pammodule::PamNodes &pamNodeDate);
    
};

} }

#endif // PARKING_LOT_GUIDANCE_PAMMAPPROCESS_H
