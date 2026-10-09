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
#ifndef MQTT_CLIENT_MEC_OM_ALARM_DATA_H
#define MQTT_CLIENT_MEC_OM_ALARM_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace afl::base;
using namespace os::v2x::protocol::om::common;

struct AlarmData : public afl::base::SerializableData
{
    AlertLevelEnum      alarmLevel;
    AlarmStatusEnum     alarmStatus;
    long                alarmRaisedTime;
    long                alarmChangedTime;
    MecAlarmTypeErrorCodeEnum         alarmType;
    std::string         addition = STR_DEFAULT_VALUE;
    bool                additionEmpty = false;
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(alarmLevel, "alarmLevel", j, false);
        JsonSerialize(alarmStatus, "alarmStatus", j, false);
        JsonSerialize(alarmRaisedTime, "alarmRaisedTime", j, false);
        JsonSerialize(alarmChangedTime, "alarmChangedTime", j, false);
        JsonSerialize(alarmType, "alarmType", j, false);
        JsonSerialize(addition, "addition", j, additionEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(alarmLevel, "alarmLevel", j, noUse_isEmptyFlag);
        JsonDeserialize(alarmStatus, "alarmStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(alarmRaisedTime, "alarmRaisedTime", j, noUse_isEmptyFlag);
        JsonDeserialize(alarmChangedTime, "alarmChangedTime", j, noUse_isEmptyFlag);
        JsonDeserialize(alarmType, "alarmType", j, noUse_isEmptyFlag);
        JsonDeserialize(addition, "addition", j, additionEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Alarm Level: " << alarmLevel <<  std::endl;
        ss << std::left << std::setw(40) << "Alarm Status: " << alarmStatus <<  std::endl;
        ss << std::left << std::setw(40) << "Alarm Raised Time: " << alarmRaisedTime <<  std::endl;
        ss << std::left << std::setw(40) << "Alarm Changed Time: " << alarmChangedTime <<  std::endl;
        ss << std::left << std::setw(40) << "Alarm Type: " << alarmType <<  std::endl;
        ss << std::left << std::setw(40) << "addition: " << addition <<  std::endl;
        return ss.str();
    }
};


struct AlarmManagementData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string protocolVersion;
    AlarmData alarm;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(alarm, "alarm", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(alarm, "alarm", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp <<  std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn <<  std::endl;
        ss << std::left << std::setw(40) << "protocolVersion: " << protocolVersion <<  std::endl;
        ss << std::left << std::setw(40) << "alarm:\n" << alarm.to_string() <<  std::endl;

        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
