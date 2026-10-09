/*********************************************************************************
* @file		radar_cloud_component
 * @brief		radar_cloud_component belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-12-6
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-12-8 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_OM_COMPONENT_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_OM_COMPONENT_H
#include "middleware/runtime/src/air_middleware_component.h"
#include "base/common/auth/Authenticator.h"
#include "configer_om.h"
#include <iomanip>
#include <sstream>
#include "middleware/protocol/proto/monitor.pb.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
#include "middleware/protocol/om/data_model/mec/data_mec_self_check.h"
NAMESPACE_START_OM
#define OM_COMPONENT AIROS_COMPONENT_CLASS_NAME(OmComponent)
#define LOG_KEY_OM_INTER "[om_inter]"
#define OM_INTER_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_INTER
#define OM_INTER_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_INTER
#define OM_INTER_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_INTER
#define OM_INTER_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_INTER
#define OM_INTER_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_INTER

#define LOG_KEY_OM_CLOUD "[om_cloud]"
#define OM_CLOUD_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_CLOUD
#define OM_CLOUD_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_CLOUD
#define OM_CLOUD_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_CLOUD
#define OM_CLOUD_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_CLOUD
#define OM_CLOUD_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_CLOUD



#define LOG_KEY_OM_MEC "[om_mec]"
#define OM_MEC_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC
#define OM_MEC_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MEC
#define OM_MEC_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MEC
#define OM_MEC_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC
#define OM_MEC_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MEC


#define LOG_KEY_OM_CAMERA "[om_camera]"
#define OM_CAMERA_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_CAMERA
#define OM_CAMERA_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_CAMERA
#define OM_CAMERA_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_CAMERA
#define OM_CAMERA_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_CAMERA
#define OM_CAMERA_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_CAMERA

#define LOG_KEY_OM_CAMERA_ALARM "[alarm_om_camera]"
#define ALARM_OM_CAMERA_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_CAMERA_ALARM
#define ALARM_OM_CAMERA_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_CAMERA_ALARM
#define ALARM_OM_CAMERA_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_CAMERA_ALARM
#define ALARM_OM_CAMERA_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_CAMERA_ALARM
#define ALARM_OM_CAMERA_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_CAMERA_ALARM

#define LOG_KEY_OM_RADAR "[om_radar]"
#define OM_RADAR_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_RADAR
#define OM_RADAR_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_RADAR
#define OM_RADAR_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_RADAR
#define OM_RADAR_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_RADAR
#define OM_RADAR_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_RADAR


#define LOG_KEY_OM_RADAR_ALARM "[alarm_om_radar]"
#define ALARM_OM_RADAR_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_RADAR_ALARM
#define ALARM_OM_RADAR_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_RADAR_ALARM
#define ALARM_OM_RADAR_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_RADAR_ALARM
#define ALARM_OM_RADAR_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_RADAR_ALARM
#define ALARM_OM_RADAR_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_RADAR_ALARM


#define LOG_KEY_OM_MQTT "[om_mqtt]"
#define OM_MQTT_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MQTT
#define OM_MQTT_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MQTT
#define OM_MQTT_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MQTT
#define OM_MQTT_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MQTT
#define OM_MQTT_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MQTT

#define LOG_KEY_OM_MEC_CHECK "[om_check]"
#define OM_CHECK_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK
#define OM_CHECK_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MEC_CHECK
#define OM_CHECK_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MEC_CHECK
#define OM_CHECK_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK
#define OM_CHECK_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MEC_CHECK

#define LOG_KEY_OM_MEC_CHECK_DATA "[om_check_data]"
#define OM_CHECK_DATA_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK_DATA
#define OM_CHECK_DATA_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MEC_CHECK_DATA
#define OM_CHECK_DATA_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MEC_CHECK_DATA
#define OM_CHECK_DATA_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK_DATA
#define OM_CHECK_DATA_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MEC_CHECK_DATA

#define LOG_KEY_OM_MEC_CHECK_LINK "[om_check_link]"
#define OM_CHECK_LINK_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK_LINK
#define OM_CHECK_LINK_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MEC_CHECK_LINK
#define OM_CHECK_LINK_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MEC_CHECK_LINK
#define OM_CHECK_LINK_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MEC_CHECK_LINK
#define OM_CHECK_LINK_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MEC_CHECK_LINK
#define MODULE_CONFIG_DIR "/home/airos/protocol/om"
#define MODULE_NAME "OmComponent"
#define MODULE_CFG_NAME "om.flag"
//#define TEST_CHECK_MEC_STATUS
using namespace os::v2x::protocol;
using namespace afl::base;
using namespace airos::base::workparam;
using namespace os::v2x::protocol;
using namespace os::v2x::protocol::om::mec;
using namespace os::v2x::protocol::om::db;
using namespace airos::monitor_mec;
struct OmMecStatus {
    std::atomic<bool> register_flag{false};
    std::atomic<bool> heartbeat_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss <<
           "[register_flag]" << (register_flag?"true":"false") <<
           "[heartbeat_flag]" << (heartbeat_flag?"true":"false") << std::endl;
        return ss.str();
    }
};

struct OmRadarsStatus {
    std::string deviceEsn = "";
    std::string deviceSn = "";
    std::atomic<bool> register_flag{false};
    std::atomic<bool> heartbeat_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
           "[sn]" << deviceSn  <<
           "[register_flag]" << (register_flag?"true":"false") <<
           "[heartbeat_flag]" << (heartbeat_flag?"true":"false") << std::endl;
        return ss.str();
    }
};
struct OmCameraStatus {
    std::string deviceEsn = "";
    std::string deviceSn = "";
    std::atomic<bool> register_flag{false};
    std::atomic<bool> heartbeat_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
           "[sn]" << deviceSn  <<
           "[register_flag]" << (register_flag?"true":"false") <<
           "[heartbeat_flag]" << (heartbeat_flag?"true":"false") << std::endl;
        return ss.str();
    }
};

struct OmRadarStatusContainer
{
    OmRadarsStatus m_OmRadarsStatus1;
    OmRadarsStatus m_OmRadarsStatus2;
    OmRadarsStatus m_OmRadarsStatus3;
    OmRadarsStatus m_OmRadarsStatus4;
    OmRadarsStatus m_OmRadarsStatus5;
    OmRadarsStatus m_OmRadarsStatus6;
    std::string to_string()
    {
        std::stringstream ss;
        ss <<
            "[m_OmRadarsStatus1]" << m_OmRadarsStatus1.to_string() <<
            "[m_OmRadarsStatus2]" << m_OmRadarsStatus2.to_string() <<
            "[m_OmRadarsStatus3]" << m_OmRadarsStatus3.to_string() <<
           "[m_OmRadarsStatus4]" << m_OmRadarsStatus4.to_string() <<
           "[m_OmRadarsStatus5]" << m_OmRadarsStatus5.to_string() <<
           "[m_OmRadarsStatus6]" << m_OmRadarsStatus6.to_string() << std::endl;
        return ss.str();
    }
};
struct OmCameraStatusContainer
{
    OmCameraStatus m_OmCameraStatus1;
    OmCameraStatus m_OmCameraStatus2;
    OmCameraStatus m_OmCameraStatus3;
    OmCameraStatus m_OmCameraStatus4;
    OmCameraStatus m_OmCameraStatus5;
    OmCameraStatus m_OmCameraStatus6;
    OmCameraStatus m_OmCameraStatus7;
    OmCameraStatus m_OmCameraStatus8;
    OmCameraStatus m_OmCameraStatus9;
    OmCameraStatus m_OmCameraStatus10;
    OmCameraStatus m_OmCameraStatus11;
    OmCameraStatus m_OmCameraStatus12;
    OmCameraStatus m_OmCameraStatus13;
    OmCameraStatus m_OmCameraStatus14;
    OmCameraStatus m_OmCameraStatus15;
    OmCameraStatus m_OmCameraStatus16;
    std::string to_string()
    {
        std::stringstream ss;
        ss <<
           "[m_OmCameraStatus1]" << m_OmCameraStatus1.to_string() <<
           "[m_OmCameraStatus2]" << m_OmCameraStatus2.to_string() <<
           "[m_OmCameraStatus3]" << m_OmCameraStatus3.to_string() <<
           "[m_OmCameraStatus4]" << m_OmCameraStatus4.to_string() <<
           "[m_OmCameraStatus5]" << m_OmCameraStatus5.to_string() <<
           "[m_OmCameraStatus6]" << m_OmCameraStatus6.to_string() <<
            "[m_OmCameraStatus7]" << m_OmCameraStatus7.to_string() <<
            "[m_OmCameraStatus8]" << m_OmCameraStatus8.to_string() <<
            "[m_OmCameraStatus9]" << m_OmCameraStatus9.to_string() <<
            "[m_OmCameraStatus10]" << m_OmCameraStatus10.to_string() <<
            "[m_OmCameraStatus11]" << m_OmCameraStatus11.to_string() <<
            "[m_OmCameraStatus12]" << m_OmCameraStatus12.to_string() <<
            "[m_OmCameraStatus13]" << m_OmCameraStatus13.to_string() <<
            "[m_OmCameraStatus14]" << m_OmCameraStatus14.to_string() <<
            "[m_OmCameraStatus15]" << m_OmCameraStatus15.to_string() <<
            "[m_OmCameraStatus16]" << m_OmCameraStatus16.to_string() <<
           std::endl;
        return ss.str();
    }
};
class OM_COMPONENT : public airos::middleware::ComponentAdapter<os::v2x::device::CloudData>
        , public Configurable<ConfigerOm>
{
public:
    OM_COMPONENT() : Configurable<ConfigerOm>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME)
           , m_FlagReceiveCountHearbeatCamera(0), m_FlagReceiveCountHearbeatRadar(0)
    {
        output_monitor_ = std::make_shared<airos::monitor::MonitorResponse>();
    };

    virtual ~OM_COMPONENT() override
    {
    };
    bool Init() override;
    bool Proc(const std::shared_ptr<const os::v2x::device::CloudData>& recv_data);
    bool doBaiscWork();
    bool initEventLoop();
    bool initEventLoopCloud();
    bool initEventLoopCloudPubFirst();
    bool initEventLoopCloudPubSecond();
    bool initEventLoopOmRadar();
    bool initEventLoopOmCamera();
    bool initEventLoopOmMec();
    bool initEventLoopOmPing();
    bool initEventLoopOmSpat();
    bool initEventLoopOmCheck();
    bool getMqttConfigerMsg();
    bool getWorkParamFromFile();
    std::string getGatewayAddress();
    std::string trim(const std::string& str);
    std::string calculateMD5(const std::string& filename);
    bool monitorMqttConnectStatus();
private:
    static void mqttConnectedCallbackCloud(void* context, char* cause);
    static void mqttConnectedCallback(void* context, char* cause);
    bool mqttInit();

    void mqttDeinit();

    bool mqttInitConnOpts();

    bool mqttConnect();

    bool mqttReconnect();

    static void mqttConnlost(void *context, char *cause);

    static void mqttOnDisconnect(void *context, MQTTAsync_successData *response);

    static void mqttOnConnect(void *context, MQTTAsync_successData *response);

    static void mqttOnConnectFailure(void *context, MQTTAsync_failureData *response);

    static void mqttOnSubscribe(void *context, MQTTAsync_successData *response);

    static void mqttOnSubscribeFailure(void *context, MQTTAsync_failureData *response);

    bool mqttPublishMsg(const std::string &topic, const std::string &msg);

    static int mqttSubscribeMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);

    void mqttDispatchSubscripeMessageOmRadar(const std::string &topic, const afl::base::json & subScribeJson);
    void mqttDispatchSubscripeMessageOmCamera(const std::string &topic, const afl::base::json & subScribeJson);
    bool mqttPublishMsgRetries(std::string topic, std::string payload, std::string hint);
    template<typename T>
    bool mqttPushMsg2Broker(string topic, T info, string hint)
    {
        std::string pData;
        std::string topicProfix = topic.substr(0, topic.find("/"));
        OM_CAMERA_ERROR_PRINT << "[topicProfix]" <<topicProfix ;
        try
        {
            afl::base::json pushInfoJson = info;
            pData = pushInfoJson.dump();
        } catch (json::exception &e)
        {
            if (topicProfix == "camera")
            {
                OM_CAMERA_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else if (topicProfix == "radar")
            {
                OM_RADAR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else
            {
                OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            return false;
        }

        if (!pData.empty())
        {
            if (!mqttPublishMsg(topic, pData))
            {
                if (topicProfix == "camera")
                {
                    OM_CAMERA_ERROR_PRINT << "[error]push " << hint.c_str() << " error!";
                }
                else if (topicProfix == "radar")
                {
                    OM_RADAR_ERROR_PRINT << "[error]push " << hint.c_str() << " error!";
                }
                else
                {
                    OM_MEC_ERROR_PRINT << "[error]push " <<   hint.c_str() << " error!";
                }

                return false;
            } else
            {
                if(getConfiger().enableDebugPrint) {
                    if (topicProfix == "camera")
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_CAMERA_DEBUG_PRINT << "[success]om >>>> inter-mosquitto 【topic】" << topic.c_str() << "[content]" << pData.c_str();
                        }

                    }
                    else if (topicProfix == "radar")
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_RADAR_DEBUG_PRINT << "[success]om >>>> inter-mosquitto 【topic】" << topic.c_str() << "[content]" << pData.c_str();
                        }
                    }
                    else
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "[success]om >>>> inter-mosquitto 【topic】" << topic.c_str() << "[content]" << pData.c_str();
                        }
                    }
                }
            }
        } else
        {
            return false;
        }
        return true;
    }
    bool mqttPushStringMsg2Broker(string topic, string info, string hint);
    bool mqttPushJsonMsg2Broker(string topic, json info, string hint);
private:
    bool getWorkParamFromFileCloud();
    bool getMqttConfigerMsgCloud();
    bool mqttInitCloud();

    void mqttDeinitCloud();

    bool mqttInitConnOptsCloud();

    bool mqttConnectCloud();

    bool mqttReconnectCloud();

    static void mqttConnlostCloud(void *context, char *cause);

    static void mqttOnDisconnectCloud(void *context, MQTTAsync_successData *response);

    static void mqttOnConnectCloud(void *context, MQTTAsync_successData *response);

    static void mqttOnConnectFailureCloud(void *context, MQTTAsync_failureData *response);

    static void mqttOnSubscribeCloud(void *context, MQTTAsync_successData *response);

    static void mqttOnSubscribeFailureCloud(void *context, MQTTAsync_failureData *response);

    bool mqttPublishMsgCloud(const std::string &topic, const std::string &msg);

    static int mqttSubscribeMsgArrvdCloud(void *context, char *topicName, int topicLen, MQTTAsync_message *message);

    void mqttDispatchSubscripeMessageCloud(const std::string &topic, const afl::base::json & subScribeJson);
    bool processSubscribeDataMecFromCloud(const std::string &topic, const afl::base::json & subScribeJson);
    bool processSubscribeDataCameraFromCloud(const std::string &topic, const afl::base::json & subScribeJson);
    bool processSubscribeDataRadarFromCloud(const std::string &topic, const afl::base::json & subScribeJson);
    template<typename T>
    bool mqttPushMsg2BrokerCloud(string topic, T info, string hint)
    {
        std::string pData;
        std::string topicProfix = topic.substr(0, topic.find("/"));
        try
        {
            afl::base::json pushInfoJson = info;
            pData = pushInfoJson.dump();
        } catch (json::exception &e)
        {
            if (topicProfix == "camera")
            {
                OM_CAMERA_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else if (topicProfix == "radar")
            {
                OM_RADAR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else
            {
                OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }

            return false;
        }

        if (!pData.empty())
        {
            if (!mqttPublishMsgCloud(topic, pData))
            {
                if (topicProfix == "camera")
                {
                    OM_CAMERA_ERROR_PRINT << "[error]push " <<   hint.c_str() << " error!";
                }
                else if (topicProfix == "radar")
                {
                    OM_RADAR_ERROR_PRINT << "[error]push " <<   hint.c_str() << " error!";
                }
                else
                {
                    if(topic.find("heartbeat"))
                    {
                        m_OmMecStatus.heartbeat_flag = false;
                    }
                    OM_MEC_ERROR_PRINT << "[error]push " <<   hint.c_str() << " error!";
                }
                if (m_MqttClientCloud)
                {
                    OM_MEC_ERROR_PRINT << "[cloud][error]Connection is lost, reconnect cloud!";
                    mqttReconnectCloud();
                }
                else
                {
                    mqttInitCloud();
                }
                return false;
            }
            else
            {
                if (getConfiger().enableDebugPrint)
                {
                    OM_MQTT_DEBUG_PRINT << "[om-->cloud]" << topic;
                }

                if(getConfiger().enableDebugPrint)
                {
                    if (topicProfix == "camera")
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_CAMERA_WARN_PRINT << "[success]om >>>>>>> cloud" << "【topic】" << topic.c_str() << " [content]" << pData.c_str();
                        }
                    }
                    else if (topicProfix == "radar")
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_RADAR_WARN_PRINT << "[success]om >>>>>>> cloud" << "【topic】" << topic.c_str() << " [content]" << pData.c_str();
                        }
                    }
                    else
                    {
                        if(topic.find("heartbeat"))
                        {
                            m_OmMecStatus.heartbeat_flag = true;
                        }
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_MEC_WARN_PRINT << "[success]om >>>>>>> cloud " << "【topic】" << topic.c_str() << " [content]" << pData.c_str();
                        }
                    }

                }
            }
        } else
        {
            return false;
        }
        return true;
    }
    bool mqttPushStringMsg2BrokerCloud(string topic, string info, string hint);
    bool mqttPushJsonMsg2BrokerCloud(string topic, json info, string hint);
private:
    ///////////////////////////////////////////////////////////////////
    void onConnectedPushMsgOnce();
    void  initProcessEventOutputResult();
    void  processProcessEventOutputResult(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);

    std::string getIPFromString(const std::string& input);
    std::string getSoftwareVersion();
    //设备信息查询
    void subscribeDeviceInfoQuery(const json &contentJson);
//    std::string extractVersion(const std::string& input);
    //设备基础信息
    void omMecConfigerMonitor();
    void subscribeDeviceBaseInfo(const json &contentJson);
    void publishDeviceBasicInfoData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData,
                                    DeviceBasicInfoDeviceTypeEnum deviceBasicInfoDeviceType, BASIC_INFO_OPT_TYPE basicInfoOptType);
    void updateMecStatusInfo();
    int transSensorType(int statusDeviceType);
    bool parseRsuMapXml(const char *fname);

    ///////////////////////////////////////////////////////////////////
    //配置查询
    void subscribeConfigQueryData(const json & subscribeJson);
    void publishConfigQueryAckData(ConfigQueryData &configQueryData, CONFIG_QUERY_INFO_OPT_TYPE opt_type);
    //配置下发
    void subscribeConfigUpdateData(const json & subscribeJson);
    void pulishConfigUpdateData(ConfigUpdateData& configUpdateData);
    void pulishConfigUpdateAckData(ConfigUpdateData& configUpdateData);
    bool processConfigUpdate(ConfigUpdateData& configUpdateData, ConfigUpdateAckData& configUpdateAckData);
    void updateLogLevel(const std::string& newLogLevel);
    ///////////////////////////////////////////////////////////////////
    //性能管理
    void publishPerformenceData(DeviceBaseInfoQueryData deviceBaseInfoQueryData, bool periodPushFlag);
    std::vector<double> extractValues(const std::string& data_str);
    double getMemoryUsagePercentage(const std::string& freeOutput);
    std::string executeCommand(const std::string& command);
    bool getRuningInfoFromDbMecStatusTable(RunningInfo& runningInfo);
    //运行状态
    void publishRunningStatusData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData, DeviceBasicInfoDeviceTypeEnum deviceBasicInfoDeviceType);
    //告警管理
    void publishAlarmManagementData(DeviceBaseInfoQueryData deviceBaseInfoQueryData, bool periodPushFlag);
    /// 告警产生
    bool alarmOccurred(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo = "",
                       std::string addition = "",   std::string dbTableName="mec-alarm", TABLE_TYPE tableType=TABLE_TYEP_MEC_DEV_ALARM, uint64_t faultStartTime = 0 );
    /// 告警消失
    bool alarmDisappeared(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo = "",
                          std::string addition = "",  uint64_t alarmErrorOccuredTimeStamp = 0,std::string dbTableName="mec-alarm", TABLE_TYPE tableType=TABLE_TYEP_MEC_DEV_ALARM);
    /// 告警监控
    void alarmMonitor();
    /// 处理角度偏移告警
    void processAlarmSensorAngleOffset(const std::string &topic, const afl::base::json & subScribeJson);
    /// 监控角度偏移告警
    void monitorAngleOffset();
    std::string getAlarmTypeInfo(MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode);
    //设备心跳
    void publishDeviceHeartbeatData();
    //版本信息
    void publishDeviceVersionData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData, bool queryFlag = true);

    //OTA升级
    bool processOtaUpdateDownData(OTAUpdateDownData& otaUpdateDownData);
    void subscribeOtaDownData(const json &contentJson);
    void publishOtaStatusData();
    void subscribeOtaStatusAckData();
    void threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData);
    void monitorDeviceNeedUpdate();
    long long getFileSize(const std::string& path);
    void handleErrorResponse(CURLcode code);
    bool checkOtaUpdateVersion();
    //ota升级取消
    void subscribeOtaCancelData(const json &contentJson);
    //设备重启
    void subscribeRebootData(const json &contentJson);
    void monitorRebootTimeConfiger();
    ///////////////////////////////////////////////////////////////////

//    double  getCpuUse(CPU_OCCUPY *o,CPU_OCCUPY *n);

    ///////////////////////////////////////////////////////////////////
    void upPtpStatus(bool isPublish=true);
    void processHttpPostRequest();
    void getUrlsBytime(std::string directory, std::vector<std::string>& urls, int64_t start_time, int64_t end_time, std::string datatype, std::string devicesn);
    NetworkInfo getNetworkInfo(const std::string& interface_name);

    //违法事件接口相关函数
    bool startHttpServer();
    bool tcpinit();
    void onTcpConnected(const afl::net::TcpConnectionPtr &conn);
    void onTcpMessage(afl::net::ByteBuffer *buffer);
    void processCameraEventXML(pugi::xml_document &doc, bool have_jpg);
    void upIllegalinfoXML2Cloud(std::string illegalinfo, std::vector<std::string> images);
    void upCameraEventXML2Mec(std::string cameraevent);
    void checkImagesTime();
    //场景
    void initProcessAngleOffsetData();
    void processAngleOffsetData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    void initProcessSpatSrcData();
    void processSpatSrcData(const std::shared_ptr<const os::v2x::device::TrafficLightBaseData>& traffic_light_data_pb);
    void initProcessV2xBsmData();
    void processV2xBsmData(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame);
    bool V2xPbAsnMessageFrame2Str(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame);
    std::vector<uint8_t> stringToVector(const std::string& str);
    std::string base64Encode(const std::vector<uint8_t>& input);
    void initProcessV2xData();
    std::string AsnTypeToString(EnAsnType type);
    void processV2xData(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame);
    bool V2xPbAsnMessageFrame2StrBroad(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame);

    //信号机检测
    void initProcessTrafficlightDetectData();
    void processTrafficlightDetectData(const std::shared_ptr<const airos::usecase::EventOutputResult> &trafficlightDetectData);
    void publishTrafficlightDetectData();
    //相机内外参数
    std::string extractDeviceID(const std::string& command, std::string regexProfix = "camera");
    bool getCameraInterExterParamFromFile();
    void publishCameraInterExterParam();
    bool findJpgFiles(const std::string& baseDir);
    std::string getSnFromFileName(const std::string& fileName);
    std::string extractFtpBaseUrl();
    bool queryCameraInterExterParam();
    std::string extractIpFromFileName(const std::string& fileName);

    bool timerMonitorRadarHearBeatData(uint64_t lastValue, std::string deviceEsn);
    bool timerMonitorCameraHearBeatData(uint64_t lastValue, std::string deviceEsn);
    std::string getDirectoryPath(const std::string& filePath);
    std::string getDirNameFromFileName(const std::string& fileName);
    std::string transVector2String(std::vector<double> paramV);
    //雷达内外参数
    void publishRadarInterExterParam();
    bool queryRadarInterExterParam();
    //获取运行状态
    void getPerformanceData();

    //监控设备离线
    bool processCameraHearBeat(std::string topic);
    bool monitorCameraHeartBeat(std::string topicTemp);
    bool monitorRadarHeartBeat(std::string topicTemp);
    bool processRadarHearBeat(std::string topic);

    //监控运行情况
    void periodMonitorOmStatus();

    std::string getContentBetweenFirstAndSecondSlash(const std::string& input);
    bool periodMonitorSensorDeviceActive();

    //从集和诚获取设备授时情况
    void getTimingInfo();
private:
    //违法事件接口相关参数
    std::unique_ptr<std::thread> m_TaskHttpCamera;
    afl::base::json m_jlocation;
    std::string m_imageDir = "";
    std::string m_mecIp = "127.0.0.1";
    std::string m_tcpIp = "127.0.0.1";
    std::string m_httpPath = "/";
    int m_tcpPort = 40414;
    int m_camerahttpPort = 40415;
    int m_ftpPort = 40416;
    std::map<std::string, std::string> m_imageNameMap;//<name, fullname>, name = fileName, fullname = UUID + absTime + fileName
    std::map<std::string, uint64_t> m_imageTimeMap;//<fullname, recvtime>
    std::unique_ptr <afl::net::TcpClient> m_TcpClient;
    afl::net::TcpConnectionPtr m_CurrConn;
    bool m_ConnetedFlag = false;
    std::unique_ptr<afl::net::ByteBuffer> m_tcpBuf;
    uint16_t m_tcpHeader = 0xfdfd;
    uint16_t m_tcpTail = 0xffff;

private:
    std::unique_ptr<std::thread>    m_Task;
    std::unique_ptr<std::thread>    m_TaskCloud;
    std::unique_ptr<std::thread>    m_TaskCloudPubFirst;
    std::unique_ptr<std::thread>    m_TaskCloudPubSecond;
    std::unique_ptr<std::thread>    m_TaskOmRadar;
    std::unique_ptr<std::thread>    m_TaskOmCamera;
    std::unique_ptr<std::thread>    m_TaskOmMec;
    std::unique_ptr<std::thread>    m_TaskHttp;
    std::unique_ptr<std::thread>    m_TaskOtaDown;
    std::unique_ptr<std::thread>    m_TaskOmPing;
    std::unique_ptr<std::thread>    m_TaskOmSpat;
    std::unique_ptr<std::thread>    m_TaskOmCheck;
    /////////////////////////////////////////////////////////////
    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopCloud;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopCloudPubFirst;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopCloudPubSecond;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmRadar;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmCamera;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmMec;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmPing;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmSpat;

    std::shared_ptr<afl::net::EventLoop>    m_EventloopOmCheck;


    std::unique_ptr<afl::net::Channel>      m_Chnl;
    MQTTAsync_connectOptions        m_MqttConnOpts;
    MQTTAsync                       m_MqttClient;
    std::atomic<bool>               m_MqttConnected{false};
    ConfigerOm 	            m_MqttClientConfig;
    std::shared_ptr <os::v2x::device::CloudData> m_CloudDataPtr = nullptr;
    bool m_HasSendOnce = false;

    static std::unique_ptr<afl::net::ByteBuffer> m_Buffer;
    /////////////////////////////////////////////////////////////////////////////////////
    std::string 		    m_UploadAsPlain;
    uint64_t                m_SeqNum = 0;
    ConfigQueryData m_ConfigQueryData;

    CURL *	m_DownloadHttpHandle = nullptr;
    CURLcode m_DownloadRepCode = CURLE_OK;
    std::mutex 				m_MutexDownload;		//互斥锁全局变量
    bool 					m_IsGetDownResponseSucess = false;
    std::string             m_OtaSeqNum = "";
    std::string 			m_DownloadChkPara ;
    std::unique_ptr<std::thread>    m_OtaDownloadTask;

    int m_TimerPublishDeviceHeartBeatData = -1;
    int m_TimerPublishDeviceRunningStatusData = -1;
    int m_TimerMaintenanceManagementRebootFd = -1;
    int m_TimerMaintenanceManagementRebootMonitorFd = -1;
    int m_OtaUpdateFd = -1;

    ////////////////////////////////////////////////////////////////////
    int m_ptpStatusUpTimer = -1;
    string m_videoAndPicturePath = "";
    string m_ftpUrl = "";
    string m_postPath = "";
    string m_httpHost = "";
    int m_httpPort = 0;
    ////////////////////////////////////////////////////////////////////
    int m_ConfigeUpdateTimer = -1;

    int m_TimerPublishPerformenceData = -1;

    int m_TimerPublishBasicDeviceInfoDataOnce = -1;

    bool m_RegisterFlag = false;
    int m_TimerDeviceVersion = -1;
    int m_TimerUpdateConfigPowerOff = -1;
    int m_TimerUpdateConfigPowerOn = -1;
    int m_TimerOtaUpdateDownData = -1;
    int m_TimerAlarmMonitor = -1;
    AlarmTypeFlag m_AlarmTypeFlag;
    OmWorkParamConfiger  m_OmWorkParamConfiger;

    int m_TimerMonitorOmWorkParamConfiger = -1;
    std::string m_OtaDownSeqnum = "";
    std::string m_OtaCancleSeqnum = "";
    bool m_OtaIsExecBashOnce = false;


    bool alarmSensorAngleOffsetReceiv = false;
    int m_TimerMonitorAlarmSensorAngleOffset = -1;
    bool alarmSensorAngleOffsetTimerPublishOnceFlag = false;  //是否周期上报完成1次
    std::unique_ptr<std::thread>    m_TaskProcessEventOutputResult;
    std::unique_ptr<std::thread>    m_TaskProcessSpatSrcData;
    std::unique_ptr<std::thread>    m_TaskProcessV2xBsmData;
    std::unique_ptr<std::thread>    m_TaskProcessV2xData;
    std::unique_ptr<std::thread>    m_TaskProcessTrafficlightDetectData;
	std::unique_ptr<std::thread>    m_TaskProcessTiming;
    std::unique_ptr<std::thread>    m_TaskProcessMecCheck;
    std::unique_ptr<std::thread>    m_TaskProcessMonitorSpatData;  //信号机数据状态
    EnAsnType                               m_AsnType{EnAsnType::YDT_3709_2020_EXT};
    int                                     m_MsgType;
    enum MESSAGE_TYPE{MT_BSM = 111, MT_MAP=3618, MT_SPAT=3619, MT_RTE=3622, MT_RTS=3620,  MT_RSM=3623};
private:
    MQTTAsync_connectOptions        m_MqttConnOptsCloud;
    MQTTAsync                       m_MqttClientCloud;
    std::atomic<bool>               m_MqttConnectedCloud{false};
    ConfigerOm 	                    m_MqttClientConfigCloud;
    uint64_t                        m_MqttReconnectCountCloud = 0;
    bool                            m_Md5NoEqualPushOnced = false;

    int                             m_TimerMqttReconnectInter = -1;
    int                             m_TimerMqttReconnectCloud = -1;
    //相机内外参
    std::map<std::string, CameraParamMapFromFileConfiger> m_CameraParamMapFromFile;  //sn,
    int m_TimerPublishCameraInterExterParam = -1;
    int m_TimerGetCameraInterExterParam = -1;
    bool m_FlagGetCameraCalibrationAll = false;  //全部相机是否已经推送标识

    std::map<std::string, std::string> m_CameraParamPathNameMap;    //fullPath, fileName
    int m_TimerQueryCameraInterExterParam = -1;
    //雷达内外参
    int m_TimerPublishRadarInterExterParam = -1;
    int m_TimerQueryRadarInterExterParam = -1;
    std::map<std::string, RadarParamMapFromFileConfiger> m_RadarParamMapFromFile; //esn,

    ///////////////////////////////////////////////////////////
    std::map<std::string, CameraRegisterMapConfiger> m_CameraRegisterMap; //esn, 是否注册
    int m_TimeMonitorCameraRegister = -1;
    std::atomic<uint64_t> m_FlagReceiveCountHearbeatCamera;
    ///////////////////////////////////////////////////////////

    std::map<std::string, RadarRegisterMapConfiger> m_RadarRegisterMap;  //esn,是否注册
    int m_TimeMonitorRadarRegister = -1;
    std::atomic<uint64_t> m_FlagReceiveCountHearbeatRadar;
    uint64_t m_FlagReceiveCountHearbeatRadarOld = 0;
    ///////////////////////////////////////////////////////////
    //信号机检测
    int m_TimerPublishTrafficlightDetectData = -1;
    int m_TimerPublishTrafficlightDetectDataInit = -1;
    std::map<TrafficlightDetectStatus, uint64_t> m_TLfaultMap;//记录信号灯故障码以及故障时间
    ///////////////////////////////////////////////////////////
    //s和ms之间换算单位
    uint32_t m_PublishAlarmDataIntervalUnit = 0;
    ///////////////////////////////////////////////////////////
    std::map<int, AlarmManagementData> m_MecAlarmMap;  //alaramType, 告警结构体
    PerformanceData m_PerformanceData;
    int m_TimerGetPerformanceData = -1;
    std::mutex 				m_MutexPerformanceData;		//PerformanceData互斥锁全局变量
    ///////////////////////////////////////////////////////////
    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicCameraHeartBeatMap;  //esn, <timerFd, 是否收到新的数据>
    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicRadarHeartBeatMap;   //

    std::mutex 				m_MutexCameraPing;
    std::unordered_map<std::string,  bool>  topicCameraPingMap;   // ping 作为设备是否离线的判断
    std::mutex 				m_MutexRadarPing;
    std::unordered_map<std::string,  bool>  topicRadarPingMap;   //  ping 作为设备是否离线的判断  esn:activate


    ///////////////////////////////////////////////////////////
    //监控
    std::shared_ptr <airos::monitor::MonitorResponse> output_monitor_ = nullptr;
    OmMecStatus m_OmMecStatus;
    OmRadarStatusContainer m_OmRadarStatusContainer;
    OmCameraStatusContainer m_OmCameraStatusContainer;

    int  m_TimerMonitorOmStatus = -1;
    std::map<std::string, std::string> m_EsnSnMap;

    int  m_TimingCount = 0;
    int64_t m_last_lastTime = -9999;
    int64_t m_last_timeDifference = -9999;
    std::atomic<uint64_t> m_gpsTime{0};//gps最后一次授时时间,ms
    std::atomic<uint64_t> m_ptpTime{0};//ptp最后一次授时时间,ms
    std::atomic<int64_t> m_gpsTimeDifference{0};//gps授时时差
    std::atomic<int64_t> m_ptpTimeDifference{0};//ptp授时时差
    std::atomic<uint8_t> m_currentSource{2};//授时源，0:GPS，1:PTP，2:other
    int m_TimerMonitorMqttConnected = -1;

    int m_TimerPingSensorDevice = -1;
	bool m_isTrafficlightNormal = true;
	
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string m_UsingVersion = "";
    std::atomic<bool>  m_CancelDownloadFlag{false};
    std::string m_UpdateVersionFromCancleMsg = ""; //取消升级的版本号
    std::string m_UpdateVersionFromDownMsg = ""; //升级版本号来自云端下发
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    void  initProcessMecCheckSpatSrcData();
    void  processMecCheckSpatSrcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckRsapData();
    void  processMecCheckRsapData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckRsapLink();
    void  processMecCheckRsapLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexTcData();
    void  processMecCheckCcindexTcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexTmData();
    void  processMecCheckCcindexTmData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexTmLink();
    void  processMecCheckCcindexTmLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexStData();
    void  processMecCheckCcindexStData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexStLink();
    void  processMecCheckCcindexStLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexRcpData();
    void  processMecCheckCcindexRcpData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);

    void  initProcessMecCheckCcindexRcpLink();
    void  processMecCheckCcindexRcpLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &recvData);
    //mec自检 ：信号机原始数据
    std::unique_ptr<std::thread>    m_TaskProcessMecCheckSpatSrcData;
    MonitorStatePtr                     m_MonitorStateSpatSrcData; // [修改] 改为指针

    //mec自检： 感知数据
    std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapData;
    MonitorStatePtr                     m_MonitorStateRsapData;    // [修改] 改为指针

    //mec自检： V2X信控数据
    std::unique_ptr<std::thread>    m_TaskProcessMecCheckCcindexTcData;
    MonitorStatePtr                 m_MonitorStateCcindexTcData; // [修改] 改为指针

    // --- 雷达动态数据 (优化后) ---
    std::unique_ptr<std::thread> m_TaskProcessMecCheckCcindexTmData;
    std::mutex m_CcindexTmMapMutex;
    // Map<DeviceId, Context>
    std::map<std::string, CcindexTmDeviceContextPtr> m_MonitorStatesCcindexTmData;
    std::map<std::string, TopicCCIndexTm> m_MecSelfCheckTmDeviceIdTopicMap;

    // --- 雷达静态数据 (优化后) ---
    std::unique_ptr<std::thread> m_TaskProcessMecCheckCcindexStData;
    std::mutex m_CcindexStMapMutex;
    // Map<DeviceId, Context>
    std::map<std::string, CcindexStDeviceContextPtr> m_MonitorStatesCcindexStData;
    std::map<std::string, TopicCCIndexSt> m_MecSelfCheckStDeviceIdTopicMap;


    //mec自检： 点云数据
    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpData;
    std::map<std::string, MonitorStatePtr>  m_MonitorStateCcindexRcpData;
    /////////////////////////////////////////////////////////////////////////
    //mec自检： 云控链路
    std::unique_ptr<std::thread>    m_TaskProcessMecCheckRsapLink;
    MonitorStatePtr                 m_MonitorStateRsapLink;      // [修改] 改为指针


    //mec自检： 雷达动态链路
    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexTmLink;
    MonitorStatePtr                         m_MonitorStateCcindexTmLink; // [修改] 改为指针

    //mec自检： 雷达静态链路
    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexStLink;
    MonitorStatePtr                         m_MonitorStateCcindexStLink; // [修改] 改为指针

    //mec自检： 点云链路
    std::unique_ptr<std::thread>            m_TaskProcessMecCheckCcindexCcindexRcpLink;
    MonitorStatePtr                         m_MonitorStateCcindexRcpLink; // [修改] 改为指针


    void handleMonitorDataUpdateCcindexTm(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp);
    void handleMonitorDataUpdateCcindexSt(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp);

    // MEC自检结果： 监控是否超时
    void checkMonitorSpatSrcDataTimeouts();
    void checkMonitorRsapDataTimeouts();
    void checkMonitorCcindexTcDataTimeouts();
    void checkMonitorCcindexTmDataTimeouts();
    void checkMonitorCcindexStDataTimeouts();
    void checkMonitorRcpDataTimeouts();
    void checkMonitorRcpDataTimeoutsImpl();

    void checkMonitorMecDataTimeoutsImpl(MonitorState& state, int tag);


    long m_TimerMonitorSpatSrcData = -1;
    long m_TimerMonitorRsapData = -1;
    long m_TimerMonitorRsapLink = -1;
    long m_TimerMonitorCcindexTcData = -1;
    long m_TimerMonitorCcindexTmData = -1;
    long m_TimerMonitorCcindexStData = -1;
    long m_TimerMonitorCcindexTmLink = -1;
    long m_TimerMonitorCcindexStLink = -1;
    long m_TimerMonitorCcindexRcp = -1;
    long m_TimerMonitorCcindexRcpData = -1;
    int m_Second2MSUnit = 1000;
    // 初始化监控规则
    void initMonitorRules();
    // 处理单个数据类型的监控逻辑
    void handleMonitorDataUpdate(MonitorState &state, int tag, uint64_t timestamp);

    // 处理单个链路类型的监控逻辑
    void handleMonitorLinkUpdate(MonitorState &state, int tag, bool status, uint64_t timestamp);

    void handleMonitorDataUpdateRcpData(const std::string & deviceId, int tag, uint64_t timestamp);

    // 获取故障类型映射，返回0表示该告警不需要上报自检数据
    int getSelfCheckFaultType(MecAlarmTypeErrorCodeEnum alarmType);

    // 发送自检结果消息
    void sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType,  int faultType, int faultStatus, uint64_t startTime, uint64_t stopTime, const std::string& desc);
    // ==========================================
    // [新增] 4. 辅助函数声明
    // ==========================================
    int getCcindexTmTopicIndex(const std::string& topic, const TopicCCIndexTm& config);
    int getCcindexStTopicIndex(const std::string& topic, const TopicCCIndexSt& config);
    std::vector<int> targetIndices = {1, 3, 6};
    std::string extractAndJoin(const std::string& path, const std::vector<int>& indices, char delimiter = '/');

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string getTLFaultDesc(TrafficlightDetectStatus tlds);
    //信号灯状态检测
    std::atomic<bool> m_SpatDataStatusFromTLAdapterFlag{false}; //是否接收到信号机适配器的信号机数据状态,
    std::atomic<bool> m_SpatDataStatusFromTLDeviceFlag{false};  //是否接收到信号机设备的信号机数据状态,
    int  m_TimerSpatDataStatus = -1;
    //信号机数据状态
    bool periodMonitorSpatDataStatus();
    void  initProcessMonitorSpat();
    void processMonitorSpatDataStatus(const std::shared_ptr<const airos::monitor_mec::MonitorSpat> &recvData);
    uint64_t m_lastSpatDataPublishTime = 0;     // 记录上次 SPAT 数据状态上报的时间戳 (毫秒)
};

REGISTER_AIROS_COMPONENT_CLASS(OmComponent, os::v2x::device::CloudData);
NAMESPACE_ENDED_OM
#endif
