/*********************************************************************************
 * @file		PAMModule.h
 * @brief		PAMModule belongs to ncs_4layer
 * @details		PAMModule belongs to ncs_4layer
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
#ifndef PARKING_LOT_GUIDANCE_PAMMODULE_H
#define PARKING_LOT_GUIDANCE_PAMMODULE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "app/modules/v2x_scenario/parkinglot_guidance/CoarseScale.h"
#include "app/modules/v2x_scenario/parkinglot_guidance/PathSegment.h"
#include "app/framework/proto/v2xpb-asn-pam.pb.h"

namespace airos {
namespace app {
namespace pammodule
{

typedef struct PamParkingType
{
    bool ParkingType_unknown;
    bool ParkingType_ordinary;
    bool ParkingType_disabled;
    bool ParkingType_mini;
    bool ParkingType_attached;
    bool ParkingType_charging;
    bool ParkingType_stereo;
    bool ParkingType_lady;
    bool ParkingType_extended;
    bool ParkingType_privat;
} PamParkingType_t;

typedef enum PamSlotStatus
{
    SlotStatus_unknown = 0,
    SlotStatus_available = 1,
    SlotStatus_occupied = 2,
    SlotStatus_reserved = 3

} PamSlotStatus_t;

typedef enum PamParkingSpaceTheta {
	ParkingSpaceTheta_unknown	= 0,
	ParkingSpaceTheta_vertical	= 1,
	ParkingSpaceTheta_side	= 2,
	ParkingSpaceTheta_oblique	= 3
} PamParkingSpaceTheta_t;

typedef enum PamParkingLock {
	ParkingLock_unknown	= 0,
	ParkingLock_nolock	= 1,
	ParkingLock_locked	= 2,
	ParkingLock_unlocked = 3
} PamParkingLock_t;

typedef struct PamPosition3D
{
    double lat;
    double Long;
    double elevation;
} PamPosition3D_t;

typedef struct PamRoadPoint
{
    double lat;
    double Long;
}PamRoadPoint_t;

typedef struct PamParkingSlotPosition
{
    PamRoadPoint_t topLeft;
    PamRoadPoint_t topRight;
    PamRoadPoint_t bottomLeft;
} PamParkingSlotPosition_t;

typedef struct PamParkingSlot
{
    long slotID;
    PamParkingSlotPosition_t position;
    std::string sign;
    PamParkingType_t ParkingType;
    long status;            // 取值:  enum PamSlotStatus_t
    long parkingSpaceTheta; // 取值:  enum PamParkingSpaceTheta_t
    long parkingLock;       // 取值:  enum PamParkingLock_t
    std::vector<v2xpb::asn::ParkingSlot> nmfpParkingSlot;  // 存放nmfp中车位的指针, 便于nmfp中车位的状态
    std::string platenum;//被占用时填充车牌号
} PamParkingSlot_t;

using PamParkingSlotPtr = std::shared_ptr<PamParkingSlot_t>;

typedef struct PamDrive
{
    long upstreamPAMNodeId;
    long driveID;
    int twowaySepration;
    long speedLimit;
    long heightRestriction;
    long driveWidth;
    long laneNum;
    std::vector<PamRoadPoint_t> points;
    std::vector<long> movements;
    std::vector<PamParkingSlotPtr> parkingSlots;

    // 用于进行路径匹配
    std::vector<PathSegmentPtr> PathSegments;
    std::shared_ptr<CoarseScale> LinkCoarseScale;
} PamDrive_t;

typedef struct PamNodeAttributes
{
    bool PAMNodeAttributes_entrance;
    bool PAMNodeAttributes_exit;
    bool PAMNodeAttributes_toUpstair;
    bool PAMNodeAttributes_toDownstair;
    bool PAMNodeAttributes_etc;
    bool PAMNodeAttributes_mtc;
    bool PAMNodeAttributes_passAfterPayment;
    bool PAMNodeAttributes_blocke;
} PamNodeAttributes_t;

typedef struct PamNode
{
    long id;
    PamPosition3D_t refPos;
    long floor;
    PamNodeAttributes_t  attributes;
    std::vector<PamDrive_t> inDrives;

    // 用于进行路径匹配
    std::shared_ptr<CoarseScale> nodeCoarseScale;
} PamNode_t;

typedef struct MatchLocation
{
    long nodeId;        // 当前节点
    long upNodeId;      // 上一个节点
    bool arrive;        // 是否到达当前节点  
} MatchLocation_t;

typedef struct NavigateResult
{
    long nodeId;                     // 目标车位当前节点
    long upNodeId;                   // 目标车位的上一个节点
    PamParkingSlotPtr aimParkingSlot; // 目标停车位
    std::vector<long> path;
    double dist;                     // 距离目标车位的距离
} NavigateResult_t;

typedef struct ExitNavigateResult
{
    long nodeId;                  // 当前车位所在的节点
    long upNodeId;                // 当前车位的上一个节点
    PamParkingSlotPtr currentSlot; // 当前的停车位
    long ExitNodeId;              // 出口节点
    std::vector<long> path;
} ExitNavigateResult_t;

typedef struct ShortLot
{
    long nodeId;                     // 当前节点
    long moventId;                   // 通向的节点
    PamParkingSlotPtr aimParkingSlot; // 目标停车位
} ShortLot_t;

typedef struct ParkingRequest
{
    bool ParkingRequest_enter;
	bool ParkingRequest_exit;
	bool ParkingRequest_park;
	bool ParkingRequest_pay;
	bool ParkingRequest_unloadPassenger;
	bool ParkingRequest_pickupPassenger;
	bool ParkingRequest_unloadCargo;
	bool ParkingRequest_loadCargo;
	bool ParkingRequest_reserved1;
	bool ParkingRequest_reverved2;
	bool ParkingRequest_reserved3;
	bool ParkingRequest_reverved4;
} ParkingRequest_t;

typedef struct ParkingVirRequest
{
    long reqID;
    ParkingRequest_t parkingReuest;
    PamParkingSlotPtr aimParkingSlot; // 车辆进场时使用该字段。
    long expectedParkingSlotID;      // 车辆离场时使用该字段
    std::vector<long> path;          // 车辆的路径规划结果
    uint64_t recvTime;               // 上次更新的时间
    bool bsmPostionFlag;             // 是否通过bsm更新Position的flag
    PositionModelExt postion;           // 车辆的当前位置,bsm获取
} ParkingVirRequest_t;

typedef std::unordered_map<long, int> PamNodesIdSeq; // <nodeId, seq>
typedef std::vector<PamNode_t> PamNodes;
typedef std::unordered_map<long, std::vector<long>> PamNodesMovents; // nodeId                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

} // namespace pammodule
} }

#endif // PARKING_LOT_GUIDANCE_PAMMODULE_H
