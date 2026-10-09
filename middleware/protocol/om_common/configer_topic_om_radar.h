/*********************************************************************************
 * @file		configer_topic_om_radar.h
 * @brief		configer_topic_om_radar belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-7
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

#ifndef AIROS2_0_CONFIGER_TOPIC_OM_RADAR_H
#define AIROS2_0_CONFIGER_TOPIC_OM_RADAR_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_OM_COMPONENT_RADAR
using namespace os::v2x::protocol::om;
using namespace afl::base;

struct MqttTopicConfigerRadarID : public afl::base::SerializableData
{
    std::string cross_id = "";  //废弃不使用
    std::vector<std::string> radarIDs;
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(cross_id, "A_cross_id", j, false);
        JsonSerialize(radarIDs, "B_radarIDs", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(cross_id, "A_cross_id", j, noUse_isEmptyFlag);
        JsonDeserialize(radarIDs, "B_radarIDs", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_cross_id: " << cross_id;
        ss << std::left << std::setw(40) << "B_radarIDs: ";
        for (const auto& id : radarIDs) {
            ss << id << " ";
        }
        ss << std::endl;
        return ss.str();
    }
};

struct MqttTopicConfigerOmRadar : public afl::base::SerializableData
{
    std::string Topic_Profix                = "radar/";
    std::string Topic_Query_Log             = "/query/log";
    std::string Topic_Query_Log_Ack         = "/ack/query/log";
    std::string Topic_Register              = "/register";
    std::string Topic_Register_Ack          = "/register/ack";
    std::string Topic_Query_Config          = "/query/config";
    std::string Topic_Query_Config_Info_Up     = "/query/config-info/up";
    std::string Topic_Query_Config_Ack      = "/ack/query/config";
    std::string Topic_Update_Config         = "/update/config";
    std::string Topic_Update_Config_Ack     = "/ack/update/config";
    std::string Topic_Restore               = "/restore";
    std::string Topic_Restore_ACK           = "/ack/restore";
    std::string Topic_Runstatus             = "/upload/status";
    std::string Topic_Warning               = "/upload/warning";
    std::string Topic_Heartbeat             = "/heartbeat";
    std::string Topic_Ota                   = "/ota";
    std::string Topic_Ota_Cancel            = "/ota/cancel";
    std::string Topic_Ota_Cancel_ACK            = "/ack/ota/cancel";
    std::string Topic_Ota_ACK               = "/ack/ota";
    std::string Topic_Reboot                = "/reboot";
    std::string Topic_Reboot_Ack            = "/ack/reboot";
    std::string Topic_Synchronize           = "/synchronize";


    std::string Topic_InternalExternalParams_Up       = "/internal-external-params/up";
    std::string Topic_InternalExternalParams_Up_Ack       = "/internal-external-params/up/ack";
    std::string Topic_InternalExternalParams_Query       = "/internal-external-params/query";
    std::string Topic_InternalExternalParams_Query_Down       = "/internal-external-params/query/down";

private:
    virtual void serialize(json &j)
    {
        JsonSerialize(Topic_Profix,           "A_Topic_Profix",          j, false);
        JsonSerialize(Topic_Query_Log,        "B_Topic_Query_Log",       j, false);
        JsonSerialize(Topic_Query_Log_Ack,    "C_Topic_Query_Log_Ack",   j, false);
        JsonSerialize(Topic_Register,         "D_Topic_Register",        j, false);
        JsonSerialize(Topic_Register_Ack,     "D_Topic_Register_Ack",    j, false);
        JsonSerialize(Topic_Query_Config,     "E_Topic_Query_Config",    j, false);
        JsonSerialize(Topic_Query_Config_Info_Up,     "E_Topic_Query_Config_Info_Up",    j, false);

        JsonSerialize(Topic_Query_Config_Ack, "F_Topic_Query_Config_Ack",j, false);
        JsonSerialize(Topic_Update_Config,    "G_Topic_Update_Config",   j, false);
        JsonSerialize(Topic_Update_Config_Ack,"H_Topic_Update_Config_Ack",j, false);
        JsonSerialize(Topic_Restore,          "I_Topic_Restore",         j, false);
        JsonSerialize(Topic_Restore_ACK,      "J_Topic_Restore_ACK",     j, false);
        JsonSerialize(Topic_Runstatus,        "K_Topic_Runstatus",       j, false);
        JsonSerialize(Topic_Warning,          "L_Topic_Warning",         j, false);
        JsonSerialize(Topic_Heartbeat,        "M_Topic_Heartbeat",       j, false);
        JsonSerialize(Topic_Ota,              "N_Topic_Ota",             j, false);
        JsonSerialize(Topic_Ota_Cancel,       "N_Topic_Ota_Cancel",             j, false);
        JsonSerialize(Topic_Ota_Cancel_ACK,   "N_Topic_Ota_Cancel_ACK",             j, false);
        JsonSerialize(Topic_Ota_ACK,          "O_Topic_Ota_ACK",         j, false);
        JsonSerialize(Topic_Reboot,           "P_Topic_Reboot",          j, false);
        JsonSerialize(Topic_Reboot_Ack,       "Q_Topic_Reboot_Ack",      j, false);
        JsonSerialize(Topic_Synchronize,      "R_Topic_Synchronize",     j, false);


        JsonSerialize(Topic_InternalExternalParams_Up, "S_Topic_InternalExternalParams_Up", j, false);
        JsonSerialize(Topic_InternalExternalParams_Up_Ack, "S_Topic_InternalExternalParams_Up_Ack", j, false);
        JsonSerialize(Topic_InternalExternalParams_Query, "S_Topic_InternalExternalParams_Query", j, false);
        JsonSerialize(Topic_InternalExternalParams_Query_Down,      "S_Topic_InternalExternalParams_Query_Down",     j, false);

    }

    virtual void deserialize(const json &j)
    {
        JsonDeserialize(Topic_Profix,           "A_Topic_Profix",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Query_Log,        "B_Topic_Query_Log",        j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Query_Log_Ack,    "C_Topic_Query_Log_Ack",    j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Register,         "D_Topic_Register",         j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Register_Ack,       "D_Topic_Register_Ack",    j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Query_Config,     "E_Topic_Query_Config",     j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Query_Config_Info_Up,     "E_Topic_Query_Config_Info_Up",    j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_Query_Config_Ack, "F_Topic_Query_Config_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Update_Config,    "G_Topic_Update_Config",    j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Update_Config_Ack,"H_Topic_Update_Config_Ack",j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Restore,          "I_Topic_Restore",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Restore_ACK,      "J_Topic_Restore_ACK",      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Runstatus,        "K_Topic_Runstatus",        j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Warning,          "L_Topic_Warning",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Heartbeat,        "M_Topic_Heartbeat",        j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota,              "N_Topic_Ota",              j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Cancel,       "N_Topic_Ota_Cancel",             j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Cancel_ACK,   "N_Topic_Ota_Cancel_ACK",             j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_ACK,          "O_Topic_Ota_ACK",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Reboot,           "P_Topic_Reboot",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Reboot_Ack,       "Q_Topic_Reboot_Ack",       j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Synchronize,      "R_Topic_Synchronize",      j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_InternalExternalParams_Up, "S_Topic_InternalExternalParams_Up", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_InternalExternalParams_Up_Ack, "S_Topic_InternalExternalParams_Up_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_InternalExternalParams_Query, "S_Topic_InternalExternalParams_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_InternalExternalParams_Query_Down,      "S_Topic_InternalExternalParams_Query_Down",     j, noUse_isEmptyFlag);

    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Topic_Profix: " << Topic_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "B_Topic_Query_Log: " << Topic_Query_Log <<  std::endl;
        ss << std::left << std::setw(40) << "C_Topic_Query_Log_Ack: " << Topic_Query_Log_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "D_Topic_Register: " << Topic_Register <<  std::endl;
        ss << std::left << std::setw(40) << "D_Topic_Register_Ack: " << Topic_Register_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "E_Topic_Query_Config: " << Topic_Query_Config <<  std::endl;
        ss << std::left << std::setw(40) << "E_Topic_Query_Config_Info_Up: " << Topic_Query_Config_Info_Up <<  std::endl;

        ss << std::left << std::setw(40) << "F_Topic_Query_Config_Ack: " << Topic_Query_Config_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "G_Topic_Update_Config: " << Topic_Update_Config <<  std::endl;
        ss << std::left << std::setw(40) << "H_Topic_Update_Config_Ack: " << Topic_Update_Config_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "I_Topic_Restore: " << Topic_Restore <<  std::endl;
        ss << std::left << std::setw(40) << "J_Topic_Restore_ACK: " << Topic_Restore_ACK <<  std::endl;
        ss << std::left << std::setw(40) << "K_Topic_Runstatus: " << Topic_Runstatus <<  std::endl;
        ss << std::left << std::setw(40) << "L_Topic_Warning: " << Topic_Warning <<  std::endl;
        ss << std::left << std::setw(40) << "M_Topic_Heartbeat: " << Topic_Heartbeat <<  std::endl;
        ss << std::left << std::setw(40) << "N_Topic_Ota: " << Topic_Ota <<  std::endl;
        ss << std::left << std::setw(40) << "N_Topic_Ota_Cancel: " << Topic_Ota_Cancel <<  std::endl;
        ss << std::left << std::setw(40) << "O_Topic_Ota_ACK: " << Topic_Ota_ACK <<  std::endl;
        ss << std::left << std::setw(40) << "P_Topic_Reboot: " << Topic_Reboot <<  std::endl;
        ss << std::left << std::setw(40) << "Q_Topic_Reboot_Ack: " << Topic_Reboot_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "R_Topic_Synchronize: " << Topic_Synchronize <<  std::endl;


        ss << std::left << std::setw(40) << "S_Topic_InternalExternalParams_Up: " << Topic_InternalExternalParams_Up <<  std::endl;
        ss << std::left << std::setw(40) << "S_Topic_InternalExternalParams_Up_Ack: " << Topic_InternalExternalParams_Up_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "S_Topic_InternalExternalParams_Query: " << Topic_InternalExternalParams_Query <<  std::endl;
        ss << std::left << std::setw(40) << "S_Topic_InternalExternalParams_Query_Down: " << Topic_InternalExternalParams_Query_Down <<  std::endl;

        return ss.str();
    }
};
struct MqttEnableConfigerOmRadar : public afl::base::SerializableData
{
public:
    bool Enable_Register = true;
    bool Enable_Register_Ack = true;
    bool Enable_Query_Log = true;
    bool Enable_Query_Log_Ack = true;
    bool Enable_Query_Config = true;
    bool Enable_Query_Config_Ack = true;
    bool Enable_Update_Config = true;
    bool Enable_Update_Config_Ack = true;
    bool Enable_Restore = true;
    bool Enable_Restore_ACK = true;
    bool Enable_Runstatus = true;
    bool Enable_Warning = true;
    bool Enable_Heartbeat = true;
    bool Enable_Ota = true;
    bool Enable_Ota_Cancel = true;
    bool Enable_Ota_ACK = true;
    bool Enable_Reboot = true;
    bool Enable_Reboot_Ack = true;
    bool Enable_Synchronize = true;

    bool Enable_InternalExternalParams_Up = true;
    bool Enable_InternalExternalParams_Up_Ack = true;
    bool Enable_InternalExternalParams_Query = true;
    bool Enable_InternalExternalParams_Query_Down = true;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Enable_Register, "A_Enable_Register", j, false);
        JsonSerialize(Enable_Register_Ack, "A_Enable_Register_Ack", j, false);
        JsonSerialize(Enable_Query_Log, "B_Enable_Query_Log", j, false);
        JsonSerialize(Enable_Query_Log_Ack, "C_Enable_Query_Log_Ack", j, false);
        JsonSerialize(Enable_Query_Config, "D_Enable_Query_Config", j, false);
        JsonSerialize(Enable_Query_Config_Ack, "E_Enable_Query_Config_Ack", j, false);
        JsonSerialize(Enable_Update_Config, "F_Enable_Update_Config", j, false);
        JsonSerialize(Enable_Update_Config_Ack, "G_Enable_Update_Config_Ack", j, false);
        JsonSerialize(Enable_Restore, "H_Enable_Restore", j, false);
        JsonSerialize(Enable_Restore_ACK, "I_Enable_Restore_ACK", j, false);
        JsonSerialize(Enable_Runstatus, "J_Enable_Runstatus", j, false);
        JsonSerialize(Enable_Warning, "K_Enable_Warning", j, false);
        JsonSerialize(Enable_Heartbeat, "L_Enable_Heartbeat", j, false);
        JsonSerialize(Enable_Ota, "M_Enable_Ota", j, false);
        JsonSerialize(Enable_Ota_Cancel, "M_Enable_Ota_Cancel", j, false);

        JsonSerialize(Enable_Ota_ACK, "N_Enable_Ota_ACK", j, false);
        JsonSerialize(Enable_Reboot, "O_Enable_Reboot", j, false);
        JsonSerialize(Enable_Reboot_Ack, "P_Enable_Reboot_Ack", j, false);
        JsonSerialize(Enable_Synchronize, "Q_Enable_Synchronize", j, false);

        JsonSerialize(Enable_InternalExternalParams_Up, "R_Enable_InternalExternalParams_Up", j, false);
        JsonSerialize(Enable_InternalExternalParams_Up_Ack, "R_Enable_InternalExternalParams_Up_Ack", j, false);
        JsonSerialize(Enable_InternalExternalParams_Query, "R_Enable_InternalExternalParams_Query", j, false);
        JsonSerialize(Enable_InternalExternalParams_Query_Down, "R_Enable_InternalExternalParams_Query_Down", j, false);

    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Enable_Register, "A_Enable_Register", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Register_Ack, "A_Enable_Register_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Query_Log, "B_Enable_Query_Log", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Query_Log_Ack, "C_Enable_Query_Log_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Query_Config, "D_Enable_Query_Config", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Query_Config_Ack, "E_Enable_Query_Config_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Update_Config, "F_Enable_Update_Config", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Update_Config_Ack, "G_Enable_Update_Config_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Restore, "H_Enable_Restore", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Restore_ACK, "I_Enable_Restore_ACK", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Runstatus, "J_Enable_Runstatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Warning, "K_Enable_Warning", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Heartbeat, "L_Enable_Heartbeat", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota, "M_Enable_Ota", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_Cancel, "M_Enable_Ota_Cancel", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_ACK, "N_Enable_Ota_ACK", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Reboot, "O_Enable_Reboot", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Reboot_Ack, "P_Enable_Reboot_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Synchronize, "Q_Enable_Synchronize", j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_InternalExternalParams_Up, "R_Enable_InternalExternalParams_Up", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_InternalExternalParams_Up_Ack, "R_Enable_InternalExternalParams_Up_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_InternalExternalParams_Query, "R_Enable_InternalExternalParams_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_InternalExternalParams_Query_Down, "R_Enable_InternalExternalParams_Query_Down", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Register: " << (Enable_Register ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Register_Ack: " << (Enable_Register_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "B_Enable_Query_Log: " << (Enable_Query_Log ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "C_Enable_Query_Log_Ack: " << (Enable_Query_Log_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "D_Enable_Query_Config: " << (Enable_Query_Config ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "E_Enable_Query_Config_Ack: " << (Enable_Query_Config_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "F_Enable_Update_Config: " << (Enable_Update_Config ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "G_Enable_Update_Config_Ack: " << (Enable_Update_Config_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "H_Enable_Restore: " << (Enable_Restore ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "I_Enable_Restore_ACK: " << (Enable_Restore_ACK ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "J_Enable_Runstatus: " << (Enable_Runstatus ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "K_Enable_Warning: " << (Enable_Warning ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "L_Enable_Heartbeat: " << (Enable_Heartbeat ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "M_Enable_Ota: " << (Enable_Ota ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "M_Enable_Ota_Cancel: " << (Enable_Ota_Cancel ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "N_Enable_Ota_ACK: " << (Enable_Ota_ACK ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "O_Enable_Reboot: " << (Enable_Reboot ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "P_Enable_Reboot_Ack: " << (Enable_Reboot_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Q_Enable_Synchronize: " << (Enable_Synchronize ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "R_Enable_InternalExternalParams_Up: " << (Enable_InternalExternalParams_Up ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "R_Enable_InternalExternalParams_Up_Ack: " << (Enable_InternalExternalParams_Up_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "R_Enable_InternalExternalParams_Query: " << (Enable_InternalExternalParams_Query ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "R_Enable_InternalExternalParams_Query_Down: " << (Enable_InternalExternalParams_Query_Down ? "True" : "False") <<  std::endl;


        return ss.str();
    }
};


NAMESPACE_ENDED_OM_COMPONENT_RADAR
#endif //AIROS2_0_CONFIGER_MEC_CLOUD_CAMERA_TOPIC_H
