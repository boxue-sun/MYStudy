/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA
#define AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA

#include "namespace.h"

NAMESPACE_PROTOCOL_THREAD_START
#pragma pack(1)
struct EventDataHead
{
    uint8_t channelId;
    uint8_t rcuId[8];
    uint8_t eventType;
    uint8_t confidence;
    uint32_t longitude;
    uint32_t latitude;
    uint8_t gnssType;
    uint64_t timestamp;
    char eventId[16];
};
#pragma pack()
///////////////////////////////////////////
#pragma pack(1)
struct EventExts
{
    uint16_t extsLen;
};
#pragma pack()

//    char exts[extsLen];
///////////////////////////////////////////
#pragma pack(1)
struct EventDataTail
{
    uint8_t targetIdsLen;
};
#pragma pack()
#pragma pack(1)
struct EventDataTargetIds
{
    char objId[16];
};
#pragma pack()

//uint8_t targetIds[16 * targetIdsLen];
///////////////////////////////////////////
#pragma pack(1)
struct EventResponeData
{
    char eventId[16];
};
#pragma pack()
NAMESPACE_PROTOCOL_THREAD_END
#endif

