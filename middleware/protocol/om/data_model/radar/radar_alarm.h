/*********************************************************************************
* @file		alarm
* @brief	alarm belongs to CICTCI
* @details
* @author		alfred
* @email       zhangenwei64@gmail.com
* @date		24-6-9
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  25-3-4 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#ifndef AIROS2_0_CONFIGER_OM_RADAR_ALARM_H
#define AIROS2_0_CONFIGER_OM_RADAR_ALARM_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"

NAMESPACE_START_OM_COMPONENT_RADAR
#include <string>
#include <vector>
struct RadarAlarm : public afl::base::SerializableData
{
    long timestamp; // 时间戳，UTC时间，单位为毫秒，精确到毫秒
    std::string seqNum; // 会话唯一标识
    std::string deviceID; // 设备唯一标识码
    int faultType; // 故障类型
    std::string addition; // 告警内容补充。无补充时：空字符串

    virtual void serialize(json& j) override
    {
        JsonSerialize(timestamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(deviceID, "deviceID", j, false);
        JsonSerialize(faultType, "faultType", j, false);
        JsonSerialize(addition, "addition", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timestamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID, "deviceID", j, noUse_isEmptyFlag);
        JsonDeserialize(faultType, "faultType", j, noUse_isEmptyFlag);
        JsonDeserialize(addition, "addition", j, noUse_isEmptyFlag);
    }
public:
    // 将信息转为字符串
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "                   << timestamp                   << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                   << seqNum                   << std::endl;
        ss << std::left << std::setw(40) << "deviceID: "                   << deviceID                  << std::endl;
        ss << std::left << std::setw(40) << "faultType: "                   << faultType                  << std::endl;
        ss << std::left << std::setw(40) << "addition: "                   << addition                  << std::endl;
        return ss.str();
    }

    std::string to_string_log() const
    {
        std::stringstream ss;
        ss << "[timestamp]"   << timestamp
           << "[seqNum]"       << seqNum
           << "[deviceID]"    << deviceID
           << "[faultType]"   << faultType
           << "[addition]"    << addition;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_RADAR
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
