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

#include "Dijkstra.h"
#include "PAMModule.h"
#include "base/common/math_util.h"

namespace airos {
namespace app {
class PAMParkingNavigate
{
public:
    PAMParkingNavigate(pammodule::PamNodes &pamNodes,
                       pammodule::PamNodesIdSeq &pamNodesIdSeq,
                       pammodule::PamNodesMovents &pamNodesMovents,
                       MGraph_t &matrix,
                       std::vector<long> &parkingEntrance,
                       std::vector<long> &parkingExit);
    
    ~PAMParkingNavigate();

    /**
     * @brief   入场：搜索最近的车位及去往最近车位的路线
     * @param refPos   当前的位置。
     * @param matchLocation  路径匹配后在pam地图中的位置, (私有消息格式,见PAMModule.h)
     * @param oldAimSlotId  上一次规划的目标车位Id。如果为-1，目前没有引导的目标车位
     * @param NavigateResult  返回规划结果, (私有消息格式,见PAMModule.h)
     * @return  true:匹配成功,找到目标车位  false:未找到目标车位
     */
    bool enterNavigate(const PositionModelExt &refPos,
                  const pammodule::MatchLocation_t &matchLocation,
                  const long &oldAimSlotId,
                  pammodule::NavigateResult_t &NavigateResult);

    /**
     * @brief   离场：搜索最近的出口及去往出口最近的路线
     * @param refPos   当前车位的ID。
     * @param ExitNavigateResult  返回规划结果(私有消息格式,见PAMModule.h)
     * @return  true: 找到离场最短路径  未找到离场最短路径
     */
    bool exitNavigate(const long currentSlotID, pammodule::ExitNavigateResult_t &ExitNavigateResult);

private:
    /**
     * @brief   搜索某个node到他所能直接通往的节点link上所挂载的距离最短的空闲车位
     * @param  NodeId   搜索节点的ID。
     * @param  oldAimSlotId  上一次本车规划的目标车位Id。如果为-1，目前没有引导的目标车位
     * @param  shortLot  返回匹配结果, (私有消息格式,见PAMModule.h)
     * @param  dist  返回当前node通往目标车位的最短距离,如果地图中未设置车位坐标, dist默认返回0
     * @return  true:匹配成功,找到目标车位  false:未找到目标车位
     */
    bool shortLotOnNode(long nodeId, const long &oldAimSlotId, pammodule::ShortLot_t &shortLot, double &dist);
    /**
     * @brief   搜索某个link上距离最短的空闲车位
     * @param  nodeId   节点的ID。
     * @param  moventNodeId  可通往的节点ID。
     * @param oldAimSlotId  上一次规划的目标车位Id。如果为-1，目前没有引导的目标车位。用于本次参考
     * @param  shortLot  返回匹配结果, (私有消息格式,见PAMModule.h)
     * @return  true:匹配成功,找到目标车位  false:未找到目标车位
     */
    bool shortLotOnLink(long nodeId, long moventNodeId, const long &oldAimSlotId, pammodule::ShortLot_t &shortLot);

    /**
     * @brief   求车位与node之间的距离
     * @param shortLot  车位信息
     * @param NodeId   节点的ID。
     * @return  车位与node的距离， -1 表示错误
     */
    double distSlotFromNode(const pammodule::ShortLot_t &shortLot, long NodeId);

    /**
     * @brief   当前位置与node之间的距离
     * @param refPos  当前位置
     * @param NodeId   节点的ID。
     * @return  车位与node的距离， -1 表示错误
     */
    double distCurrentPosToNode(const PositionModelExt &refPos, long NodeId);

    /**
     * @brief   搜索当前link上的最短车位
     * @param refPos   当前的位置。
     * @param matchLocation  路径匹配后在pam地图中的位置, (私有消息格式,见PAMModule.h)
     * @param oldAimSlotId  上一次规划的目标车位Id。如果为-1，目前没有引导的目标车位。用于本次参考
     * @param shortLot  返回最短车位, (私有消息格式,见PAMModule.h)
     * @return  true:匹配成功,找到目标车位  false:未找到目标车位
     */
    bool shortLotOnCurrentLink(const PositionModelExt &refPos,
                               const pammodule::MatchLocation_t &matchLocation,
                               const long &oldAimSlotId,
                               pammodule::ShortLot_t &shortLot);
    


private:
    pammodule::PamNodes pamNodes_;               // 存放每个节点node数据
    pammodule::PamNodesIdSeq pamNodesIdSeq_;     // 节点ID与顺序对应关系的map
    pammodule::PamNodesMovents pamNodesMovents_; // 每个节点ID通向的节点ID list
    MGraph_t matrix_;                            // 节点的邻接矩阵
    std::vector<long> parkingEntrance_;               // 停车场出口
    std::vector<long> parkingExit_;                   // 停车场入口
};

} }
