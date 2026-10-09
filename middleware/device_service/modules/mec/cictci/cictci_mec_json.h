/*
 * @Author: zhangenwei
 * @Date: 2024-01-23 9:46:21
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-23 14:26:18
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/cictci/cictci_mec_json.h
 */
#ifndef BASE_DEVICE_CONNECT_MEC_CICTCI_H_
#define BASE_DEVICE_CONNECT_MEC_CICTCI_H_

#include "base/common/network/serializable_data.h"
namespace os {
namespace v2x {
namespace device {
using namespace afl::base;
enum PtcType
{
    Ptc_unknown     = 0,
    Ptc_motor       = 1,
    Ptc_non_motor   = 2,
    Ptc_pedestrian  = 3,
    Ptc_rsu         = 4,
};
struct cictci_object_detail : public afl::base::SerializableData
{
public:
    string m_ID;
    int m_PtcType;
    double m_VehL;
    double m_VehW;
    bool m_VehW_Empty = true;
    double m_VehH;
    bool m_VehH_Empty = true;
    double m_PtcLon;
    double m_PtcLat;
    double m_PtcEle;
    bool m_PtcEle_Empty = true;
    double m_PtcSpeed;

    double m_PtcHeading;
    bool m_PtcHeading_Empty = true;

    int m_LanneNo;
    bool m_LanneNo_Empty = true;

    int m_VehType;
    bool m_VehType_Empty = true;

    int m_NonVehType;		//非机动车类型-->1:自行车   2:摩托车   3:三轮车
    bool m_NonVehType_Empty = true;

    int m_Tracking;
    bool m_Tracking_Empty = true;

    string m_PlateNum;
    bool m_PlateNum_Empty = true;

    int  m_PlateColor;
    bool m_PlateColor_Empty = true;

    int m_ObjColor;
    bool m_ObjColor_Empty = true;



public:
    cictci_object_detail()
        : m_ID("")
        , m_PtcType(0)
        , m_VehL(0.000000)
        , m_VehW(1.8)
        , m_VehH(1.8)
        , m_PtcLon(0.000000)
        , m_PtcLat(0.000000)
        , m_PtcEle(0.000000)
        , m_PtcSpeed(0.000000)
        , m_PtcHeading(0.000000)
        , m_VehType(0)
		, m_PlateNum("")
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_ID, "ID", j, false);
        JsonSerialize(m_PtcType, "PtcType", j, false);
        JsonSerialize(m_VehL, "VehL", j, false);
        JsonSerialize(m_VehW, "VehW", j, m_VehW_Empty);
        JsonSerialize(m_VehH, "VehH", j, m_VehH_Empty);
        JsonSerialize(m_PtcLon, "PtcLon", j, false);
        JsonSerialize(m_PtcLat, "PtcLat", j, false);
        JsonSerialize(m_PtcEle, "PtcEle", j, m_PtcEle_Empty);
        JsonSerialize(m_PtcSpeed, "PtcSpeed", j, false);
        JsonSerialize(m_PtcHeading, "PtcHeading", j, m_PtcHeading_Empty);
        JsonSerialize(m_LanneNo, "LaneNo", j, m_LanneNo_Empty);
        JsonSerialize(m_VehType, "VehType", j, m_VehType_Empty);

        JsonSerialize(m_NonVehType, "NonVehType", j, m_NonVehType_Empty);
        JsonSerialize(m_Tracking, "Tracking", j, m_Tracking_Empty);
        JsonSerialize(m_PlateNum, "PlateNum", j, m_PlateNum_Empty);
        JsonSerialize(m_PlateColor, "PlateCol", j, m_PlateColor_Empty);
        JsonSerialize(m_ObjColor, "Color", j, m_ObjColor_Empty);

    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_ID, "ID", j, g_flag_empty_novalid);
        JsonDeserialize(m_PtcType, "PtcType", j, g_flag_empty_novalid);
        JsonDeserialize(m_VehL, "VehL", j, g_flag_empty_novalid);
        JsonDeserialize(m_VehW, "VehW", j, m_VehW_Empty);
        JsonDeserialize(m_VehH, "VehH", j, m_VehH_Empty);
        JsonDeserialize(m_PtcLon, "PtcLon", j, g_flag_empty_novalid);
        JsonDeserialize(m_PtcLat, "PtcLat", j, g_flag_empty_novalid);
        JsonDeserialize(m_PtcEle, "PtcEle", j, m_PtcEle_Empty);
        JsonDeserialize(m_PtcSpeed, "PtcSpeed", j, g_flag_empty_novalid);
        JsonDeserialize(m_PtcHeading, "PtcHeading", j, m_PtcHeading_Empty);
        JsonDeserialize(m_LanneNo, "LaneNo", j, m_LanneNo_Empty);
        JsonDeserialize(m_VehType, "VehType", j, m_VehType_Empty);
        JsonDeserialize(m_NonVehType, "NonVehType", j, m_NonVehType_Empty);
        JsonDeserialize(m_Tracking, "Tracking", j, m_Tracking_Empty);
        JsonDeserialize(m_PlateNum, "PlateNum", j, m_PlateNum_Empty);
        JsonDeserialize(m_PlateColor, "PlateCol", j, m_PlateColor_Empty);
        JsonDeserialize(m_ObjColor, "Color", j, m_ObjColor_Empty);

    }
};

//MsgType 2011
struct cictci_object : public afl::base::SerializableData
{
public:
    int     m_MsgType;
    std::string m_DevNo;
    std::string m_RadarNo;
    bool m_RadarNo_Empty;
    std::string m_LidarNo;
    bool m_LidarNo_Empty;
    std::string m_MecNo;
    std::string m_Timestamp;
    std::vector<cictci_object_detail> m_Obj_List;
public:
    cictci_object()
        : m_MsgType(2011)
        , m_DevNo("000000")
        , m_RadarNo("000000")
        , m_LidarNo("000000")
        , m_MecNo("000000")
        , m_Timestamp("1970-01-01 00:00:00.000")
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_DevNo, "DevNo", j, false);
        JsonSerialize(m_RadarNo, "RadarNo", j, m_RadarNo_Empty);
        JsonSerialize(m_LidarNo, "LidarNo", j, m_LidarNo_Empty);
        JsonSerialize(m_MecNo, "MecNo", j, false);

        JsonSerialize(m_Timestamp, "Timestamp", j, false);
        JsonSerialize(m_Obj_List, "Obj_List", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_DevNo, "DevNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_RadarNo, "RadarNo", j, m_RadarNo_Empty);
        JsonDeserialize(m_LidarNo, "LidarNo", j, m_LidarNo_Empty);
        JsonDeserialize(m_MecNo, "MecNo", j, g_flag_empty_novalid);

        JsonDeserialize(m_Timestamp, "Timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(m_Obj_List, "Obj_List", j, g_flag_empty_novalid);
    }
};


struct CictciEventDetail : public afl::base::SerializableData
{
public:
    std::string m_ObjID;
    std::string m_ID;
    int m_EvtType;
    
    int m_EvtStatus = 0;    //0触发 1持续 2消失
    bool m_EvtStatus_Empty = true;
    
    double m_Lon;
    bool m_Lon_Empty = true;
    double m_Lat;
    bool m_Lat_Empty = true;
    double m_Ele;
    bool m_Ele_Empty = true;

    double m_VehL;
    bool m_VehL_Empty = true;
    double m_VehW;
    bool m_VehW_Empty = true;
    double m_VehH;
    bool m_VehH_Empty = true;

    std::string m_EvtV;
    bool m_EvtV_Empty = true;

    std::string m_EvtP;
    bool m_EvtP_Empty = true;

    std::string m_PlateNum;
    bool m_PlateNum_Empty = true;

    int m_RegionNo;
    bool m_RegionNo_Empty = true;

    //德清定制新增参数
    int m_EvtPtcType;
    bool m_EvtPtcType_Empty;

    int m_VehType;
    bool m_VehType_Empty;

    int m_NonVehType;
    bool m_NonVehType_Empty;



public:
	CictciEventDetail()
        : m_ObjID("")
        , m_ID("0")
        , m_EvtType(0)
        , m_Lon(0.000000)
        , m_Lat(0.000000)
        , m_Ele(0.000000)
        , m_VehL(0.000000)
        , m_VehW(0.000000)
        , m_VehH(0.000000)
        ,m_EvtV("")
        , m_EvtP("")
        ,m_PlateNum("")
        ,m_RegionNo(0)
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_ObjID, "ObjId", j, false);
        JsonSerialize(m_ID, "ID", j, false);
        JsonSerialize(m_EvtType, "EvtType", j, false);
        JsonSerialize(m_EvtStatus, "EvtStatus", j, m_EvtStatus_Empty);
        JsonSerialize(m_Lon, "Lon", j, m_Lon_Empty);
        JsonSerialize(m_Lat, "Lat", j, m_Lat_Empty);
        JsonSerialize(m_Ele, "Ele", j, m_Ele_Empty);
        JsonSerialize(m_VehL, "VehL", j, m_VehL_Empty);
        JsonSerialize(m_VehW, "VehW", j, m_VehW_Empty);
        JsonSerialize(m_VehH, "VehH", j, m_VehH_Empty);

        JsonSerialize(m_EvtV, "EvtV", j, m_EvtV_Empty);
        JsonSerialize(m_EvtP, "EvtP", j, m_EvtP_Empty);
        JsonSerialize(m_PlateNum, "PlateNum", j, m_PlateNum_Empty);
        JsonSerialize(m_RegionNo, "RegionNo", j, m_RegionNo_Empty);
        
        JsonSerialize(m_EvtPtcType, "EvtPtcType", j, m_EvtPtcType_Empty);
        JsonSerialize(m_VehType, "VehType", j, m_VehType_Empty);
        JsonSerialize(m_NonVehType, "NonVehType", j, m_NonVehType_Empty);

    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_ObjID, "ObjId", j, g_flag_empty_novalid);
        JsonDeserialize(m_ID, "ID", j, g_flag_empty_novalid);
        JsonDeserialize(m_EvtType, "EvtType", j, g_flag_empty_novalid);
        JsonDeserialize(m_EvtStatus, "EvtStatus", j, m_EvtStatus_Empty);
        JsonDeserialize(m_Lon, "Lon", j, m_Lon_Empty);
        JsonDeserialize(m_Lat, "Lat", j, m_Lat_Empty);
        JsonDeserialize(m_Ele, "Ele", j, m_Ele_Empty);
        JsonDeserialize(m_VehL, "VehL", j, m_VehL_Empty);
        JsonDeserialize(m_VehW, "VehW", j, m_VehW_Empty);
        JsonDeserialize(m_VehH, "VehH", j, m_VehH_Empty);

        JsonDeserialize(m_EvtV, "EvtV", j, m_EvtV_Empty);
        JsonDeserialize(m_EvtP, "EvtP", j, m_EvtP_Empty);
        JsonDeserialize(m_PlateNum, "PlateNum", j, m_PlateNum_Empty);
        JsonDeserialize(m_RegionNo, "RegionNo", j, m_RegionNo_Empty);

        JsonDeserialize(m_EvtPtcType, "EvtPtcType", j, m_EvtPtcType_Empty);
        JsonDeserialize(m_VehType, "VehType", j, m_VehType_Empty);
        JsonDeserialize(m_NonVehType, "NonVehType", j, m_NonVehType_Empty);

    }
};

//MsgType 2012
struct CictciEvent : public afl::base::SerializableData
{
public:
    int m_MsgType;
    std::string m_DevNo;
   
    std::string m_RadarNo;
    bool m_RadarNo_Empty;
    std::string m_LidarNo;
    bool m_LidarNo_Empty;
    std::string m_MecNo;
    std::string m_Timestamp;
    std::vector<CictciEventDetail> m_Evt_List;
public:
	CictciEvent()
        : m_MsgType(2002)
        , m_DevNo("000000")
        , m_RadarNo("000000")
        , m_LidarNo("000000")
        , m_MecNo("000000")
        , m_Timestamp("1970-01-01 00:00:00.000")
    {}

    virtual void serialize(afl::base::json& j)
    {   
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_DevNo, "DevNo", j, false);
        JsonSerialize(m_RadarNo, "RadarNo", j, m_RadarNo_Empty);
        JsonSerialize(m_LidarNo, "LidarNo", j, m_LidarNo_Empty);
        JsonSerialize(m_MecNo, "MecNo", j, false);
        JsonSerialize(m_Timestamp, "Timestamp", j, false);
        JsonSerialize(m_Evt_List, "Evt_List", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_DevNo, "DevNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_RadarNo, "RadarNo", j, m_RadarNo_Empty);
        JsonDeserialize(m_LidarNo, "LidarNo", j, m_LidarNo_Empty);
        JsonDeserialize(m_MecNo, "MecNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_Timestamp, "Timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(m_Evt_List, "Evt_List", j, g_flag_empty_novalid);
    }
};
//MsgType 5001
struct CictciDevDetail : public afl::base::SerializableData
{
public:
    int m_DevType;
    std::string m_DeviceId;
    int m_Status;
public:
    CictciDevDetail()
            : m_DevType(1)
            , m_DeviceId("000000")
            , m_Status(1)
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_DevType, "DevType", j, false);
        JsonSerialize(m_DeviceId, "Deviceld", j, false);
        JsonSerialize(m_Status, "Status", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_DevType, "DevType", j, g_flag_empty_novalid);
        JsonDeserialize(m_DeviceId, "DeviceId", j, g_flag_empty_novalid);
        JsonDeserialize(m_Status, "Status", j, g_flag_empty_novalid);
    }
};

//MsgType 5001 心跳
struct CictciHeartBeat : public afl::base::SerializableData
{
public:
    int m_MsgType;
    std::string m_Timestamp;
    std::vector<CictciDevDetail> m_Dev_List;
public:
    CictciHeartBeat()
            : m_MsgType(5001)
            , m_Timestamp("1970-01-01 00:00:00.000")
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_Timestamp, "Timestamp", j, false);
        JsonSerialize(m_Dev_List, "Dev_List", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_Timestamp, "Timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(m_Dev_List, "Dev_List", j, g_flag_empty_novalid);
    }
};
//MsgType 2016 摄像机角度偏移
struct CictciAlarm : public afl::base::SerializableData
{
public:
    int m_AlarmLevel;
    int m_AlarmStatus;
    uint64_t m_AlarmRaisedTime;
    uint64_t m_AlarmChangedTime;
    bool m_AlarmChangedTime_Empty = false;
    std::string m_DeviceID;
    int m_HorizontalOffset;
    int m_VerticalOffset;
public:
    CictciAlarm()
            : m_AlarmLevel(0)
            , m_AlarmStatus(0)
            , m_AlarmRaisedTime(0)
            , m_AlarmChangedTime(0)
            , m_HorizontalOffset(0)
            , m_VerticalOffset(0)
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_AlarmLevel, "AlarmLevel", j, false);
        JsonSerialize(m_AlarmStatus, "AlarmStatus", j, false);
        JsonSerialize(m_AlarmRaisedTime, "AlarmRaisedTime", j, false);
        JsonSerialize(m_AlarmChangedTime, "AlarmChangedTime", j, m_AlarmChangedTime_Empty);
        JsonSerialize(m_DeviceID, "DeviceID", j, false);
        JsonSerialize(m_HorizontalOffset, "HorizontalOffset", j, false);
        JsonSerialize(m_VerticalOffset, "VerticalOffset", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_AlarmLevel, "AlarmLevel", j, g_flag_empty_novalid);
        JsonDeserialize(m_AlarmStatus, "AlarmStatus", j, g_flag_empty_novalid);
        JsonDeserialize(m_AlarmRaisedTime, "AlarmRaisedTime", j, g_flag_empty_novalid);
        JsonDeserialize(m_AlarmChangedTime, "AlarmChangedTime", j, m_AlarmChangedTime_Empty);
        JsonDeserialize(m_DeviceID, "DeviceID", j, g_flag_empty_novalid);
        JsonDeserialize(m_HorizontalOffset, "HorizontalOffset", j, g_flag_empty_novalid);
        JsonDeserialize(m_VerticalOffset, "VerticalOffset", j, g_flag_empty_novalid);
    }
};
//MsgType 2016 摄像机角度偏移
struct CictciAngleOffset : public afl::base::SerializableData
{
public:
    int m_MsgType;
    uint64_t m_Timestamp;
    std::vector<CictciAlarm> m_Alarm;
public:
    CictciAngleOffset()
            : m_MsgType(2016)
            , m_Timestamp(0)
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_Timestamp, "TimeStamp", j, false);
        JsonSerialize(m_Alarm, "Alarm", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_Timestamp, "TimeStamp", j, g_flag_empty_novalid);
        JsonDeserialize(m_Alarm, "Alarm", j, g_flag_empty_novalid);
    }
};

//MsgType 2018 红绿灯推送
struct CictciSingleTrafficlightInfo : public afl::base::SerializableData
{
public:
    int m_LinkIndex = INT_MAX;
    int m_TrafficlightType = INT_MAX;
    int m_Color = INT_MAX;
    int m_Count = INT_MAX;
public:
    CictciSingleTrafficlightInfo()
    {}

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_LinkIndex, "LinkIndex", j, false);
        JsonSerialize(m_TrafficlightType, "TrafficlightType", j, false);
        JsonSerialize(m_Color, "Color", j, false);
        JsonSerialize(m_Count, "Count", j, false);

    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_LinkIndex, "LinkIndex", j, g_flag_empty_novalid);
        JsonDeserialize(m_TrafficlightType, "TrafficlightType", j, g_flag_empty_novalid);
        JsonDeserialize(m_Color, "Color", j, g_flag_empty_novalid);
        JsonDeserialize(m_Count, "Count", j, g_flag_empty_novalid);
    }

};

struct CictciTrafficlightList : public afl::base::SerializableData
{
public:
    int m_MsgType;
    std::string m_DevNo;
    std::vector<CictciSingleTrafficlightInfo> m_Trafficlight_List;
public:
    CictciTrafficlightList()
            : m_MsgType(2018)
    {}  

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_DevNo, "DevNo", j, false);
        JsonSerialize(m_Trafficlight_List, "Trafficlight_List", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_DevNo, "DevNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_Trafficlight_List, "Trafficlight_List", j, g_flag_empty_novalid);
    }      
};

struct CictciCoilDetial : public afl::base::SerializableData
{
public:
    std::string m_RegionID = "";
    std::string m_NodeID = "";
    std::string m_UpstreamCrossID = "";
    int m_MeasNo = 0;
    std::string m_LaneID = "";
    std::string m_LaneNo = "";
    int m_CoilNo = 0;
    int m_Volume = 0;
    int m_Volume1 = 0;
    int m_Volume2 = 0;
    int m_Volume3 = 0;
    int m_Volume4 = 0;
    int m_Volume5 = 0;
    double m_PCU = 0.000000;
    double m_AVSpeed = 0.000000;
    double m_Occupancy = 0.000000;
    double m_Occupancy1 = 0.000000;
    double m_Headway = 0.000000;
    double m_Distance = 0.000000;
    double m_Gap = 0.000000;
    double m_Speed_85 = 0.000000;
    double m_Delay = 0.000000;
    double m_Stop = 0.000000;
    double m_Desaturation = 0.000000;
    double m_Go_through = 0.000000;
    double m_Queue_length = 0.000000;
    double m_Number = 0.000000;    
    double m_PedestrianCount = 0.000000;
    double m_NonMotorCount = 0.000000;
    double m_ClassQueue_NaturalNumber = 0.000000;
    double m_ClassQueue_EquivalentNumber = 0.000000;

public:
    CictciCoilDetial()
    {}  

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_RegionID, "RegionID", j, false);
        JsonSerialize(m_NodeID, "NodeID", j, false);
        JsonSerialize(m_UpstreamCrossID, "UpstreamCrossID", j, false);
        JsonSerialize(m_MeasNo, "MeasNo", j, false);
        JsonSerialize(m_LaneID, "LaneID", j, false);
        JsonSerialize(m_LaneNo, "LaneNo", j, false);
        JsonSerialize(m_CoilNo, "CoilNo", j, false);
        JsonSerialize(m_Volume, "Volume", j, false);
        JsonSerialize(m_Volume1, "Volume1", j, false);
        JsonSerialize(m_Volume2, "Volume2", j, false);
        JsonSerialize(m_Volume3, "Volume3", j, false);
        JsonSerialize(m_Volume4, "Volume4", j, false);
        JsonSerialize(m_Volume5, "Volume5", j, false);
        JsonSerialize(m_PCU, "PCU", j, false);
        JsonSerialize(m_AVSpeed, "AVSpeed", j, false);
        JsonSerialize(m_Occupancy, "Occupancy", j, false);
        JsonSerialize(m_Occupancy1, "Occupancy1", j, false);
        JsonSerialize(m_Headway, "Headway", j, false);
        JsonSerialize(m_Distance, "Distance", j, false);
        JsonSerialize(m_Gap, "Gap", j, false);
        JsonSerialize(m_Speed_85, "Speed_85", j, false);
        JsonSerialize(m_Delay, "Delay", j, false);
        JsonSerialize(m_Stop, "Stop", j, false);
        JsonSerialize(m_Desaturation, "Desaturation", j, false);
        JsonSerialize(m_Go_through, "Go_through", j, false);
        JsonSerialize(m_Queue_length, "Queue_length", j, false);
        JsonSerialize(m_Number, "Number", j, false);
        JsonSerialize(m_PedestrianCount, "PedestrianCount", j, false);
        JsonSerialize(m_NonMotorCount, "NonMotorCount", j, false);
        JsonSerialize(m_ClassQueue_NaturalNumber, "ClassQueue_NaturalNumber", j, false);
        JsonSerialize(m_ClassQueue_EquivalentNumber, "ClassQueue_EquivalentNumber", j, false);


    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_RegionID, "RegionID", j, g_flag_empty_novalid);
        JsonDeserialize(m_NodeID, "NodeID", j, g_flag_empty_novalid);
        JsonDeserialize(m_UpstreamCrossID, "UpstreamCrossID", j, g_flag_empty_novalid);
        JsonDeserialize(m_MeasNo, "MeasNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_LaneID, "LaneID", j, g_flag_empty_novalid);
        JsonDeserialize(m_LaneNo, "LaneNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_CoilNo, "CoilNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume, "Volume", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume1, "Volume1", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume2, "Volume2", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume3, "Volume3", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume4, "Volume4", j, g_flag_empty_novalid);
        JsonDeserialize(m_Volume5, "Volume5", j, g_flag_empty_novalid);
        JsonDeserialize(m_PCU, "PCU", j, g_flag_empty_novalid);
        JsonDeserialize(m_AVSpeed, "AVSpeed", j, g_flag_empty_novalid);
        JsonDeserialize(m_Occupancy, "Occupancy", j, g_flag_empty_novalid);
        JsonDeserialize(m_Occupancy1, "Occupancy1", j, g_flag_empty_novalid);
        JsonDeserialize(m_Headway, "Headway", j, g_flag_empty_novalid);
        JsonDeserialize(m_Distance, "Distance", j, g_flag_empty_novalid);
        JsonDeserialize(m_Gap, "Gap", j, g_flag_empty_novalid);
        JsonDeserialize(m_Speed_85, "Speed_85", j, g_flag_empty_novalid);
        JsonDeserialize(m_Delay, "Delay", j, g_flag_empty_novalid);
        JsonDeserialize(m_Stop, "Stop", j, g_flag_empty_novalid);
        JsonDeserialize(m_Desaturation, "Desaturation", j, g_flag_empty_novalid);
        JsonDeserialize(m_Go_through, "Go_through", j, g_flag_empty_novalid);
        JsonDeserialize(m_Queue_length, "Queue_length", j, g_flag_empty_novalid);
        JsonDeserialize(m_Number, "Number", j, g_flag_empty_novalid);
        JsonDeserialize(m_PedestrianCount, "PedestrianCount", j, g_flag_empty_novalid);
        JsonDeserialize(m_NonMotorCount, "NonMotorCount", j, g_flag_empty_novalid);
        JsonDeserialize(m_ClassQueue_NaturalNumber, "ClassQueue_NaturalNumber", j, g_flag_empty_novalid);
        JsonDeserialize(m_ClassQueue_EquivalentNumber, "ClassQueue_EquivalentNumber", j, g_flag_empty_novalid);

    }      
};

//MsgType 2101 交通流量推送
struct CictciTrafficFlow : public afl::base::SerializableData
{
public:
    int m_MsgType;
    std::string m_DevNo;
    std::string m_MecNo;
    std::string m_Timestamp;
    int m_Cycle;
    int m_CoilNum; 
    std::vector<CictciCoilDetial> m_Coil_List;
public:
    CictciTrafficFlow()
            : m_MsgType(2101)
    {}  

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(m_MsgType, "MsgType", j, false);
        JsonSerialize(m_DevNo, "DevNo", j, false);
        JsonSerialize(m_MecNo, "MecNo", j, false);
        JsonSerialize(m_Timestamp, "Timestamp", j, false);
        JsonSerialize(m_Cycle, "Cycle", j, false);
        JsonSerialize(m_CoilNum, "CoilNum", j, false);
        JsonSerialize(m_Coil_List, "Coil_List", j, false);
    }

    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(m_MsgType, "MsgType", j, g_flag_empty_novalid);
        JsonDeserialize(m_DevNo, "DevNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_MecNo, "MecNo", j, g_flag_empty_novalid);
        JsonDeserialize(m_Timestamp, "Timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(m_Cycle, "Cycle", j, g_flag_empty_novalid);
        JsonDeserialize(m_CoilNum, "CoilNum", j, g_flag_empty_novalid);
        JsonDeserialize(m_Coil_List, "Coil_List", j, g_flag_empty_novalid);
    }  

};
}  // namespace device
}  // namespace v2x
}  // namespace os
#endif
