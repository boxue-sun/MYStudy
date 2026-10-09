/*********************************************************************************
 * @file		configer_om
 * @brief		configer_radar_traffic_metrics belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		25-2-10
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  25-2-11 alfred       1.0       ————             restructure
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_CONFIGER_OM_H
#define AIROS_MIDDLEWARE_PROTOCOL_CONFIGER_OM_H
#include "middleware/protocol/om_common/namespace.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "base/common/network/http/httpClient.h"
#include "base/common/network/print.h"
#include "base/common/network/tcp_client.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "middleware/protocol/om_common/data_model/data_alarm.h"
#include "middleware/protocol/om_common/configer_bs.h"
#include "middleware/protocol//om_common/data_model/data_performence.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_db_device_status.h"
#include "middleware/protocol/om_common/configer_topic_om_mec.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "middleware/protocol/om_common/data_model/camera_device_angle_offset_log_data.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/om_common/data_model/data_config.h"
#include "middleware/protocol/om_common/db_utils.h"
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/performence_utils.h"
#include "middleware/protocol/om_common/sqlite_device_status.h"
#include "middleware/protocol/om_common/data_model/camera_device_angle_offset_log_data.h"
#include "middleware/protocol/om_common/data_model/senario.h"
#include "configer/configer_om_mec.h"
#include "configer/configer_om_camera.h"
#include "configer/configer_om_radar.h"
#include "base/common/network/net_util.h"
#include "data_model/mec/data_basic_info.h"
#include "data_model/mec/data_device_info_query.h"
#include "data_model/mec/data_equipment_timing.h"
#include "data_model/mec/data_heartbeat.h"
#include "data_model/mec/data_http_post_body.h"
#include "data_model/mec/data_ota.h"
#include "data_model/mec/data_reboot.h"
#include "data_model/mec/data_running_status.h"
#include "data_model/mec/map_data.h"

#include "data_model/mec/traffic_light_src_data.h"
#include "data_model/mec/traffic_light_data.h"
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"
#include "base/device_connect/proto/traffic_light_data.pb.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>  // 添加这行
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <string>
#include <cstring>
#include "pugixml.hpp"
#include "data_model/radar/inter_exter_param_publish.h"
#include "data_model/radar/inter_exter_param_query.h"
#include "data_model/radar/radar_alarm.h"
#include "data_model/mec/data_spat_check_status.h"
#include "data_model/camera/inter_exter_param_publish.h"
#include "data_model/camera/inter_exter_param_file.h"
#include "data_model/camera/inter_exter_param_query.h"
#include "data_model/camera/camera_alarm.h"
#include "data_model/mec/data_monitor_mec.h"
#include "data_model/mec/traffic_light_data_status.h"
NAMESPACE_START_OM
using namespace os::v2x::protocol::om::common;
using namespace airos::base::workparam;
using namespace afl::base;
using namespace afl::util;
using namespace afl::thread;
using namespace os::v2x::protocol::om::mec;
using namespace os::v2x::protocol::om::camera;
using namespace os::v2x::protocol::om::radar;
using namespace afl::net;

struct MqttEnableConfiger : public afl::base::SerializableData
{
public:
    bool Enable_Mec = true;
    bool Enable_Radar = true;
    bool Enable_Camera = true;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(Enable_Mec, "A_Enable_Mec", j, false);
        JsonSerialize(Enable_Radar, "B_Enable_Radar", j, false);
        JsonSerialize(Enable_Camera, "C_Enable_Camera", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(Enable_Mec, "A_Enable_Mec", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Radar, "B_Enable_Radar", j, noUse_isEmptyFlag);
        JsonDeserialize(Enable_Camera, "C_Enable_Camera", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Enable_Mec: " << (Enable_Mec ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Radar: " << (Enable_Radar ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "Enable_Camera: " << (Enable_Camera ? "True" : "False") <<  std::endl;
        return ss.str();
    }
};
//rsu客户端配置
struct ConfigerOm : public ConfigerData<ConfigerOm>
{
    ConfigerOm()
    {
        configerMqtt.mqttClientId = "om-inter";
        configerMqttCloud.mqttClientId = "om-cloud";
        configerProjectPath.projectRoot = "/home/airos/protocol/om";
        configerMqtt.mqttSendQos = 0;
        configerMqtt.mqttSubScribeQos = 1;
        configerMqttCloud.mqttSendQos = 1;
        configerMqttCloud.mqttSubScribeQos = 1;
    }

    bool                                enableDebugPrint = true;



    std::string                         rscuEsn;
    //////////////////////////////////////////////////
    MqttConfigerCommon                  configerMqtt;
    //////////////////////////////////////////////////
    MqttConfigerCommon                  configerMqttCloud;
    //////////////////////////////////////////////////
    ProjectPathConfigerCommon           configerProjectPath;
    WorkParamConfiger                   configerWorkParam;
    ////////////////////////////////////////////////////////
    MqttEnableConfiger                  configerEnable;
    ConfigerMec                         configerMec;
    ConfigerCamera                      configerCamera;
    ConfigerRadar                       configerRadar;
    ConfigerCameraEvent                 configerCameraEvent;
    //////////////////////////////////////////////////////
    double                              periodMonitorMqttConnect = 10;
    std::vector<ConfigerSensorDeviceInfo> configerSensorDeviceInfos;
    double                              spatNormalJudgeTime = 3;//故障产生后，红绿灯正常持续时间超过该时间认为故障消失
private:
    virtual void writeToFile(ConfigBlock &j) override
    {
        JsonSerialize(enableDebugPrint, "A_enableDebugPrint", j, false);
        JsonSerialize(rscuEsn, "B_rscuEsn", j, false);
        JsonSerialize(configerMqtt, "C_configerMqtt", j, false);
        JsonSerialize(configerMqttCloud, "D_configerMqttCloud", j, false);
        JsonSerialize(configerProjectPath, "E_configerProjectPath", j, false);
        JsonSerialize(configerWorkParam, "F_configerWorkParam", j, false);
        JsonSerialize(configerEnable, "G_configerEnable", j, false);
        JsonSerialize(configerMec, "G_configerMec", j, false);
        JsonSerialize(configerCamera, "H_configerCamera", j, false);
        JsonSerialize(configerRadar, "I_configerRadar", j, false);
        JsonSerialize(configerCameraEvent, "J_configerCameraEvent", j, false);
        JsonSerialize(periodMonitorMqttConnect, "K_periodMonitorMqttConnect", j, false);
        JsonSerialize(configerSensorDeviceInfos, "L_configerSensorDeviceInfos", j, false);
        JsonSerialize(spatNormalJudgeTime, "M_spatNormalJudgeTime", j, false);
    }

    virtual void readFromFile(const ConfigBlock &j) override
    {
        JsonDeserialize(enableDebugPrint, "A_enableDebugPrint", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "B_rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqtt, "C_configerMqtt", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqttCloud, "D_configerMqttCloud", j, noUse_isEmptyFlag);
        JsonDeserialize(configerProjectPath, "E_configerProjectPath", j, noUse_isEmptyFlag);
        JsonDeserialize(configerWorkParam, "F_configerWorkParam", j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnable, "G_configerEnable", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMec, "G_configerMec", j, noUse_isEmptyFlag);
        JsonDeserialize(configerCamera, "H_configerCamera", j, noUse_isEmptyFlag);
        JsonDeserialize(configerRadar, "I_configerRadar", j, noUse_isEmptyFlag);
        JsonDeserialize(configerCameraEvent, "J_configerCameraEvent", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorMqttConnect, "K_periodMonitorMqttConnect", j, noUse_isEmptyFlag);
        JsonDeserialize(configerSensorDeviceInfos, "L_configerSensorDeviceInfos", j, noUse_isEmptyFlag);
        JsonDeserialize(spatNormalJudgeTime, "M_spatNormalJudgeTime", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "A_enableDebugPrint: " << (enableDebugPrint ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "B_rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "C_configerMqtt:" << configerMqtt.to_string() << std::endl;
        ss << std::left << std::setw(40) << "---------------------------------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "D_configerMqttCloud:" << configerMqttCloud.to_string() << std::endl;
        ss << std::left << std::setw(40) << "---------------------------------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "E_configerProjectPath:" << configerProjectPath.to_string() << std::endl;
        ss << std::left << std::setw(40) << "F_configerWorkParam:" <<  configerWorkParam.to_string() << std::endl;
        ss << std::left << std::setw(40) << "---------------------------------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "G_configerEnable: " << configerEnable.to_string() << std::endl;
        ss << std::left << std::setw(40) << "G_configerMec: " << configerMec.to_string() << std::endl;
        ss << std::left << std::setw(40) << "H_configerCamera:" << std::endl<< configerCamera.to_string() << std::endl;
        ss << std::left << std::setw(40) << "I_configerRadar:" << std::endl << configerRadar.to_string() << std::endl;
        ss << std::left << std::setw(40) << "J_configerCameraEvent:" << std::endl << configerCameraEvent.to_string() << std::endl;
        ss << std::left << std::setw(40) << "L_configerSensorDeviceInfos:" << std::endl;
        for (const auto& v : configerSensorDeviceInfos)
        {
            ss << v.to_string() <<  std::endl;
            ss << "---------------------------------------------------" << std::endl;
        }
        return ss.str();
    }
};
NAMESPACE_ENDED_OM
#endif