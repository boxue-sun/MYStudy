/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_DEVICE_SERVICE_RSAP_DATA_MODEL_STATUS_DATA
#define MIDDLEWART_DEVICE_SERVICE_RSAP_DATA_MODEL_STATUS_DATA
#include "namespace.h"
NAMESPACE_PROTOCOL_THREAD_START
#pragma pack(1)
struct  StatusDataHead
{
    uint8_t     channelId;
    uint8_t        rcuId[8];
    uint16_t    status;
};
#pragma pack()

#pragma pack(1)
struct  CamStatusHead
{
    uint8_t     camNum;
};
#pragma pack()
//    BYTE camStatus[];
#pragma pack(1)
struct  CamStatusTail{
    uint8_t id ;
    uint8_t camId[11];
    uint8_t camStatus;
};
#pragma pack()

#pragma pack(1)
struct  RadarStatusHead{
    uint8_t radarNum;
};
#pragma pack()
//      BYTE radarStatus[];
#pragma pack(1)
struct  RadarStatusTail{
    uint8_t id ;
    uint8_t radarId[11] ;
    uint8_t radarStatus;
};
#pragma pack()


#pragma pack(1)
struct  LidarStatusHead
{
    uint8_t lidarNum;
};
#pragma pack()
//      BYTE lidarStatus[];
#pragma pack(1)
struct  LidarStatusTail{
    uint8_t id ;
    uint8_t lidarId[11] ;
    uint8_t lidarStatus ;
};
#pragma pack()


enum RcuStatusType
{
    RcuStatusTypeNormal = 0x0000,
    RcuStatusTypeAbnormal = 0x0001,
    RcuStatusTypeReserved = 0x0003
};
NAMESPACE_PROTOCOL_THREAD_END
#endif
