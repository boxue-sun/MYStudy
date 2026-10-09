/*********************************************************************************
 * @file		alarm_management_data.h
 * @brief		alarm_management_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-19
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  	24-4-19  alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef MQTT_CLIENT_MEC_OM_DATA_COMMON_H
#define MQTT_CLIENT_MEC_OM_DATA_COMMON_H
#include "middleware/protocol/om_common/namespace.h"
#include <libssh/libssh.h>
NAMESPACE_START_OM_COMPONENT_COMMON
#define SUCESS_COLOR_STR "\033[1;32m"
#define SUCESS_COLOR_END "\033[0m"

#define PROTOCOL_VERSION "V1.0"
#define STR_DEFAULT_VALUE "Not reported"
#define NUM_DEFAULT_VALUE  -9999
//运维中运行状态
enum RunStatusEnum
{
    RUN_STATUS_NORMAL = 0, // 正常
    RUN_STATUS_FAULT = 1 // 故障
};

enum NetworkStatusEnum
{
    NETWORK_STATUS_ONLINE = 0, // 在线
    NETWORK_STATUS_OFFLINE = 1 // 离线
};
enum OmDeviceTypeEnum
{
    OM_DEVICE_TYPE_RSCU = 0,
    OM_DEVICE_TYPE_RSU = 1,
    OM_DEVICE_TYPE_CAMERA = 2,
    OM_DEVICE_TYPE_MMWRADAR = 3,
    OM_DEVICE_TYPE_LIDAR = 4,
    OM_DEVICE_TYPE_SIGNALCONTROLLER = 5,
    OM_DEVICE_TYPE_OTHERDEVICE = 6
};
enum DeviceBasicInfoDeviceTypeEnum
{
    DEVICE_BASIC_INFO_MEC = 0,  //mec设备
    DEVICE_BASIC_INFO_SENSOR = 1,  //mec设备
};


enum DeviceManagementInfoIdTypeEnum
{
    DEVICE_BASIC_INFO = 0,  //mec设备基础信息
    DEVICE_RUNNING_STATUS = 1, //mec运维状态信息
    CONNECTED_DEVICE_INFO = 2, //接入mec的设备信息
    CONNECTED_DEVICE_RUNNING_STATUS = 3, //接入mec的设备运行状态信息
    CLOUD_CONTROL_CONFIG_INFO = 4,  //mec云控配置参数信息
    PERFORMANCE_INFO = 5,   //mec性能信息
    ALARM_INFO = 6,         //mec告警信息
    DEVICE_VERSION_INFO = 7,  //设备版本信息查询
    DEVICE_POS_INFO = 8  //MEC设备位置信息
};

enum PowerOperationTypeEnum
{
    POWER_OPERATION_TYPE_ON = 0,
    POWER_OPERATION_TYPE_OFF = 1,
    POWER_OPERATION_TYPE_RESTART = 2
};

enum SiteTypeEnum
{
    SITE_TYPE_GROUND_CHASSIS = 0, // 地面机箱
    SITE_TYPE_MOUNTED_ON_POLE = 1 // 安装于杆体
};

enum LogLevelEnum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_NO_LOG = 4
};
enum PowerEnum {
    POWER_ON = 0,
    POWER_OFF = 1,
    POWER_RESTART = 2
};

enum StatusEnum {
    STATUS_FAILURE = 0,
    STATUS_SUCCESS = 1,
};
//告警等级枚举
enum AlertLevelEnum
{
    ALERT_CRITICAL = 0,
    ALERT_MAJOR = 1,
    ALERT_MINOR = 2,
    ALERT_WARNING = 3
};
//告警状态枚举
enum AlarmStatusEnum
{
    ALARM_DISAPPEARED = 0,
    ALARM_OCCURRED
};
enum MecAlarmTypeErrorCodeEnum {
    MEC_ALARM_TYPE_HIGH_TEMPER_CPU = 0,         // CPU温度过高
    MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU,          // CPU占用率过高
    MEC_ALARM_TYPE_HIGH_TEMPER_GPU,             // GPU温度过高
    MEC_ALARM_TYPE_HIGH_OCCUPANCY_GPU,          // GPU占用率过高
    MEC_ALARM_TYPE_OUT_OF_MEMORY,               // 内存不足
    MEC_ALARM_TYPE_BAD_FIRMWARE,               // 固件损坏
    MEC_ALARM_TYPE_STORAGE_BAD_BLOCK,           // 存储坏块
    MEC_ALARM_TYPE_OUT_OF_DISK_SPACE,            // 磁盘空间不足
    MEC_ALARM_TYPE_IRE_OFF_LINE,                // 8 感知设备离线
    MEC_ALARM_TYPE_THREAD_LOCK,                // 线程卡死
    MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET,         // 10 摄像机角度偏移
    MEC_ALARM_TYPE_ERROR_TIMING,               // 11 未搜索到授时服务器
    MEC_ALARM_TYPE_ERROR_VOLTAGE,              // 电压故障
    MEC_ALARM_TYPE_OTHER,                      // 其他

    // 1. 数据异常类，
    MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS = 101,      // 信号机原始数据异常  1

    MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS,      // 感知数据异常 2
    MEC_ALARM_TYPE_MONITOR_SENSOR_EVENT_DATA_LOSS,      // 感知事件异常 3

    MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS,          // V2X信控指标异常 4
    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,       // 雷达信控指标-动态异常 5
    MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,      // 雷达信控指标-静态异常 6
    MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS,       // 毫米波雷达原始数据异常 7
    MEC_ALARM_TYPE_MONITOR_LIDAR_RAW_DATA_LOSS,        //激光雷达原始数据 8
    // 2. 链路故障类
    MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR,          // 云控链路异常 9
    MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR,        // 信控链路-静态异常 10
    MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR,         // 信控链路-动态异常 11
    MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR,    // 点云链路异常 12
};


struct AlarmPairData
{
    uint64_t timeStamp = 0;
    uint64_t alarmErrorOccuredTimeStamp = 0;
    bool alarmErrorOccuredFlag = false;   // 告警是否产生表示
    bool alarmErrorOccuredPublishedOnceFlag = false;   // 告警是否已经推送过一次
    bool alarmErrorDisppearedPublishedOnceFlag = false;   // 告警是否已经推送过一次
    std::string alarmErrorAddition = "";
};
struct AlarmTypeFlag
{
    AlarmPairData alarmHighTemperCPUFlag;      // CPU温度过高
    AlarmPairData alarmHighOccupancyCPUFlag;    // CPU占用率过高
    AlarmPairData alarmHighTemperGPUFlag;       // GPU温度过高
    AlarmPairData alarmHighOccupancyGPUFlag;   // GPU占用率过高
    AlarmPairData alarmOutOfMemoryFlag;       // 内存不足
    AlarmPairData alarmBadFirmwareFlag;        // 固件损坏
    AlarmPairData alarmStorageBadBlockFlag;    // 存储坏块
    AlarmPairData alarmOutOfDiskSpaceFlag ;     // 磁盘空间不足
    AlarmPairData alarmIreOffLineFlag;  //设备离线
    AlarmPairData alarmThreadLockFlag;         // 线程卡死
    AlarmPairData alarmCameraAngleOffsetFlag; //摄像机角度偏移
    AlarmPairData alarmMecErrorTimingFlag;  //mec设备授时
    AlarmPairData alarmErrorTimingFlag; // 未搜索到授时服务器
    AlarmPairData alarmErrorVoltageFlag;        // 电压故障
    AlarmPairData alarmOtherFlag;                // 其他
};
//////////////////////////////////////////
//插入操作类型
enum TABLE_OPERA_TYPE
{
    TABLE_OPERA_TYPE_INSERT,  //插入操作
    TABLE_OPERA_TYPE_UPDATE  //更新
};
//表类型
enum TABLE_TYPE
{
    TABLE_TYEP_SENSOR_DEV_STATUS,  //感知设备状态表
    TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE,  //感知设备设备授时表
    TABLE_TYEP_MEC_DEV_STATUS,  //MEC状态表
    TABLE_TYEP_MEC_DEV_ALARM,  //MEC告警表
    TABLE_TYEP_RADAR_DEV_ALARM,  //设备告警表
    TABLE_TYEP_CAMERA_DEV_ALARM  //感知设备告警表
};
enum DEVICE_TYPE_DB
{
    DEVICE_TYPE_DB_MEC = 1,
    DEVICE_TYPE_DB_CAMRA = 2,
    DEVICE_TYPE_DB_MW_RADAR = 3,
    DEVICE_TYPE_DB_RADAR = 4

};
//协议1使用
enum DEVICE_STATUS_DB
{
    DEVICE_STATUS_DB_ON,  //
    DEVICE_STATUS_DB_OFF
};
//联网状态
enum DEVICE_ACTIVE_DB
{
    DEVICE_ACTIVE_DB_ON,  //
    DEVICE_ACTIVE_DB_OFF
};

enum TABLE_UPDATE_MEC_STAUTS_OP_TYPE
{
    TABLE_UPDATE_MEC_STAUTS_CPU,  //cpu
    TABLE_UPDATE_MEC_STAUTS_MEM,  //mem
    TABLE_UPDATE_MEC_STAUTS_DISK,  //磁盘信息
    TABLE_UPDATE_MEC_STAUTS_NET  //网络信息
};

enum BASIC_INFO_OPT_TYPE
{
    BASIC_INFO_OPT_TYPE_QUERY, //查询
    BASIC_INFO_OPT_TYPE_TIMER //定时
    
};

enum CONFIG_QUERY_INFO_OPT_TYPE
{
    CONFIG_QUERY_INFO_OPT_TYPE_QUERY_ACK, //查询
    CONFIG_QUERY_INFO_OPT_TYPE_DEVICE_POS //定时

};
struct MsgDeviceStatus
{
    //时间戳
    uint64_t timeStamp = 0;
    bool     timeStampNeedUpdate = false;
    //设备类型
    DEVICE_TYPE_DB deviceType;
    bool     deviceTypeNeedUpdate = false;
    //设备id
    string   deviceID = "";
    bool     deviceIDNeedUpdate = false;
    
    //设备状态：协议1使用，协议5使用
    DEVICE_STATUS_DB deviceStatus;
    bool     deviceStatusNeedUpdate = false;

    //软件版本
    string  softwareVersion = "";
    bool    softwareVersionNeedUpdate = false;

    //////////////////////////////
    //授时信息
    uint64_t devTime = 0;
    bool     devTimeNeedUpdate = false;

    uint64_t lastTime = 0;
    bool     lastTimeNeedUpdate = false;
    int64_t timeNode = 0;                     // TIME_NODE
    bool    timeNodeNeedUpdate = false;          // TIME_NODE 是否需要更新
    int64_t timeDiff = 0;
    bool     timeDiffNeedUpdate = false;

    //点位： mec使用
    std::string pointNo = "";                           // POINT_NO
    bool pointNoNeedUpdate = false;            // POINT_NO 是否需要更新

    std::string pointName = "";                 // POINT_NAME
    bool pointNameNeedUpdate = false;          // POINT_NAME 是否需要更新
    //经纬度
    double longitude = 0;
    bool   longitudeNeedUpdate = false;

    double latitude = 0;
    bool   latitudeNeedUpdate = false;

    double altitude = 0;
    bool   altitudeNeedUpdate = false;
    //////////////////////////////
    std::string mecEsn;
    bool   mecEsnNeedUpdate = false;

    DEVICE_ACTIVE_DB active;
    bool   activeNeedUpdate = false;

    std::string deviceIp;
    bool   deviceIpNeedUpdate = false;

    std::string netMask = "";
    bool netMaskNeedUpdate = false;

    std::string gateway = "";
    bool gatewayNeedUpdate = false;

    std::string cpuLoad ="";
    bool cpuLoadNeedUpdate = false;

    float cpuTemp = 0;
    bool cpuTempNeedUpdate = false;

    std::string cpuUti = "";
    bool cpuUtiNeedUpdate = false;

    float gpuLoad = 0;
    bool gpuLoadNeedUpdate = false;

    float gpuSmem = 0;
    bool gpuSmemNeedUpdate = false;

    float gpuTemp = 0;
    bool gpuTempNeedUpdate = false;

    std::string gpuUti = "";
    bool gpuUtiNeedUpdate = false;

    float memTotal = 0;
    bool memTotalNeedUpdate = false;

    float memUsed = 0;
    bool memUsedNeedUpdate = false;

    float memFree = 0;
    bool memFreeNeedUpdate = false;

    float diskTotal = 0;
    bool diskTotalNeedUpdate = false;

    float diskUsed = 0;
    bool diskUsedNeedUpdate = false;

    float diskFree = 0;
    bool diskFreeNeedUpdate = false;

    float diskTps = 0;
    bool diskTpsNeedUpdate = false;

    float diskWrite = 0;
    bool diskWriteNeedUpdate = false;

    float diskRead = 0;
    bool diskReadNeedUpdate = false;
    

    uint64_t netRx = 0;
    bool netRxNeedUpdate = false;

    uint64_t netTx = 0;
    bool netTxNeedUpdate = false;

    float netRxByte = 0;
    bool netRxByteNeedUpdate = false;
    
    float netTxByte = 0;
    bool netTxByteNeedUpdate = false;
    

    AlertLevelEnum      alarmLevel;
    bool   alarmLevelNeedUpdate = false;

    AlarmStatusEnum     alarmStatus;
    bool   alarmStatusNeedUpdate = false;

    long   alarmRaisedTime;
    bool   alarmRaisedTimeNeedUpdate = false;

    long   alarmChangedTime;
    bool   alarmChangedTimeNeedUpdate = false;

    int                 alarmType;
    bool                alarmTypeNeedUpdate = false;
    std::string         addition;
    bool                additionNeedUpdate = false;

    string               deviceNoAlarmType;
    bool                 deviceNoAlarmTypeNeedUpdate = false;

    public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "TimeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "DeviceType: " << deviceType << std::endl; // Assuming deviceType has a to_string() method
        ss << std::left << std::setw(40) << "DeviceID: " << deviceID << std::endl;
        ss << std::left << std::setw(40) << "DeviceStatus: " << deviceStatus << std::endl; // Assuming deviceStatus has a to_string() method
        ss << std::left << std::setw(40) << "SoftwareVersion: " << softwareVersion << std::endl;
        ss << std::left << std::setw(40) << "DevTime: " << devTime << std::endl;
        ss << std::left << std::setw(40) << "LastTime: " << lastTime << std::endl;
        ss << std::left << std::setw(40) << "TimeNode: " << timeNode << std::endl;
        ss << std::left << std::setw(40) << "TimeDiff: " << timeDiff << std::endl;
        ss << std::left << std::setw(40) << "PointNo: " << pointNo << std::endl;
        ss << std::left << std::setw(40) << "PointName: " << pointName << std::endl;
        ss << std::left << std::setw(40) << "Longitude: " << longitude << std::endl;
        ss << std::left << std::setw(40) << "Latitude: " << latitude << std::endl;
        ss << std::left << std::setw(40) << "Altitude: " << altitude << std::endl;
        ss << std::left << std::setw(40) << "MecEsn: " << mecEsn << std::endl;
        ss << std::left << std::setw(40) << "Active: " << active << std::endl; // Assuming active has a to_string() method
        ss << std::left << std::setw(40) << "DeviceIp: " << deviceIp << std::endl;
        ss << std::left << std::setw(40) << "NetMask: " << netMask << std::endl;
        ss << std::left << std::setw(40) << "Gateway: " << gateway << std::endl;
        ss << std::left << std::setw(40) << "CpuLoad: " << cpuLoad << std::endl;
        ss << std::left << std::setw(40) << "CpuTemp: " << cpuTemp << std::endl;
        ss << std::left << std::setw(40) << "CpuUti: " << cpuUti << std::endl;
        ss << std::left << std::setw(40) << "GpuLoad: " << gpuLoad << std::endl;
        ss << std::left << std::setw(40) << "GpuSmem: " << gpuSmem << std::endl;
        ss << std::left << std::setw(40) << "GpuTemp: " << gpuTemp << std::endl;
        ss << std::left << std::setw(40) << "GpuUti: " << gpuUti << std::endl;
        ss << std::left << std::setw(40) << "MemTotal: " << memTotal << std::endl;
        ss << std::left << std::setw(40) << "MemUsed: " << memUsed << std::endl;
        ss << std::left << std::setw(40) << "MemFree: " << memFree << std::endl;
        ss << std::left << std::setw(40) << "DiskTotal: " << diskTotal << std::endl;
        ss << std::left << std::setw(40) << "DiskUsed: " << diskUsed << std::endl;
        ss << std::left << std::setw(40) << "DiskFree: " << diskFree << std::endl;
        ss << std::left << std::setw(40) << "DiskTps: " << diskTps << std::endl;
        ss << std::left << std::setw(40) << "DiskWrite: " << diskWrite << std::endl;
        ss << std::left << std::setw(40) << "DiskRead: " << diskRead << std::endl;
        
        ss << std::left << std::setw(40) << "NetRx: " << netRx << std::endl;
        ss << std::left << std::setw(40) << "NetTx: " << netTx << std::endl;
        ss << std::left << std::setw(40) << "NetRxByte: " << netRxByte << std::endl;
        ss << std::left << std::setw(40) << "NetTxByte: " << netTxByte << std::endl;
        
        ss << std::left << std::setw(40) << "AlarmLevel: " << alarmLevel << std::endl; // Assuming alarmLevel has a to_string() method
        ss << std::left << std::setw(40) << "AlarmStatus: " << alarmStatus << std::endl; // Assuming alarmStatus has a to_string() method
        ss << std::left << std::setw(40) << "AlarmRaisedTime: " << alarmRaisedTime << std::endl;
        ss << std::left << std::setw(40) << "AlarmChangedTime: " << alarmChangedTime << std::endl;
        ss << std::left << std::setw(40) << "AlarmType: " << alarmType << std::endl;
        ss << std::left << std::setw(40) << "Addition: " << addition << std::endl;
        ss << std::left << std::setw(40) << "deviceNoAlarmType: " << deviceNoAlarmType << std::endl;
        return ss.str();
    }
};
struct NetworkInfo
{
    std::string ip_address;
    std::string gateway;
    std::string netmask;
};

struct SshInfo
{
    std::string host = "127.0.0.1";    // 从命令行参数获取宿主机IP
    std::string user;    // 从命令行参数获取用户名
    std::string password; // 从命令行参数获取密码
    std::string execCmd; //执行命令
    std::string execRet; //执行命令返回
    ssh_session session;
    bool        flagReboot = false;
    public:
    // to_string 函数
    std::string to_string() const {
        std::ostringstream oss;
        oss << "Host: " << host << "\n"
            << "User: " << user << "\n"
            << "Password: " << password << "\n"
            << "Exec Command: " << execCmd << "\n"
            << "Exec Return: " << execRet;
        return oss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
