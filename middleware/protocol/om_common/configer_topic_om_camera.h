/*********************************************************************************
 * @file		configer_topic.h
 * @brief		configer_topic belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-8
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-6-8 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS2_0_CONFIGER_TOPIC_OM_CAMERA_H
#define AIROS2_0_CONFIGER_TOPIC_OM_CAMERA_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_OM_COMPONENT_CAMERA
using namespace os::v2x::protocol::om;
using namespace afl::base;
struct MqttTopicConfigerCameraRcId : public afl::base::SerializableData
{
    std::vector<std::string> RcIds;
    virtual void serialize(afl::base::json &j) override
    {
        JSON_SERIALIZE_VARS(
                RcIds
        )
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JSON_DESERIALIZE_VARS(
                RcIds
        )

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "RcIds: ";
        for (const auto& id : RcIds) {
            ss << id << " ";
        }
        ss << std::endl;
        return ss.str();
    }
};


struct MqttTopicConfigerCamera : public afl::base::SerializableData
{
    std::string Topic_Profix            = "camera/";
    std::string Topic_Register          = "/register";
    std::string Topic_Register_Ack          = "/register/ack";
    std::string Topic_Config_Query      = "/config/query";
    std::string Topic_Config_Info_Query_Up      = "/config-info/query/up";
    std::string Topic_Config_Query_Ack  = "/position/up";
    std::string Topic_Config_Down       = "/config/down";
    std::string Topic_Config_Down_ACK   = "/config/down/ack";
    std::string Topic_Runstatus         = "/run-status/up";
    std::string Topic_Alarm             = "/alarm";
    std::string Topic_Keepalive         = "/keep-alive";
    std::string Topic_Restar            = "/restart";
    std::string Topic_Restar_Up            = "/restart/up";
    std::string Topic_Upgrade           = "/upgrade";
    std::string Topic_Upgrade_Cancel           = "/upgrade/cancel";
    std::string Topic_Upgrade_Cancel_ACK = "/upgrade/cancel/ack";
    std::string Topic_UpgradeStatus     = "/upgrade-status/up";
    std::string Topic_Synchronize       = "/synchronize";


    std::string Topic_Vehicle_Count_UP       = "/vehicle-count/up";

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Topic_Profix, "A_Topic_Profix", j, false);
        JsonSerialize(Topic_Register, "B_Topic_Register", j, false);
        JsonSerialize(Topic_Register_Ack, "B_Topic_Register_Ack", j, false);
        JsonSerialize(Topic_Config_Query, "C_Topic_Config_Query", j, false);
        JsonSerialize(Topic_Config_Info_Query_Up, "C_Topic_Config_Info_Query_Up", j, false);
        JsonSerialize(Topic_Config_Query_Ack, "D_Topic_Config_Query_Ack", j, false);
        JsonSerialize(Topic_Config_Down, "E_Topic_Config_Down", j, false);
        JsonSerialize(Topic_Config_Down_ACK, "F_Topic_Config_Down_ACK", j, false);
        JsonSerialize(Topic_Runstatus, "G_Topic_Runstatus", j, false);
        JsonSerialize(Topic_Alarm, "H_Topic_Alarm", j, false);
        JsonSerialize(Topic_Keepalive, "I_Topic_Keepalive", j, false);
        JsonSerialize(Topic_Restar, "J_Topic_Restar", j, false);
        JsonSerialize(Topic_Restar_Up, "J_Topic_Restar_Up", j, false);
        JsonSerialize(Topic_Upgrade, "K_Topic_Upgrade", j, false);
        JsonSerialize(Topic_Upgrade_Cancel, "K_Topic_Upgrade_Cancel", j, false);
        JsonSerialize(Topic_Upgrade_Cancel_ACK, "K_Topic_Upgrade_Cancel_ACK", j, false);

        JsonSerialize(Topic_UpgradeStatus, "L_Topic_UpgradeStatus", j, false);
        JsonSerialize(Topic_Synchronize, "M_Topic_Synchronize", j, false);
        JsonSerialize(Topic_Vehicle_Count_UP, "O_Topic_Vehicle_Count_UP", j, false);

    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Topic_Profix, "A_Topic_Profix", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Register, "B_Topic_Register", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Register_Ack, "B_Topic_Register_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Query, "C_Topic_Config_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Info_Query_Up, "C_Topic_Config_Info_Query_Up", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Query_Ack, "D_Topic_Config_Query_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Down, "E_Topic_Config_Down", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Down_ACK, "F_Topic_Config_Down_ACK", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Runstatus, "G_Topic_Runstatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Alarm, "H_Topic_Alarm", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Keepalive, "I_Topic_Keepalive", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Restar, "J_Topic_Restar", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Restar_Up, "J_Topic_Restar_Up", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Upgrade, "K_Topic_Upgrade", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Upgrade_Cancel, "K_Topic_Upgrade_Cancel", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Upgrade_Cancel_ACK, "K_Topic_Upgrade_Cancel_ACK", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_UpgradeStatus, "L_Topic_UpgradeStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Synchronize, "M_Topic_Synchronize", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Vehicle_Count_UP, "O_Topic_Vehicle_Count_UP", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Topic_Profix: " << Topic_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "B_Topic_Register: " << Topic_Register <<  std::endl;
        ss << std::left << std::setw(40) << "B_Topic_Register_Ack: " << Topic_Register_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "C_Topic_Config_Query: " << Topic_Config_Query <<  std::endl;
        ss << std::left << std::setw(40) << "C_Topic_Config_Info_Query_Up: " << Topic_Config_Info_Query_Up <<  std::endl;
        ss << std::left << std::setw(40) << "D_Topic_Config_Query_Ack: " << Topic_Config_Query_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "E_Topic_Config_Down: " << Topic_Config_Down <<  std::endl;
        ss << std::left << std::setw(40) << "F_Topic_Config_Down_ACK: " << Topic_Config_Down_ACK <<  std::endl;
        ss << std::left << std::setw(40) << "G_Topic_Runstatus: " << Topic_Runstatus <<  std::endl;
        ss << std::left << std::setw(40) << "H_Topic_Alarm: " << Topic_Alarm <<  std::endl;
        ss << std::left << std::setw(40) << "I_Topic_Keepalive: " << Topic_Keepalive <<  std::endl;
        ss << std::left << std::setw(40) << "J_Topic_Restar: " << Topic_Restar <<  std::endl;
        ss << std::left << std::setw(40) << "J_Topic_Restar_Up: " << Topic_Restar_Up <<  std::endl;
        ss << std::left << std::setw(40) << "K_Topic_Upgrade: " << Topic_Upgrade <<  std::endl;
        ss << std::left << std::setw(40) << "K_Topic_Upgrade_Cancel: " << Topic_Upgrade_Cancel <<  std::endl;
        ss << std::left << std::setw(40) << "L_Topic_UpgradeStatus: " << Topic_UpgradeStatus <<  std::endl;
        ss << std::left << std::setw(40) << "M_Topic_Synchronize: " << Topic_Synchronize <<  std::endl;
        ss << std::left << std::setw(40) << "Topic_Vehicle_Count_UP: " << Topic_Vehicle_Count_UP <<  std::endl;

        return ss.str();
    }
};


struct MqttEnableConfigerCamera : public afl::base::SerializableData
{
public:
    bool Enable_Register = true;
    bool Enable_Register_Ack = true;
    bool Enable_Config_Query = true;
    bool Enable_Config_Query_Ack = true;
    bool Enable_Config_Down = true;
    bool Enable_Config_Down_ACK = true;
    bool Enable_Runstatus = true;
    bool Enable_Alarm = true;
    bool Enable_Keepalive = true;
    bool Enable_Restar = true;
    bool Enable_Upgrade = true;
    bool Enable_Upgrade_Cancel = true;
    bool Enable_UpgradeStatus = true;
    bool Enable_Synchronize = true;


private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Enable_Register, "A_Enable_Register", j, false);
        JsonSerialize(Enable_Register_Ack, "A_Enable_Register_Ack", j, false);
        JsonSerialize(Enable_Config_Query, "B_Enable_Config_Query", j, false);
        JsonSerialize(Enable_Config_Query_Ack, "C_Enable_Config_Query_Ack", j, false);
        JsonSerialize(Enable_Config_Down, "D_Enable_Config_Down", j, false);
        JsonSerialize(Enable_Config_Down_ACK, "E_Enable_Config_Down_ACK", j, false);
        JsonSerialize(Enable_Runstatus, "F_Enable_Runstatus", j, false);
        JsonSerialize(Enable_Alarm, "G_Enable_Alarm", j, false);
        JsonSerialize(Enable_Keepalive, "H_Enable_Keepalive", j, false);
        JsonSerialize(Enable_Restar, "I_Enable_Restar", j, false);
        JsonSerialize(Enable_Upgrade, "J_Enable_Upgrade", j, false);
        JsonSerialize(Enable_Upgrade_Cancel, "J_Enable_Upgrade_Cancel", j, false);
        JsonSerialize(Enable_UpgradeStatus, "K_Enable_UpgradeStatus", j, false);
        JsonSerialize(Enable_Synchronize, "L_Enable_Synchronize", j, false);

    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Enable_Register, "A_Enable_Register", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Register_Ack, "A_Enable_Register_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Query, "B_Enable_Config_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Query_Ack, "C_Enable_Config_Query_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Down, "D_Enable_Config_Down", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Down_ACK, "E_Enable_Config_Down_ACK", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Runstatus, "F_Enable_Runstatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Alarm, "G_Enable_Alarm", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Keepalive, "H_Enable_Keepalive", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Restar, "I_Enable_Restar", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Upgrade, "J_Enable_Upgrade", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Upgrade_Cancel, "J_Enable_Upgrade_Cancel", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_UpgradeStatus, "K_Enable_UpgradeStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Synchronize, "L_Enable_Synchronize", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Register: " << (Enable_Register ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Register_Ack: " << (Enable_Register_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "B_Enable_Config_Query: " << (Enable_Config_Query ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "C_Enable_Config_Query_Ack: " << (Enable_Config_Query_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "D_Enable_Config_Down: " << (Enable_Config_Down ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "E_Enable_Config_Down_ACK: " << (Enable_Config_Down_ACK ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "F_Enable_Runstatus: " << (Enable_Runstatus ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "G_Enable_Alarm: " << (Enable_Alarm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "H_Enable_Keepalive: " << (Enable_Keepalive ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "I_Enable_Restar: " << (Enable_Restar ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "J_Enable_Upgrade: " << (Enable_Upgrade ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "J_Enable_Upgrade_Cancel: " << (Enable_Upgrade_Cancel ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "K_Enable_UpgradeStatus: " << (Enable_UpgradeStatus ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "L_Enable_Synchronize: " << (Enable_Synchronize ? "True" : "False") <<  std::endl;

        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif //AIROS2_0_CONFIGER_MEC_CLOUD_CAMERA_TOPIC_H
