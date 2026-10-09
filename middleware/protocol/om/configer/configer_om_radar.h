/*********************************************************************************
 * @file		configer.h
 * @brief		configer belongs to CICTCI
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
 *  24-6-9 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_RADAR_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_RADAR_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_radar.h"
NAMESPACE_START_OM_COMPONENT_RADAR
using namespace os::v2x::protocol::om::common;
using namespace airos::base::workparam;

//rsu客户端配置
struct ConfigerRadar  : public afl::base::SerializableData
{
    MqttTopicConfigerRadarID            configerTopicRadarID;
    MqttEnableConfigerOmRadar           configerEnable;
    MqttTopicConfigerOmRadar            configerTopic;
    std::unordered_map<uint32_t, MqttTopicConfigerOmRadar>  topicUnMap;
private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(configerTopicRadarID, "A_configerTopicRadarID", j, false);
        JsonSerialize(configerEnable, "B_configerEnable", j, false);
        JsonSerialize(configerTopic, "C_configerTopic", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(configerTopicRadarID, "A_configerTopicRadarID", j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnable, "B_configerEnable", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopic, "C_configerTopic", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_configerTopicRadarID:\n " << configerTopicRadarID.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "B_configerEnableOmRadar:\n " << configerEnable.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "C_configerTopicOmRadar:\n " << configerTopic.to_string() <<  std::endl;
        ss << "---------------------------------------------------\n";
        ss << "--->om-radar\n";
        ss << std::left << std::setw(40) << "D_topicUnMap-Size: " << topicUnMap.size() <<  std::endl;
        for (const auto& pair : topicUnMap)
        {
            ss << std::left << "om-Radar-topic[" << pair.first << "]" <<  std::endl ;
            ss << pair.second.to_string() <<  std::endl;
            ss << "---------------------------------------------------\n";
        }
        ss << "---------------------------------------------------\n";
        return ss.str();
    }
};



struct RadarParamMapFromFileConfiger : public afl::base::SerializableData
{
public:

    std::string         deviceSn;
    bool                isPubOnce = false;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(deviceSn, "A_deviceSn", j, false);
        JsonSerialize(isPubOnce, "B_isPubOnce", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(deviceSn, "A_deviceSn", j, noUse_isEmptyFlag);
        JsonDeserialize(isPubOnce, "B_isPubOnce", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "deviceSn: " << deviceSn <<  std::endl;
        ss << std::left << std::setw(40) << "isPubOnce: " << (isPubOnce ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};

struct RadarRegisterMapConfiger : public afl::base::SerializableData
{
public:
    bool                isRegistered = false;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(isRegistered, "A_isRegistered", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(isRegistered, "A_isRegistered", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "isRegistered: " << isRegistered <<  std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_RADAR
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
