/*********************************************************************************
 * @file		configer.h
 * @brief		configer belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-7
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-6-7 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS2_0_CONFIGER_COMMON_H
#define AIROS2_0_CONFIGER_COMMON_H
#include "namespace.h"

NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;
using namespace afl::util;
using namespace airos::base::workparam;
#ifndef SUBSCRIBE_NUM
#define SUBSCRIBE_NUM 1000

#endif
struct MqttConfigerCommon : public afl::base::SerializableData
{
    std::string mqttClientId;
    std::string mqttBrokerUrl = "tcp://127.0.0.1:1883";
    std::string mqttUserName = "root";
    std::string mqttPassword = "root";
    int         mqttVersion = 4;
    int         mqttCleanSession = 1;
    double      mqttKeepAliveInterval = 60;
    int         mqttSendQos = 2;
    int         mqttSubScribeQos = 1;
    double      mqttReconnectInterval = 10;
    double      mqttConnectTimeOut = 60;
    int         mqttRetained = 0;
    int         mqttSslVersion = MQTT_SSL_VERSION_TLS_1_2;
    std::string tlsPrivateKeyPassword = "123456";
    uint32_t 	mqttRealSubscribeTopicNum;
    char* 		mqttSubscribeTopics[SUBSCRIBE_NUM];
    int 		mqttSubscribeQoss[SUBSCRIBE_NUM];
    int         mqttPublishMaxRetries = 5;
    int         mqttReconnectMaxCount = 10;

    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(mqttClientId,    "A_mqttClientId", j, false);
        JsonSerialize(mqttBrokerUrl,    "A_mqttBrokerUrl", j, false);
        JsonSerialize(mqttUserName,     "B_mqttUserName", j, false);
        JsonSerialize(mqttPassword,     "B_mqttPassword", j, false);
        JsonSerialize(mqttVersion,              "C_mqttVersion", j, false);
        JsonSerialize(mqttCleanSession, "D_mqttCleanSession", j, false);
        JsonSerialize(mqttKeepAliveInterval,    "E_mqttKeepAliveInterval", j, false);
        JsonSerialize(mqttSendQos,      "F_mqttSendQos", j, false);
        JsonSerialize(mqttSubScribeQos, "G_mqttSubScribeQos", j, false);
        JsonSerialize(mqttReconnectInterval,    "H_mqttReconnectInterval", j, false);
        JsonSerialize(mqttConnectTimeOut,       "I_mqttConnectTimeOut", j, false);
        JsonSerialize(mqttRetained,    "J_mqttRetained", j, false);
        JsonSerialize(mqttSslVersion,           "K_mqttSslVersion", j, false);
        JsonSerialize(tlsPrivateKeyPassword,    "L_tlsPrivateKeyPassword", j, false);
        JsonSerialize(mqttPublishMaxRetries,    "M_mqttPublishMaxRetries", j, false);
        JsonSerialize(mqttReconnectMaxCount,    "N_mqttReconnectMaxCount", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(mqttClientId,    "A_mqttClientId", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttBrokerUrl,    "A_mqttBrokerUrl", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttUserName,     "B_mqttUserName", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttPassword,     "B_mqttPassword", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttVersion,              "C_mqttVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttCleanSession, "D_mqttCleanSession", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttKeepAliveInterval,    "E_mqttKeepAliveInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttSendQos,      "F_mqttSendQos", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttSubScribeQos, "G_mqttSubScribeQos", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttReconnectInterval,    "H_mqttReconnectInterval", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttConnectTimeOut,       "I_mqttConnectTimeOut", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttRetained,    "J_mqttRetained", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttSslVersion,           "K_mqttSslVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsPrivateKeyPassword,    "L_tlsPrivateKeyPassword", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttPublishMaxRetries,    "M_mqttPublishMaxRetries", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttReconnectMaxCount,    "N_mqttReconnectMaxCount", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_mqttClientId: " << mqttClientId << std::endl;
        ss << std::left << std::setw(40) << "A_MqttBrokerUrl: " << mqttBrokerUrl << std::endl;
        ss << std::left << std::setw(40) << "B_MqttUserName: " << mqttUserName << std::endl;
        ss << std::left << std::setw(40) << "B_MqttPassword: " << mqttPassword << std::endl;
        ss << std::left << std::setw(40) << "C_mqttVersion: " << mqttVersion << std::endl;
        ss << std::left << std::setw(40) << "D_mqttCleanSession: " << mqttCleanSession << std::endl;
        ss << std::left << std::setw(40) << "E_mqttKeepAliveInterval: " << mqttKeepAliveInterval << std::endl;
        ss << std::left << std::setw(40) << "F_mqttSendQos: " << mqttSendQos << std::endl;
        ss << std::left << std::setw(40) << "G_mqttSubScribeQos: " << mqttSubScribeQos << std::endl;
        ss << std::left << std::setw(40) << "H_mqttReconnectInterval: " << mqttReconnectInterval << std::endl;
        ss << std::left << std::setw(40) << "I_mqttConnectTimeOut: " << mqttConnectTimeOut << std::endl;
        ss << std::left << std::setw(40) << "J_mqttRetained: " << mqttRetained << std::endl;
        ss << std::left << std::setw(40) << "K_mqttSslVersion: " << mqttSslVersion << std::endl;
        ss << std::left << std::setw(40) << "L_tlsPrivateKeyPassword: " << tlsPrivateKeyPassword << std::endl;
        ss << std::left << std::setw(40) << "M_mqttPublishMaxRetries: " << mqttPublishMaxRetries << std::endl;
        ss << std::left << std::setw(40) << "N_mqttReconnectMaxCount: " << mqttReconnectMaxCount << std::endl;
        ss << std::left << std::setw(40) << "O_MqttRealSubscribeTopicNum: " << mqttRealSubscribeTopicNum << std::endl;
        for (uint32_t i = 0; i < mqttRealSubscribeTopicNum; i++)
        {
            ss << std::left  << "SubscribeQos[" <<  mqttSubscribeQoss[i] << "] " << "SubscribeTopic[" << i << "]: " << mqttSubscribeTopics[i] << std::endl;
        }
        return ss.str();
    }
};

//项目路径配置
struct ProjectPathConfigerCommon : public afl::base::SerializableData
{
public:
    std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag";
    std::string projectRoot = "/home/airos/protocol/ccindex";
    std::string tlsDirName = "ssl";
    std::string tlsCAFileName = "ca.crt";
    std::string tlsClientKeyFileName = "client.crt";
    std::string tlsClientPrivateKeyFileName = "client.key";
    std::string tlsClientPrivateKeyPassword = "123456";
    std::string tlsCAFilePath;
    std::string tlsClientKeyFilePath;
    std::string tlsClientPrivateKeyFilePath;

    std::string otaDirName = "ota";
    std::string otaFileName = "ota.tar.gz";

private:
    virtual void serialize(afl::base::json &j)
    {
        JsonSerialize(workParamFilePath, "A_WorkParamFilePath", j, false);
        JsonSerialize(projectRoot, "B_ProjectRoot", j, false);
        JsonSerialize(tlsDirName, "C_TlsDirName", j, false);
        JsonSerialize(tlsCAFileName, "D_TlsCAFileName", j, false);
        JsonSerialize(tlsClientKeyFileName, "E_TlsClientKeyFileName", j, false);
        JsonSerialize(tlsClientPrivateKeyFileName, "F_TlsClientPrivateKeyFileName", j, false);
        JsonSerialize(tlsClientPrivateKeyPassword, "G_TlsClientPrivateKeyPassword", j, false);
        JsonSerialize(otaDirName, "H_otaDirName", j, false);
        JsonSerialize(otaFileName, "I_otaFileName", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(workParamFilePath, "A_WorkParamFilePath", j, noUse_isEmptyFlag);
        JsonDeserialize(projectRoot, "B_ProjectRoot", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsDirName, "C_TlsDirName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsCAFileName, "D_TlsCAFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientKeyFileName, "E_TlsClientKeyFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientPrivateKeyFileName, "F_TlsClientPrivateKeyFileName", j, noUse_isEmptyFlag);
        JsonDeserialize(tlsClientPrivateKeyPassword, "G_TlsClientPrivateKeyPassword", j, noUse_isEmptyFlag);
        JsonDeserialize(otaDirName, "H_otaDirName", j, noUse_isEmptyFlag);
        JsonDeserialize(otaFileName, "I_otaFileName", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_WorkParamFilePath: " << workParamFilePath << std::endl;
        ss << std::left << std::setw(40) << "B_ProjectRoot: " << projectRoot << std::endl;
        ss << std::left << std::setw(40) << "C_TlsDirName: " << tlsDirName << std::endl;
        ss << std::left << std::setw(40) << "D_TlsCAFileName: " << tlsCAFileName << std::endl;
        ss << std::left << std::setw(40) << "E_TlsClientKeyFileName: " << tlsClientKeyFileName << std::endl;
        ss << std::left << std::setw(40) << "F_TlsClientPrivateKeyFileName: " << tlsClientPrivateKeyFileName << std::endl;
        ss << std::left << std::setw(40) << "G_TlsClientPrivateKeyPassword: " << tlsClientPrivateKeyPassword << std::endl;
        ss << std::left << std::setw(40) << "H_tlsCAFilePath: " << tlsCAFilePath << std::endl;
        ss << std::left << std::setw(40) << "I_tlsClientKeyFilePath: " << tlsClientKeyFilePath << std::endl;
        ss << std::left << std::setw(40) << "J_tlsClientPrivateKeyFilePath: " << tlsClientPrivateKeyFilePath << std::endl;
        ss << std::left << std::setw(40) << "K_otaDirName: " << otaDirName <<  std::endl;
        ss << std::left << std::setw(40) << "L_otaFileName: " << otaFileName <<  std::endl;
        return ss.str();
    }
};

struct ConfigerSensorDeviceInfo : public afl::base::SerializableData
{
    std::string         routeId; // 路口编号
    WorkParamDeviceType deviceType; // 设备类型
    std::string         mecSn; // MEC-Sn
    std::string         deviceSn; // 设备sn
    std::string         deviceEsn; // 设备esn
    std::string         deviceIp; // 设备IP
    double              deviceLongitude = 0; // 设备经度
    double              deviceLatitude = 0; // 设备维度
    double              deviceAltitude = 0; // 设备海拔
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(routeId,         "A_routeId",         j, false);
        JsonSerialize(deviceType,       "B_deviceType",     j, false);
        JsonSerialize(mecSn,           "C_mecSn",           j, false);
        JsonSerialize(deviceSn,        "D_deviceSn",        j, false);
        JsonSerialize(deviceEsn,       "E_deviceEsn",       j, false);
        JsonSerialize(deviceIp,        "F_deviceIp",        j, false);
        JsonSerialize(deviceLongitude, "G_deviceLongitude", j, false);
        JsonSerialize(deviceLatitude,  "H_deviceLatitude",  j, false);
        JsonSerialize(deviceAltitude,  "I_deviceAltitude",  j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(routeId,         "A_routeId",         j, noUse_isEmptyFlag);
        JsonDeserialize(deviceType,   "B_deviceType",      j, noUse_isEmptyFlag);
        JsonDeserialize(mecSn,           "C_mecSn",           j, noUse_isEmptyFlag);
        JsonDeserialize(deviceSn,        "D_deviceSn",        j, noUse_isEmptyFlag);
        JsonDeserialize(deviceEsn,       "E_deviceEsn",       j, noUse_isEmptyFlag);
        JsonDeserialize(deviceIp,        "F_deviceIp",        j, noUse_isEmptyFlag);
        JsonDeserialize(deviceLongitude, "G_deviceLongitude", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceLatitude,  "H_deviceLatitude",  j, noUse_isEmptyFlag);
        JsonDeserialize(deviceAltitude,  "I_deviceAltitude",  j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "RouteId: " << routeId << std::endl;
        ss << std::left << std::setw(40) << "DeviceType: " << deviceType << std::endl;
        ss << std::left << std::setw(40) << "MecSn: " << mecSn << std::endl;
        ss << std::left << std::setw(40) << "DeviceSn: " << deviceSn << std::endl;
        ss << std::left << std::setw(40) << "DeviceEsn: " << deviceEsn << std::endl;
        ss << std::left << std::setw(40) << "DeviceIp: " << deviceIp << std::endl;
        ss << std::left << std::setw(40) << "DeviceLongitude: " << deviceLongitude << std::endl;
        ss << std::left << std::setw(40) << "DeviceLatitude: " << deviceLatitude << std::endl;
        ss << std::left << std::setw(40) << "DeviceAltitude: " << deviceAltitude << std::endl;
        return ss.str();
    }
};

struct AlarmThresholdValue: public afl::base::SerializableData
{
    public:
    double 	TV_CPU_Tem 		= 80.0;
    double 	TV_CPU_Uti   	= 0.72;
    uint64_t 	TV_Mem_Free	= 1073741824;
    uint64_t  TV_Disk_Free        = 15073741824;
    uint64_t  TV_Disk_Work_Free   = 40090710016;
    double  TV_Camera_Angle_Offset     = 10;
    int     TV_Timing   = 1000;
    int     TV_Alarm_Timing_Count = 10;
    int    TV_Timing_Alarm = 6000000;  //6ms
    private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(TV_CPU_Tem,                     "A_TV_CPU_Tem",                     j, false);
        JsonSerialize(TV_CPU_Uti,                     "B_TV_CPU_Uti",                     j, false);
        JsonSerialize(TV_Mem_Free,                    "C_TV_Mem_Free",                    j, false);
        JsonSerialize(TV_Disk_Free,                   "D_TV_Disk_Free",                   j, false);
        JsonSerialize(TV_Disk_Work_Free,                   "D_TV_Disk_Work_Free",                   j, false);
        JsonSerialize(TV_Camera_Angle_Offset,         "E_TV_Camera_Angle_Offset",         j, false);
        JsonSerialize(TV_Timing,         "F_TV_Timing",         j, false);
        JsonSerialize(TV_Alarm_Timing_Count,         "G_TV_Alarm_Timing_Count",         j, false);
        JsonSerialize(TV_Timing_Alarm,         "H_TV_Timing_Alarm",         j, false);
    }
    
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(TV_CPU_Tem,                     "A_TV_CPU_Tem",                     j, noUse_isEmptyFlag);
        JsonDeserialize(TV_CPU_Uti,                     "B_TV_CPU_Uti",                     j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Mem_Free,                    "C_TV_Mem_Free",                    j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Disk_Free,                   "D_TV_Disk_Free",                   j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Disk_Work_Free,                   "D_TV_Disk_Work_Free",                   j, noUse_isEmptyFlag);

        JsonDeserialize(TV_Camera_Angle_Offset,         "E_TV_Camera_Angle_Offset",         j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Timing,                      "F_TV_Timing",                      j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Alarm_Timing_Count,                      "G_TV_Alarm_Timing_Count",                      j, noUse_isEmptyFlag);
        JsonDeserialize(TV_Timing_Alarm,         "H_TV_Timing_Alarm",         j, noUse_isEmptyFlag);
    }

    public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "TV_CPU_Tem: "                   << TV_CPU_Tem                   <<  std::endl;
        ss << std::left << std::setw(40) << "TV_CPU_Uti: "                   << TV_CPU_Uti                   <<  std::endl;
        ss << std::left << std::setw(40) << "TV_Mem_Free: "                  << TV_Mem_Free                  <<  std::endl;
        ss << std::left << std::setw(40) << "TV_Disk_Free: "                 << TV_Disk_Free                 <<  std::endl;
        ss << std::left << std::setw(40) << "TV_Disk_Work_Free: "                 << TV_Disk_Work_Free               <<  std::endl;
        ss << std::left << std::setw(40) << "TV_Camera_Angle_Offset: "       << TV_Camera_Angle_Offset       <<  std::endl;
        ss << std::left << std::setw(40) << "F_TV_Timing: "       << TV_Timing       <<  std::endl;
        ss << std::left << std::setw(40) << "TV_Alarm_Timing_Count: "       << TV_Alarm_Timing_Count       <<  std::endl;
        ss << std::left << std::setw(40) << "H_TV_Timing_Alarm: "       << TV_Timing_Alarm       <<  std::endl;
       
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //AIROS2_0_CONFIGER_COMMON_H
