/*
* @Author: zhangenwei
* @Date: 2024-02-18 9:20:01
* @LastEditors: zhangenwei
* @LastEditTime: 2024-02-20 9:21:11
* @Description:
*/
#include "om_device_status_component.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS

bool OM_DEVICE_STATUS_COMPONENT::Init()
{

#if ENABLE_ENCRYPTION
    auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
    std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
    OM_DS_DEBUG_PRINT << "License is: " << license << std::endl;
    int result = FusionService::Authenticator::GetInstance().Authorize(license);
    if (result != 0)
    {
        OM_DS_ERROR_PRINT << "License generated fail, error code:  " << result;
        exit(1);
    }
#endif

    if( afl::FileUtil::isFileExist(dbPath.c_str()))
    {
        std::string dropTableCmd = "rm " + dbPath;
        if(getConfiger().enableDebugPrint) {
            OM_DS_DEBUG_PRINT << "[dropTableCmd]" << dropTableCmd;
        }
        system(dropTableCmd.c_str());
    }
    if( afl::FileUtil::isFileExist(dbShmPath.c_str()))
    {
        std::string dropTableCmd = "rm " + dbShmPath;
        if(getConfiger().enableDebugPrint) {
            OM_DS_DEBUG_PRINT << "[dropTableCmd]" << dropTableCmd;
        }
        system(dropTableCmd.c_str());
    }
    if( afl::FileUtil::isFileExist(dbWalPath.c_str()))
    {
        std::string dropTableCmd = "rm " + dbWalPath;
        if(getConfiger().enableDebugPrint) {
            OM_DS_DEBUG_PRINT << "[dropTableCmd]" << dropTableCmd;
        }
        system(dropTableCmd.c_str());
    }

    m_Task.reset(new std::thread([&](){ initEventLoop(); }));
    m_TaskPing.reset(new std::thread([&](){ initEventLoopPing(); }));

   doBaiscWork();
   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::initEventLoop()
{
   m_Eventloop = std::make_shared<afl::net::EventLoop>();
   m_Eventloop->loop();
   return true;
}
bool OM_DEVICE_STATUS_COMPONENT::initEventLoopPing()
{
    m_EventloopPing = std::make_shared<afl::net::EventLoop>();
    m_EventloopPing->loop();
    return true;
}
bool OM_DEVICE_STATUS_COMPONENT::Proc(const std::shared_ptr<const  os::v2x::device::CloudData>& recv_data)
{
   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::getWorkParamFromFile()
{
   OmWorkParamConfiger  omWorkParamConfiger;
   WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
//    if(getConfiger().enableDebugPrint) {
//        OM_DS_DEBUG_PRINT << "[work-param]" << omWorkParamConfiger.to_string().c_str();
//    }
   bool hasCameraErased = false;
   bool hasRadarErased = false;
   if(!hasCameraErased)
   {
       getConfiger().configerTopicCameraRcId.RcIds.clear();
       hasCameraErased = true;
   }
   if(!hasRadarErased)
   {
       getConfiger().configerTopicRadarID.radarIDs.clear();
       hasRadarErased = true;
   }
    getConfiger().rscuEsn = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
   for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
   {
       if(v.deviceType == WorkParamDeviceTypeCamera && !v.deviceEsn.empty())
       {
           getConfiger().configerTopicCameraRcId.RcIds.push_back(v.deviceEsn);
       }
       if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
       {
           getConfiger().configerTopicRadarID.radarIDs.push_back(v.deviceEsn);
       }
   }
   getConfiger().configerSensorDeviceInfos.clear();
   for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
   {
       if(!v.deviceEsn.empty())
       {
           if(WorkParamDeviceTypeMec == v.deviceType)
           {
               continue;
           }
           ConfigerSensorDeviceInfo configerSensorDeviceInfo;
           configerSensorDeviceInfo.routeId = v.routeId;
           configerSensorDeviceInfo.deviceType = v.deviceType;
           configerSensorDeviceInfo.mecSn = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
           configerSensorDeviceInfo.deviceSn = v.deviceSn;
           configerSensorDeviceInfo.deviceEsn = v.deviceEsn;
           configerSensorDeviceInfo.deviceIp = v.deviceIp;
           configerSensorDeviceInfo.deviceLongitude = v.deviceLongitude;
           configerSensorDeviceInfo.deviceLatitude = v.deviceLatitude;
           configerSensorDeviceInfo.deviceAltitude = v.deviceAltitude;
           getConfiger().configerSensorDeviceInfos.push_back(configerSensorDeviceInfo);
       }
   }
   getConfiger().nic = omWorkParamConfiger.mecDeviceWorkParam.nic;
//    getConfiger().enableDebugPrint = omWorkParamConfiger.hasedInit;
   saveConfiger();
   return true;
}
void OM_DEVICE_STATUS_COMPONENT::doBaiscWork()
{
    //初始化mqtt配置
//    OM_DS_WARN_PRINT << "[hint]创建感知设备状态表";
    getWorkParamFromFile();


    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  getConfiger().configerDb.DbDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtrTemp;
    m_DeviceStatusMsgDBPtrTemp.reset(new os::v2x::protocol::om::db::DeviceStatusMsgDB(getConfiger().configerDb.DbFilePath, getConfiger().enableDebugPrint));
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);

    //    OM_DS_WARN_PRINT << "[hint]创建感知设备授时表";
    tableInfo.first =  getConfiger().configerDb.DbSynchronizeTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);

    //    OM_DS_WARN_PRINT << "[hint]创建MEC设备状态表";
    tableInfo.first =  getConfiger().configerDb.DbMecDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_STATUS;
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);

    //    OM_DS_WARN_PRINT << "[hint]创建MEC告警表";
    tableInfo.first =  getConfiger().configerDb.DbMecDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_ALARM;
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);
    //    OM_DS_WARN_PRINT << "[hint]创建雷达设备告警表";
    tableInfo.first =  getConfiger().configerDb.DbRadarDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_RADAR_DEV_ALARM;
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);
    //    OM_DS_WARN_PRINT << "[hint]创建摄像头设备告警表";
    tableInfo.first =  getConfiger().configerDb.DbCameraDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_CAMERA_DEV_ALARM;
    m_DeviceStatusMsgDBPtrTemp->ClearTable(tableInfo);
    m_DeviceStatusMsgDBPtrTemp->CreatTable(tableInfo);

    m_DeviceStatusMsgDBPtrTemp->CloseDB();
    alarmMonitor();
    periodMonitorSensorDeviceActive();
    m_TimerPingSensorDevice = m_EventloopPing->addTimer(
            std::bind(&OM_DEVICE_STATUS_COMPONENT::periodMonitorSensorDeviceActive, this), getConfiger().configerTopicMonitorPeriod.periodMonitorSensorDeviceActive, false);

    m_TimerAlarmMonitor =  m_Eventloop->addTimer(
            std::bind(&OM_DEVICE_STATUS_COMPONENT::alarmMonitor, this),  getConfiger().configerTopicMonitorPeriod.periodMonitorSensorDeviceActive, false);
    getMqttConfigerMsg();
    if (!mqttInit())
    {
        OM_DS_ERROR_PRINT << "[error]mqttInit failure!";
        return ;
    }
}

bool OM_DEVICE_STATUS_COMPONENT::periodMonitorSensorDeviceActive()
{
    std::string interface = "[active]";
    std::string hint = "Monitor-Sensor-Device-Active";
    MsgDeviceStatus msgDeviceStatus;
    MqttTopicConfiger configerTopic;
    std::string topicProfix;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << hint;
    }
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    DBUtils::getDBInfo(tableInfo, statusList, interface);
    for(auto& v : getConfiger().configerSensorDeviceInfos)
    {
       if(v.deviceType == WorkParamDeviceTypeCamera)
       {

           topicProfix = getConfiger().configerTopic.Topic_Camera_Profix +  v.deviceEsn;
           configerTopic.Topic_Camera_Keepalive        = topicProfix + getConfiger().configerTopic.Topic_Camera_Keepalive; // 摄像头->mec 心跳信息
           msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
           msgDeviceStatus.timeStampNeedUpdate = true;
           std::string deviceID = getDeviceID(configerTopic.Topic_Camera_Keepalive);
           msgDeviceStatus.deviceID = deviceID;
           msgDeviceStatus.deviceIDNeedUpdate = true;
           if(getConfiger().enableDebugPrint) {
               OM_DS_DEBUG_PRINT << interface << "Camera " << deviceID << " start ping!";
           }

           int activeFromDb = 0;
           for(auto tempStatus:statusList)
           {
               if(deviceID == tempStatus.deviceID)
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "ping find " << deviceID << "(sucess)!";
                   }
                   activeFromDb = tempStatus.active;
                   break;
               }
           }
           if(getConfiger().enableDebugPrint) {
               OM_DS_DEBUG_PRINT << interface << "ip " << v.deviceIp;
           }
           if(NetUtil::ping(v.deviceIp))
           {
               if(getConfiger().enableDebugPrint) {
                   OM_DS_WARN_PRINT << interface << "Camera " << deviceID << " ping (sucess)!";
               }
               msgDeviceStatus.active = DEVICE_ACTIVE_DB_ON;
               if(activeFromDb == msgDeviceStatus.active)
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "ping sucess, but no need update!";
                   }
               }
               else
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "Camera" << deviceID << " update!";
                   }
                   msgDeviceStatus.activeNeedUpdate = true;
                   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
               }
           }
           else
           {
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << "Camera" << deviceID << " ping (failure)!";
               }
               msgDeviceStatus.active = DEVICE_ACTIVE_DB_OFF;
               if(activeFromDb == msgDeviceStatus.active)
               {
                   OM_DS_ERROR_PRINT << interface << "ping failure,but no need update!";
               }
               else
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "Camera" << deviceID << " update!";
                   }
                   msgDeviceStatus.activeNeedUpdate = true;
                   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
               }
           }
       }

       if(v.deviceType == WorkParamDeviceTypeRadar)
       {

           topicProfix =  getConfiger().configerTopic.Topic_Radar_Profix +  v.deviceEsn;
           configerTopic.Topic_Radar_Heartbeat         = topicProfix + getConfiger().configerTopic.Topic_Radar_Heartbeat ;
           msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
           msgDeviceStatus.timeStampNeedUpdate = true;
           std::string deviceID = getDeviceID(configerTopic.Topic_Radar_Heartbeat);
           msgDeviceStatus.deviceID = deviceID;
           msgDeviceStatus.deviceIDNeedUpdate = true;
           if(getConfiger().enableDebugPrint) {
               OM_DS_DEBUG_PRINT << interface << "Radar" << deviceID << "start ping!";
           }

           int activeFromDb = 0;
           for(auto tempStatus:statusList)
           {
               if(deviceID == tempStatus.deviceID)
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[sucess]ping find " << deviceID << "(sucess)!";
                   }
                   activeFromDb = tempStatus.active;
                   break;
               }
           }
           if(getConfiger().enableDebugPrint) {
               OM_DS_WARN_PRINT << interface << "ip: " << v.deviceIp;
           }
           if(NetUtil::ping(v.deviceIp))
           {
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << "Radar" << msgDeviceStatus.deviceID << " ping (sucess)!";
               }
               msgDeviceStatus.active = DEVICE_ACTIVE_DB_ON;
               if(activeFromDb == msgDeviceStatus.active)
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[hint]ping sucess,but no need update!";
                   }
               }
               else
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[hint]ping Radar-Sensor" << msgDeviceStatus.deviceID
                                         << " update!";
                   }
                   msgDeviceStatus.activeNeedUpdate = true;
                   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
               }
           }
           else
           {
               if(getConfiger().enableDebugPrint) {
                   OM_DS_ERROR_PRINT << interface << "Radar" << msgDeviceStatus.deviceID << " ping (failure)!";
               }
               msgDeviceStatus.active = DEVICE_ACTIVE_DB_OFF;
               if(activeFromDb == msgDeviceStatus.active)
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_ERROR_PRINT << interface << "ping failure,but no need update!";
                   }
               }
               else
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "Radar" << msgDeviceStatus.deviceID << " update!";
                   }
                   msgDeviceStatus.activeNeedUpdate = true;
                   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
               }
           }
       }
    }
    return true;
}

bool OM_DEVICE_STATUS_COMPONENT::getMqttConfigerMsg()
{
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[hint]get mqtt-configer-info";
    }
   m_MqttClientConfig.rscuEsn = getConfiger().rscuEsn;
   m_MqttClientConfig.configerMqtt.mqttBrokerUrl          = getConfiger().configerMqtt.mqttBrokerUrl;
   m_MqttClientConfig.configerMqtt.mqttUserName           = getConfiger().configerMqtt.mqttUserName;
   m_MqttClientConfig.configerMqtt.mqttPassword           = getConfiger().configerMqtt.mqttPassword;
   m_MqttClientConfig.mqttClientId           = getConfiger().mqttClientId;
   m_MqttClientConfig.configerMqtt.mqttKeepAliveInterval  = getConfiger().configerMqtt.mqttKeepAliveInterval;
   m_MqttClientConfig.configerMqtt.mqttSendQos            = getConfiger().configerMqtt.mqttSendQos;
   m_MqttClientConfig.configerMqtt.mqttSubScribeQos       = getConfiger().configerMqtt.mqttSubScribeQos;
   m_MqttClientConfig.configerMqtt.mqttCleanSession       = getConfiger().configerMqtt.mqttCleanSession;
   m_MqttClientConfig.configerMqtt.mqttReconnectInterval  = getConfiger().configerMqtt.mqttReconnectInterval;
   m_MqttClientConfig.configerMqtt.mqttSslVersion         = getConfiger().configerMqtt.mqttSslVersion;
   m_MqttClientConfig.configerMqtt.mqttVersion            = getConfiger().configerMqtt.mqttVersion;
   m_MqttClientConfig.configerMqtt.mqttConnectTimeOut     = getConfiger().configerMqtt.mqttConnectTimeOut;

   m_MqttClientConfig.enableDebugPrint = getConfiger().enableDebugPrint;
   m_MqttClientConfig.configerEnable.Enable_Save_2DB = getConfiger().configerEnable.Enable_Save_2DB;
   //感知设备表
   m_MqttClientConfig.configerDb.DbSynchronizeTableName =  getConfiger().configerDb.DbSynchronizeTableName;
   m_MqttClientConfig.configerDb.DbDeviceStatusTableName =  getConfiger().configerDb.DbDeviceStatusTableName;

    //mec表格
    m_MqttClientConfig.configerDb.DbMecDeviceStatusTableName =  getConfiger().configerDb.DbMecDeviceStatusTableName;
    m_MqttClientConfig.configerDb.DbMecDeviceAlarmTableName =  getConfiger().configerDb.DbMecDeviceAlarmTableName;

    m_MqttClientConfig.configerDb.DbRadarDeviceAlarmTableName =  getConfiger().configerDb.DbRadarDeviceAlarmTableName;
    m_MqttClientConfig.configerDb.DbCameraDeviceAlarmTableName =  getConfiger().configerDb.DbCameraDeviceAlarmTableName;

   m_MqttClientConfig.configerDb.DbFileName =  getConfiger().configerDb.DbFileName;
   m_MqttClientConfig.configerDb.DbFilePath =   getConfiger().configerDb.DbFileName;


   m_MqttClientConfig.configerTopicCameraRcId.RcIds =  getConfiger().configerTopicCameraRcId.RcIds;
   std::string projectPath = getConfiger().configerProjectPath.projectRoot;
   std::string tlsDirPath = projectPath + "/" + getConfiger().configerProjectPath.tlsDirName;
   afl::FileUtil::checkDirectory(projectPath);
   afl::FileUtil::checkDirectory(tlsDirPath);
   m_MqttClientConfig.configerProjectPath.tlsCAFilePath                  = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsCAFileName;
   m_MqttClientConfig.configerProjectPath.tlsClientKeyFilePath           = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientKeyFileName;
   m_MqttClientConfig.configerProjectPath.tlsClientPrivateKeyFilePath    = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientPrivateKeyFileName;
   //订阅
   memset(&m_MqttClientConfig.configerMqtt.mqttSubscribeTopics, 0, sizeof(m_MqttClientConfig.configerMqtt.mqttSubscribeTopics));
   m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum = 0;
   //mec授时不成功
   m_MqttClientConfig.configerTopic.Topic_Timing_Alarm_Inter = getConfiger().configerTopic.Topic_Bs_Inter_Profix + getConfiger().configerTopic.Topic_Timing_Alarm_Inter;
   m_MqttClientConfig.configerTopic.Topic_AngleOffset_Alarm_Inter =  getConfiger().configerTopic.Topic_Bs_Inter_Profix + getConfiger().configerTopic.Topic_AngleOffset_Alarm_Inter;
   
   m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(m_MqttClientConfig.configerTopic.Topic_Timing_Alarm_Inter.c_str());
   m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(m_MqttClientConfig.configerTopic.Topic_AngleOffset_Alarm_Inter.c_str());

   int index = 0;
   //camera
   for(uint32_t i = 0; i < getConfiger().configerTopicCameraRcId.RcIds.size(); i++)
   {
       std::string topicCameraProfix = getConfiger().configerTopic.Topic_Camera_Profix +  getConfiger().configerTopicCameraRcId.RcIds[i];
       MqttTopicConfiger configerTopic;
       configerTopic.Topic_Camera_Keepalive        = topicCameraProfix + getConfiger().configerTopic.Topic_Camera_Keepalive           ; // 摄像头->mec 心跳信息
       configerTopic.Topic_Camera_Synchronize      = topicCameraProfix + getConfiger().configerTopic.Topic_Camera_Synchronize         ;
       configerTopic.Topic_Camera_Register     = topicCameraProfix + getConfiger().configerTopic.Topic_Camera_Register        ;
       configerTopic.Topic_Camera_Alarm     = topicCameraProfix + getConfiger().configerTopic.Topic_Camera_Alarm        ;

       m_CameraTopicUnMapSubscribe[index] = configerTopic;
       m_MqttClientConfig.topicUnMapSubscribe[index++] = configerTopic;
       std::pair<int, bool> pairTemp;
       pairTemp.first = -1;
       pairTemp.second = false;
       topicCameraUnMap[configerTopic.Topic_Camera_Keepalive] = pairTemp;
       topicCameraUnMap[configerTopic.Topic_Camera_Synchronize] = pairTemp;
       topicCameraAlarmUnMap[configerTopic.Topic_Camera_Alarm] = pairTemp;
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(configerTopic.Topic_Camera_Keepalive.c_str());
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(configerTopic.Topic_Camera_Synchronize.c_str());
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(configerTopic.Topic_Camera_Register.c_str());
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *)  strdup(configerTopic.Topic_Camera_Alarm.c_str());


       //摄像头设备心跳表初始化
       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
       if(getConfiger().enableDebugPrint) {
           OM_DS_WARN_PRINT << "[topic]" << configerTopic.Topic_Camera_Keepalive;
       }
       initTable(tableInfo, configerTopic.Topic_Camera_Keepalive, DEVICE_TYPE_DB_CAMRA, DEVICE_STATUS_DB_OFF);

       //摄像头设备授时表初始化
       tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << "[topic]" << configerTopic.Topic_Camera_Synchronize;
       }
       initTable(tableInfo, configerTopic.Topic_Camera_Synchronize,DEVICE_TYPE_DB_CAMRA, DEVICE_STATUS_DB_OFF);
   }
   ////////////////////////////////////////////////////////////////////////////////////////
   //radar
   for(uint32_t i = 0; i < getConfiger().configerTopicRadarID.radarIDs.size(); i++)
   {
       std::string topicRadarProfix = getConfiger().configerTopic.Topic_Radar_Profix + getConfiger().configerTopicRadarID.radarIDs[i];
       MqttTopicConfiger configerTopic;
       configerTopic.Topic_Radar_Heartbeat         = topicRadarProfix + getConfiger().configerTopic.Topic_Radar_Heartbeat        ;
       configerTopic.Topic_Radar_Synchronize       = topicRadarProfix + getConfiger().configerTopic.Topic_Radar_Synchronize      ;
       configerTopic.Topic_Radar_Register         = topicRadarProfix + getConfiger().configerTopic.Topic_Radar_Register          ;
       configerTopic.Topic_Radar_Alarm         = topicRadarProfix + getConfiger().configerTopic.Topic_Radar_Alarm          ;

       m_RadarTopicUnMapSubscribe[index] = configerTopic;
       m_MqttClientConfig.topicUnMapSubscribe[index++] = configerTopic;
       /////////////////////////////////////////////////////////
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] =
               (char *)  strdup(configerTopic.Topic_Radar_Heartbeat.c_str());
       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] =
               (char *)  strdup(configerTopic.Topic_Radar_Synchronize.c_str());

       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] =
               (char *)  strdup(configerTopic.Topic_Radar_Register.c_str());

       m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] =
               (char *)  strdup(configerTopic.Topic_Radar_Alarm.c_str());

       std::pair<int, bool> pairTemp;
       pairTemp.first = -1;
       pairTemp.second = false;
       topicRadarUnMap[configerTopic.Topic_Radar_Heartbeat] = pairTemp;
       topicRadarUnMap[configerTopic.Topic_Radar_Synchronize] = pairTemp;
       topicRadarAlarmUnMap[configerTopic.Topic_Radar_Alarm] = pairTemp;
       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << "[topic]" << configerTopic.Topic_Radar_Synchronize;
       }
       initTable(tableInfo, configerTopic.Topic_Radar_Synchronize, DEVICE_TYPE_DB_MW_RADAR, DEVICE_STATUS_DB_OFF);
       //雷达设备心跳表初始化
       tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << "[topic]" << configerTopic.Topic_Radar_Synchronize;
       }
       initTable(tableInfo, configerTopic.Topic_Radar_Heartbeat, DEVICE_TYPE_DB_MW_RADAR, DEVICE_STATUS_DB_OFF);
   }

    for (uint32_t i = 0; i < m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum; i++)
    {
       m_MqttClientConfig.configerMqtt.mqttSubscribeQoss[i] =  m_MqttClientConfig.configerMqtt.mqttSubScribeQos;
    }

   //MEC状态表初始化
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbMecDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_STATUS;
    std::string initMecDeviceStatusTopic = "";
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[topic]" << tableInfo.first;
    }
    initTable(tableInfo, initMecDeviceStatusTopic, DEVICE_TYPE_DB_MEC, DEVICE_STATUS_DB_OFF);

   //MEC告警表初始化
    tableInfo.first =  m_MqttClientConfig.configerDb.DbMecDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_ALARM;
    std::string initMecAlarmTopic = "";
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[topic]" << tableInfo.first;
    }
    initTable(tableInfo, initMecAlarmTopic, DEVICE_TYPE_DB_MEC, DEVICE_STATUS_DB_OFF);

    //雷达告警表初始化
    tableInfo.first =  m_MqttClientConfig.configerDb.DbRadarDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_RADAR_DEV_ALARM;
    std::string initRadarAlarmTopic = "";
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[topic]" << tableInfo.first;
    }
    initTable(tableInfo, initRadarAlarmTopic, DEVICE_TYPE_DB_RADAR, DEVICE_STATUS_DB_OFF);

    //相机告警表初始化
    tableInfo.first =  m_MqttClientConfig.configerDb.DbCameraDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_CAMERA_DEV_ALARM;
    std::string initCameraAlarmTopic = "";
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[topic]" << tableInfo.first;
    }
    initTable(tableInfo, initCameraAlarmTopic, DEVICE_TYPE_DB_RADAR, DEVICE_STATUS_DB_OFF);
    if(getConfiger().enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[configer]\n" << m_MqttClientConfig.to_string().c_str();
    }

   return true;
}

//初始化表格
bool OM_DEVICE_STATUS_COMPONENT::initTable(std::pair<std::string, TABLE_TYPE> tableInfo, std::string topic, int deviceType, int deviceStatus)
{
//    OM_DS_WARN_PRINT << "[hint]初始化 表格" ;
    std::string interface = "[init-talbe]";
   MsgDeviceStatus msgDeviceStatus;
   //时间戳
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    //设备类型
   msgDeviceStatus.deviceType = (DEVICE_TYPE_DB)deviceType;
   //设备状态
   msgDeviceStatus.deviceStatus = (DEVICE_STATUS_DB)deviceStatus;
   //判断是否是mec状态表
   if(tableInfo.second == TABLE_TYEP_MEC_DEV_STATUS)
   {
       //给MEC状态表中各个列初始化
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << interface << "[notice]init mec-device-status";
       }
       OmWorkParamConfiger  omWorkParamConfiger;
       WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
       msgDeviceStatus.deviceID = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
       std::string softwareVersionFromFile;
       if( afl::FileUtil::isFileExist(getConfiger().configerVersionFilePath.c_str()))
       {
           std::ifstream softwareVersionFile(getConfiger().configerVersionFilePath.c_str());
           if(softwareVersionFile)
           {
               // 读取第一行内容
               std::string line;
               if (std::getline(softwareVersionFile, line))
               {
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[softwareVersion]" << line;
                   }
                   softwareVersionFromFile = DBUtils::extractVersion(line);
               }
               else
               {
                   OM_DS_ERROR_PRINT << interface << "[error]file is empty！";

                   softwareVersionFromFile =  "v3.1";
               }
               softwareVersionFile.close();// 关闭文件
           }
       }
       
       msgDeviceStatus.softwareVersion = softwareVersionFromFile;
       msgDeviceStatus.devTime = 0;
       msgDeviceStatus.lastTime = 0;
       msgDeviceStatus.timeNode = 0;
       msgDeviceStatus.timeDiff = 0;
       std::string pointNo;
       std::stringstream ss(omWorkParamConfiger.mecDeviceWorkParam.routeId);
       std::getline(ss, pointNo, '-'); // 先读取到分隔符
       std::getline(ss, pointNo);       // 再读取分隔符后的部分
       
       msgDeviceStatus.pointNo = pointNo;
       msgDeviceStatus.pointName = omWorkParamConfiger.mecDeviceWorkParam.routeId;
  
       msgDeviceStatus.longitude = omWorkParamConfiger.mecDeviceWorkParam.mecLongitude;

       msgDeviceStatus.latitude = omWorkParamConfiger.mecDeviceWorkParam.mecLatitude;
       msgDeviceStatus.altitude = omWorkParamConfiger.mecDeviceWorkParam.mecAltitude;
       msgDeviceStatus.active = (DEVICE_ACTIVE_DB)0;
       msgDeviceStatus.deviceIp = omWorkParamConfiger.mecDeviceWorkParam.mecIp;
       NetworkInfo networkInfo = DBUtils::getNetworkInfo(omWorkParamConfiger.mecDeviceWorkParam.nic);
       msgDeviceStatus.netMask = networkInfo.netmask;
       msgDeviceStatus.gateway = DBUtils::getGatewayAddress();;
       msgDeviceStatus.cpuLoad = "";
       msgDeviceStatus.cpuTemp = 0;
       msgDeviceStatus.cpuUti = "";
       msgDeviceStatus.gpuLoad = 0;
       msgDeviceStatus.gpuSmem = 0;
       msgDeviceStatus.gpuTemp = 0;
       msgDeviceStatus.gpuUti = "";
       msgDeviceStatus.memTotal = 0;
       msgDeviceStatus.memUsed = 0;
       msgDeviceStatus.memFree = 0;
       msgDeviceStatus.diskTotal = 0;
       msgDeviceStatus.diskUsed = 0;
       msgDeviceStatus.diskFree = 0;
       msgDeviceStatus.diskTps = 0;
       msgDeviceStatus.diskWrite = 0;
       msgDeviceStatus.diskRead = 0;
       msgDeviceStatus.alarmLevel = (AlertLevelEnum)0;
       msgDeviceStatus.alarmStatus = (AlarmStatusEnum)0;
       msgDeviceStatus.alarmRaisedTime = 0;
       msgDeviceStatus.alarmChangedTime = 0;
       msgDeviceStatus.alarmType = (MecAlarmTypeErrorCodeEnum)13;
       msgDeviceStatus.addition = "";
//       OM_DS_ERROR_PRINT << "[status]" << msgDeviceStatus.to_string();
   }
   else
   {
       //感知设备状态表、授时表和mec告警表和感知设备告警表（雷达、相机）
       msgDeviceStatus.deviceID = getDeviceID(topic);
       for(auto& v : getConfiger().configerSensorDeviceInfos)
       {
           if(v.deviceEsn == msgDeviceStatus.deviceID)
           {
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << SUCESS_COLOR_STR << "[notice] find device!" << SUCESS_COLOR_END;
               }
               msgDeviceStatus.deviceIp = v.deviceIp;
               msgDeviceStatus.devTime = 0;
               msgDeviceStatus.lastTime = 0;
               msgDeviceStatus.timeDiff = 0;
               msgDeviceStatus.longitude = v.deviceLongitude;
               if(getConfiger().enableDebugPrint) {
                   OM_DS_WARN_PRINT << interface << "[deviceLongitude]" << v.deviceLongitude;
               }
               msgDeviceStatus.latitude = v.deviceLatitude;
               if(getConfiger().enableDebugPrint) {
                   OM_DS_WARN_PRINT << interface << "[deviceLatitude]" << v.deviceLatitude;
               }
               msgDeviceStatus.mecEsn = v.mecSn;
               msgDeviceStatus.active = DEVICE_ACTIVE_DB_OFF;
               msgDeviceStatus.deviceIp = v.deviceIp;
               msgDeviceStatus.altitude = v.deviceAltitude;
               if(getConfiger().enableDebugPrint) {
                   OM_DS_WARN_PRINT << interface << "[deviceAltitude]" << v.deviceAltitude;
               }
               msgDeviceStatus.alarmType = -1;
               msgDeviceStatus.addition = "";
//               OM_DS_ERROR_PRINT << "[status]" << msgDeviceStatus.to_string();
               break;
           }
       }
   }


   std::string hint;
   switch(tableInfo.second)
   {
       case TABLE_TYEP_SENSOR_DEV_STATUS:  //设备状态表
           hint = "device-status";
           break;
       case TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE:  //设备授时表
           hint = "synchronize";
           break;
       case TABLE_TYEP_MEC_DEV_STATUS:  //MEC设备状态表
           hint = "mec-status";
           break;
       case TABLE_TYEP_MEC_DEV_ALARM:  //MEC告警表
           hint = "mec-alarm";
           break;
       case TABLE_TYEP_RADAR_DEV_ALARM:  //雷达告警表
           hint = "radar-alarm";
           break;
       case TABLE_TYEP_CAMERA_DEV_ALARM:  //相机告警表
           hint = "camera-alarm";
           break;
       default:
           break;
   }
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[notice]init table: [" << hint << "]";
    }
   DBUtils::initDBInfo(tableInfo, msgDeviceStatus, interface);
   return true;
}

std::string OM_DEVICE_STATUS_COMPONENT::getDeviceID(const std::string& topic)
{
   size_t start = topic.find('/') + 1;
   size_t end = topic.find('/', start);
   if (start != std::string::npos && end != std::string::npos)
   {
       return topic.substr(start, end - start);
   }
   else
   {
       return "";
   }
}
void OM_DEVICE_STATUS_COMPONENT::mqttDispatchSubscripeMessage(const std::string &topic, const afl::base::json &subScribeJson)
{
   if (!m_MqttConnected)
   {
       OM_DS_ERROR_PRINT << "[error]mqtt clinet has not connected!";
       return;
   }

   if (topic.empty()) {
       OM_DS_ERROR_PRINT << "topic empty!";
       return;
   }
    if(getConfiger().enableDebugPrint)
    {
        OM_DS_ERROR_PRINT << "[topic]" << topic;
    }
   std::string hint;
   std::string topicPT;
   std::string topicProfix = topic.substr(0, topic.find("/"));
    if(getConfiger().enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[topicProfix]" << topicProfix;
    }
    if(topicProfix == getConfiger().configerTopic.Topic_Bs_Inter_Profix.substr(0, getConfiger().configerTopic.Topic_Bs_Inter_Profix.find("/")))
    {
        if(topic == m_MqttClientConfig.configerTopic.Topic_AngleOffset_Alarm_Inter)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_DS_DEBUG_PRINT << "[notice]v2x_angle_offset!";
            }
             processCameraAngleOffset(subScribeJson);
        }
        if(topic == m_MqttClientConfig.configerTopic.Topic_Timing_Alarm_Inter)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_DS_DEBUG_PRINT << "[notice]timing-inter!";
            }
            processTiming(topic, subScribeJson);
        }

    }
    else if(topicProfix == "radar")
    {
        OM_DS_DEBUG_PRINT << "[topicProfix]radar";
        for (uint32_t i = 0; i < m_RadarTopicUnMapSubscribe.size(); i++)
        {
            auto topicUnMapTemp = m_RadarTopicUnMapSubscribe[i];
            if (topic == topicUnMapTemp.Topic_Radar_Heartbeat)
            {
                //雷达心跳
                if(getConfiger().enableDebugPrint) {
                    OM_DS_DEBUG_PRINT << "[Topic_Radar_Heartbeat]" << topicUnMapTemp.Topic_Radar_Heartbeat;
                }
                topicUnMapTemp.Topic_Radar_Heartbeat_Counter++;
                OM_DS_DEBUG_PRINT << "[Topic_Radar_Heartbeat]" << topicUnMapTemp.Topic_Radar_Heartbeat_Counter;
                if (topicUnMapTemp.Topic_Radar_Heartbeat_Counter >= getConfiger().configerTopicMonitorPeriod.counterRadarHeartbeat)
                {
                    OM_DS_DEBUG_PRINT << "[Topic_Radar_Heartbeat]processRadarHearBeat";
                    processRadarHearBeat(topic);
                    topicUnMapTemp.Topic_Radar_Heartbeat_Counter = 0; // 重置计数器
                }

                break;
            }
            else if(topic == topicUnMapTemp.Topic_Radar_Synchronize)
            {
                //雷达授时
                if(getConfiger().enableDebugPrint) {
                    OM_DS_DEBUG_PRINT << "[Topic_Radar_Synchronize]" << topicUnMapTemp.Topic_Radar_Synchronize;
                }
                // topicUnMapTemp.Topic_Camera_Synchronize_Counter++;
                // if (topicUnMapTemp.Topic_Camera_Synchronize_Counter >= getConfiger().configerTopicMonitorPeriod.counterRadarSynchronize)
                // {
                //     processRadarSynchronize(topic,  subScribeJson);
                //     topicUnMapTemp.Topic_Camera_Synchronize_Counter = 0; // 重置计数器
                // }
                break;
            }
            else if(topic == topicUnMapTemp.Topic_Radar_Register)
            {
                //摄像机注册
                if(getConfiger().enableDebugPrint) {
                    OM_DS_DEBUG_PRINT << "[Topic_Radar_Register]" << topicUnMapTemp.Topic_Radar_Register;
                }
                // processCameraRegister(topic,  subScribeJson);
                break;
            }
            else if(topic == topicUnMapTemp.Topic_Radar_Alarm)
            {
                //雷达告警
                if(getConfiger().enableDebugPrint) {
                    OM_DS_DEBUG_PRINT << "[Topic_Radar_Alarm]" << topicUnMapTemp.Topic_Radar_Alarm;
                }
                processRadarAlarm(topic,  subScribeJson);
            }
            else
            {
                //            OM_DS_ERROR_PRINT << "[notice-radar]no need insert db!";
            }
        }
    }
    else if(topicProfix == "camera")
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_DS_DEBUG_PRINT << "[topicProfix] camera [m_CameraTopicUnMapSubscribe.size()]" << m_CameraTopicUnMapSubscribe.size();
        }
        for (uint32_t i = 0; i < m_CameraTopicUnMapSubscribe.size(); i++)
       {
           auto topicUnMapTemp = m_CameraTopicUnMapSubscribe[i];
           if(topic == topicUnMapTemp.Topic_Camera_Keepalive)
           {
               //摄像头心跳
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[Topic_Camera_Keepalive]find " << topicUnMapTemp.Topic_Camera_Keepalive;
               }
               topicUnMapTemp.Topic_Radar_Heartbeat_Counter++;
               if(getConfiger().enableDebugPrint)
               {
                   OM_DS_DEBUG_PRINT << "[Topic_Camera_Keepalive（count）]" << topicUnMapTemp.Topic_Radar_Heartbeat_Counter;
               }
               if (topicUnMapTemp.Topic_Radar_Heartbeat_Counter >= getConfiger().configerTopicMonitorPeriod.counterCameraHeartbeat)
               {
                   if(getConfiger().enableDebugPrint)
                   {
                       OM_DS_DEBUG_PRINT << "[Topic_Camera_Keepalive]processCameraHearBeat";
                   }
                   processCameraHearBeat(topic);
                   topicUnMapTemp.Topic_Radar_Heartbeat_Counter = 0; // 重置计数器
               }

               break;
           }
           else if (topic == topicUnMapTemp.Topic_Camera_Synchronize)
           {
               //摄像头授时
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[Topic_Radar_Synchronize]" << topicUnMapTemp.Topic_Camera_Synchronize;
               }
               // topicUnMapTemp.Topic_Radar_Synchronize++;
               // if (topicUnMapTemp.Topic_Radar_Synchronize >= getConfiger().configerTopicMonitorPeriod.counterCameraSynchronize)
               // {
               //     processCameraSynchronize(topic, subScribeJson);
               //     topicUnMapTemp.Topic_Radar_Synchronize = 0; // 重置计数器
               // }
               break;
           }
           else if(topic == topicUnMapTemp.Topic_Camera_Register)
           {
               //摄像头注册
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[Topic_Camera_Register]" << topicUnMapTemp.Topic_Camera_Register;
               }
               // processCameraRegister(topic,  subScribeJson);
               break;
           }
           else if(topic == topicUnMapTemp.Topic_Camera_Alarm)
           {
               //雷达告警
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[Topic_Camera_Alarm]" << topicUnMapTemp.Topic_Camera_Alarm;
               }
               processCameraAlarm(topic,  subScribeJson);
           }
           else
           {
               //            OM_DS_ERROR_PRINT << "[notice-radar]no need insert db!";
           }
       }

    }
}

bool  OM_DEVICE_STATUS_COMPONENT::processRadarAlarm(std::string topic, const afl::base::json &subScribeJson)
{
    if(topic.empty())
    {
        OM_DS_ERROR_PRINT  << "[notice-radar]topic empty!";
        return false;
    }
    std::string deviceID = getDeviceID(topic);
    uint64_t radarAlarmType = 13;
    if(subScribeJson.find("faultType") != subScribeJson.end())
    {
        radarAlarmType = subScribeJson["faultType"];
    }
    std::string keyTemp = deviceID + "-" + std::to_string(radarAlarmType);

    MsgDeviceStatus msgDeviceStatus;
    for (auto it = topicRadarAlarmUnMap.begin(); it != topicRadarAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            m_Eventloop->cancelTimer(it->second.first);
            it->second.first = -1;
            it->second.second = true;
            break;
        }
    }
    std::string hint;
    hint = "radar-alarm";
    msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.timeStampNeedUpdate = true;

    msgDeviceStatus.deviceID = deviceID;
    msgDeviceStatus.deviceIDNeedUpdate = true;


    msgDeviceStatus.alarmLevel = (AlertLevelEnum)ALERT_CRITICAL;
    msgDeviceStatus.alarmLevelNeedUpdate = true;


    msgDeviceStatus.alarmStatus = (AlarmStatusEnum)ALARM_OCCURRED;
    msgDeviceStatus.alarmStatusNeedUpdate = true;

    uint64_t radarFaultTypeTimestamp = afl::util::TimeStamp::now(true).millSeconds();
    if(subScribeJson.find("timestamp") != subScribeJson.end())
    {
        radarFaultTypeTimestamp = subScribeJson["timestamp"];
    }

    msgDeviceStatus.alarmRaisedTime = radarFaultTypeTimestamp;
    msgDeviceStatus.alarmRaisedTimeNeedUpdate = true;

    msgDeviceStatus.alarmChangedTime = radarFaultTypeTimestamp;
    msgDeviceStatus.alarmChangedTimeNeedUpdate = true;


    msgDeviceStatus.alarmType = radarAlarmType;
    msgDeviceStatus.alarmTypeNeedUpdate = true;


    msgDeviceStatus.deviceNoAlarmType = keyTemp;
    msgDeviceStatus.deviceNoAlarmTypeNeedUpdate = true;

    int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorRadarAlarm, this, topic, keyTemp),
                                getConfiger().configerTopicMonitorPeriod.periodMonitorRadarAlarm, false);
    bool isFindDeviceId = false;
    for (auto it = topicRadarAlarmUnMap.begin(); it != topicRadarAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            it->second.first = timerFd;
            it->second.second = false;
            isFindDeviceId = true;
            break;
        }
    }
    if(!isFindDeviceId)
    {
        std::pair<std::string, std::pair<uint32_t, bool>> pairTemp;
        pairTemp.first = keyTemp;
        pairTemp.second.first = timerFd;
        pairTemp.second.second = false;
        topicRadarAlarmUnMap.insert(pairTemp);
    }

    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbRadarDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_RADAR_DEV_ALARM;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[hint]" << hint;
    }
    std::string interface = "";
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    return true;
}


bool OM_DEVICE_STATUS_COMPONENT::monitorRadarAlarm(std::string topic, std::string keyTemp)
{
    std::string interface = "[radar-alarm]";
    bool findFlag = false;
    MsgDeviceStatus msgDeviceStatus;
    for (auto it = topicRadarAlarmUnMap.begin(); it != topicRadarAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            if(it->second.second)
            {
                m_Eventloop->cancelTimer(it->second.first);
                it->second.first = -1;
                findFlag = true;
                it->second.second = false;
            }
            else
            {
                findFlag = false;
                it->second.first = -1;
                it->second.second = false;
            }
            break;
        }
    }
    if(!findFlag)
    {
        std::string hint = "no radar-alarm info";
        msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        msgDeviceStatus.timeStampNeedUpdate = true;

        msgDeviceStatus.deviceID = getDeviceID(topic);
        msgDeviceStatus.deviceIDNeedUpdate = true;

        msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
        msgDeviceStatus.deviceStatusNeedUpdate = true;

        msgDeviceStatus.deviceNoAlarmType = keyTemp;
        msgDeviceStatus.deviceNoAlarmTypeNeedUpdate = true;

        std::pair<std::string, TABLE_TYPE> tableInfo;
        tableInfo.first =  m_MqttClientConfig.configerDb.DbRadarDeviceAlarmTableName;
        tableInfo.second = TABLE_TYEP_RADAR_DEV_ALARM;
        if(getConfiger().enableDebugPrint) {
            OM_DS_WARN_PRINT << "[hint]" << hint << "[keyTemp]" << keyTemp;
        }
        DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
//        DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    }
    return true;
}



bool  OM_DEVICE_STATUS_COMPONENT::processCameraAlarm(std::string topic, const afl::base::json &subScribeJson)
{
    if(topic.empty())
    {
        OM_DS_ERROR_PRINT  << "[notice-camera]topic empty!";
        return false;
    }
    std::string deviceID = getDeviceID(topic);
    uint64_t cameraAlarmType = 13;
    if(subScribeJson.find("type") != subScribeJson.end())
    {
        cameraAlarmType = subScribeJson["type"];
    }
    std::string keyTemp = deviceID + "-" + std::to_string(cameraAlarmType);

    MsgDeviceStatus msgDeviceStatus;
    for (auto it = topicCameraAlarmUnMap.begin(); it != topicCameraAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            m_Eventloop->cancelTimer(it->second.first);
            it->second.first = -1;
            it->second.second = true;
            break;
        }
    }
    std::string hint;
    hint = "camera-alarm";
    msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.timeStampNeedUpdate = true;

    msgDeviceStatus.deviceID = deviceID;
    msgDeviceStatus.deviceIDNeedUpdate = true;


    msgDeviceStatus.alarmLevel = (AlertLevelEnum)ALERT_CRITICAL;
    msgDeviceStatus.alarmLevelNeedUpdate = true;


    msgDeviceStatus.alarmStatus = (AlarmStatusEnum)ALARM_OCCURRED;
    msgDeviceStatus.alarmStatusNeedUpdate = true;

    uint64_t radarFaultTypeTimestamp = afl::util::TimeStamp::now(true).millSeconds();
    if(subScribeJson.find("timestamp") != subScribeJson.end())
    {
        radarFaultTypeTimestamp = subScribeJson["timestamp"];
    }

    msgDeviceStatus.alarmRaisedTime = radarFaultTypeTimestamp;
    msgDeviceStatus.alarmRaisedTimeNeedUpdate = true;

    msgDeviceStatus.alarmChangedTime = radarFaultTypeTimestamp;
    msgDeviceStatus.alarmChangedTimeNeedUpdate = true;


    msgDeviceStatus.alarmType = cameraAlarmType;
    msgDeviceStatus.alarmTypeNeedUpdate = true;


    msgDeviceStatus.deviceNoAlarmType = keyTemp;
    msgDeviceStatus.deviceNoAlarmTypeNeedUpdate = true;

    int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorCameraAlarm, this, topic, keyTemp),
                                        getConfiger().configerTopicMonitorPeriod.periodMonitorCameraAlarm, false);
    bool isFindDeviceId = false;
    for (auto it = topicCameraAlarmUnMap.begin(); it != topicCameraAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            it->second.first = timerFd;
            it->second.second = false;
            isFindDeviceId = true;
            break;
        }
    }
    if(!isFindDeviceId)
    {
        std::pair<std::string, std::pair<uint32_t, bool>> pairTemp;
        pairTemp.first = keyTemp;
        pairTemp.second.first = timerFd;
        pairTemp.second.second = false;
        topicCameraAlarmUnMap.insert(pairTemp);
    }

    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbCameraDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_CAMERA_DEV_ALARM;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[hint]" << hint;
    }
    std::string interface = "";
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    return true;
}


bool OM_DEVICE_STATUS_COMPONENT::monitorCameraAlarm(std::string topic, std::string keyTemp)
{
    std::string interface = "[camera-alarm]";
    bool findFlag = false;
    MsgDeviceStatus msgDeviceStatus;
    for (auto it = topicCameraAlarmUnMap.begin(); it != topicCameraAlarmUnMap.end(); ++it)
    {
        if (it->first == keyTemp)
        {
            if(it->second.second)
            {
                m_Eventloop->cancelTimer(it->second.first);
                it->second.first = -1;
                findFlag = true;
                it->second.second = false;
            }
            else
            {
                findFlag = false;
                it->second.first = -1;
                it->second.second = false;
            }
            break;
        }
    }
    if(!findFlag)
    {
        std::string hint = "no radar-alarm info";
        msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        msgDeviceStatus.timeStampNeedUpdate = true;

        msgDeviceStatus.deviceID = getDeviceID(topic);
        msgDeviceStatus.deviceIDNeedUpdate = true;

        msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
        msgDeviceStatus.deviceStatusNeedUpdate = true;

        msgDeviceStatus.deviceNoAlarmType = keyTemp;
        msgDeviceStatus.deviceNoAlarmTypeNeedUpdate = true;

        std::pair<std::string, TABLE_TYPE> tableInfo;
        tableInfo.first =  m_MqttClientConfig.configerDb.DbCameraDeviceAlarmTableName;
        tableInfo.second = TABLE_TYEP_CAMERA_DEV_ALARM;
        if(getConfiger().enableDebugPrint) {
            OM_DS_WARN_PRINT << "[hint]" << hint;
        }
        DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
//        DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    }
    return true;
}




void  OM_DEVICE_STATUS_COMPONENT::processCameraAngleOffset( const afl::base::json &subScribeJson)
{
    std::string interface = "[angle-offset]";
   CameraDeviceAngleOffsetLogData cameraDeviceAngleOffsetLogData;
   try{
       cameraDeviceAngleOffsetLogData = subScribeJson;
   }catch (afl::util::Exception& e)
   {
       OM_DS_ERROR_PRINT << "[error]" << e.what();
       return;
   }
   MsgDeviceStatus msgDeviceStatus;

   std::string hint;
   hint = "camera-angle-offset";
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[hint]" << hint;
    }
   msgDeviceStatus.timeStamp = cameraDeviceAngleOffsetLogData.timeStamp;
   msgDeviceStatus.timeStampNeedUpdate = true;

   CameraDeviceAngleOffsetLogDataAlarmInfo& alarmInfo =  cameraDeviceAngleOffsetLogData.alarm;
   msgDeviceStatus.alarmLevel = (AlertLevelEnum)alarmInfo.alarmLevel;
   msgDeviceStatus.alarmLevelNeedUpdate = true;

   msgDeviceStatus.alarmStatus = (AlarmStatusEnum)alarmInfo.alarmStatus;
   msgDeviceStatus.alarmStatusNeedUpdate = true;

   msgDeviceStatus.alarmRaisedTime = alarmInfo.alarmRaisedTime;
   msgDeviceStatus.alarmRaisedTimeNeedUpdate = true;

   msgDeviceStatus.alarmChangedTime = alarmInfo.alarmChangedTime;
   msgDeviceStatus.alarmChangedTimeNeedUpdate = true;

   msgDeviceStatus.alarmType = alarmInfo.alarmType;
   msgDeviceStatus.alarmTypeNeedUpdate = true;

   if(!alarmInfo.additionEmpty)
   {
       msgDeviceStatus.addition = alarmInfo.addition;
       msgDeviceStatus.additionNeedUpdate = true;
   }
//   OM_DS_ERROR_PRINT << "[msgDeviceStatus]" << msgDeviceStatus.to_string();
    std::pair<std::string, TABLE_TYPE> tableInfo;
   tableInfo.first =  m_MqttClientConfig.configerDb.DbMecDeviceAlarmTableName;
   tableInfo.second = TABLE_TYEP_MEC_DEV_ALARM;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[hint]" << hint;
    }
   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);

}

bool OM_DEVICE_STATUS_COMPONENT::processCameraRegister(std::string topic, const afl::base::json &subScribeJson)
{
    std::string interface = "[camera-register]";
    if(topic.empty())
    {
        OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
        return false;
    }
    
    CameraRegister cameraRegister;
    try{
        cameraRegister = subScribeJson;
    }
    catch(afl::util::Exception& e)
    {
        OM_DS_ERROR_PRINT << interface << "[error]" << e.what();
        return false;
    }

    std::string hint;
    hint = "camera-register";
    MsgDeviceStatus msgDeviceStatus;
    msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.timeStampNeedUpdate = true;
    std::string deviceID = getDeviceID(topic);
    msgDeviceStatus.deviceID = deviceID;
    msgDeviceStatus.deviceIDNeedUpdate = true;
    
    msgDeviceStatus.longitude = cameraRegister.longitude;
    msgDeviceStatus.longitudeNeedUpdate = true;
    
    msgDeviceStatus.latitude = cameraRegister.latitude;
    msgDeviceStatus.latitudeNeedUpdate = true;

    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus);
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    return true;
}

bool OM_DEVICE_STATUS_COMPONENT::processRadarRegister(std::string topic, const afl::base::json &subScribeJson)
{
    std::string interface = "[radar-register]";
    if(topic.empty())
    {
        OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
        return false;
    }
    
    RadarRegister radarRegister;
    try{
        radarRegister = subScribeJson;
    }
    catch(afl::util::Exception& e)
    {
        OM_DS_ERROR_PRINT << interface << "[error]" << e.what();
        return false;
    }

    std::string hint;
    hint = "radar-register";
    MsgDeviceStatus msgDeviceStatus;
    msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.timeStampNeedUpdate = true;
    std::string deviceID = getDeviceID(topic);
    msgDeviceStatus.deviceID = deviceID;
    msgDeviceStatus.deviceIDNeedUpdate = true;
    
    msgDeviceStatus.longitude = radarRegister.longitude;
    msgDeviceStatus.longitudeNeedUpdate = true;
    
    msgDeviceStatus.latitude = radarRegister.latitude;
    msgDeviceStatus.latitudeNeedUpdate = true;
    
    msgDeviceStatus.altitude = radarRegister.altitude;
    msgDeviceStatus.altitudeNeedUpdate = true;

    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus);
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   
    DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
    return true;
}

bool OM_DEVICE_STATUS_COMPONENT::processRadarSynchronize(std::string topic, const afl::base::json &subScribeJson)
{
    std::string interface = "[radar-synchronize]";
   if(topic.empty())
   {
       OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
       return false;
   }
   MsgDeviceStatus msgDeviceStatus;

   for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           m_Eventloop->cancelTimer(it->second.first);
           it->second.second = true;
           break;
       }
   }

   CommonSyschronize commonSyschronize;
   try{
       commonSyschronize = subScribeJson;
   }
   catch(afl::util::Exception& e)
   {
       OM_DS_ERROR_PRINT << interface  << "[error]" << e.what();
   }

   std::string hint;
   hint = "radar-synchronize";
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
   msgDeviceStatus.timeStampNeedUpdate = true;

   msgDeviceStatus.deviceID = getDeviceID(topic);
   msgDeviceStatus.deviceIDNeedUpdate = true;

   msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_ON;
   msgDeviceStatus.deviceStatusNeedUpdate = true;

   msgDeviceStatus.devTime = commonSyschronize.devTime;
   msgDeviceStatus.devTimeNeedUpdate = true;

   msgDeviceStatus.lastTime = commonSyschronize.lastTime;
   msgDeviceStatus.lastTimeNeedUpdate = true;

   msgDeviceStatus.timeDiff = commonSyschronize.timeDiff;
   msgDeviceStatus.timeDiffNeedUpdate = true;


   //    int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorRadarSynchronize, this, topic),
   //                                                 getConfiger().configerTopicMonitorPeriod.periodMonitorRadarSynchronize, this);
   //    for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   //    {
   //        if (it->first == topic)
   //        {
   //            it->second.first = timerFd;
   //            it->second.second = false;
   //            break;
   //        }
   //    }
//   OM_DS_ERROR_PRINT << "msgDeviceStatus:" << msgDeviceStatus.to_string();

    std::pair<std::string, TABLE_TYPE> tableInfo;
   tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
   tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   
   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::monitorRadarSynchronize(std::string topicTemp)
{
    std::string interface = "[radar-synchronize]";
   bool findFlag = false;
   MsgDeviceStatus msgDeviceStatus;

   for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   {
       if (it->first == topicTemp)
       {
           if(it->second.second)
           {
               m_Eventloop->cancelTimer(it->second.first);
               findFlag = true;
               it->second.second = false;
           }
           else
           {
               findFlag = false;
               it->second.second = false;
           }
           break;
       }
   }
   if(!findFlag)
   {
       std::string hint = "no radar synchronize info";
       msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
       msgDeviceStatus.timeStampNeedUpdate = true;

       msgDeviceStatus.deviceID = getDeviceID(topicTemp);
       msgDeviceStatus.deviceIDNeedUpdate = true;

       msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
       msgDeviceStatus.deviceStatusNeedUpdate = true;

       msgDeviceStatus.devTime = 0;
       msgDeviceStatus.devTimeNeedUpdate = true;

       msgDeviceStatus.lastTime = 0;
       msgDeviceStatus.latitudeNeedUpdate = true;

       msgDeviceStatus.timeDiff = 0;
       msgDeviceStatus.timeDiffNeedUpdate = true;

       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
       }
       
       DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   }

   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::processRadarHearBeat(std::string topic)
{
    std::string interface = "[radar-hearbeat]";
   if(topic.empty())
   {
       OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
       return false;
   }
   MsgDeviceStatus msgDeviceStatus;
   for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           m_Eventloop->cancelTimer(it->second.first);
           it->second.first = -1;
           it->second.second = true;  //是否收到心跳
           break;
       }
   }
   std::string hint;
   hint = "camera-hearbeat";
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
   msgDeviceStatus.timeStampNeedUpdate = true;

   msgDeviceStatus.deviceID = getDeviceID(topic);
   msgDeviceStatus.deviceIDNeedUpdate = true;

   msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_ON;
   msgDeviceStatus.deviceStatusNeedUpdate = true;

   int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorRadarHeartBeat, this,topic),
                                       getConfiger().configerTopicMonitorPeriod.periodMonitorRadarHeartBeat, false);
   for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           it->second.first = timerFd;
           it->second.second = false;
           break;
       }
   }

   std::pair<std::string, TABLE_TYPE> tableInfo;
   tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
   tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   
   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   return true;
}



bool OM_DEVICE_STATUS_COMPONENT::monitorRadarHeartBeat(std::string topicTemp)
{
    std::string interface = "[radar-heartbeat]";
   bool findFlag = false;
   MsgDeviceStatus msgDeviceStatus;

   for (auto it = topicRadarUnMap.begin(); it != topicRadarUnMap.end(); ++it)
   {
       if (it->first == topicTemp)
       {
           if(it->second.second)
           {
               m_Eventloop->cancelTimer(it->second.first);
               it->second.first = -1;
               findFlag = true;
               it->second.second = false;
           }
           else
           {
               findFlag = false;
               it->second.first = -1;
               it->second.second = false;
           }
           break;
       }
   }
   if(!findFlag)
   {
       std::string hint = "no radar heartbeat";
       msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
       msgDeviceStatus.timeStampNeedUpdate = true;

       msgDeviceStatus.deviceID = getDeviceID(topicTemp);
       msgDeviceStatus.deviceIDNeedUpdate = true;

       msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
       msgDeviceStatus.deviceStatusNeedUpdate = true;

       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
       if(getConfiger().enableDebugPrint) {
           OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
       }
       
       DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   }

   return true;
}
//////////////////////////////////////////////////////////////
bool OM_DEVICE_STATUS_COMPONENT::processCameraSynchronize(std::string topic, const afl::base::json &subScribeJson)
{
    std::string interface = "[camera-synchronize]";
   if(topic.empty())
   {
       OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
       return false;
   }
   MsgDeviceStatus msgDeviceStatus;
   for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           m_Eventloop->cancelTimer(it->second.first);
           it->second.second = true;
           break;
       }
   }

   CommonSyschronize commonSyschronize;
   try{
       commonSyschronize = subScribeJson;
   }
   catch(afl::util::Exception& e)
   {
       OM_DS_ERROR_PRINT << interface << "[error]" << e.what();
       return false;
   }

   std::string hint;
   hint = "Camera-Synchronize";
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
   msgDeviceStatus.timeStampNeedUpdate = true;

   msgDeviceStatus.deviceID = getDeviceID(topic);
   msgDeviceStatus.deviceIDNeedUpdate = true;

   msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_ON;
   msgDeviceStatus.deviceStatusNeedUpdate = true;

   msgDeviceStatus.devTime = commonSyschronize.devTime;
   msgDeviceStatus.devTimeNeedUpdate = true;

   msgDeviceStatus.lastTime = commonSyschronize.lastTime;
   msgDeviceStatus.lastTimeNeedUpdate = true;

   msgDeviceStatus.timeDiff = commonSyschronize.timeDiff;
   msgDeviceStatus.timeDiffNeedUpdate = true;


   //    int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorCameraSynchronize, this, topic),
   //                                                 getConfiger().configerTopicMonitorPeriod.periodMonitorCameraSynchronize, this);
   //    for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   //    {
   //        if (it->first == topic)
   //        {
   //            it->second.first = timerFd;
   //            it->second.second = false;
   //            break;
   //        }
   //    }
//   OM_DS_ERROR_PRINT << "[msgDeviceStatus]" << msgDeviceStatus.to_string();

   std::pair<std::string, TABLE_TYPE> tableInfo;
   tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
   tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   
   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::monitorCameraSynchronize(std::string topicTemp)
{
    std::string interface = "[camera-synchronize]";
   bool findFlag = false;
   MsgDeviceStatus msgDeviceStatus;
   for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   {
       if (it->first == topicTemp)
       {
           if(it->second.second)
           {
               m_Eventloop->cancelTimer(it->second.first);
               findFlag = true;
               it->second.second = false;
           }
           else
           {
               findFlag = false;
               it->second.second = false;
           }
           break;
       }
   }
   if(!findFlag)
   {
       std::string hint = "no Camera-Synchronize info";
       msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
       msgDeviceStatus.timeStampNeedUpdate = true;

       msgDeviceStatus.deviceID = getDeviceID(topicTemp);
       msgDeviceStatus.deviceIDNeedUpdate = true;

       msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
       msgDeviceStatus.deviceStatusNeedUpdate = true;

       msgDeviceStatus.devTime = 0;
       msgDeviceStatus.devTimeNeedUpdate = true;

       msgDeviceStatus.lastTime = 0;
       msgDeviceStatus.lastTimeNeedUpdate = true;

       msgDeviceStatus.timeDiff = 0;
       msgDeviceStatus.timeDiffNeedUpdate = true;
       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbSynchronizeTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_SYNCHRONIZE;
       if(getConfiger().enableDebugPrint) {
           OM_DS_WARN_PRINT << interface << "[hint]" << hint;
       }
       
       DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   }

   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::processCameraHearBeat(std::string topic)
{
    std::string interface = "[camera-hearbeat]";
   if(topic.empty())
   {
       OM_DS_ERROR_PRINT << interface << "[notice-radar]topic empty!";
       return false;
   }
    if(getConfiger().enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << "[Topic_Camera_Keepalive] processCameraHearBeat";
    }
   MsgDeviceStatus msgDeviceStatus;
   for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           m_Eventloop->cancelTimer(it->second.first);
           it->second.first = -1;
           it->second.second = true;
           break;
       }
   }
   std::string hint;
   hint = "camera-hearbeat";
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
   msgDeviceStatus.timeStampNeedUpdate = true;

   msgDeviceStatus.deviceID = getDeviceID(topic);
   msgDeviceStatus.deviceIDNeedUpdate = true;

   msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_ON;
   msgDeviceStatus.deviceStatusNeedUpdate = true;

   int timerFd = m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::monitorCameraHeartBeat, this, topic),
                                       getConfiger().configerTopicMonitorPeriod.periodMonitorCameraHeartBeat, false);
   for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   {
       if (it->first == topic)
       {
           it->second.first = timerFd;
           it->second.second = false;
           break;
       }
   }
   std::pair<std::string, TABLE_TYPE> tableInfo;
   tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
   tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
    }
   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   return true;
}


bool OM_DEVICE_STATUS_COMPONENT::monitorCameraHeartBeat(std::string topicTemp)
{
    std::string interface = "[camera-hearbeat]";
   bool findFlag = false;
   MsgDeviceStatus msgDeviceStatus;
   for (auto it = topicCameraUnMap.begin(); it != topicCameraUnMap.end(); ++it)
   {
       if (it->first == topicTemp)
       {
           if(it->second.second)
           {
               m_Eventloop->cancelTimer(it->second.first);
               it->second.first = -1;
               findFlag = true;
               it->second.second = false;
           }
           else
           {
               findFlag = false;
               it->second.first = -1;
               it->second.second = false;
           }
           break;
       }
   }
   if(!findFlag)
   {
       std::string hint = "no camera-hearbeat info";
       msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
       msgDeviceStatus.timeStampNeedUpdate = true;

       msgDeviceStatus.deviceID = getDeviceID(topicTemp);
       msgDeviceStatus.deviceIDNeedUpdate = true;

       msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_OFF;
       msgDeviceStatus.deviceStatusNeedUpdate = true;
       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
       if(getConfiger().enableDebugPrint) {
           OM_DS_WARN_PRINT << interface << "[hint]" << hint;
       }
       
       DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
   }
   return true;
}

void OM_DEVICE_STATUS_COMPONENT::alarmMonitor()
{
    std::string interface = "[alarm]";

   string hint = "Alarm-Monitor";
    if(getConfiger().enableDebugPrint)
    {
        OM_DS_DEBUG_PRINT << interface << hint ;
    }
   PerformanceData performanceData;
   
   auto &runningInfo = performanceData.runningInfo;
   bool alarmOccurredFlagCpu = false;
   bool alarmOccurredFlagMem = false;
   bool alarmOccurredFlagDisk = false;
   bool alarmOccurredFlagNet = false;
   std::string addition = m_MqttClientConfig.rscuEsn;
    //监控cpu信息：负载、温度
   {
        PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
       if(PerformenceUtils::getCpuInfo(runningInfo.cpuInfo))
       {
           hint = "cpu-info";
           updateMecStatus(hint, m_MqttClientConfig.rscuEsn, TABLE_UPDATE_MEC_STAUTS_CPU, performanceData);
           if(runningInfo.cpuInfo.temp > getConfiger().configAlarmThresholdValue.TV_CPU_Tem)
           {
               //cpu温度过高-告警产生
               uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
               string hint = "cpu-temp-high occurred, [temp]" +
                             std::to_string(runningInfo.cpuInfo.temp) + " [up]" +
                             std::to_string(getConfiger().configAlarmThresholdValue.TV_CPU_Tem);
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << "[error]" << hint;
               }
               if(!m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag)
               {
                   alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, addition, false, addition);
                   m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = nowTime;
                   m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
               }
               else
               {
                   uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp)/1000;
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[timeDiff]" << hint;
                   }
                   if(timeDiff >= getConfiger().configerTopicMonitorPeriod.periodAlarmPublishInterval)
                   {
                       alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, addition, false, addition);
                       m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = nowTime;
                   }
               }
               m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag = true;
               m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag = false;
           }
           else
           {
               if(m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag)
               {
                   //cpu温度过高-告警消失
                   string hint = "cpu-temp-high disappeared";
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << SUCESS_COLOR_STR << "[sucess]" << hint << SUCESS_COLOR_END;
                   }
                   if(!m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag)
                   {
                       alarmDisappeared(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU);
                       m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = 0;
                       m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag = false;
                       m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                       m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                   }
               }
           }
       
           if(runningInfo.cpuInfo.uti.size() > getConfiger().configAlarmThresholdValue.TV_CPU_Uti)
           {
               uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
               float value = atof(runningInfo.cpuInfo.uti.c_str());
               if(value > getConfiger().configAlarmThresholdValue.TV_CPU_Uti)
               {
                   //cpu负载过高-告警产生
                   string hint = "cpu-load-high occurred, [uti]" +
                                 std::to_string(value) + " [up]" +
                                 std::to_string(getConfiger().configAlarmThresholdValue.TV_CPU_Uti);
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[error]" << hint;
                   }
                   if(!m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag)
                   {
                       alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU, addition, false, addition);
                       m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = nowTime;
                       m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                   }
                   else
                   {
                       uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp)/1000;
                       if(getConfiger().enableDebugPrint) {
                           OM_DS_DEBUG_PRINT << interface << "[timeDiff]" << hint;
                       }
                       if(timeDiff >= getConfiger().configerTopicMonitorPeriod.periodAlarmPublishInterval)
                       {
                           alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU, addition, false, addition);
                           m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = nowTime;
                       }
                   }
                
                   m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag = true;
                   m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag = false;
               }
               else
               {
                   if(m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag)
                   {
                       //cpu负载过高-告警消失
                       string hint = "cpu-temp-high disappeared";
                       if(getConfiger().enableDebugPrint) {
                           OM_DS_DEBUG_PRINT << interface << "[hint]" << hint;
                       }
                       if(!m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag)
                       {
                           alarmDisappeared(hint, MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU);
                           m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = 0;
                           m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag = false;
                           m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                           m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                       }
                   }
               }
           }
       }
   }
   //监控mem：可用空间
   {
        PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
       if(PerformenceUtils::getMemInfo(runningInfo.memInfo))
       {
           uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
           updateMecStatus(hint, m_MqttClientConfig.rscuEsn, TABLE_UPDATE_MEC_STAUTS_MEM, performanceData);
           //        double memoryUsagePercentage = (usedMemory / totalMemory) * 100;
           //        std::string freeOutput = executeCommand("free -h");
           //        double memoryUsagePercentage = getMemoryUsagePercentage(freeOutput);
           if (runningInfo.memInfo.free <=  getConfiger().configAlarmThresholdValue.TV_Mem_Free)
           {
               //mem空间不足-告警产生
               string hint = "out-of-mem occurred, [free]" +
                             std::to_string(runningInfo.memInfo.free) + " [up]" +
                             std::to_string(getConfiger().configAlarmThresholdValue.TV_Mem_Free);
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << "[error]" << hint;
               }
               
               if(!m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag)
               {
                   alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY, addition, false, addition);
                   m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = nowTime;
                   m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag = true;
               }
               else
               {
                   uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp)/1000;
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[timeDiff]" << hint;
                   }
                   if(timeDiff >= getConfiger().configerTopicMonitorPeriod.periodAlarmPublishInterval)
                   {
                       alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY, addition, false, addition);
                       m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = nowTime;
                   }
               }
               m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag = true;
               m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag = false;
           }
           else
           {
               if(m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag)
               {
                   //mem空间不足-告警消失
                   string hint = "out-of-mem disappeared";
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << SUCESS_COLOR_STR << "[sucess]" << hint << SUCESS_COLOR_END;
                   }
                   if(! m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag)
                   {
                       alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY);
                
                       m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = 0;
                       m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag = false;
                       m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag = true;
                       m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                   }
               }
           }
       }
      
   }
   //监控disk可用空间
   {
        PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
       if(PerformenceUtils::getDiskInfo(runningInfo.diskInfo))
       {
           uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
           hint = "disk-info";
           updateMecStatus(hint, m_MqttClientConfig.rscuEsn, TABLE_UPDATE_MEC_STAUTS_DISK, performanceData);
           if(runningInfo.diskInfo.free <= getConfiger().configAlarmThresholdValue.TV_Disk_Free)
           {
               //disk空间不足-告警产生
               string hint = "out-of-disk occurred, [free]" +
                             std::to_string(runningInfo.memInfo.free) + " [up]" +
                             std::to_string(getConfiger().configAlarmThresholdValue.TV_Disk_Free);
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << interface << "[error]" << hint;
               }
               if(!m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag)
               {
                   alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE, addition, false, addition);
                   m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = nowTime;
                   m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag = true;
               }
               else
               {
                   uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp)/1000;
                   if(getConfiger().enableDebugPrint) {
                       OM_DS_DEBUG_PRINT << interface << "[timeDiff]" << hint;
                   }
                   if(timeDiff >= getConfiger().configerTopicMonitorPeriod.periodAlarmPublishInterval)
                   {
                       alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE, addition, false, addition);
                       m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = nowTime;
                   }
               }
            
               m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag = true;
               m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag = false;
           }
           else
           {
               if(m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag)
               {
                   //disk空间不足-告警消失
                   if(getConfiger().enableDebugPrint) {
                       string hint = "out-of-dick disappeared";
                       OM_DS_DEBUG_PRINT << interface << "[error]" << hint;
                   }
                   if(!m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag)
                   {
                       alarmDisappeared(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE);
                       m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = 0;
                       m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag = false;
                       m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag = true;
                       m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                   }
               }
           }
       }
   }
   //更新net信息
   {
       OmWorkParamConfiger  omWorkParamConfiger;
       WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
       std::string nic = omWorkParamConfiger.mecDeviceWorkParam.nic;
        PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
       if(PerformenceUtils::getNetInfo(runningInfo.netInfo, nic))
       {
           hint = "net-info";
           updateMecStatus(hint, m_MqttClientConfig.rscuEsn, TABLE_UPDATE_MEC_STAUTS_NET, performanceData);
       }
   }

   //监控感知设备是否离线
   {

       std::vector<MsgDeviceStatus> statusList;
       std::pair<std::string, TABLE_TYPE> tableInfo;
       tableInfo.first = m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
       tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
       DBUtils::getDBInfo(tableInfo, statusList);
       int  sensorDeviceOfflineCount = 0;
       std::string sensorDeviceOfflineAddition = "";
       for (auto status: statusList)
       {
           //遍历device-status表，查找status字段,如果离线，则告警产生
           if ( mec::db::DEVICE_STATUS_DB_OFF == status.deviceStatus)
           {
               if(DEVICE_TYPE_DB_MEC == status.deviceType)
               {
                   continue;
               }
               string hint = "sensor-device-offline occurred [device-ID]" +  status.deviceID;
               if(getConfiger().enableDebugPrint)
               {
                   OM_DS_DEBUG_PRINT << interface << hint;
               }
               sensorDeviceOfflineCount++;
               sensorDeviceOfflineAddition.append(status.deviceID);
               sensorDeviceOfflineAddition.append(",");
           }
           else
           {
               //感知设备在线：则清理告警类型标识列表中缓存，同时告警消失
//               sensorDeviceOfflineCount--;
           }
       }
   
       if(sensorDeviceOfflineCount > 0)
       {
           string hint = "ire-offline occured";
           if(getConfiger().enableDebugPrint) {
               OM_DS_DEBUG_PRINT << "[alarm]" << hint;
           }
           uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
           if(!m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag)
           {
               alarmOccurred(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, getConfiger().rscuEsn, true, sensorDeviceOfflineAddition);
               m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag = true;
               m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = nowTime;
           }
           else
           {
               uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp)/1000;
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[timeDiff]" << hint;
               }
               if(timeDiff >= getConfiger().configerTopicMonitorPeriod.periodAlarmPublishInterval)
               {
                   alarmOccurred(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, getConfiger().rscuEsn, true, sensorDeviceOfflineAddition);
                   m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = nowTime;
               }
           }
        
           m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag = true;
           m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag = false;
           
       }
       else
       {
           if(m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag)
           {
               string hint = "ire-offline disappeared";
               if(getConfiger().enableDebugPrint) {
                   OM_DS_DEBUG_PRINT << "[error]" << hint;
               }
               if(!m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag)
               {
                   alarmDisappeared(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, getConfiger().rscuEsn);
                   m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = 0;
                   m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag = false;
                   m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag = true;
                   m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag = true;
               }
           }
       }
   }
}


bool OM_DEVICE_STATUS_COMPONENT::updateMecStatus(std::string hint,  std::string deviceNo, TABLE_UPDATE_MEC_STAUTS_OP_TYPE opType, PerformanceData& performanceData,
                                              std::string dbTableName, TABLE_TYPE tableType)
{
    std::string interface = "[alarm]";
   MsgDeviceStatus msgDeviceStatus;
   msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
   msgDeviceStatus.timeStampNeedUpdate = true;
   
   msgDeviceStatus.deviceID = deviceNo;
   msgDeviceStatus.deviceIDNeedUpdate = true;
   
   //cpu信息
   if(TABLE_UPDATE_MEC_STAUTS_CPU == opType)
   {
       msgDeviceStatus.cpuLoad = performanceData.runningInfo.cpuInfo.load;
       msgDeviceStatus.cpuLoadNeedUpdate = true;
   
       msgDeviceStatus.cpuTemp = performanceData.runningInfo.cpuInfo.temp;
       msgDeviceStatus.cpuTempNeedUpdate = true;
   
       msgDeviceStatus.cpuUti =  performanceData.runningInfo.cpuInfo.uti;
       msgDeviceStatus.cpuUtiNeedUpdate = true;
       hint = "cpu-info";
   }
 
   //mem信息
   if(TABLE_UPDATE_MEC_STAUTS_MEM == opType)
   {
       msgDeviceStatus.memTotal =  performanceData.runningInfo.memInfo.total;
       msgDeviceStatus.memTotalNeedUpdate = true;
   
       msgDeviceStatus.memUsed =  performanceData.runningInfo.memInfo.used;
       msgDeviceStatus.memUsedNeedUpdate = true;
   
       msgDeviceStatus.memFree = performanceData.runningInfo.memInfo.free;
       msgDeviceStatus.memFreeNeedUpdate = true;
       hint = "mem-info";
   }
  
   //disk信息
   if(TABLE_UPDATE_MEC_STAUTS_DISK == opType)
   {
       msgDeviceStatus.diskTotal = performanceData.runningInfo.diskInfo.total;
       msgDeviceStatus.diskTotalNeedUpdate = true;
   
       msgDeviceStatus.diskUsed =  performanceData.runningInfo.diskInfo.used;
       msgDeviceStatus.diskUsedNeedUpdate = true;
   
       msgDeviceStatus.diskFree =  performanceData.runningInfo.diskInfo.free;
       msgDeviceStatus.diskFreeNeedUpdate = true;
   
       msgDeviceStatus.diskTps =  performanceData.runningInfo.diskInfo.tps;
       msgDeviceStatus.diskTpsNeedUpdate = true;
   
       msgDeviceStatus.diskWrite =  performanceData.runningInfo.diskInfo.write;
       msgDeviceStatus.diskWriteNeedUpdate = true;
   
       msgDeviceStatus.diskRead =  performanceData.runningInfo.diskInfo.read;
       msgDeviceStatus.diskReadNeedUpdate = true;
       hint = "disk-info";
   }
   //net信息
   if(TABLE_UPDATE_MEC_STAUTS_NET == opType)
   {
       msgDeviceStatus.netRx = performanceData.runningInfo.netInfo.rx;
       msgDeviceStatus.netRxNeedUpdate = true;
   
       msgDeviceStatus.netTx =  performanceData.runningInfo.netInfo.tx;
       msgDeviceStatus.netTxNeedUpdate = true;
   
       msgDeviceStatus.netRxByte =  performanceData.runningInfo.netInfo.rxByte;
       msgDeviceStatus.netRxByteNeedUpdate = true;
   
       msgDeviceStatus.netTxByte =  performanceData.runningInfo.netInfo.txByte;
       msgDeviceStatus.netTxByteNeedUpdate = true;
       hint = "net-info";
   }
   
   std::pair<std::string, TABLE_TYPE> tableInfo;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[table]" << dbTableName << " update [" << hint << "]";
    }
   tableInfo.first =  dbTableName;
   tableInfo.second = tableType;

   DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);

   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::alarmOccurred(std::string hint,  MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode,
                                               std::string sensorDeviceNo,
                                               bool isUpdateSensorStatus, std::string addition, std::string dbTableName, TABLE_TYPE tableType)
{
    std::string interface = "[alarm]";
    MsgDeviceStatus msgDeviceStatus;
    msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.timeStampNeedUpdate = true;
    
    msgDeviceStatus.deviceID = sensorDeviceNo;
    msgDeviceStatus.deviceIDNeedUpdate = true;
    
    msgDeviceStatus.alarmLevel = (AlertLevelEnum)ALERT_CRITICAL;
    msgDeviceStatus.alarmLevelNeedUpdate = true;
    
    msgDeviceStatus.alarmStatus = (AlarmStatusEnum)ALARM_OCCURRED;
    msgDeviceStatus.alarmStatusNeedUpdate = true;
    
    msgDeviceStatus.alarmRaisedTime =  afl::util::TimeStamp::now(true).millSeconds();
    msgDeviceStatus.alarmRaisedTimeNeedUpdate = true;
    
    msgDeviceStatus.alarmChangedTime = 0;
    msgDeviceStatus.alarmChangedTimeNeedUpdate = true;

    msgDeviceStatus.alarmType  = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[alarmType-name]" << msgDeviceStatus.alarmType ;
    }
    msgDeviceStatus.alarmTypeNeedUpdate = true;
    //告警类型：感知设备离线、摄像机角度偏移，增加addition字段
    if(mecAlarmTypeErrorCode == MEC_ALARM_TYPE_IRE_OFF_LINE
        || mecAlarmTypeErrorCode == MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET
        || mecAlarmTypeErrorCode == MEC_ALARM_TYPE_HIGH_TEMPER_CPU
        || mecAlarmTypeErrorCode == MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU
        || mecAlarmTypeErrorCode == MEC_ALARM_TYPE_OUT_OF_MEMORY
        ||  mecAlarmTypeErrorCode == MEC_ALARM_TYPE_OUT_OF_DISK_SPACE
        ||  mecAlarmTypeErrorCode == MEC_ALARM_TYPE_ERROR_TIMING)
    {
        msgDeviceStatus.addition = addition;
        msgDeviceStatus.additionNeedUpdate = true;
    }
   
    //告警产生
    if( TABLE_TYEP_MEC_DEV_ALARM == tableType)
    {
        std::pair<std::string, TABLE_TYPE> tableInfo;
        if(getConfiger().enableDebugPrint) {
            OM_DS_DEBUG_PRINT << interface << "[table-name]" << dbTableName << "[db-type]" << tableType;
        }
        tableInfo.first =  dbTableName;
        tableInfo.second = tableType;
#if defined(USE_RUNLOG)
        m_Eventloop->runInLoop([this, tableInfo, msgDeviceStatus, interface]()
       {
           DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
       });
#else
        if(getConfiger().enableDebugPrint)
        {
            OM_DS_DEBUG_PRINT  << "[msgDeviceStatus]" << msgDeviceStatus.to_string();
        }
        DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
#endif
    }
    
    if(isUpdateSensorStatus)
    {
        //更新感知设备状态表
        std::pair<std::string, TABLE_TYPE> tableInfo;
        tableInfo.first =  getConfiger().configerDb.DbDeviceStatusTableName;
        tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
        if(getConfiger().enableDebugPrint) {
            OM_DS_DEBUG_PRINT << interface << "[table-name]" << tableInfo.first << "[db-type]" << tableInfo.second;
        }
#if defined(USE_RUNLOG)
        m_Eventloop->runInLoop([this, msgDeviceStatus, interface]()
         {
               DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
         });
#else
        DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
#endif
    }
}

bool OM_DEVICE_STATUS_COMPONENT::alarmDisappeared(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo,
                                                 std::string dbTableName, TABLE_TYPE tableType)
{
#if defined(USE_RUNLOG)
       m_Eventloop->runInLoop([this, hint, mecAlarmTypeErrorCode, sensorDeviceNo, dbTableName, tableType]()
      {
           std::string interface = "[alarm]";
          std::vector<MsgDeviceStatus> statusList;
          std::pair<std::string, TABLE_TYPE> tableInfo;
          tableInfo.first =  dbTableName;
          tableInfo.second = tableType;
          DBUtils::getDBInfo(tableInfo, statusList);
          //读取数据表，然后更新
          for(auto& v : statusList)
          {
              if(mecAlarmTypeErrorCode == v.alarmType)
              {
                  if(getConfiger().enableDebugPrint) {
                        OM_DS_DEBUG_PRINT << interface << "[alarm]find alarmType " << "[mecAlarmTypeErrorCode]" << mecAlarmTypeErrorCode ;
                  }
                  MsgDeviceStatus msgDeviceStatus;
                  msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                  msgDeviceStatus.timeStampNeedUpdate = true;
                  
                  msgDeviceStatus.alarmType  = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
                  msgDeviceStatus.alarmTypeNeedUpdate = true;
                  DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
              }
//              {
//                  //更新device-status表，消除告警
//                  std::vector<MsgDeviceStatus> statusListTemp;
//                  std::pair<std::string, TABLE_TYPE> tableInfo;
//                  tableInfo.first =  getConfiger().configerDb.DbDeviceStatusTableName;
//                  tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
//                  DBUtils::getDBInfo(tableInfo, statusListTemp);
//                  for(auto& vTemp : statusListTemp)
//                  {
//                      if(vTemp.deviceID == v.addition)
//                      {
//                          MsgDeviceStatus msgDeviceStatus;
//                          msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
//                          msgDeviceStatus.timeStampNeedUpdate = true;
//                          std::string deviceID = vTemp.deviceID;
//
//                          msgDeviceStatus.deviceID = deviceID;
//                          msgDeviceStatus.deviceIDNeedUpdate = true;
//
//                          msgDeviceStatus.alarmLevel = -1;
//                          msgDeviceStatus.alarmLevelNeedUpdate = true;
//
//                          msgDeviceStatus.alarmStatus = ALARM_DISAPPEARED;
//                          msgDeviceStatus.alarmStatusNeedUpdate = true;
//
//
//                          msgDeviceStatus.alarmChangedTime = afl::util::TimeStamp::now(true).millSeconds();
//                          msgDeviceStatus.alarmChangedTimeNeedUpdate = true;
//
//                          msgDeviceStatus.alarmType = MEC_ALARM_TYPE_IRE_OFF_LINE;
//                          msgDeviceStatus.alarmTypeNeedUpdate = true;
//
//                          msgDeviceStatus.addition = "";
//                          msgDeviceStatus.additionNeedUpdate = true;
//
//                          tableInfo.first =  m_MqttClientConfig.configerDb.DbDeviceStatusTableName;
//                          tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
//
//                          DBUtils::updateDBInfo(tableInfo, msgDeviceStatus);
//
//                      }
//                  }
//              }
          }
    });
#else
    std::string interface = "[alarm]";
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first =  dbTableName;
    tableInfo.second = tableType;
    DBUtils::getDBInfo(tableInfo, statusList);
    //读取数据表，然后更新
    for(auto& v : statusList)
    {
        if(mecAlarmTypeErrorCode == v.alarmType)
        {
            if(getConfiger().enableDebugPrint) {
                OM_DS_DEBUG_PRINT << interface << "[alarm]find alarmType " << "[mecAlarmTypeErrorCode]"
                                  << mecAlarmTypeErrorCode;
            }
            MsgDeviceStatus msgDeviceStatus;
            msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            msgDeviceStatus.timeStampNeedUpdate = true;
            
            msgDeviceStatus.alarmType  = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
            msgDeviceStatus.alarmTypeNeedUpdate = true;
            DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
        }
    }
#endif
   return true;
}

void OM_DEVICE_STATUS_COMPONENT::processTiming(std::string topic, const afl::base::json &subScribeJson)
{
    std::string interface = "[alarm]";
    AlarmManagementData alarmManagementData;
    try{
        alarmManagementData = subScribeJson;
    }catch (afl::util::Exception& e)
    {
        OM_DS_ERROR_PRINT << interface << "[error]" << e.what();
    }
    //告警入库
    bool isUpdateFlag = false;
    MsgDeviceStatus msgDeviceStatus;
    msgDeviceStatus.timeStamp = alarmManagementData.timeStamp;
    msgDeviceStatus.timeStampNeedUpdate = true;
    
    msgDeviceStatus.deviceID = alarmManagementData.rscuEsn;
    msgDeviceStatus.deviceIDNeedUpdate = true;
    
    msgDeviceStatus.alarmLevel = (AlertLevelEnum)alarmManagementData.alarm.alarmLevel;
    msgDeviceStatus.alarmLevelNeedUpdate = true;
    
    msgDeviceStatus.alarmStatus = (AlarmStatusEnum)alarmManagementData.alarm.alarmStatus;
    msgDeviceStatus.alarmStatusNeedUpdate = true;
    
    
    msgDeviceStatus.alarmRaisedTime =  alarmManagementData.alarm.alarmRaisedTime;
    msgDeviceStatus.alarmRaisedTimeNeedUpdate = true;
    
    msgDeviceStatus.alarmChangedTime =  alarmManagementData.alarm.alarmChangedTime;
    msgDeviceStatus.alarmChangedTimeNeedUpdate = true;
    
    msgDeviceStatus.alarmType  = (MecAlarmTypeErrorCodeEnum)alarmManagementData.alarm.alarmType;
    msgDeviceStatus.alarmTypeNeedUpdate = true;
    
    if(alarmManagementData.alarm.alarmRaisedTime > 0)
    {
        if(alarmManagementData.alarm.alarmChangedTime == 0)
        {
            isUpdateFlag = true;
        }
        else
        {
            isUpdateFlag = false;
        }
    }

    msgDeviceStatus.addition = alarmManagementData.alarm.addition;
    msgDeviceStatus.additionNeedUpdate = true;
    
    std::pair<std::string, TABLE_TYPE> tableInfo;
   
    tableInfo.first =  getConfiger().configerDb.DbMecDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_ALARM;
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << interface << "[table-name]" << tableInfo.first << "[db-type]" << tableInfo.second;
    }
    if(isUpdateFlag)
    {
#if defined(USE_RUNLOG)
        m_Eventloop->runInLoop([this, tableInfo, msgDeviceStatus, interface]()
           {
               DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
           });
#else
        DBUtils::updateDBInfo(tableInfo, msgDeviceStatus, interface);
#endif

    }
    else
    {
#if defined(USE_RUNLOG)
        m_Eventloop->runInLoop([this, tableInfo, msgDeviceStatus, interface]()
       {
               DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
       });
#else
        DBUtils::delDBInfo(tableInfo, msgDeviceStatus, interface);
#endif
    
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
bool OM_DEVICE_STATUS_COMPONENT::mqttInitConnOpts()
{
   m_MqttConnOpts = MQTTAsync_connectOptions_initializer;
   m_MqttConnOpts.connectTimeout = m_MqttClientConfig.configerMqtt.mqttConnectTimeOut;
   m_MqttConnOpts.keepAliveInterval = m_MqttClientConfig.configerMqtt.mqttKeepAliveInterval;
   m_MqttConnOpts.cleansession = m_MqttClientConfig.configerMqtt.mqttCleanSession;
   m_MqttConnOpts.MQTTVersion = m_MqttClientConfig.configerMqtt.mqttVersion;
   m_MqttConnOpts.username = m_MqttClientConfig.configerMqtt.mqttUserName.c_str();
   m_MqttConnOpts.password = m_MqttClientConfig.configerMqtt.mqttPassword.c_str();
   m_MqttConnOpts.onSuccess = mqttOnConnect;
   m_MqttConnOpts.onFailure = mqttOnConnectFailure;
   m_MqttConnOpts.context = this;

   return true;
}
bool OM_DEVICE_STATUS_COMPONENT::mqttInit()
{
   m_MqttClient = nullptr;
   m_MqttConnected = false;

   mqttInitConnOpts();
   std::string clientIdTemp = m_MqttClientConfig.mqttClientId + ":" +
                              std::to_string(afl::util::TimeStamp::now(true).microSeconds());
    if(getConfiger().enableDebugPrint) {
        OM_DS_DEBUG_PRINT << "[mqttBroker-Url]" << m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str()
                          << "[client-id]" << clientIdTemp.c_str()
                          << "[username]" << m_MqttClientConfig.configerMqtt.mqttUserName.c_str()
                          << "[passwd]" << m_MqttClientConfig.configerMqtt.mqttPassword.c_str();
    }
   if (MQTTASYNC_SUCCESS != MQTTAsync_create(&m_MqttClient, m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str(),
                                             clientIdTemp.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL))
   {
       OM_DS_ERROR_PRINT << "[error]create connectiont failure!";
       return false;
   }

   MQTTAsync_setCallbacks(m_MqttClient, this, mqttConnlost, mqttSubscribeMsgArrvd, NULL);

   if (!mqttConnect())
   {
       OM_DS_ERROR_PRINT << "[error]Connect Failure!";
       return false;
   }

   return true;
}

void OM_DEVICE_STATUS_COMPONENT::mqttDeinit()
{
   if (m_MqttClient)
   {
       MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
       opts.onSuccess = mqttOnDisconnect;
       opts.context = m_MqttClient;
       MQTTAsync_disconnect(m_MqttClient, &opts);

       int cnt = 0;
       while (m_MqttConnected && (cnt < 30))
       {
           usleep(10000L);
           cnt++;
       }

       MQTTAsync_destroy(&m_MqttClient);
       m_MqttClient = nullptr;
       m_MqttConnected = false;
   }
}

void OM_DEVICE_STATUS_COMPONENT::mqttOnConnect(void *context, MQTTAsync_successData *response)
{
    OM_DS_DEBUG_PRINT << SUCESS_COLOR_STR << "[sucess]connect sucess!" << SUCESS_COLOR_END;
   auto thiz = (OM_DEVICE_STATUS_COMPONENT *) context;
   thiz->m_MqttConnected = true;
   MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
   opts.onSuccess = mqttOnSubscribe;
   opts.onFailure = mqttOnSubscribeFailure;
   opts.context = thiz;


   if(thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum > 0)
   {
       if (MQTTAsync_subscribeMany(thiz->m_MqttClient, thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum,
                                   thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeTopics,
                                   thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeQoss,
                                   &opts) != MQTTASYNC_SUCCESS)
       {
           OM_DS_ERROR_PRINT << "[error]subscribe error";
       }
   }
}

void OM_DEVICE_STATUS_COMPONENT::mqttOnConnectFailure(void *context, MQTTAsync_failureData *response)
{
   if (response)
   {
       if (response->message)
       {
           OM_DS_ERROR_PRINT << "[error]connect error [error-code]" << response->code << "[error-msg]" << response->message;
       } else
       {
           OM_DS_ERROR_PRINT << "[error]connect error [error-code]" << response->code << "[error-msg]" << response->message;
       }
   } else
   {
       OM_DS_ERROR_PRINT << "[error]connect error!";
   }

   auto thiz = (OM_DEVICE_STATUS_COMPONENT *) context;
   thiz->mqttReconnect();
}


bool OM_DEVICE_STATUS_COMPONENT::mqttConnect()
{
   int rc;
   if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(m_MqttClient, &m_MqttConnOpts)))
   {
       OM_DS_ERROR_PRINT << "[error]connect error!";
       m_MqttConnected = false;
       return false;
   }
   return true;
}

bool OM_DEVICE_STATUS_COMPONENT::mqttReconnect()
{
   OM_DS_ERROR_PRINT << "[notice]mqtt reconnect!";

   m_Eventloop->addTimer(std::bind(&OM_DEVICE_STATUS_COMPONENT::mqttConnect, this), m_MqttClientConfig.configerMqtt.mqttReconnectInterval, false);

   return true;
}

void OM_DEVICE_STATUS_COMPONENT::mqttConnlost(void *context, char *cause)
{
   auto thiz = (OM_DEVICE_STATUS_COMPONENT *) context;
   thiz->m_MqttConnected = false;
   OM_DS_ERROR_PRINT << "[error]connect lost! [cause]" << cause;
   thiz->mqttReconnect();
}

void OM_DEVICE_STATUS_COMPONENT::mqttOnDisconnect(void *context, MQTTAsync_successData *response)
{
   OM_DS_ERROR_PRINT << "[notice]disconnect!";
   auto thiz = (OM_DEVICE_STATUS_COMPONENT *) context;
   thiz->m_MqttConnected = false;
}

void OM_DEVICE_STATUS_COMPONENT::mqttOnSubscribe(void *context, MQTTAsync_successData *response)
{
   OM_DS_SUCCESS_PRINT << "[sucess]subscribe sucess!";
}

void OM_DEVICE_STATUS_COMPONENT::mqttOnSubscribeFailure(void *context, MQTTAsync_failureData *response)
{
   OM_DS_ERROR_PRINT << "[error]subscribe failure!";
}

bool OM_DEVICE_STATUS_COMPONENT::mqttPublishMsg(const std::string &topic, const std::string &msg)
{
   if (!m_MqttConnected)
   {
       OM_DS_ERROR_PRINT << "[error]mqtt has not connected!";
       return false;
   }

   if (msg.size() <= 0 || topic.empty())
   {
       OM_DS_ERROR_PRINT << "[error]len < 0 or topic empty!";
       return false;
   }

   MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
   MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
   int rc;

   opts.context = m_MqttClient;
   pubmsg.payload = (void *) msg.c_str();
   pubmsg.payloadlen = msg.length();
   pubmsg.qos = m_MqttClientConfig.configerMqtt.mqttSendQos;
   pubmsg.retained =  getConfiger().configerMqtt.mqttRetained;;

   if ((rc = MQTTAsync_sendMessage(m_MqttClient, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
   {
       if (rc == MQTTASYNC_DISCONNECTED)
       {
           OM_DS_ERROR_PRINT << "[error]Async Connect failure，will connect!";
           if (m_MqttConnected)
           {
               m_MqttConnected = false;
               mqttReconnect();
           }
       } else
       {
           OM_DS_ERROR_PRINT << "[error]push msg error，[error-code]" << rc;
       }

       return false;
   }

   return true;
}

int OM_DEVICE_STATUS_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
   int ret = 1;
   auto thiz = (OM_DEVICE_STATUS_COMPONENT *) context;
   if (message->payloadlen)
   {
       if(thiz->getConfiger().enableDebugPrint) {
           OM_DS_WARN_PRINT << "[subscirbe-topic]" << topicName << "[msg-len" << message->payloadlen << "[msg]"
                            << (char *) (message->payload);
       }
       afl::base::json j;
       try
       {
           j = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
       }
       catch (...)
       {
           OM_DS_ERROR_PRINT << "[error]parse json failure!";
           MQTTAsync_freeMessage(&message);
           MQTTAsync_free(topicName);
           return ret;
       }

       std::string topic(topicName, topicLen);
       if (topic.size() <= 0)
       {
           OM_DS_ERROR_PRINT << "[error]topic empty!";
       } else
       {
#if defined(USE_RUNLOG)
//           thiz->m_Eventloop->runInLoop([thiz, topic, j]()
//            {
//                thiz->mqttDispatchSubscripeMessage(topic, j);
//            });
           static int message_count = 1; // 用于计数的静态变量
            static int MAX_MESSAGE_COUNT = 1000; // 设置您的最大限制

            // 根据消息计数判断是偶数还是奇数
            if (message_count % 2 == 0) // 偶数
            {
                thiz->m_Eventloop->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessage(topic, j);
                 });
            }
            else // 奇数
            {
                thiz->m_Eventloop->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessage(topic, j);
                 });
            }
            if (message_count > MAX_MESSAGE_COUNT)
            {
                message_count = 0; // 重置计数器
            }
            else
            {
                message_count++;
            }
#else
           if(thiz->getConfiger().enableDebugPrint)
           {
               OM_DS_ERROR_PRINT << "[topic]" << topic;
           }
           thiz->mqttDispatchSubscripeMessage(topic, j);
#endif
       }
   }

   MQTTAsync_freeMessage(&message);
   MQTTAsync_free(topicName);

   return ret;
}

bool OM_DEVICE_STATUS_COMPONENT::mqttPushStringMsg2Broker(string topic, string info, string hint)
{
   if(!info.empty())
   {
       if(!mqttPublishMsg(topic, info))
       {
           OM_DS_ERROR_PRINT << "[error]push " << hint.c_str() << "failure!";
           return false;
       }
   }
   else
   {
       return false;
   }

   return true;
}
bool OM_DEVICE_STATUS_COMPONENT::mqttPushJsonMsg2Broker(string topic, json info, string hint)
{
   std::string pData;
   try{
       pData = info.dump();
   }
   catch (json::exception& e)
   {
       OM_DS_ERROR_PRINT << "[what]" << e.what() << "[json-exception-id]" << e.id;
       return false;
   }
   if(!pData.empty())
   {
       if(!mqttPublishMsg(topic, pData))
       {
           OM_DS_ERROR_PRINT << "[error]push " << hint.c_str() << " failure!";
           return false;
       }
   }else
   {
       return false;
   }
   return true;
}
NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
