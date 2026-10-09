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
#ifndef MQTT_CLIENT_MEC_OM_MONITOR_DATA_SENSOR_DATA_STATUS_H
#define MQTT_CLIENT_MEC_OM_MONITOR_DATA_SENSOR_DATA_STATUS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"

NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;
struct SensorDataStatusData: public afl::base::SerializableData
{
    std::string sensor_data_in_port;               // 感知数据接收端口
    bool sensor_data_in_flag;                       // 是否接收到感知数据
    int sensor_data_in_obj_package;                 // 感知目标2011大包个数
    int sensor_data_in_obj_num;                     // 感知目标2011 总目标个数
    std::string sensor_cloud_ip;                    // 感知运控的ip
    std::string sensor_cloud_port;                  // 感知运控的端口
    bool sensor_cloud_con_flag;                     // 感知上云模块连接云控是否正常
    bool sensor_data_up_flag;                       // 感知数据上云是否成功
    int sensor_data_up_obj_package;                 // 感知目标2011大包个数
    int sensor_data_up_obj_num;                     // 感知目标2011 总目标个数

    // 序列化函数
    void serialize(json &j) override {
        JsonSerialize(sensor_data_in_port,              "sensor_data_in_port",              j, false);
        JsonSerialize(sensor_data_in_flag,              "sensor_data_in_flag",              j, false);
        JsonSerialize(sensor_data_in_obj_package,       "sensor_data_in_obj_package",       j, false);
        JsonSerialize(sensor_data_in_obj_num,           "sensor_data_in_obj_num",           j, false);
        JsonSerialize(sensor_cloud_ip,                   "sensor_cloud_ip",                   j, false);
        JsonSerialize(sensor_cloud_port,                 "sensor_cloud_port",                 j, false);
        JsonSerialize(sensor_cloud_con_flag,             "sensor_cloud_con_flag",             j, false);
        JsonSerialize(sensor_data_up_flag,               "sensor_data_up_flag",               j, false);
        JsonSerialize(sensor_data_up_obj_package,        "sensor_data_up_obj_package",        j, false);
        JsonSerialize(sensor_data_up_obj_num,            "sensor_data_up_obj_num",            j, false);
    }

    // 反序列化函数
    void deserialize(const json &j) override {
        JsonDeserialize(sensor_data_in_port,              "sensor_data_in_port",              j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_in_flag,              "sensor_data_in_flag",              j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_in_obj_package,       "sensor_data_in_obj_package",       j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_in_obj_num,           "sensor_data_in_obj_num",           j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_ip,                   "sensor_cloud_ip",                   j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_port,                 "sensor_cloud_port",                 j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_con_flag,             "sensor_cloud_con_flag",             j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_up_flag,               "sensor_data_up_flag",               j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_up_obj_package,        "sensor_data_up_obj_package",        j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_data_up_obj_num,            "sensor_data_up_obj_num",            j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "sensor_data_in_port: "                         << sensor_data_in_port << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_in_flag: "         << (sensor_data_in_flag ? "true" : "false") << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_in_obj_package: "                   << sensor_data_in_obj_package << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_in_obj_num: "                  << sensor_data_in_obj_num << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_ip: "                  << sensor_cloud_ip << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_port: "                         << sensor_cloud_port << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_con_flag: "         << (sensor_cloud_con_flag ? "true" : "false") << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_up_flag: "         << (sensor_data_up_flag ? "true" : "false") << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_up_obj_package: "                  << sensor_data_up_obj_package << std::endl;
        ss << std::left << std::setw(40) << "sensor_data_up_obj_num: "                  << sensor_data_up_obj_num << std::endl;
        return ss.str();
    }
};

struct SensorDataStatus : public afl::base::SerializableData {
    QUERY_MSG_TAG tag;                                        // 接口号
    std::string seqnum;                            // 32位不重复的随机数
    uint64_t timestamp;                             // utc时间戳，毫秒
    std::string device_esn;                        // 设备esn号
    SensorDataStatusData data;                              // 消息结构

    // 序列化函数
    virtual void serialize(json &j) override {
        JsonSerialize(tag,                        "tag",                        j, false);
        JsonSerialize(seqnum,                    "seqnum",                    j, false);
        JsonSerialize(timestamp,                  "timestamp",                  j, false);
        JsonSerialize(device_esn,                 "device_esn",                 j, false);
        JsonSerialize(data,                 "data",                 j, false);
    }

    // 反序列化函数
    virtual void deserialize(const json &j) override {
        JsonDeserialize(tag,                        "tag",                        j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum,                    "seqnum",                    j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp,                  "timestamp",                  j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn,                 "device_esn",                 j, noUse_isEmptyFlag);
        JsonDeserialize(data,                 "data",                 j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "tag: "                         << tag << std::endl;
        ss << std::left << std::setw(40) << "seqnum: "                      << seqnum << std::endl;
        ss << std::left << std::setw(40) << "timestamp: "                   << timestamp << std::endl;
        ss << std::left << std::setw(40) << "device_esn: "                  << device_esn << std::endl;
        ss << std::left << std::setw(40) << "data: "                  << data.to_string() << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
