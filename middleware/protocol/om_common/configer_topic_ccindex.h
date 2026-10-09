/*********************************************************************************
 * @file		configer_topic_ccindex.h
 * @brief		configer_topic_ccindex belongs to CICTCI
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

#ifndef AIROS2_0_CONFIGER_TOPIC_CCINDEX_H
#define AIROS2_0_CONFIGER_TOPIC_CCINDEX_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_RADAR_CCINDEX
using namespace os::v2x::protocol::om;
using namespace afl::base;

struct MqttTopicConfigerRadarID : public afl::base::SerializableData
{
    std::string cross_id = "";
    std::string vendor = "";
    std::string category = ""; //废弃不使用
    std::vector<std::string> radarIDs;

private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(cross_id, "A_cross_id", j, false);
        JsonSerialize(vendor, "A_vendor", j, false);
        JsonSerialize(category, "A_category", j, false);
        JsonSerialize(radarIDs, "B_radarIDs", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(cross_id, "A_cross_id", j, noUse_isEmptyFlag);
        JsonDeserialize(vendor, "A_vendor", j, noUse_isEmptyFlag);
        JsonDeserialize(category, "A_category", j, noUse_isEmptyFlag);
        JsonDeserialize(radarIDs, "B_radarIDs", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_cross_id: " << cross_id;
        ss << std::left << std::setw(40) << "A_vendor: " << vendor;
        ss << std::left << std::setw(40) << "A_category: " << category;
        ss << std::left << std::setw(40) << "B_radarIDs: ";
        for (const auto& id : radarIDs) {
            ss << id << " ";
        }
        ss <<  std::endl;
        return ss.str();
    }
};

struct MqttTopicConfigerCCIndex : public afl::base::SerializableData
{
    std::string Topic_Tc            = "upload/tc/";         //信控指标前缀
    std::string Topic_Profix        = "trafficMetrics/";  //上行topic前缀
    std::string Topic_Postfix       = "/RADAR/merit/";  //上行topic后缀 废弃不用
    std::string Topic_Trajectories  = "trajectories/"; // mec向平台发送 实时轨迹数据
    std::string Topic_VehiclePass   = "vehiclePass/";   // mec向平台发送 实时过车数据
    std::string Topic_QueueUp       = "queueUp/";       // mec向平台发送 实时排队数据
    std::string Topic_AreaState     = "areaState/";// mec向平台发送 实时区域状态数据
    std::string Topic_Overflow      = "overflow/";// mec向平台发送 实时溢出数据
    std::string Topic_Outlane       = "outlane/";// mec向平台发送 实时出口通道数据
    std::string Topic_Statistics    = "statistics/";// mec向平台发送 定时统计数据
    std::string Topic_Evaluations   = "evaluations/";// mec向平台发送 定时评价数据
    std::string Topic_Nonmotor      = "nonmotor/";// mec向平台发送 定时评价数据
    std::string Topic_DeviceStatus = "deviceStatus/";// mec向平台发送 设备状态数据
    std::string Topic_Pulse         = "pulse/";   // mec向平台发送, 实时脉冲数据
    std::string Topic_SingleStatistics = "SingleStatistics/";   // mec向平台发送, 实时统计数据(信号机A13)

    std::string Topic_Static_Profix = "static/device/config/";
    std::string Topic_Static_Query = "query/";
    std::string Topic_Static_Query_Ack = "query/ack/";

    std::string Topic_Static_Update = "update/";
    std::string Topic_Static_Update_Ack = "update/ack/";

    std::string Topic_Test_TrafficMetrics_Count_Clean = "test/trafficMetrics/count/clean";
    std::string Topic_Test_Tc_Count_Clean = "test/tc/count/clean";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Topic_Tc, "A_Topic_Tc", j, false);
        JsonSerialize(Topic_Profix, "B_Topic_Profix", j, false);
        JsonSerialize(Topic_Postfix, "C_Topic_Postfix", j, false);
        JsonSerialize(Topic_Trajectories, "D_Topic_Trajectories", j, false);
        JsonSerialize(Topic_VehiclePass, "E_Topic_VehiclePass", j, false);
        JsonSerialize(Topic_QueueUp, "F_Topic_QueueUp", j, false);
        JsonSerialize(Topic_AreaState, "G_Topic_AreaState", j, false);
        JsonSerialize(Topic_Overflow, "H_Topic_Overflow", j, false);
        JsonSerialize(Topic_Outlane, "I_Topic_Outlane", j, false);
        JsonSerialize(Topic_Statistics, "J_Topic_Statistics", j, false);
        JsonSerialize(Topic_Evaluations, "K_Topic_Evaluations", j, false);
        JsonSerialize(Topic_Nonmotor, "L_Topic_Nonmotor", j, false);
        JsonSerialize(Topic_DeviceStatus, "M_Topic_DeviceStatus", j, false);
        JsonSerialize(Topic_Pulse, "M_Topic_Pulse", j, false);
        JsonSerialize(Topic_SingleStatistics, "T_Topic_SingleStatistics", j, false);

        JsonSerialize(Topic_Static_Profix, "N_Topic_Static_Profix", j, false);
        JsonSerialize(Topic_Static_Query, "N_Topic_Static_Query", j, false);
        JsonSerialize(Topic_Static_Query_Ack, "O_Topic_Static_Query_Ack", j, false);
        JsonSerialize(Topic_Static_Update, "P_Topic_Static_Update", j, false);
        JsonSerialize(Topic_Static_Update_Ack, "Q_Topic_Static_Update_Ack", j, false);

        JsonSerialize(Topic_Test_TrafficMetrics_Count_Clean, "R_Topic_Test_TrafficMetrics_Count_Clean", j, false);
        JsonSerialize(Topic_Static_Update_Ack, "S_Topic_Static_Update_Ack", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Topic_Tc, "A_Topic_Tc", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Profix, "B_Topic_Profix", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Postfix, "C_Topic_Postfix", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Trajectories, "D_Topic_Trajectories", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_VehiclePass, "E_Topic_VehiclePass", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_QueueUp, "F_Topic_QueueUp", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_AreaState, "G_Topic_AreaState", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Overflow, "H_Topic_Overflow", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Outlane, "I_Topic_Outlane", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Statistics, "J_Topic_Statistics", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Evaluations, "K_Topic_Evaluations", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Nonmotor, "L_Topic_Nonmotor", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_DeviceStatus, "M_Topic_DeviceStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Pulse, "M_Topic_Pulse", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_SingleStatistics, "T_Topic_SingleStatistics", j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_Static_Profix, "N_Topic_Static_Profix", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Static_Query, "N_Topic_Static_Query", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Static_Query_Ack, "O_Topic_Static_Query_Ack", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Static_Update, "P_Topic_Static_Update", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Static_Update_Ack, "Q_Topic_Static_Update_Ack", j, noUse_isEmptyFlag);


        JsonDeserialize(Topic_Test_TrafficMetrics_Count_Clean, "R_Topic_Test_TrafficMetrics_Count_Clean", j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Static_Update_Ack, "S_Topic_Static_Update_Ack", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Topic_Tc: " << Topic_Tc <<  std::endl;
        ss << std::left << std::setw(40) << "B_Topic_Profix: " << Topic_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "C_Topic_Postfix: " << Topic_Postfix <<  std::endl;
        ss << std::left << std::setw(40) << "D_Topic_Trajectories: " << Topic_Trajectories <<  std::endl;
        ss << std::left << std::setw(40) << "E_Topic_VehiclePass: " << Topic_VehiclePass <<  std::endl;
        ss << std::left << std::setw(40) << "F_Topic_QueueUp: " << Topic_QueueUp <<  std::endl;
        ss << std::left << std::setw(40) << "G_Topic_AreaState: " << Topic_AreaState <<  std::endl;
        ss << std::left << std::setw(40) << "H_Topic_Overflow: " << Topic_Overflow <<  std::endl;
        ss << std::left << std::setw(40) << "I_Topic_Outlane: " << Topic_Outlane <<  std::endl;
        ss << std::left << std::setw(40) << "J_Topic_Statistics: " << Topic_Statistics <<  std::endl;
        ss << std::left << std::setw(40) << "K_Topic_Evaluations: " << Topic_Evaluations <<  std::endl;
        ss << std::left << std::setw(40) << "L_Topic_Nonmotor: " << Topic_Nonmotor <<  std::endl;
        ss << std::left << std::setw(40) << "M_Topic_DeviceStatus: " << Topic_DeviceStatus <<  std::endl;
        ss << std::left << std::setw(40) << "M_Topic_Pulse: " << Topic_Pulse <<  std::endl;
        ss << std::left << std::setw(40) << "T_Topic_SingleStatistics: " << Topic_SingleStatistics <<  std::endl;

        ss << std::left << std::setw(40) << "N_Topic_Static_Profix: " << Topic_Static_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "N_Topic_Static_Query: " << Topic_Static_Query <<  std::endl;
        ss << std::left << std::setw(40) << "O_Topic_Static_Query_Ack: " << Topic_Static_Query_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "P_Topic_Static_Update: " << Topic_Static_Update <<  std::endl;
        ss << std::left << std::setw(40) << "Q_Topic_Static_Update_Ack: " << Topic_Static_Update_Ack <<  std::endl;

        ss << std::left << std::setw(40) << "R_Topic_Test_TrafficMetrics_Count_Clean: " << Topic_Static_Update_Ack <<  std::endl;
        ss << std::left << std::setw(40) << "S_Topic_Static_Update_Ack: " << Topic_Static_Update_Ack <<  std::endl;
        return ss.str();
    }
};

struct MqttEnableConfigerCCIndex : public afl::base::SerializableData
{
public:
    bool Enable_Tc = true;
    bool Enable_TrafficMetrics = true;
    bool Enable_Static = true;
    bool Enable_Trajectories_Count = true;
    bool Enable_Trajectories_Tsmtc = true;
    bool Enable_Trajectories_Desaysv = true;
    bool Enable_Trajectories = true;
    bool Enable_VehiclePass = true;
    bool Enable_QueueUp = true;
    bool Enable_AreaState = true;
    bool Enable_Overflow = true;
    bool Enable_Outlane = true;
    bool Enable_Statistics = true;
    bool Enable_Evaluations = true;
    bool Enable_Nonmotor = true;
    bool Enable_Device_Status = true;
    bool Enable_Pulse = true;
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(Enable_Tc, "A_Enable_Tc", j, false);
        JsonSerialize(Enable_TrafficMetrics, "A_Enable_TrafficMetrics", j, false);
        JsonSerialize(Enable_Static, "A_Enable_Static", j, false);
        JsonSerialize(Enable_Trajectories_Count, "B_Enable_Trajectories_Count", j, false);
        JsonSerialize(Enable_Trajectories_Tsmtc, "B_Enable_Trajectories_Tsmtc", j, false);
        JsonSerialize(Enable_Trajectories_Desaysv, "B_Enable_Trajectories_Desaysv", j, false);
        JsonSerialize(Enable_Trajectories, "B_Enable_Trajectories", j, false);
        JsonSerialize(Enable_VehiclePass, "C_Enable_VehiclePass", j, false);
        JsonSerialize(Enable_QueueUp, "D_Enable_QueueUp", j, false);
        JsonSerialize(Enable_AreaState, "E_Enable_AreaState", j, false);
        JsonSerialize(Enable_Overflow, "F_Enable_Overflow", j, false);
        JsonSerialize(Enable_Outlane, "G_Enable_Outlane", j, false);
        JsonSerialize(Enable_Statistics, "H_Enable_Statistics", j, false);
        JsonSerialize(Enable_Evaluations, "I_Enable_Evaluations", j, false);
        JsonSerialize(Enable_Nonmotor, "J_Enable_Nonmotor", j, false);
        JsonSerialize(Enable_Device_Status, "K_Enable_Device_Status", j, false);
        JsonSerialize(Enable_Pulse, "L_Enable_Pulse", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(Enable_Tc, "A_Enable_Tc", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_TrafficMetrics, "A_Enable_TrafficMetrics", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Static, "A_Enable_Static", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Trajectories_Count, "B_Enable_Trajectories_Count", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Trajectories, "B_Enable_Trajectories", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Trajectories_Tsmtc, "B_Enable_Trajectories_Tsmtc", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Trajectories_Desaysv, "B_Enable_Trajectories_Desaysv", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_VehiclePass, "C_Enable_VehiclePass", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_QueueUp, "D_Enable_QueueUp", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_AreaState, "E_Enable_AreaState", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Overflow, "F_Enable_Overflow", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Outlane, "G_Enable_Outlane", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Statistics, "H_Enable_Statistics", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Evaluations, "I_Enable_Evaluations", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Nonmotor, "J_Enable_Nonmotor", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Device_Status, "K_Enable_Device_Status", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Pulse, "L_Enable_Pulse", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Tc: " << (Enable_Tc ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "A_Enable_TrafficMetrics: " << (Enable_TrafficMetrics ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Static: " << (Enable_Static ? "True" : "False") <<  std::endl;

        ss << std::left << std::setw(40) << "B_Enable_Trajectories_Count: " << (Enable_Trajectories_Count ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "B_Enable_Trajectories: " << (Enable_Trajectories ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Trajectories_Tsmtc: " << (Enable_Trajectories_Tsmtc ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Trajectories_Desaysv: " << (Enable_Trajectories_Desaysv ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "D_Enable_QueueUp: " << (Enable_QueueUp ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "E_Enable_AreaState: " << (Enable_AreaState ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "F_Enable_Overflow: " << (Enable_Overflow ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "G_Enable_Outlane: " << (Enable_Outlane ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "H_Enable_Statistics: " << (Enable_Statistics ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "I_Enable_Evaluations: " << (Enable_Evaluations ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "J_Enable_Nonmotor: " << (Enable_Nonmotor ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "K_Enable_Device_Status: " << (Enable_Device_Status ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "L_Enable_Pulse: " << (Enable_Pulse ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_RADAR_CCINDEX
#endif //AIROS2_0_CONFIGER_MEC_CLOUD_CAMERA_TOPIC_H
