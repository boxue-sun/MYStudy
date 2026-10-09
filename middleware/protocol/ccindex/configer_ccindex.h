/*********************************************************************************
* @file		configer_radar_traffic_metrics
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
 *  25-2-10 alfred       1.0       ————             restructure
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_CCINDEX_CONFIGER_CCINDEX_H
#define AIROS_MIDDLEWARE_PROTOCOL_CCINDEX_CONFIGER_CCINDEX_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_ccindex.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "middleware/protocol/proto/traffic_info.pb.h"
#include "base/common/network/http/httpClient.h"
#include "base/common/network/concurrent_queue.h"
#include "data_model/config_data.h"
#include "data_model/config_query_data.h"
#include "data_model/config_update_data.h"

NAMESPACE_START_RADAR_CCINDEX
using namespace os::v2x::protocol::om::common;
using namespace airos::base::workparam;
using namespace os::v2x::protocol::ccindex;
using namespace afl::base;
using namespace afl::util;
using namespace afl::thread;
using namespace os::v2x::protocol::radar_static;
enum Data_OPT_TYPE
{
    TrafficMetircs_Json_Insert = 1,
    TrafficMetircs_Json_Update = 2,
};
struct HttpConfiger : public afl::base::SerializableData
{
    //本机作为server
    std::string httpHostServerIp = "0.0.0.0";
    uint16_t    httpHostServerPort = 8080;
    std::string httpCloudClientGetPath = "/static/device/config";
    uint16_t    mecMqttClientQueryConfigDataPeriod = 10;  //mqtt-client 推送查询数据周期
    //////////////////////////
    //本机作为client




    std::string httpCloudServerIp = "172.30.1.161";
    uint16_t    httpCloudServerPort = 8298;

    uint16_t    httpHostClientPostConfigUpdateDataPeriod = 10;  //http-client-post方法推送更新数据周期
    bool        enablePushSeparate = true;
    std::string vensorTsmtc = "tsmtc";
    std::string vensorDesaysv = "desaysv";

    bool replaceUseMsg = false;
    int replaceTsmtc = 1;
    int replaceDesaysv = 1;

    virtual void serialize(json &j) override
    {
        JsonSerialize(httpHostServerIp, "A_httpHostServerIp", j, false);
        JsonSerialize(httpHostServerPort, "B_httpHostServerPort", j, false);
        JsonSerialize(httpCloudClientGetPath, "C_httpCloudClientGetPath", j, false);
        JsonSerialize(mecMqttClientQueryConfigDataPeriod, "D_mecMqttClientQueryConfigDataPeriod", j, false);
        JsonSerialize(httpCloudServerIp, "E_httpCloudServerIp", j, false);
        JsonSerialize(httpCloudServerPort, "F_httpCloudServerPort", j, false);
        JsonSerialize(httpHostClientPostConfigUpdateDataPeriod, "G_httpHostClientPostConfigUpdateDataPeriod", j, false);
        JsonSerialize(enablePushSeparate, "H_enablePushSeparate", j, false);

        JsonSerialize(vensorTsmtc, "I_vensorTsmtc", j, false);
        JsonSerialize(vensorDesaysv, "I_vensorDesaysv", j, false);

        JsonSerialize(replaceUseMsg, "J_replaceUseMsg", j, false);
        JsonSerialize(replaceTsmtc, "J_replaceTsmtc", j, false);
        JsonSerialize(replaceDesaysv, "J_replaceDesaysv", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(httpHostServerIp, "A_httpHostServerIp", j, noUse_isEmptyFlag);
        JsonDeserialize(httpHostServerPort, "B_httpHostServerPort", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudClientGetPath, "C_httpCloudClientGetPath", j, noUse_isEmptyFlag);
        JsonDeserialize(mecMqttClientQueryConfigDataPeriod, "D_mecMqttClientQueryConfigDataPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudServerIp, "E_httpCloudServerIp", j, noUse_isEmptyFlag);
        JsonDeserialize(httpCloudServerPort, "F_httpCloudServerPort", j, noUse_isEmptyFlag);
        JsonDeserialize(httpHostClientPostConfigUpdateDataPeriod, "G_httpHostClientPostConfigUpdateDataPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(enablePushSeparate, "H_enablePushSeparate", j, noUse_isEmptyFlag);

        JsonDeserialize(vensorTsmtc, "I_vensorTsmtc", j, noUse_isEmptyFlag);
        JsonDeserialize(vensorDesaysv, "I_vensorDesaysv", j, noUse_isEmptyFlag);

        JsonDeserialize(replaceUseMsg, "J_replaceUseMsg", j, noUse_isEmptyFlag);
        JsonDeserialize(replaceTsmtc, "J_replaceTsmtc", j, noUse_isEmptyFlag);
        JsonDeserialize(replaceDesaysv, "J_replaceDesaysv", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;

        ss << std::left << std::setw(40) << "httpHostServerIp: " << httpHostServerIp << std::endl;
        ss << std::left << std::setw(40) << "httpHostServerPort: " << httpHostServerPort << std::endl;
        ss << std::left << std::setw(40) << "httpCloudClientGetPath: " << httpCloudClientGetPath << std::endl;
        ss << std::left << std::setw(40) << "mecMqttClientQueryConfigDataPeriod: " << mecMqttClientQueryConfigDataPeriod << std::endl;
        ss << std::left << std::setw(40) << "httpCloudServerIp: " << httpCloudServerIp << std::endl;
        ss << std::left << std::setw(40) << "httpCloudServerPort: " << httpCloudServerPort << std::endl;
        ss << std::left << std::setw(40) << "httpHostClientPostConfigUpdateDataPeriod: " << httpHostClientPostConfigUpdateDataPeriod << std::endl;
        return ss.str();
    }
};
struct PeriodConfiger : public afl::base::SerializableData
{
    uint16_t    trajectoriesPublishPeriod = 1;
    double      trajectoriesPublishTimeDiff = 2;
    double      periodMonitorMqttConnect = 10;

    double      periodMonitorMecLinkStatusCcindexTm = 5.0;
    double      periodMonitorMecLinkStatusCcindexSt = 5.0;
    virtual void serialize(json &j) override
    {
        JsonSerialize(trajectoriesPublishPeriod, "A_trajectoriesPublishPeriod", j, false);
        JsonSerialize(trajectoriesPublishTimeDiff, "B_trajectoriesPublishTimeDiff", j, false);
        JsonSerialize(periodMonitorMqttConnect, "C_periodMonitorMqttConnect", j, false);

        JsonSerialize(periodMonitorMecLinkStatusCcindexTm, "D_periodMonitorMecLinkStatusCcindexTm", j, false);
        JsonSerialize(periodMonitorMecLinkStatusCcindexSt, "D_periodMonitorMecLinkStatusCcindexSt", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(trajectoriesPublishPeriod, "A_trajectoriesPublishPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(trajectoriesPublishTimeDiff, "B_trajectoriesPublishTimeDiff", j, noUse_isEmptyFlag);

        JsonDeserialize(periodMonitorMqttConnect, "C_periodMonitorMqttConnect", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorMecLinkStatusCcindexTm, "D_periodMonitorMecLinkStatusCcindexTm", j, noUse_isEmptyFlag);
        JsonDeserialize(periodMonitorMecLinkStatusCcindexSt, "D_periodMonitorMecLinkStatusCcindexSt", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "trajectoriesPublishPeriod: " << trajectoriesPublishPeriod << std::endl;
        ss << std::left << std::setw(40) << "trajectoriesPublishTimeDiff: " << trajectoriesPublishTimeDiff << std::endl;
        ss << std::left << std::setw(40) << "periodMonitorMqttConnect: " << periodMonitorMqttConnect << std::endl;

        ss << std::left << std::setw(40) << "periodMonitorMecLinkStatusCcindexTm: " << periodMonitorMecLinkStatusCcindexTm << std::endl;
        ss << std::left << std::setw(40) << "periodMonitorMecLinkStatusCcindexSt: " << periodMonitorMecLinkStatusCcindexSt << std::endl;
        return ss.str();
    }
};
struct EnablePrintSubMsgFromCloudConfiger : public afl::base::SerializableData
{
    bool                                enableDebugPrintSubMsgTc = true;
    bool                                enableDebugPrintSubMsgTm = true;
    virtual void serialize(json &j) override
    {
        JsonSerialize(enableDebugPrintSubMsgTc, "A_enableDebugPrintSubMsgTc", j, false);
        JsonSerialize(enableDebugPrintSubMsgTm, "B_enableDebugPrintSubMsgTm", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(enableDebugPrintSubMsgTc, "A_enableDebugPrintSubMsgTc", j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintSubMsgTm, "B_enableDebugPrintSubMsgTm", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "enableDebugPrintSubMsgTc: " << enableDebugPrintSubMsgTc << std::endl;
        ss << std::left << std::setw(40) << "enableDebugPrintSubMsgTm: " << enableDebugPrintSubMsgTm << std::endl;
        return ss.str();
    }
};
//rsu客户端配置
struct ConfigerCcindex : public ConfigerData<ConfigerCcindex>
{
    ConfigerCcindex()
    {
        configerMqtt.mqttClientId  = "ccindex-inter";
        configerMqttCloud.mqttClientId  = "ccindex-cloud";

        configerMqtt.mqttSendQos = 0;
        configerMqtt.mqttSubScribeQos = 2;
        configerMqttCloud.mqttSendQos = 2;
        configerMqttCloud.mqttSubScribeQos = 2;
    }
    bool                                enableTransTrajectoriesDirect = true;

    bool                                enableDebugPrint = true;
    bool                                enableDebugPrintProto = true;





    EnablePrintSubMsgFromCloudConfiger           configerPrintSubMsgFromCloud;
    std::string                         rscuEsn = "";
    bool                                hasInited = false;
    MqttConfigerCommon                  configerMqtt;
    MqttConfigerCommon                  configerMqttCloud;
    MqttTopicConfigerRadarID            configerTopicRadarID;

    MqttTopicConfigerCCIndex            configerTopicCCIndex;
    ProjectPathConfigerCommon           configerProjectPath;
    MqttEnableConfigerCCIndex           configerEnableCCIndex;
    ////////////////////////////////////////////////////////
    std::unordered_map<uint32_t, MqttTopicConfigerCCIndex>  topicUnMapCCIndex;
    WorkParamConfiger                   configerWorkParam;
    //////////////////////////////////////////////////////
    //动态数据
    PeriodConfiger                      configerPeriod;
    //静态数据
    HttpConfiger                        httpConfiger;
	std::string categoryUrlValue = "RADAR";
    std::vector<std::string>    outBranchName;
    bool    enableMec = false;
private:
   virtual void writeToFile(ConfigBlock &j) override
    {
        JsonSerialize(enableTransTrajectoriesDirect, "A_enableTransTrajectoriesDirect", j, false);
        JsonSerialize(enableDebugPrint, "A_enableDebugPrint", j, false);
        JsonSerialize(enableDebugPrintProto, "A_enableDebugPrintProto", j, false);
        JsonSerialize(configerPrintSubMsgFromCloud, "A_configerPrintSubMsgFromCloud", j, false);
        JsonSerialize(rscuEsn, "B_rscuEsn", j, false);
        JsonSerialize(hasInited, "C_hasInited", j, false);
        JsonSerialize(configerMqtt, "D_configerMqtt", j, false);
        JsonSerialize(configerMqttCloud, "E_configerMqttCloud", j, false);
        JsonSerialize(configerTopicRadarID, "F_configerTopicRadarID", j, false);
        JsonSerialize(configerProjectPath, "G_configerProjectPath", j, false);
        JsonSerialize(configerEnableCCIndex, "H_configerEnableCCIndex", j, false);
        JsonSerialize(configerTopicCCIndex, "I_configerTopicCCIndex", j, false);
        JsonSerialize(configerWorkParam, "J_configerWorkParam", j, false);
        JsonSerialize(configerPeriod, "K_configerPeriod", j, false);
        JsonSerialize(httpConfiger, "L_httpConfiger", j, false);
        JsonSerialize(categoryUrlValue, "M_categoryUrlValue", j, false);
        JsonSerialize(outBranchName, "N_outBranchName", j, false);
        JsonSerialize(enableMec, "O_enableMec", j, false);

    }

    virtual void readFromFile(const ConfigBlock &j) override
    {
        JsonDeserialize(enableTransTrajectoriesDirect, "A_enableTransTrajectoriesDirect", j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrint, "A_enableDebugPrint", j, noUse_isEmptyFlag);
        JsonDeserialize(enableDebugPrintProto, "A_enableDebugPrintProto", j, noUse_isEmptyFlag);
        JsonDeserialize(configerPrintSubMsgFromCloud, "A_configerPrintSubMsgFromCloud", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "B_rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(hasInited, "C_hasInited", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqtt, "D_configerMqtt", j, noUse_isEmptyFlag);
        JsonDeserialize(configerMqttCloud, "E_configerMqttCloud", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicRadarID, "F_configerTopicRadarID", j, noUse_isEmptyFlag);
        JsonDeserialize(configerProjectPath, "G_configerProjectPath", j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnableCCIndex, "H_configerEnableCCIndex", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopicCCIndex, "I_configerTopicCCIndex", j, noUse_isEmptyFlag);
        JsonDeserialize(configerWorkParam, "J_configerWorkParam", j, noUse_isEmptyFlag);
        JsonDeserialize(configerPeriod, "K_configerPeriod", j, noUse_isEmptyFlag);
        JsonDeserialize(httpConfiger, "L_httpConfiger", j, noUse_isEmptyFlag);
        JsonDeserialize(categoryUrlValue, "M_categoryUrlValue", j, noUse_isEmptyFlag);
        JsonDeserialize(outBranchName, "N_outBranchName", j, noUse_isEmptyFlag);
        JsonDeserialize(enableMec, "O_enableMec", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "A_enableTransTrajectoriesDirect: " << (enableTransTrajectoriesDirect ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "A_enableDebugPrint: " << (enableDebugPrint ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "A_enableDebugPrintProto: " << (enableDebugPrintProto ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "A_configerPrintSubMsgFromCloud: " << (configerPrintSubMsgFromCloud.to_string()) << std::endl;
        ss << std::left << std::setw(40) << "B_rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "C_hasInited: " << (hasInited ? "True" : "False") << std::endl;
        ss << std::left << std::setw(40) << "D_configerMqtt:" << std::endl<< configerMqtt.to_string() << std::endl;
        ss << std::left << std::setw(40) << "E_configerMqttCloud:" << std::endl<< configerMqttCloud.to_string() << std::endl;
        ss << std::left << std::setw(40) << "F_configerTopicRadarID:" << std::endl << configerTopicRadarID.to_string() << std::endl;

        ss << std::left << std::setw(40) << "G_configerProjectPath:" << std::endl << configerProjectPath.to_string() << std::endl;
        ss << std::left << std::setw(40) << "H_configerEnableCCIndex:" << std::endl << configerEnableCCIndex.to_string() << std::endl;
        ss << std::left << std::setw(40) << "I_configerTopicCCIndex:" << std::endl << configerTopicCCIndex.to_string() << std::endl;
        ss << std::left << std::setw(40) << "J_configerWorkParam:" << std::endl << configerWorkParam.to_string() << std::endl;
        ss << std::left << std::setw(40) << "K_configerPeriod:" << std::endl << configerPeriod.to_string() << std::endl;
        ss << std::left << std::setw(40) << "L_httpConfiger:" << std::endl << httpConfiger.to_string() << std::endl;
        ss << std::left << std::setw(40) << "M_categoryUrlValue:" << std::endl << categoryUrlValue << std::endl;
        //ss << std::left << std::setw(40) << "N_outBranchName:" << std::endl << outBranchName << std::endl;

		return ss.str();
    }
};
NAMESPACE_ENDED_RADAR_CCINDEX
#endif