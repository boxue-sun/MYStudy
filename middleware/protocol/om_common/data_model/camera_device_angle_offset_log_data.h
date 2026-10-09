/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_PROTOCOL_DATA_MODEL_BS_ANGLE_OFFSET_CAMERA_DEVICE_ANGLE_OFFSET_LOG_DATA
#define MIDDLEWART_PROTOCOL_DATA_MODEL_BS_ANGLE_OFFSET_CAMERA_DEVICE_ANGLE_OFFSET_LOG_DATA

#include "middleware/protocol/om_common/namespace.h"
#include "data_common.h"
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;


struct CameraDeviceAngleOffsetLogDataAlarmInfo : public afl::base::SerializableData
{
    AlertLevelEnum alarmLevel;
    AlarmStatusEnum alarmStatus;
    uint64_t alarmRaisedTime;
    uint64_t alarmChangedTime;
    MecAlarmTypeErrorCodeEnum alarmType;
    std::string addition;
    bool  additionEmpty = true;
public:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(alarmLevel, "alarmLevel", j, false);
        JsonSerialize(alarmStatus, "alarmStatus", j, false);
        JsonSerialize(alarmRaisedTime, "alarmRaisedTime", j, false);
        JsonSerialize(alarmChangedTime, "alarmChangedTime", j, false);
        JsonSerialize(alarmType, "alarmType", j, false);
        JsonSerialize(addition, "addition", j, additionEmpty);
    }

    virtual void deserialize(const afl::base::json &j)
    {
        JsonDeserialize(alarmLevel, "alarmLevel", j, g_flag_empty_novalid);
        JsonDeserialize(alarmStatus, "alarmStatus", j, g_flag_empty_novalid);
        JsonDeserialize(alarmRaisedTime, "alarmRaisedTime", j, g_flag_empty_novalid);
        JsonDeserialize(alarmChangedTime, "alarmChangedTime", j, g_flag_empty_novalid);
        JsonDeserialize(alarmType, "alarmType", j, g_flag_empty_novalid);
        JsonDeserialize(addition, "addition", j, additionEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Alarm Level: "              << alarmLevel              << std::endl;
        ss << std::left << std::setw(40) << "Alarm Status: "             << alarmStatus             << std::endl;
        ss << std::left << std::setw(40) << "Alarm Raised Time: "        << alarmRaisedTime         << std::endl;
        ss << std::left << std::setw(40) << "Alarm Changed Time: "       << alarmChangedTime        << std::endl;
        ss << std::left << std::setw(40) << "Alarm Type: "               << alarmType              << std::endl;
        ss << std::left << std::setw(40) << "Addition: "                 << addition                << std::endl;
        return ss.str();
    }
};
struct CameraDeviceAngleOffsetLogData : public afl::base::SerializableData
{
    uint64_t timeStamp;  // 时间戳，UTC时间，单位为毫秒，精确到毫秒
    std::string seqNum;   // 会话唯一标识
    std::string rscuEsn;  // 设备唯一标识码
    std::string protocolVersion;  // 接口协议版本
    CameraDeviceAngleOffsetLogDataAlarmInfo alarm;   // 告警信息
public:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(alarm, "alarm", j, false);
    }

    virtual void deserialize(const afl::base::json &j)
    {
        JsonDeserialize(timeStamp, "timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(seqNum, "seqNum", j, g_flag_empty_novalid);
        JsonDeserialize(rscuEsn, "rscuEsn", j, g_flag_empty_novalid);
        JsonDeserialize(protocolVersion, "protocolVersion", j, g_flag_empty_novalid);
        JsonDeserialize(alarm, "alarm", j, g_flag_empty_novalid);
    }
public:
    
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeStamp: "                << timeStamp                  << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                   << seqNum                     << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: "                  << rscuEsn                   << std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: "          << protocolVersion             << std::endl;
        ss << std::left << std::setw(40) << "alarm: "                    << alarm.to_string()          << std::endl; // Assuming alarm has a to_string method.
        return ss.str();
    }
};


NAMESPACE_ENDED_OM_COMPONENT_COMMON

#endif