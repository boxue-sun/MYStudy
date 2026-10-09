/*********************************************************************************
 * @file		PAMMatcher.h
 * @brief		PAMMatcher belongs to ncs_4layer
 * @details		PAMMatcher belongs to ncsrt_4layer
 * @author		Changxuhui
 * @date		2022/1/23 
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

#include "base/common/math_util.h"
#include "app/modules/v2x_scenario/parkinglot_guidance/PositionModelExt.h"
#include "PAMModule.h"

namespace airos {
namespace app {
    
class PAMMatcher
{
public:
    PAMMatcher(pammodule::PamNodes &pamNodes,
               pammodule::PamNodesIdSeq &pamNodesIdSeq);
    ~PAMMatcher();

    /**
     * @brief   根据当前位置, 匹配在pamMap中的位置
     * @param refPos   当前的位置。
     * @param matchLocation   返回定位匹配的结果, (私有消息格式,见PAMModule.h)。
     * @return  bool   false: 匹配失败   true: 匹配成功
     */ 
    bool PAMMatching(const PositionModelExt &refPos, pammodule::MatchLocation_t &matchLocation);

private:
    /**
     * @brief   当前位置与某个node的距离
     * @param refPos   当前的位置。
     * @param NodeId   节点的ID。
     * @return  当前位置与node的距离
     */
    double distFromNode(const PositionModelExt &refPos, long NodeId);
    /**
     * @brief   当前位置与某个node连线与正北方向的夹角
     * @param refPos   当前的位置。
     * @param NodeId   节点的ID。
     * @return  当前位置与某个node连线与正北方向的夹角
     */
    double AngleOfConnectWithNode(const PositionModelExt &refPos, long NodeId); 
private:
    pammodule::PamNodes pamNodes_;                       // 存放每个节点node数据
    pammodule::PamNodesIdSeq pamNodesIdSeq_;             // 节点ID与顺序对应关系的map
};

} }
