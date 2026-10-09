/*********************************************************************************
 * @file		configer_topic.h
 * @brief		configer_topic belongs to CICTCI
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

#ifndef AIROS2_0_CONFIGER_TOPIC_OM_MEC_H
#define AIROS2_0_CONFIGER_TOPIC_OM_MEC_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om;
using namespace afl::base;

struct MqttTopicConfigerMec : public afl::base::SerializableData
{
    std::string Topic_Profix                    = "rscu/";  //topic前缀
    std::string Topic_Device_Info_Query         = "/query/down";                    // 设备信息查询信息
    std::string Topic_Device_BaseInfo           = "/basic-status/up";            // 设备基础信息(注册消息）
    std::string Topic_Device_BaseInfo_Ack       = "/basic-status/up/ack";            // 设备基础信息(注册消息响应）
    std::string Topic_Config_Query_Down         = "/config/query/down";         //配置查询
    std::string Topic_Config_Query_Down_Ack     = "/config/query/up";          //配置查询响应
    std::string Topic_Config_Update             = "/config/update/down";          //配置更新下发
    std::string Topic_Config_Update_Ack         = "/config/update/up";          //配置更新下发响应
    std::string Topic_Performence               = "/running-info/up";                //性能信息上报
    std::string Topic_Running_Status            = "/run-status/up";              //设备运行状态信息上报
    std::string Topic_Alarm                     = "/alarm/up";                              //告警信息上报
    std::string Topic_Device_HeartBeat          = "/heartbeat/up";                  //心跳信息上报
    std::string Topic_Ota_Down                  = "/upgrade/down";                      //远程升级下发消息
    std::string Topic_Ota_Status                = "/upgrade/up";                      //远程升级下发响应
    std::string Topic_Ota_Version               = "/upgrade-version/up";             //远程OTA版本信息上报
    std::string Topic_Ota_Cancel               = "/upgrade/cancel";             //远程OTA： 取消升级
    std::string Topic_Ota_Cancel_ACK               = "/upgrade/cancel/ack";             //远程OTA： 取消升级反馈
    std::string Topic_Reboot                    = "/power/down";                          //远程重启/关机下发
    std::string Topic_Reboot_Ack                = "/power/up";                      //远程重启/关机下发响应

    std::string Topic_Timing                ="/timing/up";  //授时
    std::string Topic_Signal                ="/signal/up"; //信号机原始数据
    std::string Topic_Sensor_Angle_Offset   = "/angle-offset/up";
    std::string Topic_Scene_Rsi             ="/scene/rsi/up";
    std::string Topic_Scene_Rsc             ="/scene/rsc/up";
    std::string Topic_Scene_Ssm             ="/scene/ssm/up";
    std::string Topic_Scene_Rtcm            ="/scene/rtcm/up";
    std::string Topic_Scene_Bsm             ="/scene/bsm/up";
    std::string Topic_Scene_Vir             ="/scene/vir/up";
    std::string Topic_Scene_Pam             ="/scene/pam/up";
    std::string Topic_Scene_Map             ="/scene/map/up";
    std::string Topic_Scene_Spat            ="/scene/spat/up";
    std::string Topic_Scene_Rsm             = "/scene/rsm/up";
    std::string Topic_Scene_Sam             = "/scene/sam/up";
    std::string Topic_Scene_Ism             = "/scene/ism/up";
    std::string Topic_Scene_Perception      ="/scene/perception/up";

    std::string Topic_Abnormal_Behavior_Up  = "/abnormal-behavior/up";
    std::string Topic_Abnormal_Behavior_Down  = "/abnormal-behavior/down";
    std::string Topic_Bs_Inter_Profix           = "om_mec2om_device_status/";
    std::string Topic_Timing_Alarm_Inter     = "timing";
    std::string Topic_AngleOffset_Alarm_Inter = "angle-offset/up";
    std::string Topic_Trafficlight_Detect      ="/trafficlight-detect/up";

    std::string Topic_Position_Up = "/position/up";
    std::string Topic_MecSelfCheckResult_Up = "/detection/data-status/up";


    std::string Topic_Camera_InterExter_Param_Up = "/internal-external-params/up";
    std::string Topic_Camera_InterExter_Param_Up_Ack = "/internal-external-params/up/ack";

    std::string Topic_Camera_InterExter_Param_Query  = "/internal-external-params/query";
    std::string Topic_Camera_InterExter_Param_Query_Ack = "/internal-external-params/query/down";

    std::string Topic_Signal_Data_Status_Up = "/signal/data-status/up";

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Topic_Profix,                     "A_Topic_Profix",                     j, false);
        JsonSerialize(Topic_Device_Info_Query,          "C_Topic_Device_Info_Query",          j, false);
        JsonSerialize(Topic_Device_BaseInfo,            "D_Topic_Device_BaseInfo",            j, false);
        JsonSerialize(Topic_Device_BaseInfo_Ack,        "E_Topic_Device_BaseInfo_Ack",        j, false);
        JsonSerialize(Topic_Config_Query_Down,          "F_Topic_Config_Query_Down",          j, false);
        JsonSerialize(Topic_Config_Query_Down_Ack,      "G_Topic_Config_Query_Down_Ack",      j, false);
        JsonSerialize(Topic_Config_Update,               "H_Topic_Config_Update",               j, false);
        JsonSerialize(Topic_Config_Update_Ack,           "I_Topic_Config_Update_Ack",           j, false);
        JsonSerialize(Topic_Performence,                 "J_Topic_Performence",                 j, false);
        JsonSerialize(Topic_Running_Status,              "K_Topic_Running_Status",              j, false);
        JsonSerialize(Topic_Alarm,                       "L_Topic_Alarm",                       j, false);
        JsonSerialize(Topic_Device_HeartBeat,           "M_Topic_Device_HeartBeat",           j, false);
        JsonSerialize(Topic_Ota_Down,                    "N_Topic_Ota_Down",                    j, false);
        JsonSerialize(Topic_Ota_Status,                  "O_Topic_Ota_Status",                  j, false);
        JsonSerialize(Topic_Ota_Version,                 "P_Topic_Ota_Version",                 j, false);
        JsonSerialize(Topic_Ota_Cancel,                 "P_Topic_Ota_Cancel",                 j, false);
        JsonSerialize(Topic_Ota_Cancel_ACK,              "P_Topic_Ota_Cancel_ACK",                 j, false);
        JsonSerialize(Topic_Reboot,                      "Q_Topic_Reboot",                      j, false);
        JsonSerialize(Topic_Reboot_Ack,                  "R_Topic_Reboot_Ack",                  j, false);
        JsonSerialize(Topic_Timing,                      "S_Topic_Timing",                      j, false);
        JsonSerialize(Topic_Signal,                      "T_Topic_Signal",                      j, false);
        JsonSerialize(Topic_Sensor_Angle_Offset,        "U_Topic_Sensor_Angle_Offset",        j, false);
        JsonSerialize(Topic_Scene_Rsi,                   "X_Topic_Scene_Rsi",                   j, false);
        JsonSerialize(Topic_Scene_Rsc,                   "Y_Topic_Scene_Rsc",                   j, false);
        JsonSerialize(Topic_Scene_Ssm,                   "Z_Topic_Scene_Ssm",                   j, false);
        JsonSerialize(Topic_Scene_Rtcm,                  "a_Topic_Scene_Rtcm",                 j, false);
        JsonSerialize(Topic_Scene_Bsm,                   "b_Topic_Scene_Bsm",                  j, false);
        JsonSerialize(Topic_Scene_Vir,                   "c_Topic_Scene_Vir",                  j, false);
        JsonSerialize(Topic_Scene_Pam,                   "d_Topic_Scene_Pam",                  j, false);
        JsonSerialize(Topic_Scene_Map,                   "e_Topic_Scene_Map",                  j, false);
        JsonSerialize(Topic_Scene_Spat,                  "f_Topic_Scene_Spat",                 j, false);
        JsonSerialize(Topic_Scene_Rsm,                   "g_Topic_Scene_Rsm",                  j, false);
        JsonSerialize(Topic_Scene_Sam,                   "h_Topic_Scene_Sam",                  j, false);
        JsonSerialize(Topic_Scene_Ism,                   "i_Topic_Scene_Ism",                  j, false);
        JsonSerialize(Topic_Scene_Perception,            "j_Topic_Scene_Perception",           j, false);
        JsonSerialize(Topic_Abnormal_Behavior_Up,            "k_Topic_Abnormal_Behavior_Up",           j, false);
        JsonSerialize(Topic_Abnormal_Behavior_Down,            "l_Topic_Abnormal_Behavior_Down",           j, false);
        JsonSerialize(Topic_Bs_Inter_Profix,            "m_Topic_Bs_Inter_Profix",            j, false);
        JsonSerialize(Topic_Timing_Alarm_Inter,            "n_Topic_Timing_Alarm_Inter",            j, false);
        JsonSerialize(Topic_AngleOffset_Alarm_Inter,            "o_Topic_AngleOffset_Alarm_Inter",            j, false);
        JsonSerialize(Topic_Trafficlight_Detect,            "p_Topic_Trafficlight_Detect",            j, false);
        JsonSerialize(Topic_Position_Up,            "q_Topic_Position_Up",            j, false);
        JsonSerialize(Topic_MecSelfCheckResult_Up,            "r_Topic_MecSelfCheckResult_Up",            j, false);


        JsonSerialize(Topic_Camera_InterExter_Param_Up,            "s_Topic_Camera_InterExter_Param_Up",            j, false);
        JsonSerialize(Topic_Camera_InterExter_Param_Up_Ack,            "s_Topic_Camera_InterExter_Param_Up_Ack",            j, false);
        JsonSerialize(Topic_Camera_InterExter_Param_Query,            "s_Topic_Camera_InterExter_Param_Query",            j, false);
        JsonSerialize(Topic_Camera_InterExter_Param_Query_Ack,            "s_Topic_Camera_InterExter_Param_Query_Ack",            j, false);
        JsonSerialize(Topic_Signal_Data_Status_Up,            "s_Topic_Signal_Data_Status_Up",            j, false);


    }
    
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Topic_Profix,                     "A_Topic_Profix",                     j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Device_Info_Query,          "C_Topic_Device_Info_Query",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Device_BaseInfo,            "D_Topic_Device_BaseInfo",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Device_BaseInfo_Ack,        "E_Topic_Device_BaseInfo_Ack",        j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Query_Down,          "F_Topic_Config_Query_Down",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Query_Down_Ack,      "G_Topic_Config_Query_Down_Ack",      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Update,               "H_Topic_Config_Update",               j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Config_Update_Ack,           "I_Topic_Config_Update_Ack",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Performence,                 "J_Topic_Performence",                 j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Running_Status,              "K_Topic_Running_Status",              j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Alarm,                       "L_Topic_Alarm",                       j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Device_HeartBeat,           "M_Topic_Device_HeartBeat",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Down,                    "N_Topic_Ota_Down",                    j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Status,                  "O_Topic_Ota_Status",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Version,                 "P_Topic_Ota_Version",                 j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Cancel,                 "P_Topic_Ota_Cancel",                 j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Ota_Cancel_ACK,              "P_Topic_Ota_Cancel_ACK",                 j, noUse_isEmptyFlag);      
        JsonDeserialize(Topic_Reboot,                      "Q_Topic_Reboot",                      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Reboot_Ack,                  "R_Topic_Reboot_Ack",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Timing,                      "S_Topic_Timing",                      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Signal,                      "T_Topic_Signal",                      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Sensor_Angle_Offset,        "U_Topic_Sensor_Angle_Offset",        j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Rsi,                   "X_Topic_Scene_Rsi",                   j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Rsc,                   "Y_Topic_Scene_Rsc",                   j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Ssm,                   "Z_Topic_Scene_Ssm",                   j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Rtcm,                  "a_Topic_Scene_Rtcm",                 j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Bsm,                   "b_Topic_Scene_Bsm",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Vir,                   "c_Topic_Scene_Vir",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Pam,                   "d_Topic_Scene_Pam",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Map,                   "e_Topic_Scene_Map",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Spat,                  "f_Topic_Scene_Spat",                 j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Rsm,                   "g_Topic_Scene_Rsm",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Sam,                   "h_Topic_Scene_Sam",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Ism,                   "i_Topic_Scene_Ism",                  j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Scene_Perception,            "j_Topic_Scene_Perception",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Abnormal_Behavior_Up,            "k_Topic_Abnormal_Behavior_Up",           j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Abnormal_Behavior_Down,            "l_Topic_Abnormal_Behavior_Down",           j, noUse_isEmptyFlag);
     
        JsonDeserialize(Topic_Bs_Inter_Profix,            "m_Topic_Bs_Inter_Profix",            j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_Timing_Alarm_Inter,            "n_Topic_Timing_Alarm_Inter",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_AngleOffset_Alarm_Inter,            "o_Topic_AngleOffset_Alarm_Inter",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Trafficlight_Detect,            "p_Topic_Trafficlight_Detect",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Position_Up,            "q_Topic_Position_Up",            j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_MecSelfCheckResult_Up,            "r_Topic_MecSelfCheckResult_Up",            j, noUse_isEmptyFlag);


        JsonDeserialize(Topic_Camera_InterExter_Param_Up,            "s_Topic_Camera_InterExter_Param_Up",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_InterExter_Param_Up_Ack,            "s_Topic_Camera_InterExter_Param_Up_Ack",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_InterExter_Param_Query,            "s_Topic_Camera_InterExter_Param_Query",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_InterExter_Param_Query_Ack,            "s_Topic_Camera_InterExter_Param_Query_Ack",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Signal_Data_Status_Up,            "s_Topic_Signal_Data_Status_Up",            j, noUse_isEmptyFlag);
    }

    public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Topic_Profix: "                   << Topic_Profix                   << std::endl;
        ss << std::left << std::setw(40) << "Topic_Device_Info_Query: "         << Topic_Device_Info_Query        << std::endl;
        ss << std::left << std::setw(40) << "Topic_Device_BaseInfo: "           << Topic_Device_BaseInfo          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Device_BaseInfo_Ack: "       << Topic_Device_BaseInfo_Ack      << std::endl;
        ss << std::left << std::setw(40) << "Topic_Config_Query_Down: "         << Topic_Config_Query_Down        << std::endl;
        ss << std::left << std::setw(40) << "Topic_Config_Query_Down_Ack: "     << Topic_Config_Query_Down_Ack    << std::endl;
        ss << std::left << std::setw(40) << "Topic_Config_Update: "              << Topic_Config_Update             << std::endl;
        ss << std::left << std::setw(40) << "Topic_Config_Update_Ack: "          << Topic_Config_Update_Ack         << std::endl;
        ss << std::left << std::setw(40) << "Topic_Performence: "                << Topic_Performence               << std::endl;
        ss << std::left << std::setw(40) << "Topic_Running_Status: "             << Topic_Running_Status            << std::endl;
        ss << std::left << std::setw(40) << "Topic_Alarm: "                      << Topic_Alarm                     << std::endl;
        ss << std::left << std::setw(40) << "Topic_Device_HeartBeat: "          << Topic_Device_HeartBeat          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Ota_Down: "                   << Topic_Ota_Down                  << std::endl;
        ss << std::left << std::setw(40) << "Topic_Ota_Status: "                 << Topic_Ota_Status                << std::endl;
        ss << std::left << std::setw(40) << "Topic_Ota_Version: "                << Topic_Ota_Version               << std::endl;
        ss << std::left << std::setw(40) << "Topic_Ota_Cancel: "                << Topic_Ota_Cancel               << std::endl;
        ss << std::left << std::setw(40) << "Topic_Reboot: "                     << Topic_Reboot                    << std::endl;
        ss << std::left << std::setw(40) << "Topic_Reboot_Ack: "                 << Topic_Reboot_Ack                << std::endl;
        ss << std::left << std::setw(40) << "Topic_Timing: "                     << Topic_Timing                    << std::endl;
        ss << std::left << std::setw(40) << "Topic_Signal: "                     << Topic_Signal                    << std::endl;
        ss << std::left << std::setw(40) << "Topic_Sensor_Angle_Offset: "       << Topic_Sensor_Angle_Offset      << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Rsi: "                  << Topic_Scene_Rsi                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Rsc: "                  << Topic_Scene_Rsc                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Ssm: "                  << Topic_Scene_Ssm                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Rtcm: "                 << Topic_Scene_Rtcm                << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Bsm: "                  << Topic_Scene_Bsm                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Vir: "                  << Topic_Scene_Vir                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Pam: "                  << Topic_Scene_Pam                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Map: "                  << Topic_Scene_Map                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Spat: "                 << Topic_Scene_Spat                << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Rsm: "                  << Topic_Scene_Rsm                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Sam: "                  << Topic_Scene_Sam                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Ism: "                  << Topic_Scene_Ism                 << std::endl;
        ss << std::left << std::setw(40) << "Topic_Scene_Perception: "           << Topic_Scene_Perception          << std::endl;

        ss << std::left << std::setw(40) << "Topic_Abnormal_Behavior_Up: "           << Topic_Abnormal_Behavior_Up          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Abnormal_Behavior_Down: "           << Topic_Abnormal_Behavior_Down          << std::endl;
       
        ss << std::left << std::setw(40) << "Topic_Bs_Inter_Profix: "           << Topic_Bs_Inter_Profix          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Timing_Alarm_Inter: "           << Topic_Timing_Alarm_Inter          << std::endl;
        ss << std::left << std::setw(40) << "Topic_AngleOffset_Alarm_Inter: "           << Topic_AngleOffset_Alarm_Inter          << std::endl;

        ss << std::left << std::setw(40) << "Topic_Trafficlight_Detect: "           << Topic_Trafficlight_Detect          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Position_Up: "           << Topic_Position_Up          << std::endl;

        ss << std::left << std::setw(40) << "Topic_MecSelfCheckResult_Up: "           << Topic_MecSelfCheckResult_Up          << std::endl;

        ss << std::left << std::setw(40) << "Topic_Camera_InterExter_Param_Up: "           << Topic_Camera_InterExter_Param_Up          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Camera_InterExter_Param_Up_Ack: "           << Topic_Camera_InterExter_Param_Up_Ack          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Camera_InterExter_Param_Query: "           << Topic_Camera_InterExter_Param_Query          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Camera_InterExter_Param_Query_Ack: "           << Topic_Camera_InterExter_Param_Query_Ack          << std::endl;

        ss << std::left << std::setw(40) << "Topic_Signal_Data_Status_Up: "           << Topic_Signal_Data_Status_Up          << std::endl;

        return ss.str();
    }
};

struct MqttEnableConfigerMec : public afl::base::SerializableData
{
public:


    bool Enable_Use_DB_For_Monitor_Offline = false;
    bool Enable_Device_Info_Query = false;

















































































    bool Enable_Device_BaseInfo = true;         //主动上报
    bool Enable_Device_BaseInfo_Ack = true;

    //基本信息
    bool Enable_Device_BaseInfo_Query_Mec = true;         //查询
    bool Enable_Device_BaseInfo_Query_Mec_Ack = true;

    bool Enable_Device_BaseInfo_Query_Sensor = true;         //查询
    bool Enable_Device_BaseInfo_Query_Sensor_Ack = true;

    bool Enable_Config_Query_Down = true;
    bool Enable_Config_Query_Down_Ack = true;

    bool Enable_Config_Update = true;
    bool Enable_Config_Update_Ack = true;
    bool Enable_Performence = true;
    //运行状态
    bool Enable_Running_Status = true;

    bool Enable_Running_Status_Query_Mec = true;
    bool Enable_Running_Status_Query_Mec_ACK = true;

    bool Enable_Running_Status_Query_Sensor = true;
    bool Enable_Running_Status_Query_Sensor_ACK = true;

    bool Enable_Alarm = true;
    bool Enable_Alarm_Monitor = true;
    bool Enable_Device_HeartBeat = true;
    bool Enable_Ota_Down = true;
    bool Enable_Ota_Status = true;
    bool Enable_Ota_Version = true;
    bool Enable_Ota_Cancel = true;
    bool Enable_Reboot = true;
    bool Enable_Reboot_Ack = true;

    bool Enable_Timing = true;
    bool Enable_Signal = true;
    bool Enable_Scene_Rsi = true;
    bool Enable_Scene_Rsc = true;
    bool Enable_Scene_Ssm = true;
    bool Enable_Scene_Rtcm = true;
    bool Enable_Scene_Bsm = true;
    bool Enable_Scene_Vir = true;
    bool Enable_Scene_Pam = true;
    bool Enable_Scene_Map = true;
    bool Enable_Scene_Spat = true;
    
    bool Enable_Scene_Rsm = true;
    bool Enable_Scene_Sam = true;
    bool Enable_Scene_Ism = true;
    
    bool Enable_Scene_Perception = true;
    bool Enable_Change_Cloud_Address = true;
    bool Enable_Check_ProtocolVersion = true;


    bool Enable_AngleOffset = true;
    bool Enable_SpatSrcData = true;
    bool Enable_V2xData = true;

    bool Enable_TrafficlightDetectData = true;

    bool Enable_IllegalEventDetectResult = true; //违法事件检测结果上报
    bool Enable_TrafficlighDataStatus = true;  //信号机数据状态
    bool Enable_MecSelfCheck = true;  //MEC自检结果上报


    bool Enable_Camera_InternalExternalParams_Up = true;
    bool Enable_Camera_InternalExternalParams_Up_Ack = true;
    bool Enable_Camera_InternalExternalParams_Query = true;
    bool Enable_Camera_InternalExternalParams_Query_Down = true;

    bool Enable_Monitor_Mec_Om_Status = true;

    bool Enable_Spat_Data_Status = true;

    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(Enable_Use_DB_For_Monitor_Offline,  "A_Enable_Use_DB_For_Monitor_Offline",  j, false);
        JsonSerialize(Enable_Device_Info_Query,  "A_Enable_Device_Info_Query",  j, false);
        JsonSerialize(Enable_Device_BaseInfo,    "B_Enable_Device_BaseInfo",    j, false);
        JsonSerialize(Enable_Device_BaseInfo_Ack,    "B_Enable_Device_BaseInfo_Ack",    j, false);

        JsonSerialize(Enable_Device_BaseInfo_Query_Mec,    "B_Enable_Device_BaseInfo_Query_Mec",    j, false);
        JsonSerialize(Enable_Device_BaseInfo_Query_Mec_Ack,    "B_Enable_Device_BaseInfo_Query_Mec_Ack",    j, false);

        JsonSerialize(Enable_Device_BaseInfo_Query_Sensor,    "B_Enable_Device_BaseInfo_Query_Sensor",    j, false);
        JsonSerialize(Enable_Device_BaseInfo_Query_Sensor_Ack,    "B_Enable_Device_BaseInfo_Query_Sensor_Ack",    j, false);


        JsonSerialize(Enable_Config_Query_Down, "C_Enable_Config_Query_Down", j, false);
        JsonSerialize(Enable_Config_Query_Down_Ack, "D_Enable_Config_Query_Down_Ack", j, false);
        JsonSerialize(Enable_Config_Update,       "E_Enable_Config_Update",       j, false);
        JsonSerialize(Enable_Config_Update_Ack, "F_Enable_Config_Update_Ack", j, false);
        JsonSerialize(Enable_Performence,       "G_Enable_Performence",       j, false);

        JsonSerialize(Enable_Running_Status,    "H_Enable_Running_Status",    j, false);
        JsonSerialize(Enable_Running_Status_Query_Mec,    "H_Enable_Running_Status_Query_Mec",    j, false);
        JsonSerialize(Enable_Running_Status_Query_Mec_ACK,    "H_Enable_Running_Status_Query_Mec_ACK",    j, false);

        JsonSerialize(Enable_Running_Status_Query_Sensor,    "H_Enable_Running_Status_Query_Sensor",    j, false);
        JsonSerialize(Enable_Running_Status_Query_Sensor_ACK,    "H_Enable_Running_Status_Query_Sensor_ACK",    j, false);


        JsonSerialize(Enable_Alarm,             "I_Enable_Alarm",             j, false);
        JsonSerialize(Enable_Alarm_Monitor,      "I_Enable_Alarm_Monitor",             j, false);
        JsonSerialize(Enable_Device_HeartBeat,  "J_Enable_Device_HeartBeat",  j, false);
        JsonSerialize(Enable_Ota_Down,         "K_Enable_Ota_Down",         j, false);
        JsonSerialize(Enable_Ota_Status,       "L_Enable_Ota_Status",       j, false);
        JsonSerialize(Enable_Ota_Version,     "M_Enable_Ota_Version",     j, false);
        JsonSerialize(Enable_Ota_Cancel,     "M_Enable_Ota_Cancel",     j, false);
        JsonSerialize(Enable_Reboot,          "N_Enable_Reboot",          j, false);
        JsonSerialize(Enable_Reboot_Ack,      "O_Enable_Reboot_Ack",      j, false);
        JsonSerialize(Enable_Timing,           "P_Enable_Timing",           j, false);
        JsonSerialize(Enable_Signal,           "Q_Enable_Signal",           j, false);

        JsonSerialize(Enable_Scene_Rsi,       "U_Enable_Scene_Rsi",       j, false);
        JsonSerialize(Enable_Scene_Rsc,       "V_Enable_Scene_Rsc",       j, false);
        JsonSerialize(Enable_Scene_Ssm,       "W_Enable_Scene_Ssm",       j, false);
        JsonSerialize(Enable_Scene_Rtcm,      "X_Enable_Scene_Rtcm",      j, false);
        JsonSerialize(Enable_Scene_Bsm,       "Y_Enable_Scene_Bsm",       j, false);
        JsonSerialize(Enable_Scene_Vir,       "Z_Enable_Scene_Vir",       j, false);
        JsonSerialize(Enable_Scene_Pam,       "a_Enable_Scene_Pam",       j, false);
        JsonSerialize(Enable_Scene_Map,       "b_Enable_Scene_Map",       j, false);
        JsonSerialize(Enable_Scene_Spat,      "c_Enable_Scene_Spat",      j, false);
        
        JsonSerialize(Enable_Scene_Rsm,       "d_Enable_Scene_Rsm",       j, false);
        JsonSerialize(Enable_Scene_Sam,       "f_Enable_Scene_Sam",       j, false);
        JsonSerialize(Enable_Scene_Ism,       "e_Enable_Scene_Ism",      j, false);

        
        JsonSerialize(Enable_Scene_Perception, "g_Enable_Scene_Perception", j, false);
        JsonSerialize(Enable_Change_Cloud_Address, "h_Enable_Change_Cloud_Address", j, false);
        JsonSerialize(Enable_Check_ProtocolVersion, "i_Enable_Check_ProtocolVersion", j, false);

        JsonSerialize(Enable_AngleOffset, "j_Enable_AngleOffset", j, false);
        JsonSerialize(Enable_SpatSrcData, "k_Enable_SpatSrcData", j, false);
        JsonSerialize(Enable_V2xData, "l_Enable_V2xData", j, false);

        JsonSerialize(Enable_TrafficlightDetectData, "m_Enable_TrafficlightDetectData", j, false);

        JsonSerialize(Enable_IllegalEventDetectResult, "n_Enable_IllegalEventDetectResult", j, false);
        JsonSerialize(Enable_TrafficlighDataStatus, "o_Enable_TrafficlighDataStatus", j, false);
        JsonSerialize(Enable_MecSelfCheck, "p_Enable_MecSelfCheck", j, false);
        JsonSerialize(Enable_Camera_InternalExternalParams_Up, "q_Enable_Camera_InternalExternalParams_Up", j, false);
        JsonSerialize(Enable_Camera_InternalExternalParams_Up_Ack, "q_Enable_Camera_InternalExternalParams_Up_Ack", j, false);
        JsonSerialize(Enable_Camera_InternalExternalParams_Query, "q_Enable_Camera_InternalExternalParams_Query", j, false);
        JsonSerialize(Enable_Camera_InternalExternalParams_Query_Down, "q_Enable_Camera_InternalExternalParams_Query_Down", j, false);
        JsonSerialize(Enable_Monitor_Mec_Om_Status, "r_Enable_Monitor_Mec_Om_Status", j, false);
        JsonSerialize(Enable_Spat_Data_Status, "s_Enable_Spat_Data_Status", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(Enable_Use_DB_For_Monitor_Offline,  "A_Enable_Use_DB_For_Monitor_Offline",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_Info_Query,  "A_Enable_Device_Info_Query",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_BaseInfo,    "B_Enable_Device_BaseInfo",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_BaseInfo_Ack,    "B_Enable_Device_BaseInfo_Ack",    j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_Device_BaseInfo_Query_Mec,    "B_Enable_Device_BaseInfo_Query_Mec",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_BaseInfo_Query_Mec_Ack,    "B_Enable_Device_BaseInfo_Query_Mec_Ack",    j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_Device_BaseInfo_Query_Sensor,    "B_Enable_Device_BaseInfo_Query_Sensor",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_BaseInfo_Query_Sensor_Ack,    "B_Enable_Device_BaseInfo_Query_Sensor_Ack",    j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_Config_Query_Down, "C_Enable_Config_Query_Down", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Query_Down_Ack, "D_Enable_Config_Query_Down_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Update,       "E_Enable_Config_Update",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Config_Update_Ack, "F_Enable_Config_Update_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Performence,       "G_Enable_Performence",       j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_Running_Status,    "H_Enable_Running_Status",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Running_Status_Query_Mec,    "H_Enable_Running_Status_Query_Mec",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Running_Status_Query_Mec_ACK,    "H_Enable_Running_Status_Query_Mec_ACK",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Running_Status_Query_Sensor,    "H_Enable_Running_Status_Query_Sensor",    j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Running_Status_Query_Sensor_ACK,    "H_Enable_Running_Status_Query_Sensor_ACK",    j, noUse_isEmptyFlag);


        JsonDeserialize(Enable_Alarm,             "I_Enable_Alarm",             j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Alarm_Monitor,      "I_Enable_Alarm_Monitor",             j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_HeartBeat,  "J_Enable_Device_HeartBeat",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_Down,         "K_Enable_Ota_Down",         j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_Status,       "L_Enable_Ota_Status",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_Version,     "M_Enable_Ota_Version",     j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Ota_Cancel,     "M_Enable_Ota_Cancel",     j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Reboot,          "N_Enable_Reboot",          j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Reboot_Ack,      "O_Enable_Reboot_Ack",      j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Timing,           "P_Enable_Timing",           j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Signal,           "Q_Enable_Signal",           j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_Scene_Rsi,       "U_Enable_Scene_Rsi",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Rsc,       "V_Enable_Scene_Rsc",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Ssm,       "W_Enable_Scene_Ssm",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Rtcm,      "X_Enable_Scene_Rtcm",      j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Bsm,       "Y_Enable_Scene_Bsm",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Vir,       "Z_Enable_Scene_Vir",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Pam,       "a_Enable_Scene_Pam",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Map,       "b_Enable_Scene_Map",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Spat,      "c_Enable_Scene_Spat",      j, noUse_isEmptyFlag);
        
        JsonDeserialize(Enable_Scene_Rsm,       "d_Enable_Scene_Map",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Sam,       "f_Enable_Scene_Sam",       j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Scene_Ism,       "e_Enable_Scene_Ism",      j, noUse_isEmptyFlag);
        
        JsonDeserialize(Enable_Scene_Perception, "g_Enable_Scene_Perception", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Change_Cloud_Address, "h_Enable_Change_Cloud_Address", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Check_ProtocolVersion, "i_Enable_Check_ProtocolVersion", j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_AngleOffset, "j_Enable_AngleOffset", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_SpatSrcData, "k_Enable_SpatSrcData", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_V2xData, "l_Enable_V2xData", j, noUse_isEmptyFlag);

        JsonDeserialize(Enable_TrafficlightDetectData, "m_Enable_TrafficlightDetectData", j, noUse_isEmptyFlag);


        JsonDeserialize(Enable_IllegalEventDetectResult, "n_Enable_IllegalEventDetectResult", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_TrafficlighDataStatus, "o_Enable_TrafficlighDataStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MecSelfCheck, "p_Enable_MecSelfCheck", j, noUse_isEmptyFlag);


        JsonDeserialize(Enable_Camera_InternalExternalParams_Up, "q_Enable_Camera_InternalExternalParams_Up", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Camera_InternalExternalParams_Up_Ack, "q_Enable_Camera_InternalExternalParams_Up_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Camera_InternalExternalParams_Query, "q_Enable_Camera_InternalExternalParams_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Camera_InternalExternalParams_Query_Down, "q_Enable_Camera_InternalExternalParams_Query_Down", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Monitor_Mec_Om_Status, "r_Enable_Monitor_Mec_Om_Status", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Spat_Data_Status, "s_Enable_Spat_Data_Status", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_Info_Query: " << (Enable_Device_Info_Query ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo: " << (Enable_Device_BaseInfo ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo_Ack: " << (Enable_Device_BaseInfo_Ack ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo_Query_Mec: " << (Enable_Device_BaseInfo_Query_Mec ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo_Query_Mec_Ack: " << (Enable_Device_BaseInfo_Query_Mec_Ack ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo_Query_Sensor: " << (Enable_Device_BaseInfo_Query_Sensor ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_BaseInfo_Query_Sensor_Ack: " << (Enable_Device_BaseInfo_Query_Sensor_Ack ? "True" : "False") <<  std::endl;


        ss << std::left << std::setw(40) << "Enable_Config_Query_Down: " << (Enable_Config_Query_Down ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Config_Query_Down_Ack: " << (Enable_Config_Query_Down_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Config_Update: " << (Enable_Config_Update ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Config_Update_Ack: " << (Enable_Config_Update_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Performence: " << (Enable_Performence ? "True" : "False") <<  std::endl;


        ss << std::left << std::setw(40) << "Enable_Running_Status: " << (Enable_Running_Status ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Running_Status_Query_Mec: " << (Enable_Running_Status_Query_Mec ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Running_Status_Query_Mec_ACK: " << (Enable_Running_Status_Query_Mec_ACK ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Running_Status_Query_Sensor: " << (Enable_Running_Status_Query_Sensor ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Running_Status_Query_Sensor_ACK: " << (Enable_Running_Status_Query_Sensor_ACK ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Alarm: " << (Enable_Alarm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Alarm_Monitor: " << (Enable_Alarm_Monitor ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Device_HeartBeat: " << (Enable_Device_HeartBeat ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Ota_Down: " << (Enable_Ota_Down ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Ota_Status: " << (Enable_Ota_Status ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Ota_Version: " << (Enable_Ota_Version ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Ota_Cancel: " << (Enable_Ota_Cancel ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Reboot: " << (Enable_Reboot ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Reboot_Ack: " << (Enable_Reboot_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Timing: " << (Enable_Timing ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Signal: " << (Enable_Signal ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Scene_Rsi: " << (Enable_Scene_Rsi ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Rsc: " << (Enable_Scene_Rsc ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Ssm: " << (Enable_Scene_Ssm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Rtcm: " << (Enable_Scene_Rtcm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Bsm: " << (Enable_Scene_Bsm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Vir: " << (Enable_Scene_Vir ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Pam: " << (Enable_Scene_Pam ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Map: " << (Enable_Scene_Map ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Spat: " << (Enable_Scene_Spat ? "True" : "False") <<  std::endl;
        
        ss << std::left << std::setw(40) << "Enable_Scene_Rsm: " << (Enable_Scene_Rsm ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Sam: " << (Enable_Scene_Sam ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Scene_Ism: " << (Enable_Scene_Ism ? "True" : "False") <<  std::endl;
        
        ss << std::left << std::setw(40) << "Enable_Scene_Perception: " << (Enable_Scene_Perception ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Change_Cloud_Address: " << (Enable_Change_Cloud_Address ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Check_ProtocolVersion: " << (Enable_Check_ProtocolVersion ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_AngleOffset: " << (Enable_AngleOffset ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_SpatSrcData: " << (Enable_SpatSrcData ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_V2xData: " << (Enable_V2xData ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_TrafficlightDetectData: " << (Enable_TrafficlightDetectData ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_IllegalEventDetectResult: " << (Enable_TrafficlightDetectData ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_TrafficlighDataStatus: " << (Enable_TrafficlightDetectData ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_MecSelfCheck: " << (Enable_TrafficlightDetectData ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Camera_InternalExternalParams_Up: " << (Enable_Camera_InternalExternalParams_Up ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Camera_InternalExternalParams_Up_Ack: " << (Enable_Camera_InternalExternalParams_Up_Ack ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Camera_InternalExternalParams_Query: " << (Enable_Camera_InternalExternalParams_Query ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Camera_InternalExternalParams_Query_Down: " << (Enable_Camera_InternalExternalParams_Query_Down ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Monitor_Mec_Om_Status: " << (Enable_Monitor_Mec_Om_Status ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_Spat_Data_Status: " << (Enable_Spat_Data_Status ? "True" : "False") <<  std::endl;

        return ss.str();
    }
};


struct ScenarioEnableConfigerMec : public afl::base::SerializableData
{
public:
    bool                            enableDebugPrintV2xDataBroad = false;
    bool                            enableDebugPrintV2xDataReceive = false;
    bool                            enableDebugPrintSpatSrcData = false;

    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(enableDebugPrintV2xDataBroad,  "A_enableDebugPrintV2xDataBroad",  j, false);
        JsonSerialize(enableDebugPrintV2xDataReceive,    "B_enableDebugPrintV2xDataReceive",    j, false);
        JsonSerialize(enableDebugPrintSpatSrcData,    "C_enableDebugPrintSpatSrcData",    j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {;
        JsonDeserialize(enableDebugPrintV2xDataBroad,  "A_enableDebugPrintV2xDataBroad",  j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintV2xDataReceive,    "B_enableDebugPrintV2xDataReceive",    j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintSpatSrcData,    "C_enableDebugPrintSpatSrcData",    j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintV2xDataBroad: " << (enableDebugPrintV2xDataBroad ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintV2xDataReceive: " << (enableDebugPrintV2xDataReceive ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintSpatSrcData: " << (enableDebugPrintSpatSrcData ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};


struct SensorInterExterParamEnablePeriodConfiger : public afl::base::SerializableData
{
public:
    bool                            enablePeriodUpCameraInterExterParam = true;  //周期推送相机内外参
    bool                            enablePeriodUpRadarInterExterParam = true; //是否

    bool                            enablePeriodQueryCameraInterExterParam = true;  //周期查询相机内外参数
    bool                            enablePeriodQueryRadarInterExterParam = true;

    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(enablePeriodUpCameraInterExterParam,  "A_enable_Period_Up_Camera_InterExterParam",  j, false);
        JsonSerialize(enablePeriodQueryCameraInterExterParam,    "B_enable_Period_Query_CameraInterExterParam",    j, false);
        JsonSerialize(enablePeriodUpRadarInterExterParam,    "C_enable_Period_Up_Radar_InterExterParam",    j, false);
        JsonSerialize(enablePeriodQueryRadarInterExterParam,    "D_enable_Period_Query_RadarInterExterParam",    j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {;
        JsonDeserialize(enablePeriodUpCameraInterExterParam,  "A_enable_Period_Up_Camera_InterExterParam",  j, noUse_isEmptyFlag);
        JsonDeserialize(enablePeriodQueryCameraInterExterParam,    "B_enable_Period_Query_CameraInterExterParam",    j, noUse_isEmptyFlag);
        JsonDeserialize(enablePeriodUpRadarInterExterParam,    "C_enable_Period_Up_Radar_InterExterParam",    j, noUse_isEmptyFlag);
        JsonDeserialize(enablePeriodQueryRadarInterExterParam,    "D_enable_Period_Query_RadarInterExterParam",    j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "enablePeriodUpCameraInterExterParam: " << (enablePeriodUpCameraInterExterParam ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enablePeriodQueryCameraInterExterParam: " << (enablePeriodQueryCameraInterExterParam ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enablePeriodUpRadarInterExterParam: " << (enablePeriodUpRadarInterExterParam ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "enablePeriodQueryRadarInterExterParam: " << (enablePeriodQueryRadarInterExterParam ? "True" : "False") <<  std::endl;
        return ss.str();
    }
                    };
NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif //AIROS2_0_CONFIGER_MEC_CLOUD_CAMERA_TOPIC_H
