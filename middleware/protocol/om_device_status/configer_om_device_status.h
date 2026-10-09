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

#ifndef AIROS2_0_CONFIGER_OM_DEVICE_STAUTS_H
#define AIROS2_0_CONFIGER_OM_DEVICE_STAUTS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_radar.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "data_model/data_syschronize.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_db_device_status.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "base/common/network/net_util.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/om_common/data_model/camera_device_angle_offset_log_data.h"
#include "middleware/protocol/om_common/data_model/data_performence.h"
#include "middleware/protocol/om_common/db_utils.h"
#include "data_model/data_register.h"
#include "base/common/network/net_util.h"
#include "middleware/protocol/om_common/data_model/data_alarm.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
using namespace airos::base::workparam;
using namespace afl::net;
using namespace os::v2x::protocol;
using namespace os::v2x::protocol::om::common;
using namespace os::v2x::protocol::om::radar;
using namespace os::v2x::protocol::om::camera;
struct EnableConfiger : public afl::base::SerializableData
{
public:
    bool Enable_Save_2DB = true;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Enable_Save_2DB, "A_Enable_Save_2DB", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Enable_Save_2DB, "A_Enable_Save_2DB", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Enable_Save_2DB: " << (Enable_Save_2DB ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};
//项目路径配置
struct ProjectPathConfiger : public afl::base::SerializableData
{
public:
    std::string projectRoot = "/home/airos/protocol/ds";
    std::string tlsDirName = "ssl";
    std::string tlsCAFileName = "ca.crt";
    std::string tlsClientKeyFileName = "client.crt";
    std::string tlsClientPrivateKeyFileName = "client.key";
    std::string tlsClientPrivateKeyPassword = "123456";
    std::string tlsCAFilePath;
    std::string tlsClientKeyFilePath;
    std::string tlsClientPrivateKeyFilePath;

private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(projectRoot, "A_projectRoot", j, false);
        JsonSerialize(tlsDirName, "B_tlsDirName", j, false);
        JsonSerialize(tlsCAFileName, "C_tlsCAFileName", j, false);
        JsonSerialize(tlsClientKeyFileName, "D_tlsClientKeyFileName", j, false);
        JsonSerialize(tlsClientPrivateKeyFileName, "E_tlsClientPrivateKeyFileName", j, false);
        JsonSerialize(tlsClientPrivateKeyPassword, "F_tlsClientPrivateKeyPassword", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(projectRoot, "A_projectRoot", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsDirName, "B_tlsDirName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsCAFileName, "C_tlsCAFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientKeyFileName, "D_tlsClientKeyFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientPrivateKeyFileName, "E_tlsClientPrivateKeyFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientPrivateKeyPassword, "F_tlsClientPrivateKeyPassword", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_projectRoot: " << projectRoot <<  std::endl;
        ss << std::left << std::setw(40) << "B_tlsDirName: " << tlsDirName <<  std::endl;
        ss << std::left << std::setw(40) << "C_tlsCAFileName: " << tlsCAFileName <<  std::endl;
        ss << std::left << std::setw(40) << "D_tlsClientKeyFileName: " << tlsClientKeyFileName <<  std::endl;
        ss << std::left << std::setw(40) << "E_tlsClientPrivateKeyFileName: " << tlsClientPrivateKeyFileName <<  std::endl;
        ss << std::left << std::setw(40) << "F_tlsClientPrivateKeyPassword: " << tlsClientPrivateKeyPassword <<  std::endl;
        return ss.str();
    }
};

struct MqttTopicConfiger : public afl::base::SerializableData
{
    std::string Topic_Camera_Profix = "camera/";  //上行topic前缀
    std::string Topic_Camera_Keepalive = "/keep-alive";//  mec->云控 心跳信息
    int         Topic_Camera_Keepalive_Counter = 0;
    std::string Topic_Camera_Synchronize = "/synchronize";
    int         Topic_Camera_Synchronize_Counter = 0;
    std::string Topic_Camera_Register = "/register";// mec向摄像头设备推送 配置查询
    int         Topic_Camera_Register_Counter = 0;
    ////////////////////////////////////////////////////

    std::string Topic_Radar_Profix = "radar/";  //下行topic前缀
    std::string Topic_Radar_Heartbeat = "/heartbeat";// 雷达->mec 心跳信息
    int         Topic_Radar_Heartbeat_Counter = 0 ;
    std::string Topic_Radar_Synchronize = "/synchronize";// 雷达->mec 设备授时
    int         Topic_Radar_Synchronize_Counter = 0;
    std::string Topic_Radar_Register = "/register";//  mec—>雷达设备 配置信息
    int         Topic_Radar_Register_Counter = 0;

    std::string Topic_Bs_Inter_Profix           = "om_mec2om_device_status/";
    std::string Topic_Timing_Alarm_Inter     = "timing";
    std::string Topic_AngleOffset_Alarm_Inter = "angle-offset/up";

    std::string Topic_Camera_Alarm = "/alarm";//  摄像机告警
    std::string Topic_Radar_Alarm = "/upload/warning";// 雷达告警


private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Topic_Camera_Profix,         "A_Topic_Camera_Profix",         j, false);
        JsonSerialize(Topic_Camera_Keepalive,      "B_Topic_Camera_Keepalive",      j, false);
        JsonSerialize(Topic_Camera_Synchronize,    "C_Topic_Camera_Synchronize",    j, false);
        JsonSerialize(Topic_Camera_Register,   "D_Topic_Camera_Config_Query",   j, false);

        JsonSerialize(Topic_Radar_Profix,          "E_Topic_Radar_Profix",          j, false);
        JsonSerialize(Topic_Radar_Heartbeat,       "F_Topic_Radar_Heartbeat",       j, false);
        JsonSerialize(Topic_Radar_Synchronize,     "G_Topic_Radar_Synchronize",     j, false);
        JsonSerialize(Topic_Radar_Register,    "H_Topic_Radar_Register",    j, false);
        JsonSerialize(Topic_Bs_Inter_Profix,            "I_Topic_Bs_Inter_Profix",            j, false);
        JsonSerialize(Topic_Timing_Alarm_Inter,            "J_Topic_Timing_Alarm_Inter",            j, false);
        JsonSerialize(Topic_AngleOffset_Alarm_Inter,            "K_Topic_AngleOffset_Alarm_Inter",            j, false);

        JsonSerialize(Topic_Camera_Alarm,            "L_Topic_Camera_Alarm",            j, false);
        JsonSerialize(Topic_Radar_Alarm,            "M_Topic_Radar_Alarm",            j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Topic_Camera_Profix,         "A_Topic_Camera_Profix",         j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_Keepalive,      "B_Topic_Camera_Keepalive",      j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_Synchronize,    "C_Topic_Camera_Synchronize",    j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Camera_Register,   "D_Topic_Camera_Config_Query",   j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Radar_Profix,          "E_Topic_Radar_Profix",          j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Radar_Heartbeat,       "F_Topic_Radar_Heartbeat",       j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Radar_Synchronize,     "G_Topic_Radar_Synchronize",     j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Radar_Register,    "H_Topic_Radar_Register",    j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_Bs_Inter_Profix,            "I_Topic_Bs_Inter_Profix",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Timing_Alarm_Inter,            "J_Topic_Timing_Alarm_Inter",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_AngleOffset_Alarm_Inter,            "K_Topic_AngleOffset_Alarm_Inter",            j, noUse_isEmptyFlag);

        JsonDeserialize(Topic_Camera_Alarm,            "L_Topic_Camera_Alarm",            j, noUse_isEmptyFlag);
        JsonDeserialize(Topic_Radar_Alarm,            "M_Topic_Radar_Alarm",            j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_Topic_Camera_Profix: " << Topic_Camera_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "B_Topic_Camera_Keepalive: " << Topic_Camera_Keepalive <<  std::endl;
        ss << std::left << std::setw(40) << "C_Topic_Camera_Synchronize: " << Topic_Camera_Synchronize <<  std::endl;
        ss << std::left << std::setw(40) << "D_Topic_Camera_Config_Query: " << Topic_Camera_Register <<  std::endl;
        ss << std::left << std::setw(40) << "E_Topic_Radar_Profix: " << Topic_Radar_Profix <<  std::endl;
        ss << std::left << std::setw(40) << "F_Topic_Radar_Heartbeat: " << Topic_Radar_Heartbeat <<  std::endl;
        ss << std::left << std::setw(40) << "G_Topic_Radar_Synchronize: " << Topic_Radar_Synchronize <<  std::endl;
        ss << std::left << std::setw(40) << "H_Topic_Radar_Register: " << Topic_Radar_Synchronize <<  std::endl;
        ss << std::left << std::setw(40) << "Topic_Bs_Inter_Profix: "           << Topic_Bs_Inter_Profix          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Timing_Alarm_Inter: "           << Topic_Timing_Alarm_Inter          << std::endl;
        ss << std::left << std::setw(40) << "Topic_AngleOffset_Alarm_Inter: "           << Topic_AngleOffset_Alarm_Inter          << std::endl;

        ss << std::left << std::setw(40) << "Topic_Camera_Alarm: "           << Topic_Camera_Alarm          << std::endl;
        ss << std::left << std::setw(40) << "Topic_Radar_Alarm: "           << Topic_Radar_Alarm          << std::endl;
        return ss.str();
    }
};
struct MqttTopicConfigerMonitorPeriod: public afl::base::SerializableData
{
public:
    double 	periodMonitorCameraHeartBeat 	= 61.0;
    double 	periodMonitorCameraSynchronize 	= 61.0;
    double 	periodMonitorRadarHeartBeat 	= 61.0;
    double 	periodMonitorRadarSynchronize 	= 61.0;
    double  periodMonitorSensorDeviceActive = 100;
    double  periodMonitorAlarm = 300;
    double  periodAlarmPublishInterval = 600;
    double  counterRadarHeartbeat = 200;
    double  counterRadarSynchronize = 200;
    double  counterCameraHeartbeat = 200;
    double  counterCameraSynchronize = 200;

    double 	periodMonitorCameraAlarm 	= 61.0;
    double 	periodMonitorRadarAlarm 	= 61.0;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(periodMonitorCameraHeartBeat, "A_PeriodMonitorCameraHeartBeat", j, false);
        JsonSerialize(periodMonitorCameraSynchronize, "B_PeriodMonitorCameraSynchronize", j, false);
        JsonSerialize(periodMonitorRadarHeartBeat, "C_PeriodMonitorRadarHeartBeat", j, false);
        JsonSerialize(periodMonitorRadarSynchronize, "D_PeriodMonitorRadarSynchronize", j, false);
        JsonSerialize(periodMonitorSensorDeviceActive, "E_periodMonitorSensorDeviceActive", j, false);
        JsonSerialize(periodMonitorAlarm, "F_periodMonitorAlarm", j, false);
        JsonSerialize(periodAlarmPublishInterval, "G_periodAlarmPublishInterval", j, false);

        JsonSerialize(counterRadarHeartbeat, "H_counterRadarHeartbeat", j, false);
        JsonSerialize(counterRadarSynchronize, "H_counterRadarSynchronize", j, false);
        JsonSerialize(counterCameraHeartbeat, "I_counterCameraHeartbeat", j, false);
        JsonSerialize(counterCameraSynchronize, "I_counterCameraSynchronize", j, false);

        JsonSerialize(periodMonitorCameraAlarm, "J_periodMonitorCameraAlarm", j, false);
        JsonSerialize(periodMonitorRadarAlarm, "K_periodMonitorRadarAlarm", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(periodMonitorCameraHeartBeat, "A_PeriodMonitorCameraHeartBeat", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorCameraSynchronize, "B_PeriodMonitorCameraSynchronize", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorRadarHeartBeat, "C_PeriodMonitorRadarHeartBeat", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorRadarSynchronize, "D_PeriodMonitorRadarSynchronize", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorSensorDeviceActive, "E_periodMonitorSensorDeviceActive", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorAlarm, "F_periodMonitorAlarm", j, noUse_isEmptyFlag);
        JsonDeserialize(periodAlarmPublishInterval, "G_periodAlarmPublishInterval", j, noUse_isEmptyFlag);

        JsonDeserialize(counterRadarHeartbeat, "H_counterRadarHeartbeat", j, noUse_isEmptyFlag);
        JsonDeserialize(counterRadarSynchronize, "H_counterRadarSynchronize", j, noUse_isEmptyFlag);
        JsonDeserialize(counterCameraHeartbeat, "I_counterCameraHeartbeat", j, noUse_isEmptyFlag);
        JsonDeserialize(counterCameraSynchronize, "I_counterCameraSynchronize", j, noUse_isEmptyFlag);

        JsonDeserialize(periodMonitorCameraAlarm, "J_periodMonitorCameraAlarm", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorRadarAlarm, "K_periodMonitorRadarAlarm", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_PeriodMonitorCameraHeartBeat: " << periodMonitorCameraHeartBeat <<  std::endl;
        ss << std::left << std::setw(40) << "B_PeriodMonitorCameraSynchronize: " << periodMonitorCameraSynchronize <<  std::endl;
        ss << std::left << std::setw(40) << "C_PeriodMonitorRadarHeartBeat: " << periodMonitorRadarHeartBeat <<  std::endl;
        ss << std::left << std::setw(40) << "D_PeriodMonitorRadarSynchronize: " << periodMonitorRadarSynchronize <<  std::endl;
        ss << std::left << std::setw(40) << "E_periodMonitorSensorDeviceActive: " << periodMonitorSensorDeviceActive <<  std::endl;
        ss << std::left << std::setw(40) << "F_periodMonitorAlarm: " << periodMonitorAlarm <<  std::endl;
        ss << std::left << std::setw(40) << "G_periodAlarmPublishInterval: " << periodAlarmPublishInterval <<  std::endl;
        ss << std::left << std::setw(40) << "H_counterRadarHeartbeat: " << counterRadarHeartbeat <<  std::endl;
        ss << std::left << std::setw(40) << "H_counterRadarSynchronize: " << counterRadarSynchronize <<  std::endl;
        ss << std::left << std::setw(40) << "I_counterCameraHeartbeat: " << counterCameraHeartbeat <<  std::endl;
        ss << std::left << std::setw(40) << "I_counterCameraSynchronize: " << counterCameraSynchronize <<  std::endl;

        ss << std::left << std::setw(40) << "J_periodMonitorCameraAlarm: " << periodMonitorRadarAlarm <<  std::endl;
        ss << std::left << std::setw(40) << "K_periodMonitorRadarAlarm: " << periodMonitorRadarAlarm <<  std::endl;
        return ss.str();
    }
};

//rsu客户端配置
struct MqttClientConfigerDeviceStatus : public ConfigerData<MqttClientConfigerDeviceStatus>
{
    bool                                enableDebugPrint = true;
    std::string                         nic;
    std::string                         rscuEsn;
    std::string                         mqttClientId = "om-device-status";
    MqttTopicConfigerCameraRcId         configerTopicCameraRcId;
    MqttTopicConfigerRadarID            configerTopicRadarID;
    MqttConfigerCommon                  configerMqtt;
    MqttTopicConfiger                   configerTopic;
    MqttTopicConfigerMonitorPeriod      configerTopicMonitorPeriod;
    EnableConfiger                      configerEnable;
    ProjectPathConfiger                 configerProjectPath;
    ////////////////////////////////////////////////////////
    std::unordered_map<uint32_t, MqttTopicConfiger>  topicUnMapSubscribe;
    ////////////////////////////////////////////////////////
    DBConfiger                      configerDb;
    WorkParamConfiger               configerWorkParam;
    //////////////////////////////////////////////////////
    std::vector<ConfigerSensorDeviceInfo> configerSensorDeviceInfos;
    AlarmThresholdValue             configAlarmThresholdValue;
    std::string                     configerVersionFilePath = "/home/airos/os/.airos_version";
private:
    virtual void writeToFile(ConfigBlock &j) override
    {
        JsonSerialize(enableDebugPrint,                "A_enableDebugPrint",                j, false);
        JsonSerialize(nic,                   "A_nic",                   j, false);
        JsonSerialize(rscuEsn,                   "A_rscuEsn",                   j, false);
        JsonSerialize(mqttClientId,                   "B_mqttClientId",                   j, false);
        JsonSerialize(configerTopicCameraRcId,              "C_configerTopicCameraRcId",              j, false);
        JsonSerialize(configerTopicRadarID,           "D_configerTopicRadarID",           j, false);
        JsonSerialize(configerMqtt,                   "E_configerMqtt",                   j, false);
        JsonSerialize(configerTopic,                  "F_configerTopic",                  j, false);
        JsonSerialize(configerTopicMonitorPeriod,     "G_configerTopicMonitorPeriod",     j, false);
        JsonSerialize(configerEnable,                 "H_configerEnable",                 j, false);
        JsonSerialize(configerProjectPath,            "I_configerProjectPath",            j, false);
        JsonSerialize(configerDb, "J_configerDb", j, false);
        JsonSerialize(configerWorkParam, "K_configerWorkParam", j, false);
        JsonSerialize(configerSensorDeviceInfos, "L_configerSensorDeviceInfos", j, false);
        JsonSerialize(configAlarmThresholdValue, "M_configAlarmThresholdValue", j, false);
        JsonSerialize(configerVersionFilePath, "N_configerVersionFilePath", j, false);
    }

    virtual void readFromFile(const ConfigBlock &j) override
    {
        JsonDeserialize(enableDebugPrint,                "A_enableDebugPrint",                j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,                   "A_rscuEsn",                   j, noUse_isEmptyFlag);
        JsonDeserialize(nic,                   "A_nic",                   j, noUse_isEmptyFlag);
        JsonDeserialize(mqttClientId,                   "B_mqttClientId",                   j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicCameraRcId,              "C_configerTopicCameraRcId",              j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicRadarID,           "D_configerTopicRadarID",           j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqtt,                   "E_configerMqtt",                   j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopic,                  "F_configerTopic",                  j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicMonitorPeriod,     "G_configerTopicMonitorPeriod",     j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnable,                 "H_configerEnable",                 j, noUse_isEmptyFlag);
        JsonDeserialize(configerProjectPath,            "I_configerProjectPath",            j, noUse_isEmptyFlag);
        JsonDeserialize(configerDb, "J_configerDb", j, noUse_isEmptyFlag);
        JsonDeserialize(configerWorkParam, "K_configerWorkParam", j, noUse_isEmptyFlag);
        JsonDeserialize(configerSensorDeviceInfos, "L_configerSensorDeviceInfos", j, noUse_isEmptyFlag);
        JsonDeserialize(configAlarmThresholdValue, "M_configAlarmThresholdValue", j, noUse_isEmptyFlag);
        JsonDeserialize(configerVersionFilePath, "N_configerVersionFilePath", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_enableDebugPrint: " << (enableDebugPrint ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "A_nic: " << nic <<  std::endl;
        ss << std::left << std::setw(40) << "A_rscuEsn: " << rscuEsn <<  std::endl;
        ss << std::left << std::setw(40) << "B_mqttClientId: " << mqttClientId <<  std::endl;
        ss << std::left << std::setw(40) << "C_configerTopicCameraRcId: " << std::endl << configerTopicCameraRcId.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "D_configerTopicRadarID:" << std::endl << configerTopicRadarID.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "E_configerMqtt:" << std::endl << configerMqtt.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "F_configerTopic:" << std::endl << configerTopic.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "G_configerTopicMonitorPeriod:" << std::endl << configerTopicMonitorPeriod.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "H_configerEnable:" << std::endl << configerEnable.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "I_configerProjectPath:" << std::endl << configerProjectPath.to_string() <<  std::endl;
        ss << "---------------------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "L_Topic UnMap Size: " << topicUnMapSubscribe.size() <<  std::endl;
        for (const auto& pair : topicUnMapSubscribe)
        {
            ss << std::left << "MEC-Camera-topic[" << pair.first << "]" <<  std::endl ;
            ss << pair.second.to_string() <<  std::endl;
            ss << "---------------------------------------------------" << std::endl;
        }
        ss << std::left << std::setw(40) << "configerDb:" << std::endl << configerDb.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "configerWorkParam:" << std::endl  << configerWorkParam.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "configerSensorDeviceInfos:" << std::endl;
        for (const auto& v : configerSensorDeviceInfos)
        {
            ss << v.to_string() <<  std::endl;
            ss << "---------------------------------------------------" << std::endl;
        }
        ss << std::left << std::setw(40) << "configAlarmThresholdValue:" << std::endl << configAlarmThresholdValue.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "configerVersionFilePath:" << std::endl << configerVersionFilePath <<  std::endl;
       
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
