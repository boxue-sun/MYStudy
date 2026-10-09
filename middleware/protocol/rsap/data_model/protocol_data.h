/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA_PROTOCOL_DATA
#define AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA_PROTOCOL_DATA
#include "namespace.h"
#define UTC8_SECODND_DIFF_VALUE 28800
#define UTC8_MILLSECODND_DIFF_VALUE UTC8_SECODND_DIFF_VALUE * 1000
NAMESPACE_PROTOCOL_THREAD_START
//数据包结构
#pragma pack(1)
struct RsapMsgHeader
{
    uint8_t StartFlag;
    uint8_t DataLen[3];
    uint8_t DataType;
    uint8_t Version;
    uint16_t TimestampMS;
    uint32_t TimestampMIN;
//    uint8_t Data[0];
};
#pragma pack()
//数据类别定义
enum RsapDataType
{
    RCU2CLOUD_OBJS_DATA_TYPE = 121,
    RCU2CLOUD_EVENT_DATA_TYPE = 123,
    CLOUD2RCU_EVENT_RES_DATA_TYPE = 124,
    RCU2CLOUD_STATUS_DATA_TYPE = 129,
    CLOUD2RCU_STATUS_RES_DATA_TYPE = 130,
    RCU2CLOUD_HEARTBEAT_DATA_TYPE = 141,
    CLOUD2_HEARTBEAT_RES_DATA_TYPE = 142,
} ;
enum RsapVersion
{
    RCU2CLOUD_OBJS_VERSION = 0x0C,
    RCU2CLOUD_EVENT_VERSION = 0x04,
    CLOUD2RCU_EVENT_RES_VERSION = 0x03,
    RCU2CLOUD_STATUS_VERSION = 0x03,
    CLOUD2RCU_STATUS_RES_VERSION = 0x03,
    RCU2CLOUD_HEARTBEAT_VERSION = 0x01,
    CLOUD2_HEARTBEAT_RES_VERSION = 0x01,
} ;
enum RsapStartFlag
{
    RSAP_PACKAGE_START_FLAG = 0xF2,
} ;

enum DeviceStatusCode
{
    DEVICE_STATUS_ON = 0,
    DEVICE_STATUS_OFF = 1
};
enum ObjType {
    Person = 0,
    Bicycle = 1,
    Car = 2,
    Motorbike = 3,
    Aeroplane = 4,
    Bus = 5,
    Train = 6,
    Truck = 7,
    Boat = 8,
    TrafficLight = 9,
    FireHydrant = 10,
    StopSign = 11,
    ParkingMeter = 12,
    Bench = 13,
    Bird = 14,
    Cat = 15,
    Dog = 16,
    Horse = 17,
    Sheep = 18,
    Cow = 19,
    NonMotorVehicle = 20,
    Backpack = 24,
    Umbrella = 25,
    Handbag = 26,
    Suitcase = 28,
    Frisbee = 29,
    Skis = 30,
    Snowboard = 31,
    SportsBall = 32,
    Kite = 33,
    Skateboard = 36,
    Surfboard = 37,
    Bottle = 38,
    WineGlass = 40,
    Chair = 56,
    Sofa = 57,
    PottedPlant = 58,
    Bed = 59,
    Other = 254,
    Unknown = 255
};

NAMESPACE_PROTOCOL_THREAD_END
#endif

