/*
* @Author: zhangenwei
* @Date: 2024-05-15 10:16:15
* @LastEditors: zhangenwei
* @LastEditTime: 2024-05-15 10:16:15
* @Description: 云控管理平台南向接口交互-摄像机与云控
*/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_RADAR_COMPONENT
#define AIROS_MIDDLEWARE_PROTOCOL_OM_RADAR_COMPONENT
#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include  "configer_om_device_status.h"
#include "base/common/network/print.h"
#include "middleware/protocol/om_common/sqlite_device_status.h"
#include "middleware/protocol/om_common/performence_utils.h"
#include "base/common/auth/Authenticator.h"

NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
#define OM_DEVICE_STATUS_COMPONENT AIROS_COMPONENT_CLASS_NAME(OmDeviceStatusComponent)
using namespace os::v2x::protocol;
using namespace os::v2x::protocol::om::db;
using namespace afl::base;
using namespace os::v2x::protocol::om::mec;
#define MODULE_CONFIG_DIR "/home/airos/protocol/ds"
#define MODULE_NAME "OmDeviceStatusComponent"
#define MODULE_CFG_NAME "om_device_status.flag"
//#define USE_RUNLOG
class OM_DEVICE_STATUS_COMPONENT : public airos::middleware::ComponentAdapter<os::v2x::device::CloudData>
   , public Configurable<MqttClientConfigerDeviceStatus>
{
public:
    OM_DEVICE_STATUS_COMPONENT() : Configurable<MqttClientConfigerDeviceStatus>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME){};

    virtual ~OM_DEVICE_STATUS_COMPONENT() override
    {
    };

    bool Init() override;

    bool Proc(const std::shared_ptr<const os::v2x::device::CloudData>& recv_data);
    void doBaiscWork();
    bool initEventLoop();
    bool initEventLoopPing();
    bool getMqttConfigerMsg();
    bool mqttPushStringMsg2Broker(string topic, string info, string hint);
    bool mqttPushJsonMsg2Broker(string topic, json info, string hint);

    bool monitorCameraHeartBeat(std::string topicTemp);
    bool processCameraHearBeat(std::string topic);
    bool monitorCameraSynchronize(std::string topicTemp);
    bool processRadarSynchronize(std::string topic, const afl::base::json &subScribeJson);


    bool monitorRadarHeartBeat(std::string topicTemp);
    bool processRadarHearBeat(std::string topic);
    bool monitorRadarSynchronize(std::string topicTemp);
    bool processCameraSynchronize(std::string topic, const afl::base::json &subScribeJson);
    std::string getDeviceID(const std::string& topic);
    bool initTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string topic,
                              int deviceType, int deviceStatus);
    bool initSynchronizeTable(std::string topic);
    bool getWorkParamFromFile();
    bool periodMonitorSensorDeviceActive();
    void processCameraAngleOffset(const afl::base::json &subScribeJson);
    bool alarmOccurred(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo = "",
                      bool isUpdateSensorStatus = false, std::string addition = "", std::string dbTableName="mec-alarm", TABLE_TYPE tableType=TABLE_TYEP_MEC_DEV_ALARM);
    bool alarmDisappeared(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode,
                         std::string sensorDeviceNo = "", std::string dbTableName="mec-alarm", TABLE_TYPE tableType=TABLE_TYEP_MEC_DEV_ALARM);
    void alarmMonitor();
    bool updateMecStatus(std::string hint,  std::string deviceNo,  TABLE_UPDATE_MEC_STAUTS_OP_TYPE opType, PerformanceData& performanceData,
                                                    std::string dbTableName="mec-status", TABLE_TYPE tableType=TABLE_TYEP_MEC_DEV_STATUS);
    bool processRadarRegister(std::string topic, const afl::base::json &subScribeJson);
    bool processCameraRegister(std::string topic, const afl::base::json &subScribeJson);
    void processTiming(std::string topic, const afl::base::json &subScribeJson);


    bool  processRadarAlarm(std::string topic, const afl::base::json &subScribeJson);
    bool  monitorRadarAlarm(std::string topic, std::string keyTemp);
    bool  processCameraAlarm(std::string topic, const afl::base::json &subScribeJson);
    bool  monitorCameraAlarm(std::string topic, std::string keyTemp);

private:
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

    void mqttDispatchSubscripeMessage(const std::string &topic, const afl::base::json & subScribeJson);

    template<typename T>
    bool mqttPushMsg2Broker(string topic, T info, string hint)
    {
       std::string pData;

       try
       {
           afl::base::json pushInfoJson = info;
           pData = pushInfoJson.dump();
       } catch (json::exception &e)
       {
           OM_DS_ERROR_PRINT<< "[what]" <<  e.what() << "[json-exception-id]" << e.id;
           return false;
       }

       if (!pData.empty())
       {
           if (!mqttPublishMsg(topic, pData))
           {
               OM_DS_ERROR_PRINT << "[error]push " << hint.c_str() << " error!";
               return false;
           } else
           {
                if(getConfiger().enableDebugPrint){
                    OM_DS_DEBUG_PRINT << "[topic]" << topic.c_str() << " [content]" << pData.c_str();
                }
           }
       } else
       {
           return false;
       }
       return true;
    }

private:
    std::unique_ptr<std::thread>    m_Task;
    std::unique_ptr<std::thread>    m_TaskPing;
    /////////////////////////////////////////////////////////////
    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopPing;

    std::unique_ptr<afl::net::Channel>      m_Chnl;
    MQTTAsync_connectOptions        m_MqttConnOpts;
    MQTTAsync                       m_MqttClient;
    bool                            m_MqttConnected = false;
    MqttClientConfigerDeviceStatus 	            m_MqttClientConfig;

    int m_UpdateDeviceStatusTimer = -1;
    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicCameraUnMap;  //topic, <timerFd, 是否收到新的数据>
    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicRadarUnMap;
//    std::pair<std::string, TABLE_TYPE> tableInfo;  //tableName :
    int m_TimerPingSensorDevice = -1;

    int     m_TimerAlarmMonitor = -1;
    AlarmTypeFlag m_AlarmTypeFlag;
    AlarmTypeFlag m_SensorAlarmTypeFlag;

    std::unordered_map<uint32_t, MqttTopicConfiger>  m_RadarTopicUnMapSubscribe;
    std::unordered_map<uint32_t, MqttTopicConfiger>  m_CameraTopicUnMapSubscribe;
	std::string dbPath = "/home/airos/protocol/device-status.db";
    std::string dbShmPath = "/home/airos/protocol/device-status.db-shm";
    std::string dbWalPath = "/home/airos/protocol/device-status.db-wal";

    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicCameraAlarmUnMap;  //topic-alarmType, <timerFd,>
    std::unordered_map<std::string, std::pair<uint32_t, bool>>  topicRadarAlarmUnMap;


};

REGISTER_AIROS_COMPONENT_CLASS(OmDeviceStatusComponent, os::v2x::device::CloudData);
NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
#endif
