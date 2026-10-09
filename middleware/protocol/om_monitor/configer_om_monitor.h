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

#ifndef AIROS2_0_CONFIGER_OM_OM_MONITOR_COMPONENT_H
#define AIROS2_0_CONFIGER_OM_OM_MONITOR_COMPONENT_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_ccindex.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "base/common/network/net_util.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/om_common/db_utils.h"
#include <algorithm>
#include "base/common/network/concurrent_queue.h"
#include "middleware/protocol/proto/monitor.pb.h"
NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace airos::base::workparam;
using namespace afl::net;
using namespace os::v2x::protocol;
using namespace os::v2x::protocol::om::common;
using namespace os::v2x::protocol::ccindex;
using namespace airos::monitor;
struct SensorIpPortData: public afl::base::SerializableData
{
    std::string sensor_data_in_port;               // 感知数据接收端口
    std::string sensor_cloud_ip;                    // 感知运控的ip
    std::string sensor_cloud_port;                  // 感知运控的端口

    // 序列化函数
    void serialize(json &j) override {
        JsonSerialize(sensor_data_in_port,              "sensor_data_in_port",              j, false);
        JsonSerialize(sensor_cloud_ip,                   "sensor_cloud_ip",                   j, false);
        JsonSerialize(sensor_cloud_port,                 "sensor_cloud_port",                 j, false);
    }

    // 反序列化函数
    void deserialize(const json &j) override {
        JsonDeserialize(sensor_data_in_port,              "sensor_data_in_port",              j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_ip,                   "sensor_cloud_ip",                   j, noUse_isEmptyFlag);
        JsonDeserialize(sensor_cloud_port,                 "sensor_cloud_port",                 j, noUse_isEmptyFlag);
    }

    // 转换为字符串
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "sensor_data_in_port: "                         << sensor_data_in_port << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_ip: "                  << sensor_cloud_ip << std::endl;
        ss << std::left << std::setw(40) << "sensor_cloud_port: "                         << sensor_cloud_port << std::endl;
        return ss.str();
    }
};
struct NeedFilePathConfiger: public afl::base::SerializableData
{
public:
    std::string mec_device_yaml_path = "/home/airos/param/device/mec/device.yaml";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(mec_device_yaml_path, "A_mec_device_yaml_path", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(mec_device_yaml_path, "A_mec_device_yaml_path", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_mec_device_yaml_path: " << mec_device_yaml_path <<  std::endl;
        return ss.str();
    }
};
struct MonitorPeriod: public afl::base::SerializableData
{
public:
    double 	periodMonitorDockerStatus 		        = 300.0;
    double 	periodMonitorAirosModulesStatus 		= 200.0;
    double 	periodMonitorMecRadarCloudStatus        = 5.0;
    double 	periodMonitorDiskStatus 		        = 360.0;
private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(periodMonitorDockerStatus, "A_PeriodMonitorDockerStatus", j, false);
        JsonSerialize(periodMonitorAirosModulesStatus, "B_PeriodMonitorAirosModulesStatus", j, false);
        JsonSerialize(periodMonitorMecRadarCloudStatus, "C_PeriodMonitorMecRadarCloudStatus", j, false);
        JsonSerialize(periodMonitorDiskStatus, "D_PeriodMonitorDiskStatus", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(periodMonitorDockerStatus, "A_PeriodMonitorDockerStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorAirosModulesStatus, "B_PeriodMonitorAirosModulesStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorMecRadarCloudStatus, "C_PeriodMonitorMecRadarCloudStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorDiskStatus, "D_PeriodMonitorDiskStatus", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_PeriodMonitorDockerStatus: " << periodMonitorDockerStatus <<  std::endl;
        ss << std::left << std::setw(40) << "B_PeriodMonitorAirosModulesStatus: " << periodMonitorAirosModulesStatus <<  std::endl;
        ss << std::left << std::setw(40) << "C_PeriodMonitorMecRadarCloudStatus: " << periodMonitorMecRadarCloudStatus <<  std::endl;
        ss << std::left << std::setw(40) << "D_PeriodMonitorDiskStatus: " << periodMonitorDiskStatus <<  std::endl;
        return ss.str();
    }
};


struct MonitorEnableConfiger : public afl::base::SerializableData
{
public:
    bool Enable_MonitorDockerStatus = true;
    bool Enable_MonitorAirosModulesStatus = true;
    bool Enable_MonitorDiskStatus = true;
    bool Enable_MonitorPerformanceStatus = true;
    bool Enable_MonitorRsapStatus = true;
    bool Enable_MonitorOmStatus = true;
    bool Enable_MonitorCcindexData = true;
    bool Enable_Judge_Quey_Time  = false;

    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(Enable_MonitorDockerStatus,  "A_Enable_MonitorDockerStatus",  j, false);
        JsonSerialize(Enable_MonitorAirosModulesStatus,  "B_Enable_MonitorAirosModulesStatus",  j, false);
        JsonSerialize(Enable_MonitorDiskStatus,  "C_Enable_MonitorDiskStatus",  j, false);
        JsonSerialize(Enable_MonitorPerformanceStatus,  "D_Enable_MonitorPerformanceStatus",  j, false);


        JsonSerialize(Enable_MonitorRsapStatus,  "E_Enable_MonitorRsapStatus",  j, false);
        JsonSerialize(Enable_MonitorOmStatus,  "F_Enable_MonitorOmStatus",  j, false);
        JsonSerialize(Enable_MonitorCcindexData,  "G_Enable_MonitorCcindexData",  j, false);
        JsonSerialize(Enable_Judge_Quey_Time,  "H_Enable_Judge_Quey_Time",  j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {;

        JsonDeserialize(Enable_MonitorDockerStatus,  "A_Enable_MonitorDockerStatus",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MonitorAirosModulesStatus,  "B_Enable_MonitorAirosModulesStatus",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MonitorDiskStatus,  "C_Enable_MonitorDiskStatus",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MonitorPerformanceStatus,  "D_Enable_MonitorPerformanceStatus",  j, noUse_isEmptyFlag);


        JsonDeserialize(Enable_MonitorRsapStatus,  "E_Enable_MonitorRsapStatus",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MonitorOmStatus,  "F_Enable_MonitorOmStatus",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_MonitorCcindexData,  "G_Enable_MonitorCcindexData",  j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Judge_Quey_Time,  "H_Enable_Judge_Quey_Time",  j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorDockerStatus: " << (Enable_MonitorDockerStatus ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorAirosModulesStatus: " << (Enable_MonitorAirosModulesStatus ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorDiskStatus: " << (Enable_MonitorDiskStatus ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorPerformanceStatus: " << (Enable_MonitorPerformanceStatus ? "True" :  "False") <<  std::endl;

        ss << std::left << std::setw(40) << "Enable_MonitorRsapStatus: " << (Enable_MonitorRsapStatus ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorOmStatus: " << (Enable_MonitorOmStatus ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_MonitorCcindexData: " << (Enable_MonitorCcindexData ? "True" :  "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Judge_Quey_Time: " << (Enable_Judge_Quey_Time ? "True" :  "False") <<  std::endl;
        return ss.str();
    }
};

//mqtt客户端配置
struct MqttClientConfigerRadarMonitor :  public afl::base::SerializableData
{
    bool                                enableDebugPrint = true;
    bool                                enableDebugPrintProto = true;
    std::string                         rscuEsn = "";
    bool                                hasInited = false;
    std::string                         mqttClientId = "mqtt_client";
    MqttTopicConfigerRadarID            configerTopicRadarID;
    MqttConfigerCommon                  configerMqtt;
    ////////////////////////////////////////////////////////
    MqttTopicConfigerCCIndex            configerTopicCCIndex;

private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(enableDebugPrint, "A_enableDebugPrint", j, false);
        JsonSerialize(enableDebugPrintProto, "B_enableDebugPrintProto", j, false);
        JsonSerialize(rscuEsn, "C_rscuEsn", j, false);
        JsonSerialize(hasInited, "D_hasInited", j, false);
        JsonSerialize(mqttClientId, "E_mqttClientId", j, false);
        JsonSerialize(configerTopicRadarID, "F_configerTopicRadarID", j, false);
        JsonSerialize(configerMqtt, "H_configerMqtt", j, false);
        JsonSerialize(configerTopicCCIndex, "K_configerTopicCCIndex", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(enableDebugPrint, "A_enableDebugPrint", j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintProto, "B_enableDebugPrintProto", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "C_rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(hasInited, "D_hasInited", j, noUse_isEmptyFlag);
        JsonDeserialize(mqttClientId, "E_mqttClientId", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicRadarID, "F_configerTopicRadarID", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqtt, "H_configerMqtt", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicCCIndex, "K_configerTopicCCIndex", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "enableDebugPrint: " << (enableDebugPrint ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintProto: " << (enableDebugPrintProto ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "hasInited: " << (hasInited ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "mqttClientId: " << mqttClientId << std::endl;
        ss << std::left << std::setw(40) << "configerTopicRadarID:" << std::endl << configerTopicRadarID.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerMqtt:" << std::endl<< configerMqtt.to_string() << std::endl;
        ss << std::left << std::setw(40) << "configerTopicCCIndex:" << std::endl << configerTopicCCIndex.to_string() << std::endl;
        return ss.str();
    }
};

//rsu客户端配置
struct UdpConfigerMonitor : public ConfigerData<UdpConfigerMonitor>
{
    bool                                enableDebugPrint = true;
    int                                 udpPort = 50088;
    WorkParamConfiger                   configerWorkParam;
    std::string                         rscuEsn = "110220330";
    NeedFilePathConfiger                needFilePathConfiger;
    MonitorPeriod                       configerMonitorPeriod;
    MonitorEnableConfiger               configerEanbleMonitor;

    //mqtt客户端配置
    MqttClientConfigerRadarMonitor      clientConfigerRadarMonitor;
private:
    virtual void writeToFile(ConfigBlock &j) override
    {
        JsonSerialize(enableDebugPrint,                "A_enableDebugPrint",                j, false);
        JsonSerialize(udpPort,                   "B_udpPort",                   j, false);
        JsonSerialize(configerWorkParam,                   "C_configerWorkParam",                   j, false);
        JsonSerialize(rscuEsn,                   "D_rscuEsn",                   j, false);
        JsonSerialize(needFilePathConfiger,                   "E_needFilePathConfiger",                   j, false);
        JsonSerialize(configerMonitorPeriod,                   "F_configerMonitorPeriod",                   j, false);
        JsonSerialize(configerEanbleMonitor,                   "G_configerEanbleMonitor",                   j, false);
        JsonSerialize(clientConfigerRadarMonitor,              "H_clientConfigerRadarMonitor",                   j, false);
    }

    virtual void readFromFile(const ConfigBlock &j) override
    {
        JsonDeserialize(enableDebugPrint,                "A_enableDebugPrint",                j, noUse_isEmptyFlag);
        JsonDeserialize(udpPort,                   "B_udpPort",                   j, noUse_isEmptyFlag);
        JsonDeserialize(configerWorkParam,                   "C_configerWorkParam",                   j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,                   "D_rscuEsn",                   j, noUse_isEmptyFlag);
        JsonDeserialize(needFilePathConfiger,                   "E_needFilePathConfiger",                   j, noUse_isEmptyFlag);
        JsonDeserialize(configerMonitorPeriod,                   "F_configerMonitorPeriod",                   j, noUse_isEmptyFlag);

        JsonDeserialize(configerEanbleMonitor,                   "G_configerEanbleMonitor",                   j, noUse_isEmptyFlag);
        JsonDeserialize(clientConfigerRadarMonitor,              "H_clientConfigerRadarMonitor",                   j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_enableDebugPrint: " << (enableDebugPrint ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "B_udpPort: " << udpPort <<  std::endl;
        ss << std::left << std::setw(40) << "C_configerWorkParam: " << configerWorkParam.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "D_rscuEsn: " << rscuEsn <<  std::endl;
        ss << std::left << std::setw(40) << "E_needFilePathConfiger: " << needFilePathConfiger.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "F_configerMonitorPeriod: " << configerMonitorPeriod.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "G_configerEanbleMonitor: " << configerEanbleMonitor.to_string() <<  std::endl;
        ss << std::left << std::setw(40) << "H_clientConfigerRadarMonitor: " << clientConfigerRadarMonitor.to_string() <<  std::endl;
        return ss.str();
    }
};


NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
