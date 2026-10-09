/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA_OBJS_DATA
#define AIROS_MIDDLEWARE_PROTOCOL_RSAP_COMPONENT_DATAMODEL_EVENT_DATA_OBJS_DATA
#include "namespace.h"
NAMESPACE_PROTOCOL_THREAD_START
#pragma pack(1)
struct ObjsDataHead
{
    uint8_t     channelId;
    uint8_t    rcuId[8];
    uint8_t     deviceType;
    uint8_t     deviceId[11];
    uint64_t    timestampOfDevOut;
    uint64_t    timestampOfDetIn;
    uint64_t    timestampOfDetOut;
    uint8_t     gnssType;
    uint16_t    targetsNum;
};
#pragma pack()

#pragma pack(1)
struct ObjHead
{
    uint8_t     uuid[16];
    uint16_t    objId;
    uint8_t     type;
    uint8_t     status;
    uint16_t    len;
    uint16_t    width;
    uint16_t    height;
    uint32_t    longitude;
    uint32_t    latitude;
    uint32_t    locEast;
    uint32_t    locNorth;
    uint8_t     posConfidence;
    uint32_t    elevation;
    uint8_t     elevConfidence;
    uint16_t    speed;
    uint8_t     speedConfidence;
    uint16_t    speedEast;
    uint8_t     speedEastConfidence;
    uint16_t    speedNorth;
    uint8_t     speedNorthConfidence;
    uint32_t    heading;
    uint8_t     headConfidence;
    uint16_t    accelVert;
    uint8_t     accelVertConfidence;
    uint32_t    trackedTimes;
};
#pragma pack()


////////////////////////////////////////////////////
#pragma pack(1)
struct ObjHistLoc
{
    uint16_t    histLocNum;
};
#pragma pack()


//histLocs:目标历史轨迹列表
////////////////////////////////////////////////////
#pragma pack(1)
struct ObjPredLoc
{
    // 目标历史轨迹列表
    uint16_t    predLocNum;
};
#pragma pack()

//predLocs :目标预测轨迹列表
////////////////////////////////////////////////////
#pragma pack(1)
struct ObjLaneId
{
    uint16_t    laneId;
    uint8_t     filterInfoType = 0;
};
#pragma pack()

////////////////////////////////////////////////////
#pragma pack(1)
struct ObjPlateNum
{
    uint8_t     lenplateNum;
};
#pragma pack()
//plateNo : 车牌号
////////////////////////////////////////////////////

#pragma pack(1)
struct ObjsTail
{
    uint8_t     plateType;
    uint8_t     plateColor;
    uint8_t     objColor;
    uint64_t    funtionTimestamp;
	uint8_t		deviceNum;
};
#pragma pack()

////////////////////////////////////////////////////
//轨迹点
#pragma pack(1)
struct VehiclePositionInfo
{
    uint64_t 	longitude;  // 经度, 【0..1800000001】, 1800000001 表示无效, 单位 10e-7 deg
    uint64_t 	latitude;   // 纬度, 【0.. 900000001】, 900000001 表示无效, 单位 10e-7 deg
    uint8_t 	posConfidence;  // 位置精度等级, 【0..255】枚举, 定义见附录 6.3.1
    uint16_t 	speed;       // 速度, 【0..65535】, 65535 表示无效, 单位：0.01mps
    uint8_t 	speedConfidence; // 速度精度等级, 【0..255】枚举, 定义见附录 6.3.2
    uint64_t 	heading;    // 航向角, 【0..3600001】, 360001 表示无效, 车头指向方向与正北方向顺时针夹角, 单位：10e-4 deg
    uint8_t 	headConfidence; // 航向精度等级, 【0..255】枚举, 定义见附录 6.3.3
};
#pragma pack()
////////////////////////////////////////////////////
#pragma pack(1)
struct FilteredInfoDimension
{
    uint16_t dimension;
};
#pragma pack()
////////////////////////////////////////////////////
#pragma pack(1)
struct ObjDeviceNum
{
    uint8_t     deviceNum;
};
#pragma pack()
//plateNo : 车牌号
////////////////////////////////////////////////////

enum ObjDeviceTypeEnum
{
    OBJ_DEVICE_TYPE_UNKNOWN = 0,
    OBJ_DEVICE_TYPE_FUSIONRESULT = 1,
    OBJ_DEVICE_TYPE_CAMERA = 2,
    OBJ_DEVICE_TYPE_MILLIMETERWAVERADAR = 3,
    OBJ_DEVICE_TYPE_LASERRADAR = 4
};


enum ObjTypeEnum {
    OBJ_TYPE_PERSON,                 // 0 行人
    OBJ_TYPE_BICYCLE,                // 1 自行车
    OBJ_TYPE_CAR,                   // 2 汽车
    OBJ_TYPE_MOTORBIKE,             // 3 摩托车
    OBJ_TYPE_AEROPLANE,             // 4 飞机
    OBJ_TYPE_BUS,                   // 5 公交车
    OBJ_TYPE_TRAIN,                 // 6 火车
    OBJ_TYPE_TRUCK,                 // 7 卡车
    OBJ_TYPE_BOAT,                  // 8 船
    OBJ_TYPE_TRAFFIC_LIGHT,          // 9 交通灯
    OBJ_TYPE_FIRE_HYDRANT,           // 10 消防栓
    OBJ_TYPE_STOP_SIGN,              // 11 停车标志
    OBJ_TYPE_PARKING_METER,          // 12 停车咪表
    OBJ_TYPE_BENCH,                 // 13 长椅
    OBJ_TYPE_BIRD,                  // 14 鸟
    OBJ_TYPE_CAT,                   // 15 猫
    OBJ_TYPE_DOG,                   // 16 狗
    OBJ_TYPE_HORSE,                 // 17 马
    OBJ_TYPE_SHEEP,                 // 18 绵羊
    OBJ_TYPE_COW,                   // 19 牛
    OBJ_TYPE_NON_MOTOR_VEHICLE ,       // 20 非机动车
    OBJ_TYPE_BACKPACK = 24,              // 24 背包
    OBJ_TYPE_UMBRELLA,              // 25 雨伞
    OBJ_TYPE_HANDBAG,               // 26 手提包
    OBJ_TYPE_SUITCASE = 27,              // 28 行李箱
    OBJ_TYPE_FRISBEE,               // 29 飞盘
    OBJ_TYPE_SKIS,                  // 30 滑雪板
    OBJ_TYPE_SNOWBOARD,             // 31 单板滑雪
    OBJ_TYPE_SPORTS_BALL,            // 32 运动球
    OBJ_TYPE_KITE,                  // 33 风筝
    OBJ_TYPE_SKATEBOARD = 36,            // 36 滑板
    OBJ_TYPE_SURFBOARD,             // 37 冲浪板
    OBJ_TYPE_BOTTLE,                // 38 瓶子
    OBJ_TYPE_WINE_GLASS = 40,             // 40 酒杯
    OBJ_TYPE_CHAIR = 56,                 // 56 椅子
    OBJ_TYPE_SOFA = 57,                  // 57 沙发
    OBJ_TYPE_POTTEDPLANT = 58,           // 58 盆栽
    OBJ_TYPE_BED = 59,                   // 59 床

    // 警车
    OBJ_TYPE_PoliceCar = 253,
    // 警用摩托车
    OBJ_TYPE_PoliceMotorcycle = 252,
    // 消防车
    OBJ_TYPE_FireEngine = 251,
    // 救护车
    OBJ_TYPE_Ambulance = 250,
    // 工程作业车
    OBJ_TYPE_EngineeringVehicle = 249,
    // 校车
    OBJ_TYPE_SchoolBus = 248,
    // 洒水车
    OBJ_TYPE_SprinklerTruck = 247,
    // 渣土车
    OBJ_TYPE_DirtTruck = 246,
    // 混凝土搅拌车
    OBJ_TYPE_ConcreteMixer = 245,
    // 环卫车
    OBJ_TYPE_SanitationVehicle = 244,
    // 货车
    OBJ_TYPE_Truck = 243,
    // 危化品车
    OBJ_TYPE_HazardousChemicalVehicle = 242,
    // 吊车
    OBJ_TYPE_Crane = 241,
    // 皮卡车
    OBJ_TYPE_PickupTruck = 240,
    // 三角警示牌
    OBJ_TYPE_TriangleWarningSign = 238,
    // 水马
    OBJ_TYPE_WaterBarrier = 237,
    // 交通桩（金属柱、水泥柱）
    OBJ_TYPE_TrafficPole = 236,
    // 交通锥
    OBJ_TYPE_TrafficCone = 235,
    OBJ_TYPE_OTHER = 254,                 // 254 其他
    OBJ_TYPE_UNKNOWN,               // 255 未知
};


//车牌
enum PlateType{
    VEHICLE_TYPE_LargeCar = 1, // 大型汽车
    VEHICLE_TYPE_SmallCar = 2, // 小型汽车
    VEHICLE_TYPE_EmbassyCar = 3, // 使馆汽车
    VEHICLE_TYPE_ConsulateCar = 4, // 领馆汽车
    VEHICLE_TYPE_ForeignCar = 5, // 境外汽车
    VEHICLE_TYPE_ExpatriateCar = 6, // 外籍汽车
    VEHICLE_TYPE_Motorcycle = 7, // 两、三轮摩托车号牌
    VEHICLE_TYPE_Scooter = 8, // 轻便摩托车
    VEHICLE_TYPE_EmbassyMotorcycle = 9, // 使馆摩托车
    VEHICLE_TYPE_ConsulateMotorcycle = 10, // 领馆摩托车
    VEHICLE_TYPE_ForeignMotorcycle = 11, // 境外摩托车
    VEHICLE_TYPE_ExpatriateMotorcycle = 12, // 外籍摩托车
    VEHICLE_TYPE_AgriculturalVehicle = 13, // 农用运输车
    VEHICLE_TYPE_Tractor = 14, // 拖拉机
    VEHICLE_TYPE_Trailer = 15, // 挂车
    VEHICLE_TYPE_DrivingSchoolCar = 16, // 教练汽车
    VEHICLE_TYPE_DrivingSchoolMotorcycle = 17, // 教练摩托车
    VEHICLE_TYPE_TestCar = 18, // 试验汽车
    VEHICLE_TYPE_TestMotorcycle = 19, // 试验摩托车
    VEHICLE_TYPE_TemporaryImportCar = 20, // 临时入境汽车
    VEHICLE_TYPE_TemporaryImportMotorcycle = 21, // 临时入境摩托车
    VEHICLE_TYPE_TemporaryLicenseCar = 22, // 临时行驶车
    VEHICLE_TYPE_PoliceCar = 23, // 警用汽车
    VEHICLE_TYPE_PoliceMotorcycle = 24, // 警用摩托
    VEHICLE_TYPE_OldAgriculturalVehicle = 25, // 原农机号牌
    VEHICLE_TYPE_HongKongEntryExitCar = 26, // 香港入出境车
    VEHICLE_TYPE_MacauEntryExitCar = 27, // 澳门入出境车
    VEHICLE_TYPE_MediumCar = 28, // 中型车
    VEHICLE_TYPE_ArmedPolice = 31, // 武警号牌
    VEHICLE_TYPE_Military = 32, // 军队号牌
    VEHICLE_TYPE_Pedestrian = 33, // 行人
    VEHICLE_TYPE_NonMotorVehicle = 34, // 非机动车
    VEHICLE_TYPE_LargeNewEnergyCar = 51, // 大型新能源车牌
    VEHICLE_TYPE_SmallNewEnergyCar = 52, // 小型新能源车牌
    VEHICLE_TYPE_Normal = 53, // "Normal" 蓝牌黑牌
    VEHICLE_TYPE_Yellow = 54, // "Yellow" 黄牌
    VEHICLE_TYPE_DoubleYellow = 55, // "DoubleYellow" 双层黄尾牌
    VEHICLE_TYPE_DoubleMilitary = 56, // "DoubleMilitary" 部队双层
    VEHICLE_TYPE_SAR = 57, // "SAR" 港澳特区号牌
    VEHICLE_TYPE_Personal = 58, // "Personal" 个性号牌
    VEHICLE_TYPE_Agri = 59, // "Agri" 农用牌
    VEHICLE_TYPE_Moto = 60, // "Moto" 摩托车号牌
    VEHICLE_TYPE_OfficialCar = 61, // "OfficialCar " 公务车
    VEHICLE_TYPE_PersonalCar = 62, // "PersonalCar" 私家车
    VEHICLE_TYPE_WarCar = 63, // "WarCar" 军用
    VEHICLE_TYPE_Other = 64, // "Other" 其他号牌
    VEHICLE_TYPE_CivilAviation = 65, // "Civilaviation" 民航号牌
    VEHICLE_TYPE_Black = 66, // "Black" 黑牌
    VEHICLE_TYPE_PureNewEnergyMicroCar = 67, // "PureNewEnergyMicroCar"
    // 纯电动新能源小车
    VEHICLE_TYPE_MixedNewEnergyMicroCar = 68, // "MixedNewEnergyMicroCar"
    // 混合新能源小车
    VEHICLE_TYPE_PureNewEnergyLargeCar = 69, // "PureNewEnergyLargeCar"
    // 纯电动新能源大车
    VEHICLE_TYPE_MixedNewEnergyLargeCar = 70, // "MixedNewEnergyLargeCar"
    // 混合新能源大车
    VEHICLE_TYPE_Other2 = 99, // 其它
};

//车牌颜色
enum PlateColor {
    PLATE_COLOR_YELLOW = 1, // 黄色
    PLATE_COLOR_BLUE = 2, // 蓝色
    PLATE_COLOR_BLACK = 3, // 黑色
    PLATE_COLOR_OTHER = 4, // 其他
    PLATE_COLOR_GREEN_AGRICULTURAL = 5, // 绿色，农用车
    PLATE_COLOR_RED = 6, // 红色
    PLATE_COLOR_YELLOW_GREEN = 7, // 黄绿双色
    PLATE_COLOR_GRADIENT_GREEN = 8, // 渐变绿色
    PLATE_COLOR_YELLOWBOTTOM_BLACKTEXT = 9, // 黄底黑字
    PLATE_COLOR_BLUEBOTTOM_WHITETEXT = 10, // 蓝底白字
    PLATE_COLOR_BLACKBOTTOM_WHITETEXT = 11 // 黑底白字
};
//车身颜色
enum VehicleColor {
    VEHICLE_COLOR_WHITE = 1,          // 白色
    VEHICLE_COLOR_LIGHTWHITE = 2,      // 浅白
    VEHICLE_COLOR_DEEPWHITE = 3,       // 深白
    VEHICLE_COLOR_GRAY = 4,           // 灰色
    VEHICLE_COLOR_LIGHTGRAY = 5,      // 浅灰
    VEHICLE_COLOR_DEEPGRAY = 6,       // 深灰
    VEHICLE_COLOR_YELLOW = 7,         // 黄色
    VEHICLE_COLOR_LIGHTYELLOW = 8,    // 浅黄
    VEHICLE_COLOR_DEEPYELLOW = 9,     // 深黄
    VEHICLE_COLOR_PINK = 10,          // 粉色
    VEHICLE_COLOR_LIGHTPINK = 11,     // 浅粉
    VEHICLE_COLOR_DEEPPINK = 12,      // 深粉
    VEHICLE_COLOR_RED = 13,           // 红色
    VEHICLE_COLOR_LIGHTRED = 14,      // 浅红
    VEHICLE_COLOR_DEEPRED = 15,       // 深红
    VEHICLE_COLOR_PURPLE = 16,        // 紫色
    VEHICLE_COLOR_LIGHTPURPLE = 17,   // 浅紫
    VEHICLE_COLOR_DEEPPURPLE = 18,    // 深紫
    VEHICLE_COLOR_GREEN = 19,         // 绿色
    VEHICLE_COLOR_LIGHTGREEN = 20,    // 浅绿
    VEHICLE_COLOR_DEEPGREEN = 21,     // 深绿
    VEHICLE_COLOR_BLUE = 22,          // 蓝色
    VEHICLE_COLOR_LIGHTBLUE = 23,     // 浅蓝
    VEHICLE_COLOR_DEEPBLUE = 24,      // 深蓝
    VEHICLE_COLOR_BROWN = 25,         // 棕色
    VEHICLE_COLOR_LIGHTBROWN = 26,    // 浅棕
    VEHICLE_COLOR_DEEPBROWN = 27,     // 深棕
    VEHICLE_COLOR_BLACK = 28,         // 黑色
    VEHICLE_COLOR_LIGHTBLACK = 29,    // 浅黑
    VEHICLE_COLOR_DEEPBLACK = 30,     // 深黑
    VEHICLE_COLOR_ORANGE = 31,        // 橙色
    VEHICLE_COLOR_LIGHTORANGE = 32,   // 浅橙
    VEHICLE_COLOR_DEEPORANGE = 33,    // 深橙
    VEHICLE_COLOR_CYAN = 34,          // 青色
    VEHICLE_COLOR_LIGHTCYAN = 35,     // 浅青
    VEHICLE_COLOR_DEEPCYAN = 36,      // 深青
    VEHICLE_COLOR_SILVER = 37,        // 银色
    VEHICLE_COLOR_LIGHTSILVER = 38,   // 浅银
    VEHICLE_COLOR_DEEPSILVER = 39,    // 深银
    VEHICLE_COLOR_SILVERWHITE = 40,   // 银白色
    VEHICLE_COLOR_LIGHTSILVERWHITE = 41, // 浅银白
    VEHICLE_COLOR_DEEPSILVERWHITE = 42, // 深银白
    VEHICLE_COLOR_OTHER = 43,         // 其他
    VEHICLE_COLOR_LIGHTOTHER = 44,    // 浅其他
    VEHICLE_COLOR_DEEPOTHER = 45,     // 深其他
    VEHICLE_COLOR_NO_VALID = 0xFF,     // 无效
};
//渠道来源
enum ChannelSource {
    // 云控基础平台
    CHANNEL_SOURCE_CLOUD_CONTROL_PLATFORM = 1,
    // 百度边缘计算节点
    CHANNEL_SOURCE_BAIDU_EDGE_COMPUTING_NODE = 11,
    // 大唐路侧感知设备
    CHANNEL_SOURCE_DATANG_ROADSIDE_PERCEPTION_DEVICE = 12,
    // 千方路侧感知设备
    CHANNEL_SOURCE_QIANFANG_ROADSIDE_PERCEPTION_DEVICE = 13,
    // 星云互联路侧感知设备
    CHANNEL_SOURCE_XINGYUN_INTERCONNECTION_ROADSIDE_PERCEPTION_DEVICE = 14,
    // 新岸线路侧感知设备
    CHANNEL_SOURCE_XIN_ANLINE_ROADSIDE_PERCEPTION_DEVICE = 15,
    // 预留
    CHANNEL_SOURCE_RESERVED = 16,
};
enum CoordinateType
{
    COORDINATE_TYPE_GCJ02 = 0,
    COORDINATE_TYPE_CUSTOM = 1,
    COORDINATE_TYPE_WGS84 = 2,
    COORDINATE_TYPE_RESERVED = 3,
} ;

// 车牌颜色类型
enum OBJ_Plate_Color_Type
{
    LICENSE_PLATE_COLOR_TYPE_YELLOW = 1,                // 黄色
    LICENSE_PLATE_COLOR_TYPE_BLUE = 2,                 // 蓝色
    LICENSE_PLATE_COLOR_TYPE_BLACK = 3,                // 黑色
    LICENSE_PLATE_COLOR_TYPE_OTHER = 4,                // 其他
    LICENSE_PLATE_COLOR_TYPE_GREEN_AGRICULTURAL = 5,   // 绿色，农用车
    LICENSE_PLATE_COLOR_TYPE_RED = 6,                  // 红色
    LICENSE_PLATE_COLOR_TYPE_YELLOW_GREEN = 7,          // 黄绿双色
    LICENSE_PLATE_COLOR_TYPE_GRADIENT_GREEN = 8,       // 渐变绿色
    LICENSE_PLATE_COLOR_TYPE_YELLOWBOTTOM_BLACKTEXT = 9, // 黄底黑字
    LICENSE_PLATE_COLOR_TYPE_BLUEBOTTOM_WHITEEXT = 10,  // 蓝底白字
    LICENSE_PLATE_COLOR_TYPE_BLACKBOTTOM_WHITEEXT = 11, // 黑底白字
    LICENSE_PLATE_COLOR_TYPE_NO_VALID = 0xFF, // 黑底白字
};
NAMESPACE_PROTOCOL_THREAD_END

#endif