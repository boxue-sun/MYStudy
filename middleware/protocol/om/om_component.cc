/*********************************************************************************
 * @file		OM_COMPONENT
 * @brief		OM_COMPONENT belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		25-02-09
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  25-02-09 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#include "om_component.h"
#include <arpa/inet.h>

NAMESPACE_START_OM

bool OM_COMPONENT::Init()
{
    if (getConfiger().enableDebugPrint)
    {
        OM_MQTT_DEBUG_PRINT << "[notice]start  om!";
    }
#if ENABLE_ENCRYPTION
    auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
    std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
    OM_MEC_DEBUG_PRINT << "License is: " << license << std::endl;
    int result = FusionService::Authenticator::GetInstance().Authorize(license);
    if (result != 0)
    {
        OM_MEC_ERROR_PRINT << "License generated fail, error code:  " << result;
        exit(1);
    }
#endif

    m_Task.reset(new std::thread([&](){ initEventLoop(); }));

    m_TaskOmSpat.reset(new std::thread([&](){ initEventLoopOmSpat(); }));
    m_TaskCloud.reset(new std::thread([&](){ initEventLoopCloud(); }));
    m_TaskOmRadar.reset(new std::thread([&](){ initEventLoopOmRadar(); }));
    m_TaskOmCamera.reset(new std::thread([&](){ initEventLoopOmCamera(); }));
    m_TaskOmMec.reset(new std::thread([&](){ initEventLoopOmMec(); }));
    m_TaskCloudPubFirst.reset(new std::thread([&](){ initEventLoopCloudPubFirst(); }));
    m_TaskCloudPubSecond.reset(new std::thread([&](){ initEventLoopCloudPubSecond(); }));
    m_TaskOmPing.reset(new std::thread([&](){ initEventLoopOmPing(); }));
    m_TaskOmCheck.reset(new std::thread([&](){ initEventLoopOmCheck(); }));

    if(!getConfiger().configerMec.enableRegister)
    {
        m_RegisterFlag = true;
    }

    doBaiscWork();
    return true;
}

bool OM_COMPONENT::doBaiscWork()
{
    //初始化mqtt配置
    getWorkParamFromFile();
    getMqttConfigerMsg();
    initMonitorRules();
    if(getConfiger().configerEnable.Enable_Mec)
    {
        m_TaskProcessEventOutputResult.reset(new std::thread([&](){ initProcessEventOutputResult(); }));
        m_TaskProcessSpatSrcData.reset(new std::thread([&](){ initProcessSpatSrcData(); }));
        m_TaskProcessV2xBsmData.reset(new std::thread([&](){ initProcessV2xBsmData(); }));
        m_TaskProcessV2xData.reset(new std::thread([&](){ initProcessV2xData(); }));
        m_TaskProcessTrafficlightDetectData.reset(new std::thread([&](){ initProcessTrafficlightDetectData(); }));
  		m_TaskProcessTiming.reset(new std::thread([&](){ getTimingInfo(); }));;

        //mec自检
        m_TaskProcessMecCheckSpatSrcData.reset(new std::thread([&](){ initProcessMecCheckSpatSrcData(); }));
        m_TaskProcessMecCheckRsapData.reset(new std::thread([&](){ initProcessMecCheckRsapData(); }));
        m_TaskProcessMecCheckRsapLink.reset(new std::thread([&](){ initProcessMecCheckRsapLink(); }));
        m_TaskProcessMecCheckCcindexTcData.reset(new std::thread([&](){ initProcessMecCheckCcindexTcData(); }));
        m_TaskProcessMecCheckCcindexTmData.reset(new std::thread([&](){ initProcessMecCheckCcindexTmData(); }));
        m_TaskProcessMecCheckCcindexTmLink.reset(new std::thread([&](){ initProcessMecCheckCcindexTmLink(); }));
        m_TaskProcessMecCheckCcindexStData.reset(new std::thread([&](){ initProcessMecCheckCcindexStData(); }));
        m_TaskProcessMecCheckCcindexStLink.reset(new std::thread([&](){ initProcessMecCheckCcindexStLink(); }));
        m_TaskProcessMecCheckCcindexCcindexRcpData.reset(new std::thread([&](){ initProcessMecCheckCcindexRcpData(); }));
        m_TaskProcessMecCheckCcindexCcindexRcpLink.reset(new std::thread([&](){ initProcessMecCheckCcindexRcpLink(); }));
        m_TaskProcessMonitorSpatData.reset(new std::thread([&](){ initProcessMonitorSpat(); }));
    }

    if (!mqttInit())
    {
        OM_MEC_ERROR_PRINT << "[error] mqtt-inter Init failure!";
        return false;
    }
    getMqttConfigerMsgCloud();
    if (!mqttInitCloud())
    {
        OM_MEC_ERROR_PRINT << "[error] mqtt-cloud init failure!";
        return false;
    }
    if(getConfiger().configerEnable.Enable_Mec)
    {
        //注册信息
        m_PublishAlarmDataIntervalUnit = getConfiger().configerMec.configerPublishPeriod.publishAlarmDataIntervalUnit;
#ifndef TEST_CHECK_MEC_STATUS
        m_TimerMonitorOmStatus = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::periodMonitorOmStatus, this),
                getConfiger().configerMec.configerPublishPeriod.periodMonitorOmStatus, true);

        m_TimerPublishBasicDeviceInfoDataOnce = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::onConnectedPushMsgOnce, this),
                3, true);

//	    m_TimerMaintenanceManagementRebootMonitorFd = m_EventloopOmMec->addTimer(std::bind(&OM_COMPONENT::monitorRebootTimeConfiger, this), 1, true);
        //tcp接口初始化
        tcpinit();

        m_TaskHttp.reset(new std::thread([&]() { processHttpPostRequest(); }));//处理平台http post
        m_TaskHttpCamera.reset(new std::thread([&]() { startHttpServer(); }));//处理相机http post

        //定时删除违法事件图片
        m_EventloopOmMec->addTimer(std::bind(&OM_COMPONENT::checkImagesTime, this), 60, true);

        //ptp授时
        m_ptpStatusUpTimer = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::upPtpStatus, this, true),
                getConfiger().configerMec.configerPublishPeriod.periodTimingUp, true);

        //定时设备心跳信息
        m_TimerPublishDeviceHeartBeatData = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishDeviceHeartbeatData, this),
                getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp, true);

        //运行状态
        DeviceBaseInfoQueryData deviceBaseInfoQueryData;
        deviceBaseInfoQueryData.infoId = CONNECTED_DEVICE_RUNNING_STATUS;
        m_TimerPublishDeviceRunningStatusData = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishRunningStatusData, this, deviceBaseInfoQueryData,
                          DEVICE_BASIC_INFO_SENSOR),
                getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp, true);
        //ota升级
        m_OtaUpdateFd = m_EventloopOmMec->addTimer(std::bind(&OM_COMPONENT::monitorDeviceNeedUpdate, this),
                                              getConfiger().configerMec.configerPublishPeriod.periodDownloadBin, true);
        //性能
        m_TimerPublishPerformenceData = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishPerformenceData, this, deviceBaseInfoQueryData, true),
                getConfiger().configerMec.configerPublishPeriod.periodPerformenceUp, true);

        //版本信息
        m_TimerDeviceVersion = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishDeviceVersionData, this, deviceBaseInfoQueryData, false),
                getConfiger().configerMec.configerPublishPeriod.periodDeviceVersion, true);

        //实时告警
        m_TimerAlarmMonitor = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::alarmMonitor, this),
                getConfiger().configerMec.configerPublishPeriod.periodAlarmMonitor, true);


        if(getConfiger().configerMec.configerEnable.Enable_Camera_InternalExternalParams_Up)
        {
            if(!findJpgFiles(getConfiger().configerMec.configerCameraInterExterParam.path))
            {
                OM_MEC_ERROR_PRINT << "[camera-param][error]open dir failure!";
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[camera-param][m_CameraParamPathNameMap-size] " << m_CameraParamPathNameMap.size() << std::endl;
                    for (auto pair: m_CameraParamPathNameMap)
                    {
                        OM_MEC_DEBUG_PRINT << "[camera-param][Path] " << pair.first << "[File]" << pair.second << std::endl;
                    }
                }
            }
        }
        //[相机]定时推送内外参数
        getCameraInterExterParamFromFile();
        m_TimerPublishCameraInterExterParam = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishCameraInterExterParam, this),
                getConfiger().configerMec.configerPublishPeriod.periodPublishCameraInterExterParamInterval, true);

        //[相机]定时读取文件获得内外参
        m_TimerGetCameraInterExterParam = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::getCameraInterExterParamFromFile, this),
                getConfiger().configerMec.configerPublishPeriod.periodGetCameraInterExterParamInterval, true);

        //[相机]定时从云控获取内外参
        m_TimerQueryCameraInterExterParam = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::queryCameraInterExterParam, this),
                getConfiger().configerMec.configerPublishPeriod.periodQueryCameraInterExterParamInterval, true);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //[雷达]定时推送雷达:连接成功后，推送一次配置
        m_TimerPublishRadarInterExterParam = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::publishRadarInterExterParam, this),
                getConfiger().configerMec.configerPublishPeriod.periodPublishRadarInterExterParamInterval, true);


        //[雷达]定时查询
        m_TimerQueryRadarInterExterParam = m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::queryRadarInterExterParam, this),
                getConfiger().configerMec.configerPublishPeriod.periodQueryRadarInterExterParamInterval, true);
        // m_TimerPublishTrafficlightDetectDataInit = m_EventloopOmMec->addTimer(
        //         std::bind(&OM_COMPONENT::publishTrafficlightDetectData, this),
        //         getConfiger().configerMec.configerPublishPeriod.periodPublishTrafficlightDetectDataInterval, true);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //定时获取运行状态
        getPerformanceData();
        m_TimerGetPerformanceData =  m_EventloopOmMec->addTimer(
                std::bind(&OM_COMPONENT::getPerformanceData, this),
                getConfiger().configerMec.configerPublishPeriod.periodMonitorPerformanceData, true);
        //定时监控设备是否在线
        periodMonitorSensorDeviceActive();
        m_TimerPingSensorDevice = m_EventloopOmPing->addTimer(
            std::bind(&OM_COMPONENT::periodMonitorSensorDeviceActive, this),
            getConfiger().configerMec.configerPublishPeriod.periodPingSensorDeviceActive, true);
        //信号机数据状态
        m_TimerSpatDataStatus =  m_EventloopOmSpat->addTimer(
            std::bind(&OM_COMPONENT::periodMonitorSpatDataStatus, this),
            getConfiger().configerMec.configerPublishPeriod.periodCheckSpatDataStatus, true);
#endif
    }
}


bool OM_COMPONENT::initEventLoopOmCheck()
{
    m_EventloopOmCheck = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmCheck->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopOmPing()
{
    m_EventloopOmPing = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmPing->loop();
    return true;
}
bool OM_COMPONENT::initEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    m_TimerMonitorMqttConnected = m_Eventloop->addTimer(std::bind(&OM_COMPONENT::monitorMqttConnectStatus, this),
                                                        getConfiger().periodMonitorMqttConnect, true);
    m_Eventloop->loop();
    return true;
}

bool OM_COMPONENT::monitorMqttConnectStatus()
{
    // bool mqttConnected = false;
    // bool mqttConnectedCloud = false;
    // if (MQTTAsync_isConnected(m_MqttClient))
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         OM_MQTT_SUCCESS_PRINT << "[inter][success]Connection is still active";
    //     }
    //     mqttConnected = true;
    // }
    // else
    // {
    //     OM_MQTT_ERROR_PRINT << "[inter][error]Connection is lost";
    //     mqttConnected = false;
    // }
    // if (MQTTAsync_isConnected(m_MqttClientCloud))
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         OM_MQTT_SUCCESS_PRINT << "[cloud][success]Connection is still active";
    //     }
    //     mqttConnectedCloud = true;
    // }
    // else
    // {
    //     OM_MQTT_DEBUG_PRINT << "[cloud][error]Connection is lost";
    //     mqttConnectedCloud = false;
    // }
    //
    // if (!mqttConnectedCloud)
    // {
    //     if (m_MqttClientCloud)
    //     {
    //         OM_MQTT_SUCCESS_PRINT << "[cloud][error]Connection is lost, reconnect cloud!";
    //         mqttReconnectCloud();
    //     }
    //     else
    //     {
    //         mqttInitCloud();
    //     }
    //
    // }
    // if (!mqttConnected)
    // {
    //     if (m_MqttClient)
    //     {
    //         OM_MQTT_SUCCESS_PRINT << "[cloud][error]Connection is lost, reconnect cloud!";
    //         mqttReconnect();
    //     }
    //     else
    //     {
    //         mqttInit();
    //     }
    // }
    // 仅仅用于监控和打日志，不要在这里发起重连！
    m_MqttConnected = (MQTTAsync_isConnected(m_MqttClient) != 0);
    m_MqttConnectedCloud = (MQTTAsync_isConnected(m_MqttClientCloud) != 0);
}

bool OM_COMPONENT::initEventLoopCloud()
{
    m_EventloopCloud = std::make_shared<afl::net::EventLoop>();
    m_EventloopCloud->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopCloudPubFirst()
{
    m_EventloopCloudPubFirst = std::make_shared<afl::net::EventLoop>();
    m_EventloopCloudPubFirst->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopCloudPubSecond()
{
    m_EventloopCloudPubSecond = std::make_shared<afl::net::EventLoop>();
    m_EventloopCloudPubSecond->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopOmRadar()
{
    m_EventloopOmRadar = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmRadar->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopOmCamera()
{
    m_EventloopOmCamera = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmCamera->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopOmMec()
{
    m_EventloopOmMec = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmMec->loop();
    return true;
}
bool OM_COMPONENT::initEventLoopOmSpat()
{
    m_EventloopOmSpat = std::make_shared<afl::net::EventLoop>();
    m_EventloopOmSpat->loop();
    return true;
}

bool OM_COMPONENT::Proc(const std::shared_ptr<const  os::v2x::device::CloudData>& recv_data)
{
    if (!recv_data)
    {
        OM_MEC_ERROR_PRINT << "data empty!";
        return false;
    }
    if (recv_data->payload_case() != os::v2x::device::CloudData::PayloadCase::kMqttData)
    {
        OM_MEC_ERROR_PRINT << "NOT MQTT MSG.";
        return false;
    }

    return true;
}
//信号机数据状态
void  OM_COMPONENT::initProcessMonitorSpat()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorSpat>(
            m_MqttClientConfig.configerMec.configerMonitorSpat.channel_readers,
            std::bind(&OM_COMPONENT::processMonitorSpatDataStatus, this, std::placeholders::_1));
}

//信号机数据状态
void OM_COMPONENT::processMonitorSpatDataStatus(const std::shared_ptr<const airos::monitor_mec::MonitorSpat> &recvData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Spat_Data_Status)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[monitor_spat_data_status]" << recvData->DebugString() ;
    }
    if (recvData->has_md_spat_data_status() )
    {
        m_SpatDataStatusFromTLAdapterFlag.store(true, std::memory_order_relaxed);

        // --- 频控逻辑开始 ---
        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();

        // 获取配置的上报间隔，单位毫秒。
        // 假设配置项为 SpatDataStatusUploadIntervalMs
        // 如果配置为 1000ms 则为 1Hz，配置为 500ms 则为 2Hz
        uint64_t uploadInterval = getConfiger().configerMec.configerPublishPeriod.periodSpatDataStatusUploadInterval * 1000;

        // 容错：如果配置为0，默认设为1000ms，或者按原频率(0)处理
        if (uploadInterval == 0) uploadInterval = 1000;

        // 检查是否达到上报时间间隔
        if (now - m_lastSpatDataPublishTime < uploadInterval)
        {
            return; // 未达到时间间隔，跳过上报
        }
        // 更新上次上报时间
        m_lastSpatDataPublishTime = now;
        // --- 频控逻辑结束 ---
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[monitor_spat_data_status]has_md_spat_data_status" ;
        }

        // 2. 获取内部消息的引用 (建议使用 const auto& 避免拷贝，提高性能)
        const auto& spat_data = recvData->md_spat_data_status();

        // spat_data.status() 返回的是 MonitorSpatDataStatus 枚举类型
        airos::monitor_mec::MonitorSpatDataStatus current_status = spat_data.status();

        std::string hint = "Monitor-Spat-Data-Status";
        if (getConfiger().enableDebugPrint)
        {
            OM_CLOUD_DEBUG_PRINT << "[hint]" << hint ;
        }
        TrafficLightDataStatus trafficLightDataStatus;
        trafficLightDataStatus.timestamp = afl::util::TimeStamp::now(true).millSeconds();
        trafficLightDataStatus.seqNum = afl::util::Srand::srandStr(32);
        trafficLightDataStatus.rscuEsn = m_MqttClientConfig.rscuEsn;
        trafficLightDataStatus.lightEsn = m_MqttClientConfig.rscuEsn;
        if (current_status == airos::monitor_mec::MONITOR_STATUS_SPAT_DATA_NORMAL)
            {
            // 正常情况
            trafficLightDataStatus.status = 0 ;  //正常
        }
        else
        {
            // 异常情况
            trafficLightDataStatus.status = 1 ;  //异常
        }


        if(!mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Signal_Data_Status_Up, trafficLightDataStatus, hint))
        {
            OM_MEC_ERROR_PRINT << "[error] push spat data status  failure!";
        }
    }

    return;
}

//////////////////////////////////////////////////////////////////
//MEC自检 - SpatSrcData
void OM_COMPONENT::initProcessMecCheckSpatSrcData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecSpatSrcData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckSpatSrcData, this, std::placeholders::_1));
}
//MEC自检 - SpatSrcData
void OM_COMPONENT::processMecCheckSpatSrcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }
    //信号机原始数据
    if (mecDeviceData->has_md_spat_src_data() && getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_spat_src_data" ;
        }
        if (m_MonitorStateSpatSrcData) {
            handleMonitorDataUpdate(*m_MonitorStateSpatSrcData, mecDeviceData->md_spat_src_data().tag(), mecDeviceData->md_spat_src_data().timestamp());
        }
    }

}

//MEC自检 - RsapData
void OM_COMPONENT::initProcessMecCheckRsapData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecRsapData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckRsapData, this, std::placeholders::_1));
}
//MEC自检 - RsapData
void OM_COMPONENT::processMecCheckRsapData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }
    //感知数据
    if (mecDeviceData->has_md_sensor_objs()  && getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_sensor_objs" ;
        }
        if (m_MonitorStateRsapData) {
            handleMonitorDataUpdate(*m_MonitorStateRsapData, mecDeviceData->md_sensor_objs().tag(), mecDeviceData->md_sensor_objs().timestamp());
        }
    }
}

//MEC自检 - RsapLink
void OM_COMPONENT::initProcessMecCheckRsapLink()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecRsapLink.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckRsapLink, this, std::placeholders::_1));
}
//MEC自检 - RsapLink
void OM_COMPONENT::processMecCheckRsapLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]" << mecDeviceData->DebugString() ;
    }
    // 2. 处理链路类消息
    //云控链路状态
    if (mecDeviceData->has_ml_rsap() && getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_rsap" ;
        }
        if (m_MonitorStateRsapLink)
            {
            handleMonitorLinkUpdate(*m_MonitorStateRsapLink, mecDeviceData->ml_rsap().tag(), mecDeviceData->ml_rsap().con_flag(), mecDeviceData->ml_rsap().timestamp());
        }
    }
}

//MEC自检 - CcindexV2xData
void OM_COMPONENT::initProcessMecCheckCcindexTcData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecCcindexV2xData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexTcData, this, std::placeholders::_1));
}
//MEC自检 - CcindexV2xData
void OM_COMPONENT::processMecCheckCcindexTcData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }

    //v2x信控指标
    if (mecDeviceData->has_md_ccindex_tc()  && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_ccindex_tc" ;
        }
        if (m_MonitorStateCcindexTcData) {
            handleMonitorDataUpdate(*m_MonitorStateCcindexTcData, mecDeviceData->md_ccindex_tc().tag(), mecDeviceData->md_ccindex_tc().timestamp());
        }
    }
}

//MEC自检 - CcindexTmData
void OM_COMPONENT::initProcessMecCheckCcindexTmData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecTmData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexTmData, this, std::placeholders::_1));
}
//MEC自检 - CcindexTmData
void OM_COMPONENT::processMecCheckCcindexTmData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }
    //雷达信控指标-动态
    if (mecDeviceData->has_md_ccindex_tm() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_ccindex_tm" ;
        }
        handleMonitorDataUpdateCcindexTm(mecDeviceData->md_ccindex_tm().device_id(),
            mecDeviceData->md_ccindex_tm().topic(), mecDeviceData->md_ccindex_tm().tag(), mecDeviceData->md_ccindex_tm().timestamp());
    }
}

//MEC自检 - CcindexTmLink
void OM_COMPONENT::initProcessMecCheckCcindexTmLink()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecTmLink.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexTmLink, this, std::placeholders::_1));
}
//MEC自检 - CcindexTmLink
void OM_COMPONENT::processMecCheckCcindexTmLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_link]" << mecDeviceData->DebugString() ;
    }

    //信控链路状态-动态
    if (mecDeviceData->has_ml_ccindex_tm() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_ccindex_tm" ;
        }
        if (m_MonitorStateCcindexTmLink){
            handleMonitorLinkUpdate(*m_MonitorStateCcindexTmLink, mecDeviceData->ml_ccindex_tm().tag(), mecDeviceData->ml_ccindex_tm().con_flag(), mecDeviceData->ml_ccindex_tm().timestamp());
        }
    }
}

//MEC自检 - CcindexStData
void OM_COMPONENT::initProcessMecCheckCcindexStData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecStData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexStData, this, std::placeholders::_1));
}
//MEC自检 - CcindexStData
void OM_COMPONENT::processMecCheckCcindexStData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }

    //雷达信控指标-静态
    if (mecDeviceData->has_md_ccindex_st() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_ccindex_st" ;
        }
        handleMonitorDataUpdateCcindexSt(mecDeviceData->md_ccindex_st().device_id(),
            mecDeviceData->md_ccindex_st().topic(), mecDeviceData->md_ccindex_st().tag(), mecDeviceData->md_ccindex_st().timestamp());
    }


}

//MEC自检 - CcindexStLink
void OM_COMPONENT::initProcessMecCheckCcindexStLink()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecStLink.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexStLink, this, std::placeholders::_1));
}
//MEC自检 - CcindexStLink
void OM_COMPONENT::processMecCheckCcindexStLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_link]" << mecDeviceData->DebugString() ;
    }

    //信控链路状态-静态
    if (mecDeviceData->has_ml_ccindex_st() && getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_ccindex_st" ;
        }
        if (m_MonitorStateCcindexStLink){
            handleMonitorLinkUpdate(*m_MonitorStateCcindexStLink, mecDeviceData->ml_ccindex_st().tag(), mecDeviceData->ml_ccindex_st().con_flag(), mecDeviceData->ml_ccindex_st().timestamp());
        }
    }
}

//MEC自检 - CcindexRcpData
void OM_COMPONENT::initProcessMecCheckCcindexRcpData()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecRcpData.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexRcpData, this, std::placeholders::_1));
}
//MEC自检 - CcindexRcpData
void OM_COMPONENT::processMecCheckCcindexRcpData(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]" << mecDeviceData->DebugString() ;
    }
    //毫米波雷达原始数据
    if (mecDeviceData->has_md_radar_cp() && getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data]has_md_radar_cp" ; ;
        }
        handleMonitorDataUpdateRcpData(mecDeviceData->md_radar_cp().device_id(), mecDeviceData->md_radar_cp().tag(), mecDeviceData->md_radar_cp().timestamp());
    }

}

//MEC自检 - CcindexRcpData
void OM_COMPONENT::initProcessMecCheckCcindexRcpLink()
{
    auto reader = node_->CreateReader<airos::monitor_mec::MonitorMec>(
            m_MqttClientConfig.configerMec.configerMonitorMecRcpLink.channel_readers,
            std::bind(&OM_COMPONENT::processMecCheckCcindexRcpLink, this, std::placeholders::_1));
}
//MEC自检 - CcindexRcpData
void OM_COMPONENT::processMecCheckCcindexRcpLink(const std::shared_ptr<const airos::monitor_mec::MonitorMec> &mecDeviceData)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_radar_cp" ;
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_radar_cp" ;
        return;
    }
    // 1. 处理数据类消息
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_link]" << mecDeviceData->DebugString() ;
    }

    //点云链路状态
    if (mecDeviceData->has_ml_radar_cp() && getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link]has_ml_radar_cp" ;
        }
        if (m_MonitorStateCcindexRcpLink){
            handleMonitorLinkUpdate(*m_MonitorStateCcindexRcpLink,
                mecDeviceData->ml_radar_cp().tag(), mecDeviceData->ml_radar_cp().con_flag(), mecDeviceData->ml_radar_cp().timestamp());
        }
    }
}

//mec自检：处理数据更新逻辑
void OM_COMPONENT::handleMonitorDataUpdate(MonitorState &state, int tag, uint64_t timestamp)
{
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][tag]" << tag << "[state]" << state.to_string();
    }
    std::lock_guard<std::mutex> lock(state.stateMutex);
    if (tag != state.tag)
    {
        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data]m_MonitorRulesData.find(tag) == m_MonitorRulesData.end()" ;
        return;
    }

    if (!state.enable)
    {
        return;
    }
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();

    // 1. 喂狗：更新最后收到时间
    state.lastReceivedTime = now;
    state.isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][lastReceivedTime]" << state.lastReceivedTime;
    }
    // 2. 检查恢复逻辑
    if (state.isAlarmActive) {
        if (state.recoveryStartTime == 0)
        {
            // 收到告警后的第一帧数据，开始恢复计时
            state.recoveryStartTime = now;
            if(getConfiger().enableDebugPrint)
            {
                OM_CHECK_DATA_WARN_PRINT << "[om_check_data][recovery-start] tag:" << tag;
            }
        }
        else
        {
            // 检查数据是否持续了足够长的时间
            if (now - state.recoveryStartTime >= state.recoveryMs)
            {
                // 满足恢复条件 -> 消除告警
                if (tag == MONITOR_TAG_DATA_SPAT_SRC)
                {
                    m_SpatDataStatusFromTLDeviceFlag.store(true, std::memory_order_relaxed);
                }
                alarmDisappeared(state.name, state.alarmType, getConfiger().rscuEsn, state.name, state.alarmOccurredTime);
                state.isAlarmActive = false;
                state.recoveryStartTime = 0;
                state.lastAlarmPublishTime = 0;  // 重置告警上报时间
                state.continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data] RECOVERED: " << state.name;
                }
            }
        }
    }
}

//点云数据：
void OM_COMPONENT::handleMonitorDataUpdateRcpData(const std::string & deviceId, int tag, uint64_t timestamp)
{
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data_ccindex_tm][tag]" << tag << "[device_id]" << deviceId;
    }
    for (auto& pair : m_MonitorStateCcindexRcpData)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][id]" << pair.first ;
    }
    // 使用 find 查找
    auto it = m_MonitorStateCcindexRcpData.find(deviceId);
    // 判断是否找到
    if (it == m_MonitorStateCcindexRcpData.end())
    {
        OM_CHECK_DATA_ERROR_PRINT << "[om_check_data][ccindex_tm]m_MonitorRulesCcindexTm.find(deviceid) == m_MonitorRulesCcindexTm.end()";
        return;
    }

    auto statePtr = it->second;
    if (statePtr)
    {
        // 【新增】加锁
        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
        if (! statePtr->enable)
        {
            return;
        }

        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();

        // 1. 喂狗：更新最后收到时间
        statePtr->lastReceivedTime = now;
        statePtr->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]"<< "[lastReceivedTime]" << statePtr->lastReceivedTime;
        }
        // 2. 检查恢复逻辑
        if (statePtr->isAlarmActive) {
            if (statePtr->recoveryStartTime == 0)
            {
                // 收到告警后的第一帧数据，开始恢复计时
                statePtr->recoveryStartTime = now;
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag ;
                }
            }
            else
            {
                // 检查数据是否持续了足够长的时间
                if (now - statePtr->recoveryStartTime >=  statePtr->recoveryMs)
                {
                    // 满足恢复条件 -> 消除告警
                    std::string errorDetail = statePtr->name + " [deviceId]" + deviceId ;
                    alarmDisappeared(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail, statePtr->alarmOccurredTime);
                    statePtr->isAlarmActive = false;
                    statePtr->recoveryStartTime = 0;
                    statePtr->lastAlarmPublishTime = 0;  // 重置告警上报时间
                    statePtr->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << statePtr->name << " [deviceId: " << deviceId << "]";
                    }
                }
            }
        }
    }

}

//////////////////////////
//
//雷达信控指标-动态
void OM_COMPONENT::handleMonitorDataUpdateCcindexTm(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp)
{
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][tag]" << tag << "[topic]" << topic << "[device_id]" << deviceId;
    }
    for (auto& pair : m_MonitorStatesCcindexTmData)
    {

        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][id]" << pair.first ;
    }
    MonitorStatePtr targetState = nullptr;
   // --- 第一步：查找设备并获取上下文 ---
    {
        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
        auto it = m_MonitorStatesCcindexTmData.find(deviceId);
        if (it != m_MonitorStatesCcindexTmData.end())
        {
            auto deviceCtx = it->second;
            // 查找配置以进行 topic 匹配 (如果配置Map也是线程安全的或只读的)
            // 注意：m_MecSelfCheckTmDeviceIdTopicMap 应该在初始化后只读，所以这里直接访问是安全的
            auto itConfig = m_MecSelfCheckTmDeviceIdTopicMap.find(deviceId);
            if (itConfig != m_MecSelfCheckTmDeviceIdTopicMap.end())
            {
                // 将字符串转为索引
                int index = getCcindexTmTopicIndex(topic, itConfig->second);
                if (index >= 0 && index < TM_TOPIC_COUNT)
                {
                    targetState = deviceCtx->states[index];
                }
            }
        }
        else
        {
             // 没找到设备，打印一次即可，防止刷屏
             // OM_CHECK_DATA_ERROR_PRINT << ...
        }
    }

    // --- 第二步：更新状态 (持有对象锁) ---
    if (targetState)
    {
        std::lock_guard<std::mutex> lock(targetState->stateMutex);
        if (! targetState->enable) return;

        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();

        // 1. 喂狗
        targetState->lastReceivedTime = now;
        targetState->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm][topic]" << topic << "[lastReceivedTime]" << targetState->lastReceivedTime;
        }

        // 2. 检查恢复逻辑
        if (targetState->isAlarmActive) {
            if (targetState->recoveryStartTime == 0)
            {
                targetState->recoveryStartTime = now;
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm][recovery-start] tag:" << tag << "[topic]" << topic;
                }
            }
            else
            {
                if (now - targetState->recoveryStartTime >= targetState->recoveryMs)
                {
                    std::string errorDetail = targetState->name + " [topic: " + topic + "]";
                    alarmDisappeared(errorDetail, targetState->alarmType, getConfiger().rscuEsn, topic, targetState->alarmOccurredTime);
                    targetState->isAlarmActive = false;
                    targetState->recoveryStartTime = 0;
                    targetState->lastAlarmPublishTime = 0;
                    targetState->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
                    if(getConfiger().enableDebugPrint) {
                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_tm] RECOVERED: " << targetState->name << " [topic: " << topic << "]";
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////
//【数据更新】雷达信控指标-静态
void OM_COMPONENT::handleMonitorDataUpdateCcindexSt(const std::string & deviceId, const std::string& topic, int tag, uint64_t timestamp)
{
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][device_id]" << deviceId  << "[tag]" << tag << "[topic]" << topic;
    }

    MonitorStatePtr targetState = nullptr;
    // --- 第一步：查找设备并获取上下文 ---
    {
        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
        auto it = m_MonitorStatesCcindexStData.find(deviceId);
        // 查找配置以进行 topic 匹配 (如果配置Map也是线程安全的或只读的)
        // 注意：m_MecSelfCheckStDeviceIdTopicMap 应该在初始化后只读，所以这里直接访问是安全的
        if (it != m_MonitorStatesCcindexStData.end())
        {
            auto deviceCtx = it->second;
            auto itConfig = m_MecSelfCheckStDeviceIdTopicMap.find(deviceId);
            if (itConfig != m_MecSelfCheckStDeviceIdTopicMap.end())
            {
                // 将字符串转为索引
                int index = getCcindexStTopicIndex(topic, itConfig->second);
                if (index >= 0 && index < ST_TOPIC_COUNT)
                {
                    targetState = deviceCtx->states[index];
                }
            }
        }
    }

    // --- 第二步：更新状态 (持有对象锁) ---
    if (targetState)
    {
        std::lock_guard<std::mutex> lock(targetState->stateMutex);
        if (! targetState->enable) return;

        uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
        // 1. 喂狗
        targetState->lastReceivedTime = now;
        targetState->isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st][topic]" << topic << "[lastReceivedTime]" << targetState->lastReceivedTime;
        }
        // 2. 检查恢复逻辑
        if (targetState->isAlarmActive)
        {
            if (targetState->recoveryStartTime == 0)
            {
                targetState->recoveryStartTime = now;
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_st][recovery-start] tag:" << tag ;
                }
            }
            else
            {
                if (now - targetState->recoveryStartTime >= targetState->recoveryMs)
                {
                    std::string errorDetail = targetState->name ;
                    alarmDisappeared(errorDetail, targetState->alarmType, getConfiger().rscuEsn, topic, targetState->alarmOccurredTime);

                    targetState->isAlarmActive = false;
                    targetState->recoveryStartTime = 0;
                    targetState->lastAlarmPublishTime = 0;
                    targetState->continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DATA_WARN_PRINT << "[om_check_data][ccindex_st] RECOVERED: " << targetState->name << " [deviceId: " << deviceId << "]";
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//MEC自检结果 ：监控链路状态
void OM_COMPONENT::handleMonitorLinkUpdate(MonitorState &state, int tag, bool status, uint64_t timestamp)
{
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_DEBUG_PRINT << "[om_check_link][tag]" << tag << "[state]" << state.to_string();
    }
    // 【新增】细粒度锁
    std::lock_guard<std::mutex> lock(state.stateMutex);
    if (tag != state.tag)
    {
        OM_CHECK_DATA_ERROR_PRINT << "[om_check_link]m_MonitorRulesData.find(tag) == m_MonitorRulesData.end()" ;
        return;
    }

    if (!state.enable)
    {
        return;
    }
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
    // 1. 喂狗：只要收到消息，说明模块在线，更新心跳时间
    state.lastReceivedTime = now;
    state.isFirstCheck = false; // 【新增】收到数据后，不再是首次检测
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_LINK_DEBUG_PRINT << "[om_check_link][lastReceivedTime]" << state.lastReceivedTime;
    }
    // 2. 检查链路内容状态
    // 如果之前是告警状态 (可能是因为超时导致的，也可能是因为status=false导致的)
    // 或者是正常状态但收到了 false

    if (status == false)
    {
        // 链路显式报错
        if (!state.isAlarmActive)
        {
            state.isAlarmActive = true;
            state.alarmOccurredTime = now;
            state.lastAlarmPublishTime = now;  // 记录首次告警上 报时间
            alarmOccurred(state.name + " [link down]", state.alarmType, getConfiger().rscuEsn, state.name,
                "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime );

        }
        // 如果已经在告警，检查是否需要重复上报
        else
        {
            // 检查是否需要重复上报告警
            if (state.lastAlarmPublishTime > 0)
            {
                uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime) ;
                if (getConfiger().enableDebugPrint)
                {
                    OM_CHECK_LINK_WARN_PRINT << "[om_check_link][link-alarm-repeat][tag]" << tag
                                       << "[name]" << state.name
                                       << "[timeDiff(ms)]" << alarmPublishTimeDiff
                                       << "[interval(ms)]" << state.timeoutMs;
                }

                // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
                if (alarmPublishTimeDiff >= state.timeoutMs)
                {
                    state.lastAlarmPublishTime = now;  // 更新上次上报时间
                    alarmOccurred(state.name + " [link down]", state.alarmType, getConfiger().rscuEsn, state.name,
                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_LINK_WARN_PRINT << "[om_check_link] LINK ALARM REPEATED: " << state.name
                                          << " (repeat interval: "
                                          << state.timeoutMs << "ms)";
                    }
                }
            }
        }
    }
    else // status == true
    {
        // 链路正常
        if (state.isAlarmActive) {
            // 链路恢复通常不需要持续时间，收到 true 即认为恢复
            // 如果需要持续时间，可以在这里加类似 handleMonitorDataUpdate 的逻辑
            alarmDisappeared(state.name, state.alarmType, getConfiger().rscuEsn, state.name, state.alarmOccurredTime);
            state.isAlarmActive = false;
            state.recoveryStartTime = 0;
            state.lastAlarmPublishTime = 0;  // 重置告警上报时间
            state.continuousTimeoutCount = 0; // 【新增】告警消失后，计数恢复为0
            if(getConfiger().enableDebugPrint)
            {
                OM_CHECK_LINK_SUCCESS_PRINT << "[om_check_link] LINK RECOVERED: " << state.name;
            }
        }
    }

    state.lastLinkStatus = status;
}

//MEC自检结果：初始化监控规则
void OM_COMPONENT::initMonitorRules()
{
    // 1. 信号机原始数据
    if(getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable)
    {
        m_MonitorStateSpatSrcData = std::make_shared<MonitorState>(
            MONITOR_TAG_DATA_SPAT_SRC,
            "[data ] spat src ",
            MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS,
            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.recoverySecond * m_Second2MSUnit,
            false,
            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable,
            OmFaultType::OFT_SPAT_SRC_DATA,
            getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.maxTimeoutTolerance
        );
    }
    //感知数据
    if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
    {
        m_MonitorStateRsapData =std::make_shared<MonitorState>(
            MONITOR_TAG_DATA_SENSOR_OBJ,
            "[data ] sensor obj ", MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS,
            getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable,
            OmFaultType::OFT_SENSOR_OBJS_DATA,
            getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.maxTimeoutTolerance
        );
    }
    //v2x信控数据
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
    {
        m_MonitorStateCcindexTcData =std::make_shared<MonitorState>(
            MONITOR_TAG_DATA_CCINDEX_TC,
            "[data ] cc tc ", MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable,
            OmFaultType::OFT_CCINDEX_TC_DATA,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.maxTimeoutTolerance
        );
    }

    //雷达动态数据
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
    {
        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex); // 初始化也要加容器锁
        for (auto pair : m_MecSelfCheckTmDeviceIdTopicMap)
        {
            std::string id = pair.first;
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]id: " << id;
            TopicCCIndexTm topicCCIndexTm = pair.second;

            // [修改] 创建设备上下文
            auto deviceCtx = std::make_shared<CcindexTmDeviceContext>();
            m_MonitorStatesCcindexTmData[id] = deviceCtx;

            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable)
            {
                // [修改] 使用数组索引赋值
                deviceCtx->states[TM_TOPIC_TRAJECTORIES] = std::make_shared<MonitorState>(
                    MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_trajectories_tr.maxTimeoutTolerance
                );
                // 保存 topic 字符串以便日志打印
                deviceCtx->states[TM_TOPIC_TRAJECTORIES]->topic = topicCCIndexTm.Topic_Trajectories;
            }
            //实时过车数
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable) {
                deviceCtx->states[TM_TOPIC_VEHICLE_PASS] = std::make_shared<MonitorState>(
                    MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS, "[data ]cc dynamic ", MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.recoverySecond * m_Second2MSUnit,
                    false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_vehiclePass_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_VEHICLE_PASS]->topic = topicCCIndexTm.Topic_VehiclePass;
            }
            //实时排队数
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable)
            {
                deviceCtx->states[TM_TOPIC_QUEUE_UP] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_queueUp_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_QUEUE_UP]->topic = topicCCIndexTm.Topic_QueueUp;
            }
            //实时区域状态数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable)
            {
                deviceCtx->states[TM_TOPIC_AREA_STATE]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_areaState_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_AREA_STATE]->topic = topicCCIndexTm.Topic_AreaState;
            }

            //实时溢出数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable)
            {
               deviceCtx->states[TM_TOPIC_OVERFLOW] = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_overflow_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_OVERFLOW]->topic = topicCCIndexTm.Topic_Overflow;
            }
            //实时出口通道数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable)
            {
                deviceCtx->states[TM_TOPIC_OUTLANE]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_outlane_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_OUTLANE]->topic = topicCCIndexTm.Topic_Outlane;
            }

            //    定时统计数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable)
            {
               deviceCtx->states[TM_TOPIC_STATISTICS]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_statistics_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_STATISTICS]->topic = topicCCIndexTm.Topic_Statistics;
            }

            // 定时评价数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable)
            {
                deviceCtx->states[TM_TOPIC_EVALUATIONS]  =std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_evaluations_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_EVALUATIONS]->topic = topicCCIndexTm.Topic_Evaluations;
            }
            //定时行人及非机动车数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable)
            {
               deviceCtx->states[TM_TOPIC_NONMOTOR]  = std::make_shared<os::v2x::protocol::om::mec::MonitorState>
                (
                    MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR,
                    "[data ]cc dynamic ",
                    MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.recoverySecond * m_Second2MSUnit,
                    false,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.enable,
                    OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_nonmotor_tr.maxTimeoutTolerance
                );
                deviceCtx->states[TM_TOPIC_NONMOTOR]->topic = topicCCIndexTm.Topic_Nonmotor;
            }
            //设备状态数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable) {
                deviceCtx->states[TM_TOPIC_DEVICE_STATUS] = std::make_shared<MonitorState>(
                   MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS, "[data ]cc dynamic ", MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
                   getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.timeoutSecond * m_Second2MSUnit,
                   getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.recoverySecond * m_Second2MSUnit,
                   false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.enable,
                   OmFaultType::OFT_CCINDEX_TM_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_deviceStatus_tr.maxTimeoutTolerance
               );
                deviceCtx->states[TM_TOPIC_DEVICE_STATUS]->topic = topicCCIndexTm.Topic_DeviceStatus;
            }
            // //实时脉冲数据
            // m_MonitorStatesCcindexTmData[id][topicCCIndexTm.Topic_Pulse] = {
            //     "[data ]cc dynamic ",
            //     MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS,
            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.timeoutSecond * m_Second2MSUnit,
            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.recoverySecond * m_Second2MSUnit,
            //     false,
            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable,
            //     OmFaultType::OFT_CCINDEX_TM_DATA
            // };

        }
    }
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
    {
        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
        for (auto pair:m_MecSelfCheckStDeviceIdTopicMap)
        {
            std::string id = pair.first;
            TopicCCIndexSt topicCCIndexSt = pair.second;
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_st]id: " << id;

            // [修改] 创建静态数据上下文
            auto deviceCtx = std::make_shared<CcindexStDeviceContext>();
            m_MonitorStatesCcindexStData[id] = deviceCtx;

            // deviceCtx->states[ST_TOPIC_QUERY_ACK] = std::make_shared<MonitorState>(
            //     MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK,
            //     "[data ] cc static ",
            //     MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
            //     getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
            //     false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
            //     OmFaultType::OFT_CCINDEX_ST_DATA,
            //         getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.maxTimeoutTolerance
            // );
            // deviceCtx->states[ST_TOPIC_QUERY_ACK]->topic = topicCCIndexSt.Topic_Static_Query_Ack;

            deviceCtx->states[ST_TOPIC_UPDATE] = std::make_shared<MonitorState>(
                MONITOR_TAG_DATA_CCINDEX_ST_UPDATE,
                "[data ] cc static ", MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS,
                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.timeoutSecond * m_Second2MSUnit,
                getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.recoverySecond * m_Second2MSUnit,
                false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable,
                OmFaultType::OFT_CCINDEX_ST_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.maxTimeoutTolerance
            );
            deviceCtx->states[ST_TOPIC_UPDATE]->topic = topicCCIndexSt.Topic_Static_Update;
        }
    }
    //雷达点云数据
    if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
    {
        for (auto pair:m_MecSelfCheckTmDeviceIdTopicMap)
        {

            std::string id = pair.first;
            OM_CHECK_DATA_DEBUG_PRINT << "[om_check_data][ccindex_tm]id: " << id  ;
            //点云数据
            if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
            {
                m_MonitorStateCcindexRcpData[id] = std::make_shared<MonitorState>(
                    MONITOR_TAG_DATA_RADAR_DATA,
                    "[data ] radar raw ", MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS,
                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.timeoutSecond * m_Second2MSUnit,
                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.recoverySecond * m_Second2MSUnit,
                    false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable,
                    OmFaultType::OFT_RADAR_RAW_DATA,
                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.maxTimeoutTolerance
                );
            }
        }
    }

    // 2. 链路故障规则 (状态触发，状态恢复)
    //感知数据上云：链路
    if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable)
    {
        m_MonitorStateRsapLink =std::make_shared<MonitorState>(
            MONITOR_TAG_LINK_RSAP,
            "[link ] sensor cloud ", MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR,
            getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.enable,
            OmFaultType::OFT_SENSOR_CLOUD_LINK_STATUS,
                    getConfiger().configerMec.configerMecSelfCheckTime.sensor_cloud_link_tr.maxTimeoutTolerance
        );
    }
    //雷达信控指标：静态链路
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable)
    {
       m_MonitorStateCcindexStLink =std::make_shared<MonitorState>(
           MONITOR_TAG_LINK_CCINDEX_ST,
            "[link ] cc static ", MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.enable,
            OmFaultType::OFT_CCINDEX_ST_LINK_STATUS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_link_tr.maxTimeoutTolerance
        );
    }
        //动态数据：链路
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable)
    {
        m_MonitorStateCcindexTmLink =std::make_shared<MonitorState>(
            MONITOR_TAG_LINK_CCINDEX_TM,
            "[link ] cc dynamic ", MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.enable,
            OmFaultType::OFT_CCINDEX_TM_LINK_STATUS,
                    getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_link_tr.maxTimeoutTolerance
        );
    }

    //点云链路
    if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable)
    {
        m_MonitorStateCcindexRcpLink =std::make_shared<MonitorState>(
            MONITOR_TAG_LINK_DAR,
            "[link ] point cloud", MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR,
            getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.timeoutSecond * m_Second2MSUnit,
            getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.recoverySecond * m_Second2MSUnit,
            false, getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.enable,
            OmFaultType::OFT_CLOUD_POINT_LINK_STATUS,
                    getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_link_tr.maxTimeoutTolerance
            );
    }


    // 获取检查周期配置
    auto checkPeriod = getConfiger().configerMec.configerPublishPeriod.periodCheckMecStatus;
    // MEC自检结果: 信号机原始数据 - SpatSrcData
    if(getConfiger().configerMec.configerMecSelfCheckTime.spat_src_data_tr.enable)
    {
        m_TimerMonitorSpatSrcData = m_EventloopOmCheck->addTimer(
        std::bind(&OM_COMPONENT::checkMonitorSpatSrcDataTimeouts, this), checkPeriod, true);
    }
    // MEC自检结果:感知数据 - RsapData
    if(getConfiger().configerMec.configerMecSelfCheckTime.sensor_objs_data_tr.enable)
    {
        m_TimerMonitorRsapData = m_EventloopOmCheck->addTimer(
        std::bind(&OM_COMPONENT::checkMonitorRsapDataTimeouts, this), checkPeriod, true);
    }


    // MEC自检结果: V2x信控数据 - CcindexTcData
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tc_data_tr.enable)
    {
        m_TimerMonitorCcindexTcData = m_EventloopOmCheck->addTimer(
            std::bind(&OM_COMPONENT::checkMonitorCcindexTcDataTimeouts, this), checkPeriod, true);
    }

    // MEC自检结果: 雷达动态数据 - CcindexTmData
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_tm_data_tr.enable)
    {	        m_TimerMonitorCcindexTmData = m_EventloopOmCheck->addTimer(
            std::bind(&OM_COMPONENT::checkMonitorCcindexTmDataTimeouts, this), checkPeriod, true);
    }

        // MEC自检结果: 雷达静态数据 - CcindexStData
    if(getConfiger().configerMec.configerMecSelfCheckTime.ccindex_st_data_tr.enable)
    {
        m_TimerMonitorCcindexStData = m_EventloopOmCheck->addTimer(
        std::bind(&OM_COMPONENT::checkMonitorCcindexStDataTimeouts, this), checkPeriod, true);
    }

    // MEC自检结果: 雷达点云数据 - RcpData
    if(getConfiger().configerMec.configerMecSelfCheckTime.cloud_point_data_tr.enable)
    {
        m_TimerMonitorCcindexRcpData = m_EventloopOmCheck->addTimer(
        std::bind(&OM_COMPONENT::checkMonitorRcpDataTimeouts, this), checkPeriod, true);
    }

}

// MEC自检结果： 监控是否超时 - SpatSrcData
void OM_COMPONENT::checkMonitorSpatSrcDataTimeouts()
{
    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateSpatSrcData, MONITOR_TAG_DATA_SPAT_SRC);
}

// MEC自检结果： 监控是否超时 - RsapData
void OM_COMPONENT::checkMonitorRsapDataTimeouts()
{
    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateRsapData, MONITOR_TAG_DATA_SENSOR_OBJ);
}


// MEC自检结果： 监控是否超时 - CcindexTcData
void OM_COMPONENT::checkMonitorCcindexTcDataTimeouts()
{
    checkMonitorMecDataTimeoutsImpl(*m_MonitorStateCcindexTcData, MONITOR_TAG_DATA_CCINDEX_TC);
}

// MEC自检结果： 监控是否超时 - CcindexRcp
void OM_COMPONENT::checkMonitorRcpDataTimeouts()
{
    if (getConfiger().enableDebugPrint)
    {
        OM_CHECK_DATA_ERROR_PRINT << "[check]start Rcp Data Timer!" ;
    }

    checkMonitorRcpDataTimeoutsImpl();
}

//MEC自检结果： 监控数据是否超时
void OM_COMPONENT::checkMonitorMecDataTimeoutsImpl(MonitorState& state, int tag)
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    // 【新增】加锁
    std::lock_guard<std::mutex> lock(state.stateMutex);
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
    /////////////////////////////////////////////////////////////////////////////////////////
    // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
    if (state.isFirstCheck)
    {
        state.isFirstCheck = false;
        state.lastReceivedTime = now;
        return;
    }
    uint64_t timeDiff = now - state.lastReceivedTime;
    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DEBUG_PRINT << "[om-check]" << state.to_string();
    }
    // 【新增】0. 数据正常未超时：重置连续超时计数器
    if ((timeDiff + 1000) < state.timeoutMs)
    {
        state.continuousTimeoutCount = 0;
        // 注意：这里不要直接 return，因为可能还需要处理告警恢复的逻辑（如果你的恢复逻辑写在函数后面的话）
    }
    // --- 1. 检查超时 (故障触发) ---
   else  if ((timeDiff+1000) >= state.timeoutMs)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_WARN_PRINT << "[om-check] timeDiff > rule.timeoutMs" ;
        }
        if (!state.isAlarmActive)
        {
            // 【新增】累加连续超时次数
            state.continuousTimeoutCount++;
            // 【新增】判断是否达到容忍上限（防抖）
            if (state.continuousTimeoutCount >= state.maxTimeoutTolerance)
            {
                std::string errorDetail = state.name + " loss";
                state.isAlarmActive = true;
                state.alarmOccurredTime = now;
                state.lastAlarmPublishTime = now;  // 记录首次告警上报时间
                state.recoveryStartTime = 0;
                if (tag == MONITOR_TAG_DATA_SPAT_SRC)
                {
                    //没有信号机原始数据
                    m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
                }
                //产生告警
                alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
                    "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);

                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DEBUG_PRINT <<" [---->][om-check] !state.isAlarmActive [errorDetail[errorDetail]"<< errorDetail
                    << " [timeDiff]" << timeDiff
                    << "[timeout]" << state.timeoutMs
                    << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
                     << "[alarmOccurredTime]" << state.alarmOccurredTime;
                }
            }
            else
            {
                // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DEBUG_PRINT <<" [---->][om-check] Timeout detected but within tolerance. Count: "
                                         << state.continuousTimeoutCount << "/" << state.maxTimeoutTolerance;
                }
            }
        }
        // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_CHECK_DEBUG_PRINT << "[====>][om-check]  state.isAlarmActive " ;
            }
            // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
            // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
            if (!state.isLinkCheck && state.recoveryStartTime != 0)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_WARN_PRINT << "[====>][om-check]  recovery reset for " << state.name
                                       << " (data stopped again for " << timeDiff << "ms)";
                }
                state.recoveryStartTime = 0;
            }

            // 检查是否需要重复上报告警
            // 如果上次上报时间为0，说明是首次告警，已经在上面的分支处理了
            if (state.lastAlarmPublishTime > 0)
            {
                //推送时间间隔
                uint64_t alarmPublishTimeDiff = (now - state.lastAlarmPublishTime) ;
                if (getConfiger().enableDebugPrint)
                {
                    OM_CHECK_WARN_PRINT << "[====>][om-check] alarm-repeat [tag]" << tag
                                       << "[name]" << state.name
                                       << "[timeDiff(ms)]" << alarmPublishTimeDiff
                                       << "[interval(ms)]" << state.timeoutMs
                                        << "[lastAlarmPublishTime]" << state.lastAlarmPublishTime
                                        << "[alarmOccurredTime]" << state.alarmOccurredTime;
                }

                // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
                if (alarmPublishTimeDiff >= state.timeoutMs)
                {
                    std::string errorDetail =  state.name + " loss";
                    state.lastAlarmPublishTime = now;  // 更新上次上报时间
                    if (tag == MONITOR_TAG_DATA_SPAT_SRC)
                    {
                        //没有信号机原始数据
                        m_SpatDataStatusFromTLDeviceFlag.store(false, std::memory_order_relaxed);
                    }
                    alarmOccurred(errorDetail, state.alarmType, getConfiger().rscuEsn, state.name,
                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, state.alarmOccurredTime);

                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check] alarm repeated [errorDetail]" << errorDetail
                        << "[timeout]" << timeDiff
                        << "[interval]" << state.timeoutMs;
                    }
                }
            }
        }
    }
}


// MEC自检结果： 监控是否超时 - RoadCloudPointData
void OM_COMPONENT::checkMonitorRcpDataTimeoutsImpl()
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }

    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
    /////////////////////////////////////////////////////////////////////////////////////////
    //检查多雷达动态数据的超时
    for (auto& statePair : m_MonitorStateCcindexRcpData)
    {
        std::string deviceId = statePair.first;
        auto statePtr = statePair.second;
        // 【新增】加锁
        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
        if (!statePtr->enable)
        {
            continue;
        }
        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
        if (statePtr->isFirstCheck)
        {
            statePtr->isFirstCheck = false;
            statePtr->lastReceivedTime = now;
            continue;
        }
        uint64_t timeDiff = now - statePtr->lastReceivedTime;
        if(getConfiger().enableDebugPrint)
        {
            OM_CHECK_DEBUG_PRINT << "[om-check][rcp]" << statePtr->to_string();
        }
        // 【新增】0. 数据正常未超时：重置连续超时计数器
        if ((timeDiff + 1000) < statePtr->timeoutMs)
        {
            statePtr->continuousTimeoutCount = 0;
        }
        // --- 1. 检查超时 (故障触发) ---
        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
        {
            if (getConfiger().enableDebugPrint)
            {
                OM_CHECK_WARN_PRINT << "[om-check][rcp] timeDiff >= rule.timeoutMs" ;
            }
            // 【新增】累加连续超时次数
            statePtr->continuousTimeoutCount++;
            // 【新增】判断是否达到容忍上限（防抖）
            if (!statePtr->isAlarmActive)
            {
                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                {
                    std::string errorDetail = statePtr->name + "[deviceid]" + deviceId;
                    statePtr->isAlarmActive = true;
                    statePtr->alarmOccurredTime = now;
                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
                    statePtr->recoveryStartTime = 0;
                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail,
                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][rcp] !statePtr->isAlarmActive [errorDetail [errorDetail]"<< errorDetail
                        << " [timeDiff]" << timeDiff
                        << "[timeout]" << statePtr->timeoutMs
                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                         << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }
                }
                else
                {
                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][rcp] Timeout detected but within tolerance. Count: "
                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
                    }
                }
            }
            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][rcp]  statePtr->isAlarmActive " ;
                }
                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check]  recovery reset for " << statePtr->name
                                           << " (data stopped again for " << timeDiff << "ms)";
                    }
                    statePtr->recoveryStartTime = 0;
                }

                // 检查是否需要重复上报告警
                if (statePtr->lastAlarmPublishTime > 0)
                {
                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check][rcp] alarm-repeat "
                                           << "[name]" << statePtr->name
                                           << "[timeDiff(ms)]" << alarmPublishTimeDiff
                                           << "[interval(ms)]" << statePtr->timeoutMs
                                            << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                                            << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }

                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                    {
                        std::string errorDetail = statePtr->name + "[deviceid]" +  deviceId;
                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, errorDetail,
                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);

                        if (getConfiger().enableDebugPrint)
                        {
                            OM_CHECK_WARN_PRINT << "[====>][om-check][rcp] alarm repeated [errorDetail]" << errorDetail
                            << "[timeout]" << timeDiff
                            << "[interval]" << statePtr->timeoutMs;
                        }
                    }
                }
            }
        }
    }
}


// MEC自检结果： 监控是否超时 - CcindexTmData
void OM_COMPONENT::checkMonitorCcindexTmDataTimeouts()
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    // 临时容器，用于存放待检查的指针 (快照)
    std::vector<MonitorStatePtr> statesToCheck;

    // --- 第一步：获取快照 ---
    {
        std::lock_guard<std::mutex> mapLock(m_CcindexTmMapMutex);
        // 遍历所有设备
        for (auto& devicePair : m_MonitorStatesCcindexTmData) {
            auto& deviceCtx = devicePair.second;
            if (deviceCtx) {
                // 遍历设备下的所有 Topic (数组遍历，极快)
                for (auto& statePtr : deviceCtx->states) {
                    if (statePtr) {
                        statesToCheck.push_back(statePtr);
                    }
                }
            }
        }
    }

    // --- 第二步：遍历快照进行检查 ---
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();

    for (auto& statePtr : statesToCheck)
    {
        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
        if (!statePtr->enable) continue;
        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
        if (statePtr->isFirstCheck)
        {
            statePtr->isFirstCheck = false;
            statePtr->lastReceivedTime = now;
            continue;
        }
        uint64_t timeDiff = now - statePtr->lastReceivedTime;
        if (getConfiger().enableDebugPrint)
        {
            OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
                                << "[state]" << statePtr->to_string();
        }

        // 【新增】0. 数据正常未超时：重置连续超时计数器
        if ((timeDiff + 1000) < statePtr->timeoutMs)
        {
            statePtr->continuousTimeoutCount = 0;
        }
        // --- 1. 检查超时 (故障触发) ---
        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
        {
            if (getConfiger().enableDebugPrint)
            {
                OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
            }
            if (!statePtr->isAlarmActive)
            {
                // 【新增】累加连续超时次数
                statePtr->continuousTimeoutCount++;
                // 【新增】判断是否达到容忍上限（防抖）
                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                {
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[om-check][ccindex-tm] !statePtr->isAlarmActive for topic: " << statePtr->topic;
                    }
                    std::string errorDetail = statePtr->name + "[data-loss]" + " [topic] " + statePtr->topic;
                    statePtr->isAlarmActive = true;
                    statePtr->alarmOccurredTime = now;
                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
                    statePtr->recoveryStartTime = 0;
                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);

                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-tm] !statePtr->isAlarmActive [errorDetail [errorDetail]"<< errorDetail
                        << " [timeDiff]" << timeDiff
                        << "[timeout]" << statePtr->timeoutMs
                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }
                }
                else
                {
                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-tm] Timeout detected but within tolerance. Count: "
                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
                    }
                }
            }
            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-tm] statePtr->isAlarmActive " ;
                }
                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
                                        << " (data stopped again for " << timeDiff << "ms)";
                    }
                    statePtr->recoveryStartTime = 0;
                }

                // 检查是否需要重复上报告警
                if (statePtr->lastAlarmPublishTime > 0)
                {
                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm-repeat "
                                        << "[name]" << statePtr->name
                                        << "[timeDiff(ms)]" << alarmPublishTimeDiff
                                        << "[interval(ms)]" << statePtr->timeoutMs
                                            << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                                            << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }

                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                    {
                        std::string errorDetail = statePtr->name + " [topic: " + statePtr->topic + "] [no data]";
                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
                            "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);

                        if (getConfiger().enableDebugPrint)
                        {
                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] alarm repeated [errorDetail]" << errorDetail
                            << "[timeout]" << timeDiff
                            << "[interval]" << statePtr->timeoutMs;
                        }
                    }
                }
            }
        }
    }
}

// MEC自检结果： 监控是否超时 - CcindexStData
void OM_COMPONENT::checkMonitorCcindexStDataTimeouts()
{
    if (!m_MqttClientCloud || !m_RegisterFlag)
    {
        return;
    }
    std::vector<MonitorStatePtr> statesToCheck;

    {
        std::lock_guard<std::mutex> mapLock(m_CcindexStMapMutex);
        for (auto& devicePair : m_MonitorStatesCcindexStData) {
            auto& deviceCtx = devicePair.second;
            if (deviceCtx) {
                for (auto& statePtr : deviceCtx->states) {
                    if (statePtr) {
                        statesToCheck.push_back(statePtr);
                    }
                }
            }
        }
    }

    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
   /////////////////////////////////////////////////////////////////////////////////////////
    // 遍历每个雷达的每个topic
    for (auto& statePtr : statesToCheck)
    {
        std::lock_guard<std::mutex> lock(statePtr->stateMutex);
        if (!statePtr->enable) continue;
        // 【新增】如果是启动后的第一次有效检测，跳过并重置起始时间
        if (statePtr->isFirstCheck)
        {
            statePtr->isFirstCheck = false;
            statePtr->lastReceivedTime = now;
            continue;
        }
        uint64_t timeDiff = now - statePtr->lastReceivedTime;
        if (getConfiger().enableDebugPrint)
        {
            OM_CHECK_ERROR_PRINT << "[om-check][ccindex-st]" << "[timeDiff]" << timeDiff  << "[now]" << now  << "[topic]" << statePtr->topic;
            OM_CHECK_WARN_PRINT << "[om-check][ccindex-st]"<< "[timeDiff]" << timeDiff << "[now]" << now  << "[topic]" << statePtr->topic
                                << "[state]" << statePtr->to_string();
        }
        // 【新增】0. 数据正常未超时：重置连续超时计数器
        if ((timeDiff + 1000) < statePtr->timeoutMs)
        {
            statePtr->continuousTimeoutCount = 0;
        }
        // --- 1. 检查超时 (故障触发) ---
        else if ((timeDiff + 1000) >= statePtr->timeoutMs)
        {
            if (getConfiger().enableDebugPrint)
            {
                OM_CHECK_WARN_PRINT << "[om-check][ccindex-st] timeDiff >= rule.timeoutMs for topic: " << statePtr->topic;
            }
            if (!statePtr->isAlarmActive)
            {
                // 【新增】累加连续超时次数
                statePtr->continuousTimeoutCount++;
                // 【新增】判断是否达到容忍上限（防抖）
                if (statePtr->continuousTimeoutCount >= statePtr->maxTimeoutTolerance)
                {
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[---->][om-check][ccindex-st] !statePtr->isAlarmActive for topic: " << statePtr->topic;
                    }
                    std::string errorDetail = statePtr->name + "[data-loss]" + " [topic] " + statePtr->topic;
                    statePtr->isAlarmActive = true;
                    statePtr->alarmOccurredTime = now;
                    statePtr->lastAlarmPublishTime = now;  // 记录首次告警上报时间
                    statePtr->recoveryStartTime = 0;
                    alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
                    "mec-check", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);

                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-st] !statePtr->isAlarmActive  [errorDetail]"<< errorDetail
                        << " [timeDiff]" << timeDiff
                        << "[timeout]" << statePtr->timeoutMs
                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }
                }
                else
                {
                    // 【新增】未达到阈值，仅打印 Debug 日志，不触发告警
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_DEBUG_PRINT <<" [---->][om-check][ccindex-st] Timeout detected but within tolerance. Count: "
                                             << statePtr->continuousTimeoutCount << "/" << statePtr->maxTimeoutTolerance;
                    }
                }
            }
            // B. 如果已经在告警状态 -> 检查是否中断了恢复过程，并检查是否需要重复上报
            else
            {
               if(getConfiger().enableDebugPrint)
                {
                    OM_CHECK_DEBUG_PRINT << "[====>][om-check][ccindex-st] statePtr->isAlarmActive " ;
                }
                // 如果已经在告警，但数据又断了（超时），说明之前的恢复尝试失败
                // 必须重置 recoveryStartTime，否则可能导致断断续续的数据也被误判为恢复
                if (!statePtr->isLinkCheck && statePtr->recoveryStartTime != 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-tm] recovery reset for " << statePtr->name
                                        << " (data stopped again for " << timeDiff << "ms)";
                    }
                    statePtr->recoveryStartTime = 0;
                }

                // 检查是否需要重复上报告警
                if (statePtr->lastAlarmPublishTime > 0)
                {
                    uint64_t alarmPublishTimeDiff = (now - statePtr->lastAlarmPublishTime);
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm-repeat "
                                        << "[name]" << statePtr->name
                                        << "[timeDiff(ms)]" << alarmPublishTimeDiff
                                        << "[interval(ms)]" << statePtr->timeoutMs
                                        << "[lastAlarmPublishTime]" << statePtr->lastAlarmPublishTime
                                        << "[alarmOccurredTime]" << statePtr->alarmOccurredTime;
                    }

                    // 如果距离上次上报时间已经超过配置的重复上报间隔，则重复上报
                    if (alarmPublishTimeDiff >= statePtr->timeoutMs)
                    {
                        std::string errorDetail = statePtr->name +  "[data-loss]" + " [topic] " + statePtr->topic;
                        statePtr->lastAlarmPublishTime = now;  // 更新上次上报时间
                        alarmOccurred(errorDetail, statePtr->alarmType, getConfiger().rscuEsn, statePtr->topic,
                        "mec-alarm", TABLE_TYEP_MEC_DEV_ALARM, statePtr->alarmOccurredTime);

                        if (getConfiger().enableDebugPrint)
                        {
                            OM_CHECK_WARN_PRINT << "[====>][om-check][ccindex-st] alarm repeated [errorDetail]" << errorDetail
                            << "[timeout]" << timeDiff
                            << "[interval]" << statePtr->timeoutMs;
                        }
                    }
                }
            }
        }
    }
}

//MEC自检结果：错误码映射
int OM_COMPONENT::getSelfCheckFaultType(MecAlarmTypeErrorCodeEnum alarmType)
{
    switch(alarmType) {
        // 1: 信号机原始数据
    case MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS:
        return 1;
        // 2: 感知数据 (包含感知数据异常 和 云控链路异常)
    case MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS:
        return 2;
        // 3:  事件
    case MEC_ALARM_TYPE_MONITOR_SENSOR_EVENT_DATA_LOSS:
        return 3;
        // 4: v2x信控指标
    case MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS:
        return 4;

        // 5: 雷达信控指标-动态 (包含数据和链路)
    case MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS:
        return 5;
        // 6: 雷达信控指标-静态 (包含数据和链路)
    case MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS:
        return 6;
        // 7: 毫米波雷达原始数据
    case MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS:
        return 7;
        // 8: 激光雷达原始数据 (包含数据和链路)
    case MEC_ALARM_TYPE_MONITOR_LIDAR_RAW_DATA_LOSS:
        return 8;
        //9: 云控链路状态
    case MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR:
        return 9;
        //10：信控链路状态-静态
    case MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR:
        return 10;
        //11：信控链路状态-动态
    case MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR:
        return 11;
    case MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR:
        return 12;
    default:
        return 0; // 0 表示非自检类告警，不上报
    }
}

/**
 * 通用字符串提取拼接函数
 * @param path 原始字符串
 * @param indices 需要提取的索引列表
 * @param delimiter 分隔符，默认为 '/'
 * @return 拼接后的新字符串
 */
std::string OM_COMPONENT::extractAndJoin(const std::string& path, const std::vector<int>& indices, char delimiter)
{
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string item;

    // 1. 按分隔符拆分字符串并存入 vector
    while (std::getline(ss, item, delimiter)) {
        parts.push_back(item);
    }

    std::string result = "";
    bool isFirst = true;

    // 2. 提取指定索引的元素并重新拼接
    for (int idx : indices) {
        // 边界检查，防止索引越界
        if (idx >= 0 && idx < parts.size()) {
            if (!isFirst) {
                result += delimiter; // 非首个元素前添加分隔符
            }
            result += parts[idx];
            isFirst = false;
        }
    }

    return result;
}
//MEC自检结果：发送自检消息
void OM_COMPONENT::sendMecSelfCheckMsg(MecAlarmTypeErrorCodeEnum alarmType, int faultType, int faultStatus, uint64_t startTime, uint64_t stopTime, const std::string& desc)
{
    if (faultType == 0) return; // 无效类型不发送

    MecSelfCheckResult data;
    data.timestamp = afl::util::TimeStamp::now(true).millSeconds();
    data.seqNum = afl::util::Srand::srandStr(32);
    data.rscuEsn = getConfiger().rscuEsn;
    data.faultType = (OmFaultType)faultType;
    if (faultType <= 8)
    {
        data.faultCategory = (OmFaultCategory)0;
    }
    else
    {
        data.faultCategory = (OmFaultCategory)1;

    }

    data.faultStatus = (OmFaultStatus)faultStatus;
    data.faultStartTime = startTime;
    data.faultStopTime = stopTime;
    data.faultDescription = desc;

    if(getConfiger().enableDebugPrint)
    {
        OM_CHECK_DEBUG_PRINT << "[monitor] Push msg: " << data.to_string();
    }
    std::string topicUpload = "";
#ifdef OM_SELF_CHECK
    topicUpload = m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up + std::to_string(faultType);
    if (faultType == OmFaultType::OFT_CCINDEX_TM_DATA)
    {
        std::string result = extractAndJoin(desc, targetIndices);
       topicUpload +=  "/" + result;
    }
#else
    topicUpload = m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up;
#endif
    if(m_MqttConnectedCloud && m_RegisterFlag)
    {
        mqttPushMsg2BrokerCloud(topicUpload, data, desc);
    }
}

// 将动态数据的 topic 字符串转换为数组索引
int OM_COMPONENT::getCcindexTmTopicIndex(const std::string& topic, const TopicCCIndexTm& config)
{
    // 这里进行字符串比较，虽然有开销，但比 map 查找快，且只在 handle 时调用
    if (topic == config.Topic_Trajectories) return TM_TOPIC_TRAJECTORIES;
    if (topic == config.Topic_VehiclePass) return TM_TOPIC_VEHICLE_PASS;
    if (topic == config.Topic_QueueUp) return TM_TOPIC_QUEUE_UP;
    if (topic == config.Topic_AreaState) return TM_TOPIC_AREA_STATE;
    if (topic == config.Topic_Overflow) return TM_TOPIC_OVERFLOW;
    if (topic == config.Topic_Outlane) return TM_TOPIC_OUTLANE;
    if (topic == config.Topic_Statistics) return TM_TOPIC_STATISTICS;
    if (topic == config.Topic_Evaluations) return TM_TOPIC_EVALUATIONS;
    if (topic == config.Topic_Nonmotor) return TM_TOPIC_NONMOTOR;
    if (topic == config.Topic_DeviceStatus) return TM_TOPIC_DEVICE_STATUS;
    return -1;
}

// 将静态数据的 topic 字符串转换为数组索引
int OM_COMPONENT::getCcindexStTopicIndex(const std::string& topic, const TopicCCIndexSt& config)
{
    if (topic == config.Topic_Static_Query_Ack) return ST_TOPIC_QUERY_ACK;
    if (topic == config.Topic_Static_Update) return ST_TOPIC_UPDATE;
    return -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///
//告警：摄像头角度偏移
void  OM_COMPONENT::initProcessEventOutputResult()
{
    auto reader = node_->CreateReader<airos::usecase::EventOutputResult>(
            m_MqttClientConfig.configerMec.configerAngleOffset.channel_readers,
            std::bind(&OM_COMPONENT::processProcessEventOutputResult, this, std::placeholders::_1));
}
//告警：摄像头角度偏移
void  OM_COMPONENT::processProcessEventOutputResult(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!mecDeviceData)
    {
        return ;
    }

    if (mecDeviceData->has_mec_angle_offset())
    {
        processAngleOffsetData(mecDeviceData);
    }

}
//告警：摄像头角度偏移-处理
void  OM_COMPONENT::processAngleOffsetData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if(!getConfiger().configerMec.configerEnable.Enable_AngleOffset)
    {
        return ;
    }
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!mecDeviceData)
    {
        OM_MEC_ERROR_PRINT << "Received mec-data failed!";
        return ;
    }
    if (mecDeviceData->has_mec_angle_offset())
    {
        CameraDeviceAngleOffsetLogData cameraDeviceAngleOffsetLogData;
        auto mec_angle_offset = mecDeviceData->mec_angle_offset();
        cameraDeviceAngleOffsetLogData.timeStamp = mec_angle_offset.time_stamp();
        cameraDeviceAngleOffsetLogData.rscuEsn = m_MqttClientConfig.rscuEsn;
        cameraDeviceAngleOffsetLogData.protocolVersion =     m_MqttClientConfig.configerMec.configerOmCommon.protocolVersion;
        for(int i = 0; i < mec_angle_offset.alarm_info().size(); i++)
        {
            cameraDeviceAngleOffsetLogData.seqNum = afl::util::Srand::srandStr(32);
            auto alarm_info = mec_angle_offset.alarm_info()[i];
            cameraDeviceAngleOffsetLogData.alarm.alarmLevel = (AlertLevelEnum)alarm_info.alarm_level();
            cameraDeviceAngleOffsetLogData.alarm.alarmStatus = (AlarmStatusEnum)alarm_info.alarm_status();
            cameraDeviceAngleOffsetLogData.alarm.alarmRaisedTime = alarm_info.alarm_raised_time();
            if(alarm_info.has_alarm_changed_time())
            {
                cameraDeviceAngleOffsetLogData.alarm.alarmChangedTime = alarm_info.alarm_changed_time();
            }
            cameraDeviceAngleOffsetLogData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET;
            cameraDeviceAngleOffsetLogData.alarm.addition = alarm_info.device_id();
            cameraDeviceAngleOffsetLogData.alarm.additionEmpty = false;


            afl::base::json uploadJson;
            try
            {
                uploadJson = cameraDeviceAngleOffsetLogData;
            } catch (json::exception &e)
            {
                OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
                return ;
            }

            std::string hint = "angle-offset-data";

            processAlarmSensorAngleOffset(m_MqttClientConfig.configerMec.configerTopic.Topic_AngleOffset_Alarm_Inter, uploadJson);
            std::string topicPT = m_MqttClientConfig.configerMec.configerTopic.Topic_Sensor_Angle_Offset;
            if(!topicPT.empty())
            {
                mqttPushMsg2BrokerCloud(topicPT, cameraDeviceAngleOffsetLogData, hint);
            }
        }
    }
}
//场景：信号机原始数据
void  OM_COMPONENT::initProcessSpatSrcData()
{
    auto reader = node_->CreateReader<os::v2x::device::TrafficLightBaseData>(
            m_MqttClientConfig.configerMec.configerSpatSrcData.channel_readers,
            std::bind(&OM_COMPONENT::processSpatSrcData, this, std::placeholders::_1));
}
//场景：信号机原始数据
void  OM_COMPONENT::processSpatSrcData(const std::shared_ptr<const os::v2x::device::TrafficLightBaseData>& traffic_light_data_pb)
{
    if(!getConfiger().configerMec.configerEnable.Enable_SpatSrcData)
    {
        return ;
    }
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!traffic_light_data_pb)
    {
        OM_MEC_ERROR_PRINT << "Received traffic light data failed!";
        return ;
    }
    if(getConfiger().configerMec.enableDebugPrintScenario.enableDebugPrintSpatSrcData)
    {
        OM_MEC_DEBUG_PRINT << "[notice]bs_spat_src_data <<<<<< trafficlight:[channel]"
                                       << traffic_light_data_pb->DebugString();
    }
//    OM_MEC_DEBUG_PRINT << "[notice]bs_spat_src_data <<<<<< trafficlight:[channel]" << traffic_light_data_pb->DebugString();
    std::shared_ptr<os::v2x::device::TrafficLightBaseData>
            traffic_light_device_pb = std::make_shared<os::v2x::device::TrafficLightBaseData>(*traffic_light_data_pb);
    TrafficLightSrcData trafficLightSrcData;
    trafficLightSrcData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    trafficLightSrcData.seqNum = afl::util::Srand::srandStr(32);
    trafficLightSrcData.rscuEsn = m_MqttClientConfig.rscuEsn;
    trafficLightSrcData.lightEsn = m_MqttClientConfig.rscuEsn;
    std::string msgType;
    switch (traffic_light_device_pb->mess_type())
    {
    case 0x0101: // 信号灯运行状态 257
        msgType = "A.1";
        break;
    case 0x0102: // 信号灯控制方式 258
        msgType = "A.2";
        break;
    case 0x0103: // 信号机灯色状态 259
        msgType = "A.3";
        break;
    case 0x0301: // 当前信号方案色步信息 769
        msgType = "A.6";
        break;
    case 0x0302: // 下一周期信号方案色步信息 770
        msgType = "A.7";
        break;
    default:
        msgType = "";
        break;
    }
    trafficLightSrcData.messType = msgType;
    if(traffic_light_device_pb->has_src_data())
    {
        std::string spatSrcData = traffic_light_device_pb->src_data();
        if(spatSrcData.size() > 0)
        {
            trafficLightSrcData.message =  traffic_light_device_pb->src_data();
        }
        else
        {
            OM_MEC_ERROR_PRINT << "spat src data empty!";
            return ;
        }
    }
    else
    {
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "spat src data empty!";
        }

        return ;
    }

    std::string hint = "spat-src-data";
    std::string topicPT = m_MqttClientConfig.configerMec.configerTopic.Topic_Signal;
    if(!mqttPushMsg2BrokerCloud(topicPT, trafficLightSrcData, hint))
    {
        OM_MEC_ERROR_PRINT << "[error] push spat data failure!";
    }

    return ;
}
//场景：bsm数据
void  OM_COMPONENT::initProcessV2xBsmData()
{
    auto reader = node_->CreateReader<v2xpb::asn::MessageFrame>(
             m_MqttClientConfig.configerMec.configerV2xData.channel_readers_received,
            std::bind(&OM_COMPONENT::processV2xBsmData, this, std::placeholders::_1));
}
void  OM_COMPONENT::processV2xBsmData(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame)
{
    if(!getConfiger().configerMec.configerEnable.Enable_V2xData)
    {
        return ;
    }
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!frame)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "[error]frame is nullptr";
        }
        return ;
    }
    if(getConfiger().configerMec.enableDebugPrintScenario.enableDebugPrintV2xDataReceive)
    {
        OM_MEC_DEBUG_PRINT << "[notice]bs_v2x_data <<<<<< /airos/message/received:[channel]"  << frame->DebugString();
    }
    V2xPbAsnMessageFrame2Str(frame);
}

//场景：asn
bool OM_COMPONENT::V2xPbAsnMessageFrame2Str(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame)
{
    if (!frame)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "[error]input is nullptr!";
        }
        return false;
    }

    std::string strAsn;
    string strPb;
    std::string hint = "sensor-data";
    std::string asn_json_str;
    if (!frame->SerializePartialToString(&strPb))
    {
        OM_MEC_ERROR_PRINT << "pb serialize false";
        return false;
    }
    SenarioData senarioData;
    senarioData.timeStamp =  afl::util::TimeStamp::now(true).millSeconds();
    senarioData.seqNum = afl::util::Srand::srandStr(32);
    senarioData.rscuEsn =  m_MqttClientConfig.rscuEsn;

    std::string topic;
    switch ((int)frame->payload_case())
    {
        case v2xpb::asn::MessageFrame::PayloadCase::kBsmFrame:
        {
            hint = "bsm-json";
            m_MsgType = MT_BSM;
            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Bsm;
        }
        break;
        case v2xpb::asn::MessageFrame::PayloadCase::kExtFrame:
        {
            const auto& extFrame = frame->extframe();
            const auto& messageValue = extFrame.messagevalue();
            switch (messageValue.typresent())
            {
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_TestMsg:
                    // 处理 TestMsg 消息
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_RTCMcorrections:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Rtcm)
                    {
                        // 处理 RTCM 更正消息
                        hint = "rtcm-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rtcm;
                    }
                    else
                    {
                        return false;
                    }

                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_PAMData:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Pam)
                    {
                        // 处理 PAM 数据
                        hint = "pam-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Pam;
                    }
                    else
                    {
                        return false;
                    }

                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_PersonalSafetyMessage:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Pam)
                    {
                        hint = "psm-json";
                        //                    topic = m_MqttClientConfig.configerMecBs.v2xDataComponentConfiger.topic_v2x_psm;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_RoadsideCoordination:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Rsc)
                    {
                        hint = "rsc-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsc;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_SensorSharingMsg:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Ssm)
                    {
                        hint = "ssm-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ssm;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_VehIntentionAndRequest:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Vir)
                    {
                        hint = "vir-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Vir;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_VehiclePaymentMessage:
                    //                    hint = "vpm-json";
                    //                    topic = m_MqttClientConfig.v2xDataComponentConfiger.topic_v2x_vir;
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_IntentionSharingMessage:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Ism)
                    {
                        hint = "ism-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ism;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_ServiceAnnouncementMessage:
                    if (getConfiger().configerMec.configerEnable.Enable_Scene_Sam)
                    {
                        hint = "sam-json";
                        topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Sam;
                    }
                    else
                    {
                        return false;
                    }
                    break;
                default:
                    hint = "no-json";
                    topic = "";
                    break;
            }
        }
            break;
        default:
            OM_MEC_ERROR_PRINT << "msg type not support ";
        break;
    }

    if (0 > message_frame_pbstr2uper_adapter(strPb, &strAsn, m_AsnType))
    {
        OM_MEC_ERROR_PRINT << "[error]pb to asn false!";
        return false;
    }
    std::vector<uint8_t> bytes = stringToVector(strAsn);
//    // 打印字节内容
//    for (uint8_t byte : bytes)
//    {
//        std::cout << std::hex << static_cast<int>(byte) << " ";
//    }
//    std::cout << std::endl;

    senarioData.asn1Data =  base64Encode(bytes);;
    std::string topicPT = topic;

    static int message_count_senario_in = 1; // 用于计数的静态变量
    static int MAX_MESSAGE_COUNT_SENARIO_IN = 1000; // 设置您的最大限制

    // 根据消息计数判断是偶数还是奇数
    if (message_count_senario_in % 2 == 0) // 偶数
    {
        m_EventloopCloudPubFirst->runInLoop([this, topicPT, senarioData, hint]()
      {
          if(!mqttPushMsg2BrokerCloud(topicPT, senarioData, hint))
          {
              OM_MEC_ERROR_PRINT << "[notice][error] push " << hint.c_str() << " failure!";
          }
      });
    }
    else // 奇数
    {
        m_EventloopCloudPubSecond->runInLoop([this, topicPT, senarioData, hint]()
        {
            if(!mqttPushMsg2BrokerCloud(topicPT, senarioData, hint))
            {
                OM_MEC_ERROR_PRINT << "[notice][error] push " << hint.c_str() << " failure!";
            }
        });
    }
    if (message_count_senario_in > MAX_MESSAGE_COUNT_SENARIO_IN)
    {
        message_count_senario_in = 0; // 重置计数器
    }
    else
    {
        message_count_senario_in++;
    }

    return true;
}
std::vector<uint8_t> OM_COMPONENT::stringToVector(const std::string& str)
{
    // 直接转换为 std::vector<uint8_t>
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string OM_COMPONENT::base64Encode(const std::vector<uint8_t>& input)
{
    BIO* bio;
    BIO* b64;
    BUF_MEM* bufferPtr;

    // 创建 Base64 BIO
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // 不输出换行
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    // 写入数据
    BIO_write(bio, input.data(), input.size());
    BIO_flush(bio);

    // 获取编码后的数据
    BIO_get_mem_ptr(bio, &bufferPtr);

    // 创建一个字符串来保存编码后的数据
    std::string encoded(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);  // 释放 BIO

    return encoded;
}
void  OM_COMPONENT::initProcessV2xData()
{
    auto reader = node_->CreateReader<v2xpb::asn::MessageFrame>(
            m_MqttClientConfig.configerMec.configerV2xData.channel_readers_generated,
            std::bind(&OM_COMPONENT::processV2xData, this, std::placeholders::_1));
}
std::string OM_COMPONENT::AsnTypeToString(EnAsnType type)
{
    switch (type)
    {
    case EnAsnType::CASE_53_2020: return "CASE_53_2020";
    case EnAsnType::YDT_3709_2020: return "YDT_3709_2020";
    case EnAsnType::YDT_3709_2020_EXT: return "YDT_3709_2020_EXT";
    default: return "Unknown";
    }
}
void  OM_COMPONENT::processV2xData(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame)
{
    if(!getConfiger().configerMec.configerEnable.Enable_V2xData)
    {
        return ;
    }
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!frame)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "[error]frame is nullptr";
        }
        return ;
    }
    if(getConfiger().configerMec.enableDebugPrintScenario.enableDebugPrintV2xDataBroad)
    {
        OM_MEC_DEBUG_PRINT << "[notice]bs_v2x_data <<<<<< /airos/message/received:[channel] "
                                       << frame->DebugString();
    }
    V2xPbAsnMessageFrame2StrBroad(frame);
}
bool OM_COMPONENT::V2xPbAsnMessageFrame2StrBroad(const std::shared_ptr<const v2xpb::asn::MessageFrame> &frame)
{
    if (!frame)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "[error]input is nullptr!";
        }
        return false;
    }

    std::string strAsn;
    string strPb;
    std::string hint = "sensor-data";
    std::string asn_json_str;
    if (!frame->SerializePartialToString(&strPb))
    {
        OM_MEC_DEBUG_PRINT << "pb serialize false";
        return false;
    }
    SenarioData senarioData;
    senarioData.timeStamp =  afl::util::TimeStamp::now(true).millSeconds();
    senarioData.seqNum = afl::util::Srand::srandStr(32);
    senarioData.rscuEsn =  m_MqttClientConfig.rscuEsn;

    std::string topic;
    switch ((int)frame->payload_case())
    {
        //业务数据-地图数据
        case v2xpb::asn::MessageFrame::PayloadCase::kMapFrame:
            if (getConfiger().configerMec.configerEnable.Enable_Scene_Map)
            {
                hint = "map-json";
                m_MsgType = MT_MAP;
                topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Map;
            }
            else
            {
                return false;
            }
            break;
        // 业务数据-信号灯数据
        case v2xpb::asn::MessageFrame::PayloadCase::kSpatFrame:
            if (getConfiger().configerMec.configerEnable.Enable_Scene_Spat)
            {
                hint = "spat-json";
                m_MsgType = MT_SPAT;
                topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Spat;
            }
            else
            {
                return false;
            }
            break;
            //持续性场景
            //监控性场景
        case v2xpb::asn::MessageFrame::PayloadCase::kRsiFrame:
            if (getConfiger().configerMec.configerEnable.Enable_Scene_Rsi)
            {
                hint = "rsi-json";
                auto rsiFramePb = frame->rsiframe();
                if(rsiFramePb.rtes_size() > 0 ||rsiFramePb.rtss_size() > 0 )
                {
                    topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsi;
                }
            }
            else
            {
                return false;
            }
            break;
        case v2xpb::asn::MessageFrame::PayloadCase::kRsmFrame:
            if (getConfiger().configerMec.configerEnable.Enable_Scene_Rsm)
            {
                hint = "rsm-json";
                m_MsgType = MT_RSM;
                topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsm;
            }
            else
            {
                return false;
            }
            break;
        case v2xpb::asn::MessageFrame::PayloadCase::kExtFrame:
            {
                const auto& extFrame = frame->extframe();
                const auto& messageValue = extFrame.messagevalue();
                switch (messageValue.typresent())
                {
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_TestMsg:
                        // 处理 TestMsg 消息
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_RTCMcorrections:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Rtcm)
                        {
                            // 处理 RTCM 更正消息
                            hint = "rtcm-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rtcm;
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_PAMData:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Pam)
                        {
                            // 处理 PAM 数据
                            hint = "pam-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Pam;
                        }
                        else
                        {
                            return false;
                        }

                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_PersonalSafetyMessage:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Pam)
                        {
                            hint = "psm-json";
                            //                    topic = m_MqttClientConfig.v2xDataComponentConfiger.topic_v2x_psm;

                        }
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_RoadsideCoordination:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Rsc)
                        {
                            hint = "rsc-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsc;
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_SensorSharingMsg:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Ssm)
                        {
                            hint = "ssm-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ssm;
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_VehIntentionAndRequest:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Vir)
                        {
                            hint = "vir-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Vir;
                        }
                        else
                        {
                            return false;
                        }

                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_VehiclePaymentMessage:
                        //                    hint = "vpm-json";
                        //                    topic = m_MqttClientConfig.v2xDataComponentConfiger.topic_v2x_vir;
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_IntentionSharingMessage:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Ism)
                        {
                            hint = "ism-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ism;
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    case v2xpb::asn::MessageFrameExt__value::MessageFrameExt__value_PR_ServiceAnnouncementMessage:
                        if (getConfiger().configerMec.configerEnable.Enable_Scene_Sam)
                        {
                            hint = "sam-json";
                            topic = m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Sam;
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    default:
                        hint = "no-json";
                        topic = "";
                        break;
                }
            }
            break;
        default:
            hint = "no-json";
            topic = "";
            OM_MEC_ERROR_PRINT << "msg type not support ";
            break;
    }
//    pushCommunicationDeviceScenarioReceivingData();
//    pushCommunicationDeviceScenarioTransmissionData();
    std::string asnJson;

    if (0 > message_frame_pbstr2uper_adapter(strPb, &strAsn, m_AsnType, &asnJson))
    {
        OM_MEC_ERROR_PRINT << "[error]pb to asn false!";
        return false;
    }
    std::vector<uint8_t> bytes = stringToVector(strAsn);

    std::string str_xml("");
    if (0 >= message_frame_uper2xer_adapter(strAsn, &str_xml, m_AsnType)) {
        std::cout << "message_frame_uper2xer_adapter fail" << std::endl;
        return false;
    }
     // OM_MEC_ERROR_PRINT << "【xml out】" << str_xml.c_str();
//    // 打印字节内容
//    for (uint8_t byte : bytes)
//    {
//        std::cout << std::hex << static_cast<int>(byte) << " ";
//    }
//    std::cout << std::endl;

    senarioData.asn1Data =  base64Encode(bytes);
//    OM_MEC_BS_V2X_DATA_ERROR_PRINT << "[strPb]" << strPb << "\n[strAsn]" << strAsn << "[asnJson]" << asnJson;
    // std::string topicPT = topic;
    // if (!mqttPushMsg2BrokerCloud(topicPT, senarioData, hint))
    // {
    //     OM_MEC_ERROR_PRINT << "[notice][error] push " << hint.c_str() << " failure!";
    // }
#if 1
    std::string topicPT = topic;
    static int message_count_senario_out = 1; // 用于计数的静态变量
    static int MAX_MESSAGE_COUNT_SENARIO_OUT = 1000; // 设置您的最大限制

    // 根据消息计数判断是偶数还是奇数
    if (message_count_senario_out % 2 == 0) // 偶数
    {
        m_EventloopCloudPubFirst->runInLoop([this, topicPT, senarioData, hint]()
        {
            if(!mqttPushMsg2BrokerCloud(topicPT, senarioData, hint))
            {
                OM_MEC_ERROR_PRINT << "[notice][error] push " << hint.c_str() << " failure!";
            }
        });
    }
    else // 奇数
    {
        m_EventloopCloudPubSecond->runInLoop([this, topicPT, senarioData, hint]()
         {
             if(!mqttPushMsg2BrokerCloud(topicPT, senarioData, hint))
             {
                 OM_MEC_ERROR_PRINT << "[notice][error] push " << hint.c_str() << " failure!";
             }
         });
    }
    if (message_count_senario_out > MAX_MESSAGE_COUNT_SENARIO_OUT)
    {
        message_count_senario_out = 0; // 重置计数器
    }
    else
    {
        message_count_senario_out++;
    }

#endif
    return true;
}
//监控om运维状态：路侧监控工具使用
void OM_COMPONENT::periodMonitorOmStatus()
{
    if (!getConfiger().configerMec.configerEnable.Enable_Monitor_Mec_Om_Status)
    {
        return;
    }
    if (output_monitor_)
    {
        output_monitor_->Clear();
    }
    auto* om_response =  output_monitor_->mutable_om_response();
    om_response->set_tag(airos::monitor::MonitorMsgTag::MONITOR_TAG_MONITOR_OM);
    om_response->set_seqnum(afl::util::Srand::srandStr(32));
    om_response->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    om_response->set_device_esn(m_MqttClientConfig.rscuEsn);

    auto* om_response_data = om_response->mutable_data();
    om_response_data->set_con_flag(m_MqttConnectedCloud);
    //mec运维
    auto* om_mec_status = om_response_data->mutable_om_mec_status();
    om_mec_status->set_register_flag(m_OmMecStatus.register_flag);
    om_mec_status->set_heartbeat_flag(m_OmMecStatus.heartbeat_flag);
    //radar运维
    if (getConfiger().enableDebugPrint)
    {
        OM_MEC_WARN_PRINT << "[radar-size]" << getConfiger().configerRadar.configerTopicRadarID.radarIDs.size();
    }

    for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++){
        std::string deviceEsn = "";
        std::string deviceSn = "";
        bool radarRegisterFlag = false;
        bool radarHeartbeatFlag = false;
        if (i == 0)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus1.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus1.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        else if (i == 1)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus2.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus2.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        else if (i == 2)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        else if (i == 3)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus4.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus4.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        else if (i == 4)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus5.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus5.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        else if (i == 5)
        {
            deviceEsn = m_OmRadarStatusContainer.m_OmRadarsStatus6.deviceEsn;
            deviceSn = m_OmRadarStatusContainer.m_OmRadarsStatus6.deviceSn;
            auto itRadarRegister = m_RadarRegisterMap.find(deviceEsn);
            if (itRadarRegister != m_RadarRegisterMap.end()) {
                radarRegisterFlag = itRadarRegister->second.isRegistered;
            }
            else
            {
                radarRegisterFlag = false;
            }

            auto itRadarHeartbeat = topicRadarHeartBeatMap.find(deviceEsn);

            if (itRadarHeartbeat != topicRadarHeartBeatMap.end()) {
                radarHeartbeatFlag = itRadarHeartbeat->second.second;
            }
            else
            {
                radarHeartbeatFlag = false;
            }
        }
        auto* om_radars_status = om_response_data->add_om_radars_status();
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[radar]" << i  <<  "[deviceEsn]" << deviceEsn << "[deviceSn]" << deviceSn;
        }
        om_radars_status->set_device_esn(deviceEsn);
        om_radars_status->set_device_sn(deviceSn);
        om_radars_status->set_register_flag(radarRegisterFlag);
        om_radars_status->set_heartbeat_flag(radarHeartbeatFlag);
    }
    // camera 运维
    if (getConfiger().enableDebugPrint)
    {
        OM_MEC_WARN_PRINT << "[camera-size]" << getConfiger().configerCamera.configerTopicRcId.RcIds.size();
    }
    for(uint32_t i = 0; i < getConfiger().configerCamera.configerTopicRcId.RcIds.size(); i++)
    {
        std::string deviceEsn = "";
        std::string deviceSn = "";
        bool cameraRegisterFlag = false;
        bool cameraHeartbeatFlag = false;
        if (i == 0)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus1.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus1.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
        else if (i == 1)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus2.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus2.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }else if (i == 2)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus3.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus3.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
        else if (i == 3)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus4.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus4.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
        else if (i == 4)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus5.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus5.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
         else if (i == 5)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus6.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus6.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
        else if (i == 6)
        {
            deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus7.deviceEsn;
            deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus7.deviceSn;
            auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
            if (itCameraRegister != m_CameraRegisterMap.end()) {
                cameraRegisterFlag = itCameraRegister->second.isRegistered;
            }
            else
            {
                cameraRegisterFlag = false;
            }

            auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
            if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                cameraHeartbeatFlag = itCameraHeartbeat->second.second;
            }
            else
            {
                cameraHeartbeatFlag = false;
            }
        }
         else if (i == 7) {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus8.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus8.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             } else {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             } else {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 8) {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus9.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus9.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             } else {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             } else {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 9)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus10.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus10.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 10)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus11.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus11.deviceSn;
             auto  itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 11)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus12.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus12.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }else if (i == 12)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus13.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus13.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 13)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus14.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus14.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 14)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus15.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus15.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }
         else if (i == 15)
         {
             deviceEsn = m_OmCameraStatusContainer.m_OmCameraStatus16.deviceEsn;
             deviceSn = m_OmCameraStatusContainer.m_OmCameraStatus16.deviceSn;
             auto itCameraRegister = m_CameraRegisterMap.find(deviceEsn);
             if (itCameraRegister != m_CameraRegisterMap.end()) {
                 cameraRegisterFlag = itCameraRegister->second.isRegistered;
             }
             else
             {
                 cameraRegisterFlag = false;
             }

             auto itCameraHeartbeat = topicCameraHeartBeatMap.find(deviceEsn);
             if (itCameraHeartbeat != topicCameraHeartBeatMap.end()) {
                 cameraHeartbeatFlag = itCameraHeartbeat->second.second;
             }
             else
             {
                 cameraHeartbeatFlag = false;
             }
         }
         else
         {
         }
        auto* om_camera_status = om_response_data->add_om_camera_status();
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[camera]" << i  <<  "[deviceEsn]" << deviceEsn << "[deviceSn]" << deviceSn;
        }

        om_camera_status->set_device_esn(deviceEsn);
        om_camera_status->set_device_sn(deviceSn);
        om_camera_status->set_register_flag(cameraRegisterFlag);
        om_camera_status->set_heartbeat_flag(cameraHeartbeatFlag);
    }
    if (getConfiger().enableDebugPrint)
    {
        OM_CLOUD_WARN_PRINT << "[monitor-out]" << output_monitor_->ShortDebugString();
    }
    Send("/v2x/monitor", output_monitor_);
}


bool OM_COMPONENT::periodMonitorSensorDeviceActive()
{
    std::string interface = "[active]";
    std::string hint = "Monitor-Sensor-Device-Active";
    if (getConfiger().enableDebugPrint)
    {
        OM_CLOUD_DEBUG_PRINT << "[hint]" << hint;
    }
    for(auto& v : getConfiger().configerSensorDeviceInfos)
    {
       if(v.deviceType == WorkParamDeviceTypeCamera)
       {
           bool pingFlag = false;
           if(NetUtil::ping(v.deviceIp))
           {
               if(getConfiger().enableDebugPrint)
                {
                   OM_CLOUD_WARN_PRINT << "Camera:" << v.deviceIp << " ping (sucess)!";
               }
               pingFlag = true;
           }
           else
           {
               if(getConfiger().enableDebugPrint)
                {
                   OM_CLOUD_WARN_PRINT << "Camera:" << v.deviceIp << " ping (failure)!";
               }
               pingFlag = false;
           }
           auto it = topicCameraPingMap.find(v.deviceEsn);
           if (it != topicCameraPingMap.end()) {
               // OM_CLOUD_WARN_PRINT << "find:" << v.deviceIp;
               it->second = pingFlag;
           }
           //
           // std::lock_guard<std::mutex> lock(m_MutexCameraPing);
           // {
           //
           //
           // }
       }

       if(v.deviceType == WorkParamDeviceTypeRadar)
       {
           bool pingFlag = false;
           if(NetUtil::ping(v.deviceIp))
           {
               if(getConfiger().enableDebugPrint)
                {
                   OM_CLOUD_WARN_PRINT << "radar:" << v.deviceIp << " ping (sucess)!";
               }
               pingFlag = true;
           }
           else
           {
               if(getConfiger().enableDebugPrint)
                {
                   OM_CLOUD_WARN_PRINT << "radar:" << v.deviceIp << " ping (failure)!";
               }
               pingFlag = false;
           }
           auto it = topicRadarPingMap.find(v.deviceEsn);
           if (it != topicRadarPingMap.end())
            {
               OM_CLOUD_WARN_PRINT << "find:" << v.deviceIp;
               it->second = pingFlag;
           }
       }
    }
    return true;
}
//信号机数据状态
bool OM_COMPONENT::periodMonitorSpatDataStatus()
{
    std::string hint = "Monitor-Spat-Data-Status";
    if (!m_MqttConnectedCloud||!m_RegisterFlag)
    {
        OM_CLOUD_DEBUG_PRINT << "[hint]" << hint ;
        return false;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Spat_Data_Status)
    {
        return false;
    }
    // bool spatDataStatusFromTLDeviceFlag = m_SpatDataStatusFromTLDeviceFlag.load(std::memory_order_relaxed);
    bool flag = m_SpatDataStatusFromTLDeviceFlag.load(std::memory_order_relaxed);
    if (flag)
    {
        if (getConfiger().enableDebugPrint)
        {
            OM_CLOUD_DEBUG_PRINT << "[hint]" << hint << "[flag]"<< (flag?"true":"fasle");
        }
        return false;
    }

    TrafficLightDataStatus trafficLightDataStatus;
    trafficLightDataStatus.timestamp = afl::util::TimeStamp::now(true).millSeconds();
    trafficLightDataStatus.seqNum = afl::util::Srand::srandStr(32);
    trafficLightDataStatus.rscuEsn = m_MqttClientConfig.rscuEsn;
    trafficLightDataStatus.lightEsn = m_MqttClientConfig.rscuEsn;
    if (!flag)
    {
        trafficLightDataStatus.status = 1 ;  //异常

    }
    // m_SpatDataStatusFromTLAdapterFlag.store(false, std::memory_order_relaxed);
    if(!mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Signal_Data_Status_Up, trafficLightDataStatus, hint))
    {
        OM_MEC_ERROR_PRINT << "[error] push spat data status  failure!";
    }
    return true;
}



///////////////////////////////////////////////////////////////////////////
//获取云控的的配置内容从工参配置文件中
bool OM_COMPONENT::getWorkParamFromFileCloud()
{
   OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    if (getConfiger().enableDebugPrint)
    {
        OM_CLOUD_ERROR_PRINT << "[work-param]" << omWorkParamConfiger.to_string().c_str();
    }
    bool hasCameraErased = false;
    bool hasRadarErased = false;
    for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
    {
        if(v.deviceType == WorkParamDeviceTypeCamera && !v.deviceEsn.empty())
        {
            if(!hasCameraErased)
            {
                getConfiger().configerCamera.configerTopicRcId.RcIds.clear();
                hasCameraErased = true;
            }
            getConfiger().configerCamera.configerTopicRcId.RcIds.push_back(v.deviceEsn);
        }
        if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
        {
            if(!hasRadarErased)
            {
                getConfiger().configerRadar.configerTopicRadarID.radarIDs.clear();
                hasRadarErased = true;
            }
            std::string topicPostfix = "/" +  v.vendor + "/" + v.category + "/" + v.radarCrossId + "/" + v.deviceEsn;
            getConfiger().configerRadar.configerTopicRadarID.cross_id = v.radarCrossId;
            getConfiger().configerRadar.configerTopicRadarID.radarIDs.push_back(topicPostfix);
        }
    }

    getConfiger().configerMqttCloud.mqttBrokerUrl = omWorkParamConfiger.mecDeviceWorkParam.maintenanceCloudUrl;
    getConfiger().configerMqttCloud.mqttUserName = omWorkParamConfiger.mecDeviceWorkParam.maintenanceMqttUsername;
    getConfiger().configerMqttCloud.mqttPassword = omWorkParamConfiger.mecDeviceWorkParam.maintenanceMqttPasswd;
    getConfiger().configerProjectPath.tlsCAFileName = omWorkParamConfiger.mecDeviceWorkParam.maintenanceCaCertName;
    getConfiger().configerProjectPath.tlsClientKeyFileName = omWorkParamConfiger.mecDeviceWorkParam.maintenanceClientCertName;
    getConfiger().configerProjectPath.tlsClientPrivateKeyFileName = omWorkParamConfiger.mecDeviceWorkParam.maintenanceClientPrivateKeyFile;
    getConfiger().configerProjectPath.tlsClientPrivateKeyPassword = omWorkParamConfiger.mecDeviceWorkParam.maintenanceClientPrivateKeyPwd;
    getConfiger().rscuEsn = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
//    getConfiger().enableDebugPrint = omWorkParamConfiger.hasedInit;
    saveConfiger();
}
//从工参中获取配置文件
bool OM_COMPONENT::getWorkParamFromFile()
{
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, m_OmWorkParamConfiger);
    // if(getConfiger().enableDebugPrint)
    // {
    //     OM_MEC_DEBUG_PRINT << "[work-param]" << m_OmWorkParamConfiger.to_string().c_str();
    // }
    getConfiger().rscuEsn = m_OmWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
    getConfiger().configerMec.nic = m_OmWorkParamConfiger.mecDeviceWorkParam.nic;
    getConfiger().configerMqttCloud.mqttBrokerUrl = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceCloudUrl;
    getConfiger().configerMqttCloud.mqttUserName = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceMqttUsername;
    getConfiger().configerMqttCloud.mqttPassword = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceMqttPasswd;
    getConfiger().configerProjectPath.tlsCAFileName = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceCaCertName;
    getConfiger().configerProjectPath.tlsClientKeyFileName = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceClientCertName;
    getConfiger().configerProjectPath.tlsClientPrivateKeyFileName = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceClientPrivateKeyFile;
    getConfiger().configerProjectPath.tlsClientPrivateKeyPassword = m_OmWorkParamConfiger.mecDeviceWorkParam.maintenanceClientPrivateKeyPwd;


    getConfiger().configerMec.configerHttpServerMec.ftpUrl = m_OmWorkParamConfiger.mecDeviceWorkParam.videoFtpUrl;
    getConfiger().configerMec.configerHttpServerMec.httpHost = m_OmWorkParamConfiger.mecDeviceWorkParam.videoHttpHost;
    getConfiger().configerMec.configerHttpServerMec.httpPort = m_OmWorkParamConfiger.mecDeviceWorkParam.videoHttpPort;
    getConfiger().configerMec.configerOmCommon.latitude = m_OmWorkParamConfiger.mecDeviceWorkParam.mecLatitude;
    getConfiger().configerMec.configerOmCommon.longitude = m_OmWorkParamConfiger.mecDeviceWorkParam.mecLongitude;
    getConfiger().configerMec.configerOmCommon.elevation = m_OmWorkParamConfiger.mecDeviceWorkParam.mecAltitude;
    getConfiger().configerMec.configerOmCommon.regionId =  std::to_string(m_OmWorkParamConfiger.mecDeviceWorkParam.regionID);
    getConfiger().configerMec.configerOmCommon.crossId =  std::to_string(m_OmWorkParamConfiger.mecDeviceWorkParam.crossID);
    std::string pointNo;
    std::stringstream ss(m_OmWorkParamConfiger.mecDeviceWorkParam.routeId);
    std::getline(ss, pointNo, '-');
    std::getline(ss, pointNo);
    getConfiger().configerMec.configerOmCommon.pointName = m_OmWorkParamConfiger.mecDeviceWorkParam.routeId;
    getConfiger().configerMec.configerOmCommon.pointNo = pointNo;
//    getConfiger().enableDebugPrint = m_OmWorkParamConfiger.hasedInit;
    std::string valueLogLevel;
    {
        std::string filename = "/home/airos/os/setup.bash";
        std::ifstream file(filename);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                // 检查行是否包含 "GLOG_minloglevel"
                if (line.find("GLOG_minloglevel") != std::string::npos)
                {
                    // 提取值
                    size_t pos = line.find("=");
                    if (pos != std::string::npos)
                    {
                        valueLogLevel = line.substr(pos + 1);
                        break;
                    }
                }
            }
            file.close();
        }
    }
    int logLevel = 0;
    int level = atoi(valueLogLevel.c_str());
    switch (level)
    {
        case 0: //debug
            logLevel = LOG_LEVEL_DEBUG ;
            break;
        case 1: //INFO
            logLevel = LOG_LEVEL_INFO ;
            break;
        case 2: //WARNING
            logLevel = LOG_LEVEL_WARN ;
            break;
        case 3: //ERROR
            logLevel = LOG_LEVEL_ERROR;
            break;
        case 4: //No-log
            logLevel = LOG_LEVEL_ERROR;
            break;
        default:
            break;
    }
    getConfiger().configerMec.configerOmCommon.logLevel = logLevel;
    NetworkInfo networkInfo = getNetworkInfo(m_OmWorkParamConfiger.mecDeviceWorkParam.nic);
    if(networkInfo.ip_address.empty())
    {
        getConfiger().configerMec.configerOmCommon.addressIP = STR_DEFAULT_VALUE;
    }
    else
    {
        getConfiger().configerMec.configerOmCommon.addressIP = networkInfo.ip_address;
    }
    if(networkInfo.ip_address.empty())
    {
        getConfiger().configerMec.configerOmCommon.netMask  = STR_DEFAULT_VALUE;
    }
    else
    {
        getConfiger().configerMec.configerOmCommon.netMask = networkInfo.netmask;
    }

    if(getConfiger().enableDebugPrint)
	{
        OM_MEC_WARN_PRINT << getGatewayAddress();
    }
    getConfiger().configerMec.configerOmCommon.gateway = getGatewayAddress();;

    getSoftwareVersion();
    saveConfiger();

    //违法事件接口参数
    m_jlocation["type"] = "gcj02";
    m_jlocation["longitude"] = m_OmWorkParamConfiger.mecDeviceWorkParam.mecLongitude;
    m_jlocation["latitude"] = m_OmWorkParamConfiger.mecDeviceWorkParam.mecLatitude;

    m_imageDir = m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpDir + getConfiger().configerCameraEvent.imageDir;
    m_camerahttpPort = getConfiger().configerCameraEvent.httpPort;
    m_httpPath = getConfiger().configerCameraEvent.httpPath;
    m_mecIp = m_OmWorkParamConfiger.mecDeviceWorkParam.mecIp;
    m_tcpIp = m_OmWorkParamConfiger.cameraEventParamConfiger.tcpIp;
    m_tcpPort = m_OmWorkParamConfiger.cameraEventParamConfiger.tcpPort;
    m_ftpPort = m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpPort;

    m_tcpBuf = std::unique_ptr<afl::net::ByteBuffer>(new afl::net::ByteBuffer(128, 40960));


    //camera
    bool needSaveConfiger = false;
    bool hasErased = false;
    int i = 0;
    for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
    {
        if(v.deviceType == WorkParamDeviceTypeCamera && !v.deviceEsn.empty())
        {
            needSaveConfiger = true;
            if(!hasErased)
            {
                getConfiger().configerCamera.configerTopicRcId.RcIds.clear();
                hasErased = true;
            }
            getConfiger().configerCamera.configerTopicRcId.RcIds.push_back(v.deviceEsn);
            //camera
            std::pair<string, string> esnSnPair;
            esnSnPair.first = v.deviceEsn;
            esnSnPair.second = v.deviceSn;
            m_EsnSnMap.insert(esnSnPair);
            if(i == 0 ){
                m_OmCameraStatusContainer.m_OmCameraStatus1.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus1.deviceSn = v.deviceSn;
            }
            else if(i == 1){
                m_OmCameraStatusContainer.m_OmCameraStatus2.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus2.deviceSn = v.deviceSn;
            }
            else if(i == 2){
                m_OmCameraStatusContainer.m_OmCameraStatus3.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus3.deviceSn = v.deviceSn;
            }
            else if(i == 3){
                m_OmCameraStatusContainer.m_OmCameraStatus4.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus4.deviceSn = v.deviceSn;
            }
            else if(i == 4){
                m_OmCameraStatusContainer.m_OmCameraStatus5.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus5.deviceSn = v.deviceSn;
            }
            else if(i == 5){
                m_OmCameraStatusContainer.m_OmCameraStatus6.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus6.deviceSn = v.deviceSn;
            }
            else if(i == 6){
                m_OmCameraStatusContainer.m_OmCameraStatus7.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus7.deviceSn = v.deviceSn;
            }
            else if(i == 7){
                m_OmCameraStatusContainer.m_OmCameraStatus8.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus8.deviceSn = v.deviceSn;
            }
            else if(i == 8){
                m_OmCameraStatusContainer.m_OmCameraStatus9.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus9.deviceSn = v.deviceSn;
            }
            else if(i == 9){
                m_OmCameraStatusContainer.m_OmCameraStatus10.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus10.deviceSn = v.deviceSn;
            }
            else if(i == 10){
                m_OmCameraStatusContainer.m_OmCameraStatus11.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus11.deviceSn = v.deviceSn;
            }
            else if(i == 11){
                m_OmCameraStatusContainer.m_OmCameraStatus12.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus12.deviceSn = v.deviceSn;
            }
            else if(i == 12){
                m_OmCameraStatusContainer.m_OmCameraStatus13.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus13.deviceSn = v.deviceSn;
            }
            else if(i == 13){
                m_OmCameraStatusContainer.m_OmCameraStatus14.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus14.deviceSn = v.deviceSn;
            }
            else if(i == 14){
                m_OmCameraStatusContainer.m_OmCameraStatus15.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus15.deviceSn = v.deviceSn;
            }
            else if(i == 15){
                m_OmCameraStatusContainer.m_OmCameraStatus16.deviceEsn = v.deviceEsn;
                m_OmCameraStatusContainer.m_OmCameraStatus16.deviceSn = v.deviceSn;
            }
            else {}
            i++;
        }
    }
    getConfiger().configerSensorDeviceInfos.clear();
    for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
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
            configerSensorDeviceInfo.mecSn = m_OmWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
            configerSensorDeviceInfo.deviceSn = v.deviceSn;
            configerSensorDeviceInfo.deviceEsn = v.deviceEsn;
            configerSensorDeviceInfo.deviceIp = v.deviceIp;
            configerSensorDeviceInfo.deviceLongitude = v.deviceLongitude;
            configerSensorDeviceInfo.deviceLatitude = v.deviceLatitude;
            configerSensorDeviceInfo.deviceAltitude = v.deviceAltitude;
            getConfiger().configerSensorDeviceInfos.push_back(configerSensorDeviceInfo);
        }
    }
    bool hasErasedRadar = false;
    if (getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
        {
            if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
            {
                if(!hasErasedRadar)
                {
                    getConfiger().configerMec.configerTopicRadarID.radarIDs.clear();
                    hasErasedRadar = true;
                }
                // getConfiger().configerMec.configerTopicRadarID.radarIDs.push_back(v.deviceEsn);


                std::string topicPostfix = "/" +  v.vendor + "/" + v.category + "/" + v.radarCrossId + "/" + v.deviceEsn;
                getConfiger().configerMec.configerTopicRadarID.radarIDs.push_back(topicPostfix);
                getConfiger().configerMec.configerTopicRadarID.cross_id = v.radarCrossId; //丢弃不用
                getConfiger().configerMec.configerTopicRadarID.vendor = v.vendor;
                getConfiger().configerMec.configerTopicRadarID.category = v.category;
            }
        }
    }

    if(needSaveConfiger)
    {
        saveConfiger();
    }

    //radar
    needSaveConfiger = false;
    hasErased = false;
    int j = 0;
    for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
    {
        if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
        {
            needSaveConfiger = true;
            if(!hasErased)
            {
                getConfiger().configerRadar.configerTopicRadarID.radarIDs.clear();
                hasErased = true;
            }
            getConfiger().configerRadar.configerTopicRadarID.radarIDs.push_back(v.deviceEsn);
            getConfiger().configerRadar.configerTopicRadarID.cross_id = v.radarCrossId; //丢弃不用
            //camera
            std::pair<string, string> esnSnPair;
            esnSnPair.first = v.deviceEsn;
            esnSnPair.second = v.deviceSn;
            m_EsnSnMap.insert(esnSnPair);
            if(j == 0 ){
                m_OmRadarStatusContainer.m_OmRadarsStatus1.deviceEsn = v.deviceEsn;
                m_OmRadarStatusContainer.m_OmRadarsStatus1.deviceSn = v.deviceSn;
            }
            else if(j == 1){
               m_OmRadarStatusContainer.m_OmRadarsStatus2.deviceEsn = v.deviceEsn;
               m_OmRadarStatusContainer.m_OmRadarsStatus2.deviceSn = v.deviceSn;
            }
            else if(j == 2){
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceEsn = v.deviceEsn;
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceSn = v.deviceSn;
            }
            else if(j == 3){
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceEsn = v.deviceEsn;
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceSn = v.deviceSn;
            }
            else if(j == 4){
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceEsn = v.deviceEsn;
               m_OmRadarStatusContainer.m_OmRadarsStatus3.deviceSn = v.deviceSn;
            }
            else {}
            j++;
        }
    }

    if(needSaveConfiger)
    {
        saveConfiger();
    }
    return true;
}
//获取软件版本
std::string OM_COMPONENT::getSoftwareVersion()
{
    std::string softwareVersionFromFile;
    if( afl::FileUtil::isFileExist(getConfiger().configerWorkParam.configerVersionFilePath.c_str()))
    {
        std::ifstream softwareVersionFile(getConfiger().configerWorkParam.configerVersionFilePath.c_str());
        if(softwareVersionFile)
        {
            // 读取第一行内容
            std::string line;
            if (std::getline(softwareVersionFile, line))
            {
                if( getConfiger().configerMec.configerOmCommon.softwareVersion  != line)
                {
                    m_UsingVersion = softwareVersionFromFile =  getConfiger().configerMec.configerOmCommon.softwareVersion = line;
                    getConfiger().configerMec.configerOmCommon.protocolVersion = DBUtils::extractVersion(getConfiger().configerMec.configerOmCommon.softwareVersion);
                    saveConfiger();
                }
                else
                {
                    getConfiger().configerMec.configerOmCommon.protocolVersion = DBUtils::extractVersion(getConfiger().configerMec.configerOmCommon.softwareVersion);
                    saveConfiger();
                }
            }
            else
            {
                OM_MEC_ERROR_PRINT << "[error]file is empty！";
                m_UsingVersion = softwareVersionFromFile =  getConfiger().configerMec.configerOmCommon.softwareVersion;
            }
            softwareVersionFile.close();// 关闭文件
        }
    }
    else
    {
        softwareVersionFromFile = getConfiger().configerMec.configerOmCommon.softwareVersion;
        OM_MEC_ERROR_PRINT << "[error]" << getConfiger().configerWorkParam.configerVersionFilePath << " 文件不存在！";
    }
    return softwareVersionFromFile;
}

void OM_COMPONENT::monitorRebootTimeConfiger()
{
    if(getConfiger().configerMec.configerOtaAndRebootTime.OtaNeedExec)
    {
        uint64_t nowTimeStamp = afl::util::TimeStamp::now(true).millSeconds();
        uint64_t deviceRebootUtcMillSecondTime = getConfiger().configerMec.configerOtaAndRebootTime.DeviceRebootUtcMillSecondTime;
        if(nowTimeStamp <= deviceRebootUtcMillSecondTime)
        {
            OM_MEC_DEBUG_PRINT << "[reboot]nowTimeStamp <= deviceRebootUtcMillSecondTime";
            //小于1s，则立刻重启
            if(deviceRebootUtcMillSecondTime - nowTimeStamp < 1000)
            {
                getConfiger().configerMec.configerOtaAndRebootTime.DeviceRebootUtcMillSecondTime = 0;
                saveConfiger();
            }
            else
            {
                //大于1s，则更新监控信息
                getConfiger().configerMec.configerOtaAndRebootTime.DeviceRebootUtcMillSecondTime = deviceRebootUtcMillSecondTime - 1000;
                saveConfiger();
            }
        }
        else
        {
            OM_MEC_DEBUG_PRINT << "[reboot]nowTimeStamp > deviceRebootUtcMillSecondTime";
            getConfiger().configerMec.configerOtaAndRebootTime.DeviceRebootUtcMillSecondTime = 0;
            getConfiger().configerMec.configerOtaAndRebootTime.OtaNeedExec = false;
            saveConfiger();
        }

    }
    else
    {
//        OM_MEC_DEBUG_PRINT << "[reboot]no need reboot!";
    }

    return;
}
void OM_COMPONENT::onConnectedPushMsgOnce()
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!m_RegisterFlag)
    {

        DeviceBaseInfoQueryData deviceBaseInfoQueryData;

        deviceBaseInfoQueryData.infoId = DEVICE_BASIC_INFO;
        if(m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo)
        {
            OM_MEC_ERROR_PRINT << "[error]no register-ack, publish register-data!";
            publishDeviceBasicInfoData(deviceBaseInfoQueryData, DEVICE_BASIC_INFO_SENSOR, BASIC_INFO_OPT_TYPE_TIMER);
            return;
        }
       return;
    }
    else
    {
        //ota检测版本
        if(checkOtaUpdateVersion())
        {
            if(m_TimerPublishBasicDeviceInfoDataOnce > 0)
            {
                m_EventloopOmMec->cancelTimer(m_TimerPublishBasicDeviceInfoDataOnce);
            }
        }

    }
}
/**
 * @func：检测设备升级版本
 * @param contentJson
 */
bool OM_COMPONENT::checkOtaUpdateVersion()
{
    if (!m_MqttConnectedCloud)
    {
//        OM_MEC_ERROR_PRINT << "[error]mqtt not connected!";
        return false;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Ota_Down)
    {
        return false;
    }
    if(getConfiger().enableDebugPrint)
    {
         OM_MEC_DEBUG_PRINT << "[update][softwareVersion]" << getConfiger().configerMec.configerOmCommon.softwareVersion
        <<  "[airosVersionFromFile]"  << getConfiger().configerMec.configerOtaParam.airosVersionFromFile
        <<  "[airosVersionTarFlag]" << getConfiger().configerMec.configerOtaParam.airosVersionTarFlag
         << "[airosVersionSeqNum]" << getConfiger().configerMec.configerOtaParam.airosVersionSeqNum;
    }
    if(getConfiger().configerMec.configerOmCommon.softwareVersion == getConfiger().configerMec.configerOtaParam.airosVersionFromFile)
    {
        OTAUpdateStatusUpData otaUpdateStatusUpData;
        otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        otaUpdateStatusUpData.seqNum =  getConfiger().configerMec.configerOtaParam.airosVersionSeqNum;
        otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
        otaUpdateStatusUpData.code  = SUCCESS;
        otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
        otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;

        std::string hint = "update success!";
        otaUpdateStatusUpData.description = hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
        getConfiger().configerMec.configerOtaParam.airosVersionFromFile = "";
        getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = "";
        getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = false;
        saveConfiger();
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[success]" <<  hint;
        }
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[update]airos-version is not equal!";
        }
        if (getConfiger().configerMec.configerOtaParam.airosVersionTarFlag)
        {
            OTAUpdateStatusUpData otaUpdateStatusUpData;
            otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            otaUpdateStatusUpData.seqNum =  getConfiger().configerMec.configerOtaParam.airosVersionSeqNum;
            otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
            otaUpdateStatusUpData.code  = FAILURE;
            otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
            otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;

            std::string hint = "update failure!";
            otaUpdateStatusUpData.description = hint;
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
            getConfiger().configerMec.configerOtaParam.airosVersionFromFile = "";
            getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = "";
            getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = false;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[error]" <<  hint;
            }
            saveConfiger();
        }
    }
    return true;
}

/**
 * @func：设备基础信息响应
 * @param contentJson
 */
void OM_COMPONENT::subscribeDeviceBaseInfo(const json &contentJson)
{
    if (contentJson.size() < 0)
    {
        return;
    }
    DeviceBasicInfoAckData deviceBasicInfoAckData;
    try
    {
        deviceBasicInfoAckData = contentJson;
    } catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT  << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }

    if(deviceBasicInfoAckData.rscuEsn == getConfiger().rscuEsn)
    {
        if(deviceBasicInfoAckData.status == SUCESS_REGISTER)
        {
            m_RegisterFlag = true;
            m_OmMecStatus.register_flag = true;
            OM_MEC_SUCCESS_PRINT   << "[success]" << "Register Success!";
        }
        else
        {
            m_RegisterFlag = false;
            OM_MEC_ERROR_PRINT   << "[error]" << "Register Failure!";
        }
    }
    else
    {
        m_RegisterFlag = false;
        OM_MEC_ERROR_PRINT   << "[error]" << "rscuEsn no  equal!";
    }
}


/**
 * @func：设备信息查询
 * @param contentJson
 */
void OM_COMPONENT::subscribeDeviceInfoQuery(const json &contentJson)
{
    if (contentJson.size() < 0)
    {
        return;
    }
    DeviceBaseInfoQueryData deviceBaseInfoQueryData;
    try
    {
        deviceBaseInfoQueryData = contentJson;
    } catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }
    if(deviceBaseInfoQueryData.rscuEsn != getConfiger().rscuEsn)
    {
        OM_MEC_ERROR_PRINT << "[error]" << "rscuEsn no equal!";
        return ;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[infoId]" << deviceBaseInfoQueryData.infoId;
    }
    switch (deviceBaseInfoQueryData.infoId)
    {
        case DEVICE_BASIC_INFO:  //0：MEC 设备基础信息（7.3.2.2 路测设备及接入基础信息）
            {
                if (m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Query_Mec)
                {
                    publishDeviceBasicInfoData(deviceBaseInfoQueryData, DEVICE_BASIC_INFO_MEC, BASIC_INFO_OPT_TYPE_QUERY);
                }
            }
            break;
        case CONNECTED_DEVICE_INFO:  //2. 接入 MEC 的设备信息；（7.3.2.2 路测设备及接入基础信息）
            {
                if (m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Query_Mec_Ack)
                {
                    publishDeviceBasicInfoData(deviceBaseInfoQueryData, DEVICE_BASIC_INFO_SENSOR, BASIC_INFO_OPT_TYPE_QUERY);
                }
            }
           break;
        ////////////////////////////////////////////////////////////////////
        case DEVICE_RUNNING_STATUS:  //1：MEC 运行状态信息；（7.3.4.2.2 路测计算及接入设备运行状态信息）
            {
                if (m_MqttClientConfig.configerMec.configerEnable.Enable_Running_Status_Query_Mec)
                {
                    publishRunningStatusData(deviceBaseInfoQueryData, DEVICE_BASIC_INFO_MEC);
                }
            }
            break;
        case CONNECTED_DEVICE_RUNNING_STATUS:  //3：接入 MEC 的设备运行状态信息；（7.3.4.2.2 路测计算及接入设备运行状态信息）
            {
                if (m_MqttClientConfig.configerMec.configerEnable.Enable_Running_Status_Query_Sensor)
                {
                    publishRunningStatusData(deviceBaseInfoQueryData, DEVICE_BASIC_INFO_SENSOR);
                }
            }

            break;
        ////////////////////////////////////////////////////////////////////
        case CLOUD_CONTROL_CONFIG_INFO: //4：MEC 云控配置参数信息（7.3.3.2.2 配置修改）
            {
                ConfigUpdateData configUpdateData;
                configUpdateData.seqNum = deviceBaseInfoQueryData.seqNum;
                pulishConfigUpdateData(configUpdateData);
            }
            break;
        case PERFORMANCE_INFO:  //5：MEC 性能信息（7.3.4.1.2 性能上报）
            publishPerformenceData(deviceBaseInfoQueryData, false);
            break;
        case ALARM_INFO:  //6：MEC 告警信息
            publishAlarmManagementData(deviceBaseInfoQueryData, false);
            break;
        case DEVICE_VERSION_INFO:  //7：设备版本信息查询（7.3.6.2.2 OTAs升级）
            publishDeviceVersionData(deviceBaseInfoQueryData);
            break;
        case DEVICE_POS_INFO:
            {
                ConfigQueryData configQueryData;
                configQueryData.seqNum = deviceBaseInfoQueryData.seqNum;
                publishConfigQueryAckData(configQueryData, CONFIG_QUERY_INFO_OPT_TYPE_DEVICE_POS);
            }

            break;
        default:
            break;
    }

}
/**
 * @func 设备重启
 * @param contentJson
 */
void OM_COMPONENT::subscribeRebootData(const json &contentJson)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Reboot || !m_RegisterFlag)
    {
        return;
    }
    RebootData rebootData;
    try
    {
        rebootData = contentJson;
    }
    catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }
    if(rebootData.rscuEsn != getConfiger().rscuEsn)
    {
        OM_MEC_ERROR_PRINT << "[error] rscuEsn no equal!";
        return;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[rebootData]" << rebootData.to_string();
    }
    string hint = "Device-Reboot";
    RebootAckData rebootAckData;
    rebootAckData.timeStamp  = afl::util::TimeStamp::now(true).millSeconds();
    rebootAckData.seqNum = rebootData.seqNum;
    rebootAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
    OM_MEC_DEBUG_PRINT << "[m_MqttClientConfig.rscuEsn]" << m_MqttClientConfig.rscuEsn;
    if(rebootData.protocolVersion != PROTOCOL_VERSION)
    {
        rebootAckData.status = (StatusEnum)STATUS_FAILURE;
        hint = "[error]protocolversion is not V1.0";
        OM_MEC_ERROR_PRINT << hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
        return;
    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[hint]" << hint;
    }

    if (rebootData.time == 0)
    {
        rebootAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        rebootAckData.seqNum = rebootData.seqNum;
        rebootAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[rebootAckData.rscuEsn]" << rebootAckData.rscuEsn;
        }
        rebootAckData.status = (StatusEnum)STATUS_SUCCESS;


        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[notice]will reboot device!";
        }
#if 0
        {
            std::string rebootOmMecCmd = "kill -9 `ps -ef|grep om.dag|grep -v grep|awk '{print $2}'`";
            int retRebootOmMec = system(rebootOmMecCmd.c_str());
            if(retRebootOmMec != 0)
            {
                OM_MEC_ERROR_PRINT << "[error]reboot failure!";
            }
            int ret = system("reboot");
            if(ret ==0)
            {
                OM_MEC_SUCCESS_PRINT << "reboot success!";
            }

        }
#endif
        OmWorkParamConfiger  omWorkParamConfiger;
        WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
        SshInfo sshInfo;
        sshInfo.host = "127.0.0.1";
        sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
        sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
        sshInfo.execCmd = "sleep 3 && reboot";
        sshInfo.flagReboot = true;
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[sshInfo]" << sshInfo.to_string();
        }
        if(!DBUtils::sshExec(sshInfo))
        {
            OM_MEC_ERROR_PRINT << "[error]reboot device failure!";
            rebootAckData.status = (StatusEnum) STATUS_FAILURE;
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
        }
        else
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
        }

    }
    else if (rebootData.time > 0)
    {
        uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
        if (rebootData.time < nowTime)
        {
            rebootAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            rebootAckData.seqNum = rebootData.seqNum;
            rebootAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
            rebootAckData.status =(StatusEnum)STATUS_FAILURE;

            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
        }
        else if (rebootData.time == nowTime)
        {
            rebootAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            rebootAckData.seqNum = rebootData.seqNum;
            rebootAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
            rebootAckData.status = (StatusEnum)STATUS_SUCCESS;


#if 0
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]will kill om_mec";
                }
                std::string rebootOmMecCmd = "kill -9 `ps -ef|grep om_mec.dag|grep -v grep|awk '{print $2}'`";
                int retRebootOmMec = system(rebootOmMecCmd.c_str());
                if(retRebootOmMec != 0)
                {
                    OM_MEC_ERROR_PRINT << "[error]reboot failure!";
                }
            }
#else

            OmWorkParamConfiger  omWorkParamConfiger;
            WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
            SshInfo sshInfo;
            sshInfo.host = "127.0.0.1";
            sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
            sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
            sshInfo.execCmd = "sleep 3 && reboot";
            sshInfo.flagReboot = true;
            if(!DBUtils::sshExec(sshInfo))
            {
                OM_MEC_ERROR_PRINT << "[error]reboot device failure!";
                rebootAckData.status = (StatusEnum) STATUS_FAILURE;
                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
            }
            else
            {
                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
            }
#endif
        }
        else
        {
            double rebootInterval = (rebootData.time - nowTime) / 1000;
            m_TimerMaintenanceManagementRebootFd = m_EventloopOmMec->addTimer([&]()
              {
                  rebootAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                  rebootAckData.seqNum = rebootData.seqNum;
                  rebootAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
                if (getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[rebootAckData.rscuEsn]" << rebootAckData.rscuEsn << "[rebootData.rscuEsn]" << rebootData.rscuEsn;
                }
                rebootAckData.status = (StatusEnum)STATUS_SUCCESS;


                  if(getConfiger().enableDebugPrint)
				  {
                      OM_MEC_DEBUG_PRINT << "[notice]will kill om_mec";
                  }
#if 0
                  std::string rebootOmMecCmd = "kill -9 `ps -ef|grep om_mec.dag|grep -v grep|awk '{print $2}'`";
                  int retRebootOmMec = system(rebootOmMecCmd.c_str());
                  if(retRebootOmMec != 0)
                  {
                      OM_MEC_ERROR_PRINT << "[error]reboot failure!";
                  }
                  int ret = system("reboot");
                  if(ret == 0)
                  {
                      OM_MEC_SUCCESS_PRINT << "reboot success!";
                  }
#else
                  OmWorkParamConfiger  omWorkParamConfiger;
                  WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
                  SshInfo sshInfo;
                  sshInfo.host = "127.0.0.1";
                  sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
                  sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
                  sshInfo.execCmd = "sleep 3 && reboot";
                  sshInfo.flagReboot = true;
                 if(getConfiger().enableDebugPrint)
                 {
                     OM_MEC_DEBUG_PRINT << "[sshInfo]" << sshInfo.to_string();
                 }
                  if(!DBUtils::sshExec(sshInfo))
                  {
                      OM_MEC_ERROR_PRINT << "[error]reboot device failure!";
                      rebootAckData.status = (StatusEnum) STATUS_FAILURE;
                      mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
                  }
                  else
                  {
                      if(getConfiger().enableDebugPrint)
                      {
                          OM_MEC_DEBUG_PRINT << "[rebootAckData]" << rebootAckData.to_string();
                      }
                      mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack, rebootAckData, hint);
                  }
#endif
              }, rebootInterval, true);
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[error] time < 0!";
    }
}
//取消升级
void OM_COMPONENT::subscribeOtaCancelData(const json &contentJson)
{
    if (!m_MqttConnectedCloud)
    {
        return;
    }
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Cancel || !m_RegisterFlag)
    {
        return;
    }

    OtaUpdateCancelData otaUpdateCancelData;
    try
    {
        otaUpdateCancelData = contentJson;
    }
    catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }

    // 1. 校验 ESN
    if (otaUpdateCancelData.rscuEsn != getConfiger().rscuEsn)
    {
        OM_MEC_WARN_PRINT << "[cancel] ESN mismatch. Recv: " << otaUpdateCancelData.rscuEsn;
        return;
    }
    // 2. 执行核心取消操作
    m_CancelDownloadFlag.store(true, std::memory_order_relaxed);
    m_UpdateVersionFromCancleMsg = otaUpdateCancelData.updateVersion;
    m_OtaCancleSeqnum = otaUpdateCancelData.seqNum;
    // std::string hint = "[cancel] OTA Cancelled Successfully.";
    // OM_MEC_SUCCESS_PRINT << hint;
    // OTAUpdateStatusUpData otaUpdateStatusUpData;
    // otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    // otaUpdateStatusUpData.seqNum =  otaUpdateCancelData.seqNum;
    // otaUpdateStatusUpData.rscuEsn = otaUpdateCancelData.rscuEsn;
    // otaUpdateStatusUpData.code  = FAILURE;
    // otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
    // otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
    // otaUpdateStatusUpData.description = hint;
    std::string hint = "[cancel] OTA Cancelled received.";
    OtaUpdateCancelAckData otaUpdateCancelAckData;
    otaUpdateCancelAckData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
    otaUpdateCancelAckData.seqNum = otaUpdateCancelData.seqNum;
    otaUpdateCancelAckData.deviceID = otaUpdateCancelData.rscuEsn;
    otaUpdateCancelAckData.state = "received";

    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK ,otaUpdateCancelAckData, hint);

    hint = "[cancel] OTA Cancelled cancel.";
    otaUpdateCancelAckData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
    otaUpdateCancelAckData.seqNum = otaUpdateCancelData.seqNum;
    otaUpdateCancelAckData.deviceID = otaUpdateCancelData.rscuEsn;
    otaUpdateCancelAckData.state = "cancel";

    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK ,otaUpdateCancelAckData, hint);

}


//升级
void OM_COMPONENT::subscribeOtaDownData(const json &contentJson)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Down || !m_RegisterFlag)
    {
        return;
    }
    std::string hint;
    OTAUpdateDownData otaUpdateDownData;
    OTAUpdateStatusUpData otaUpdateStatusUpData;
    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    otaUpdateStatusUpData.seqNum =  otaUpdateDownData.seqNum;
    otaUpdateStatusUpData.rscuEsn = otaUpdateDownData.rscuEsn;
    otaUpdateStatusUpData.code  = FAILURE;
    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
    try
    {
        otaUpdateDownData = contentJson;
    }
    catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        hint = "parse ota-down-json error！";
        otaUpdateStatusUpData.description = hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Down ,otaUpdateStatusUpData, hint);
        return;
    }


    if(otaUpdateDownData.protocolVersion != PROTOCOL_VERSION)
    {
        otaUpdateStatusUpData.code = FAILURE;

        hint = "[error]protocolversion is not V1.0";
        otaUpdateStatusUpData.description = hint;
        OM_MEC_ERROR_PRINT << hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status ,otaUpdateStatusUpData, hint);
        return;
    }

    m_OtaDownSeqnum = otaUpdateDownData.seqNum;
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[info]" << otaUpdateDownData.to_string();
    }
    if(otaUpdateDownData.rscuEsn != getConfiger().rscuEsn)
    {
        OM_MEC_ERROR_PRINT << "[error]rscuEsn no equal";
        hint =  "rsuEsn no equal！down-info [rscuEsn]" + otaUpdateDownData.rscuEsn
            +" mec[rscuEsn]" + getConfiger().rscuEsn;
        otaUpdateStatusUpData.description = hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status ,otaUpdateStatusUpData, hint);
        return;
    }
    //升级版本检测: 如果相同，则不升级
    if (m_UsingVersion == otaUpdateDownData.updateVersion)
    {
        hint =  "[notice]updateVersion is equal！";
        OM_MEC_DEBUG_PRINT << hint;
        return;
    }
    m_UpdateVersionFromDownMsg =  otaUpdateDownData.updateVersion;
    m_DownloadHttpHandle = curl_easy_init();
    m_DownloadChkPara = otaUpdateDownData.downloadChkPara;
    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[updateTime]" << otaUpdateDownData.updateTime;
    }
    if(otaUpdateDownData.updateTime == 0)
    {
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[updateTime]start update thread!";
        }
        std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData));
        th.detach();
    }
    else if(otaUpdateDownData.updateTime <  0)
    {
        hint =  "ota-down-json updateTime < 0";
        otaUpdateStatusUpData.description = hint;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
    }
    else
    {
        if (otaUpdateDownData.updateTime > nowTime )
        {
            uint64_t updateInterTime = (otaUpdateDownData.updateTime - nowTime)/1000;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[notice]start timer, interval: " << updateInterTime << " s";
            }
            m_TimerOtaUpdateDownData =  m_EventloopOmMec->addTimer(
                    std::bind(&OM_COMPONENT::processOtaUpdateDownData, this,  otaUpdateDownData), updateInterTime, true);
        }
        else
        {
            hint = "ota-down-json updateTime("
                   + std::to_string(otaUpdateDownData.updateTime) + ") < rscu nowTime("
                   + std::to_string(nowTime) + ")";
            otaUpdateStatusUpData.description = hint;
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
        }
    }
}
bool OM_COMPONENT::processOtaUpdateDownData(OTAUpdateDownData& otaUpdateDownData)
{
    if(m_TimerOtaUpdateDownData > 0)
    {
        m_EventloopOmMec->cancelTimer(m_TimerOtaUpdateDownData);
        m_TimerOtaUpdateDownData = -1;
    }
    std::thread th(std::bind(&OM_COMPONENT::threadDownloadOtaFile, this, this, otaUpdateDownData));
    th.detach();
    return true;
}
std::string OM_COMPONENT::calculateMD5(const std::string& filename)
{
    std::string command = "md5sum " + filename;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        OM_MEC_ERROR_PRINT << "Error executing command: " << command;
        return "";
    }

    char buffer[128];
    std::stringstream output;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        output << buffer;
    }

    int exitCode = pclose(pipe);
    if (exitCode != 0)
    {
        OM_MEC_ERROR_PRINT << "Command failed with exit code: " << exitCode;
        return "";
    }

    std::string md5 = output.str();
    size_t spacePos = md5.find(' ');
    if (spacePos != std::string::npos)
    {
        return md5.substr(0, spacePos);
    }
    else
    {
        return "";
    }
}

void  OM_COMPONENT::monitorDeviceNeedUpdate()
{
    if(!m_RegisterFlag)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Ota_Down)
    {
        return ;
    }

    bool m_IsGetDownResponseSucessMain;
    std::string otaSeqNumTemp = "";
    std::unique_lock<std::mutex> locker(m_MutexDownload);
    m_IsGetDownResponseSucessMain = m_IsGetDownResponseSucess;
    otaSeqNumTemp = m_OtaSeqNum;
    locker.unlock();
    if(m_IsGetDownResponseSucessMain)
    {
        m_Md5NoEqualPushOnced = false;
        std::string downloadTarRootPath =  getConfiger().configerProjectPath.projectRoot
                                           + "/" +  getConfiger().configerProjectPath.otaDirName;
        std::string tarFilePath = downloadTarRootPath + "/" +  getConfiger().configerProjectPath.otaFileName;


        //取消升级
        bool val = m_CancelDownloadFlag.load(std::memory_order_relaxed);
        OM_MEC_ERROR_PRINT << "[notice] " << (val?"true":"false");
        if (val)
        {
            if (m_UpdateVersionFromDownMsg !=  m_UpdateVersionFromCancleMsg)
            {
                OM_MEC_ERROR_PRINT << "[error] UpdateVersion is not equal!";

                std::string hint = "[cancel] OTA Cancelled failed.";
                OtaUpdateCancelAckData otaUpdateCancelAckData;
                otaUpdateCancelAckData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
                otaUpdateCancelAckData.seqNum = m_OtaCancleSeqnum;
                otaUpdateCancelAckData.deviceID = getConfiger().rscuEsn;
                otaUpdateCancelAckData.state = "failed";

                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK ,otaUpdateCancelAckData, hint);
                return ;
            }
            if (-1 == remove(tarFilePath.c_str()))
            {
                OM_MEC_ERROR_PRINT << "[error] Failed to delete existing file!";
                std::string hint = "[cancel] OTA Cancelled reboot.";
                OtaUpdateCancelAckData otaUpdateCancelAckData;
                otaUpdateCancelAckData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
                otaUpdateCancelAckData.seqNum = m_OtaCancleSeqnum;
                otaUpdateCancelAckData.deviceID = getConfiger().rscuEsn;
                otaUpdateCancelAckData.state = "reboot";

                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK ,otaUpdateCancelAckData, hint);

            }
            else
            {
                std::string hint = "[cancel] OTA Cancelled success.";
                OtaUpdateCancelAckData otaUpdateCancelAckData;
                otaUpdateCancelAckData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
                otaUpdateCancelAckData.seqNum = m_OtaCancleSeqnum;
                otaUpdateCancelAckData.deviceID = getConfiger().rscuEsn;
                otaUpdateCancelAckData.state = "success";

                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK ,otaUpdateCancelAckData, hint);
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[success] Successfully deleted existing file!";
                    return ;
                }
            }
        }
        else
        {
            //正常升级
            std::string tarXvfPath = "/home/airos";

            std::string tarMd5Value = calculateMD5(tarFilePath.c_str());
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[update][md5]" << tarMd5Value << "[m_DownloadChkPara]" << m_DownloadChkPara;
            }
            if(m_DownloadChkPara == tarMd5Value)
            {
                m_Md5NoEqualPushOnced = false;
                OM_MEC_DEBUG_PRINT << "[update]md5 equal!";
                if(m_OtaIsExecBashOnce == false)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]just exec once!";
                    }
                    m_OtaIsExecBashOnce = true;
                    std::string command_tar = "cd /home/airos/ && tar xvf " + tarFilePath;
                    int ret = system(command_tar.c_str());
                    if (ret == 0)
                    {

                        if( afl::FileUtil::isFileExist(getConfiger().configerWorkParam.configerVersionFilePath.c_str()))
                        {
                            std::ifstream softwareVersionFile(getConfiger().configerWorkParam.configerVersionFilePath.c_str());
                            if (softwareVersionFile)
                            {
                                // 读取第一行内容
                                std::string line;
                                if (std::getline(softwareVersionFile, line))
                                {
                                    if(getConfiger().enableDebugPrint)
                                    {
                                        OM_MEC_DEBUG_PRINT << "[update][notice]read success!";
                                        OM_MEC_DEBUG_PRINT <<  "[update][airosVersionFromFile]" <<  line
                                        << "[airosVersionTarFlag]" <<  getConfiger().configerMec.configerOtaParam.airosVersionTarFlag;
                                    }

                                    getConfiger().configerMec.configerOtaParam.airosVersionFromFile = line;
                                    getConfiger().configerMec.configerOtaParam.airosVersionTarFlag = true;
                                    getConfiger().configerMec.configerOtaParam.airosVersionSeqNum = m_OtaDownSeqnum;
                                    saveConfiger();

                                    OTAUpdateStatusUpData otaUpdateStatusUpData;
                                    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                                    otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
                                    otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
                                    otaUpdateStatusUpData.code  = SUCCESS;
                                    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
                                    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;

                                    std::string hint = "[success] upgrade success!";
                                    otaUpdateStatusUpData.description = hint;
                                    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);

                                    std::string command_start = "cd /home/airos/ && launch all";
                                    int ret = system(command_start.c_str());
                                    if (ret == 0)
                                    {
                                        if(getConfiger().enableDebugPrint)
                                        {
                                            OM_MEC_DEBUG_PRINT <<  "[update][success] Sucess to execute command: " << command_start;
                                        }
                                        m_OtaIsExecBashOnce = true;
                                    }
                                    else
                                    {
                                        OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_start;
                                    }
                                }
                            }
                            else
                            {
                                OM_MEC_ERROR_PRINT << "[update][error]read file failure!";
                            }
                        }
                        else
                        {
                            OM_MEC_ERROR_PRINT << "[update][error]file not exsit!";
                        }

                    }
                    else
                    {
                        OM_MEC_ERROR_PRINT <<  "[update][error] Failed to execute command: " << command_tar;
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_SUCCESS_PRINT << "[update][error] Failed to execute command: tar";
                        }
                        OTAUpdateStatusUpData otaUpdateStatusUpData;
                        otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                        otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
                        otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
                        otaUpdateStatusUpData.code  = FAILURE;
                        otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
                        otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;

                        std::string hint = "tar xvf failure!";
                        otaUpdateStatusUpData.description = hint;
                        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
                    }

                }
            }
            else
            {
                if(!m_Md5NoEqualPushOnced)
                {
                    OM_MEC_ERROR_PRINT << "[update][error] md5 is not equal！";
                    OTAUpdateStatusUpData otaUpdateStatusUpData;
                    otaUpdateStatusUpData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
                    otaUpdateStatusUpData.seqNum =  m_OtaDownSeqnum;
                    otaUpdateStatusUpData.rscuEsn = getConfiger().rscuEsn;
                    otaUpdateStatusUpData.code  = FAILURE;
                    otaUpdateStatusUpData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
                    otaUpdateStatusUpData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;

                    std::string hint = "md5 no equal!";
                    otaUpdateStatusUpData.description = hint;
                    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status, otaUpdateStatusUpData, hint);
                    m_Md5NoEqualPushOnced = true;
                    return ;
                }

            }
        }

        m_IsGetDownResponseSucessMain = false;
    }
    else
    {
//        OM_MEC_ERROR_PRINT << "[error]m_IsGetDownResponseSucessMain is false！";
    }
}

std::size_t curlWriteCallback(char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
{
    ((std::string*) userdata)->append(ptr, nmemb);
    return nmemb * size;
}
// Helper function to handle errors
void OM_COMPONENT::handleErrorResponse(CURLcode code)
{
    switch (code)
    {
        case 400:
            OM_MEC_ERROR_PRINT << "[error] Invalid parameters!";
            break;
        case 403:
            OM_MEC_ERROR_PRINT << "[error] Authentication failed!";
            break;
        case 500:
            OM_MEC_ERROR_PRINT << "[error] Internal server error!";
            break;
        default:
            OM_MEC_ERROR_PRINT << "[error] Request failed! Response code: [" << code << "]";
    }
}
void OM_COMPONENT::threadDownloadOtaFile(void* context, OTAUpdateDownData &otaUpdateDownData)
{
    try {
        struct curl_slist *headers = NULL;
        std::string httpQueryResponseStr;
        OTAUpdateDownData otaUpdateDownDataTemp = otaUpdateDownData;
        // Set up CURL options
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_CONNECTTIMEOUT, 60);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_TIMEOUT, 400);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_TCP_KEEPIDLE, 120L);
        curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_TCP_KEEPINTVL, 60L);

        if (m_DownloadHttpHandle)
        {
            curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_WRITEFUNCTION, curlWriteCallback);
            curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_WRITEDATA, (void *)&httpQueryResponseStr);

            // Retry loop for 3 attempts
            const int maxAttempts = 3;
            for (int attempt = 0; attempt < maxAttempts; ++attempt)
            {
                // Set the Authorization header
                std::string tokenDataHead = "Authorization: " + otaUpdateDownDataTemp.token;
                headers = curl_slist_append(headers, tokenDataHead.c_str());
                curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_HTTPHEADER, headers);

                // Construct the download URL
                std::string authUrl = otaUpdateDownDataTemp.downloadUrl + "?token=" + otaUpdateDownDataTemp.token;
                curl_easy_setopt(m_DownloadHttpHandle, CURLOPT_URL, authUrl.c_str());

                // Perform the request
                m_DownloadRepCode = curl_easy_perform(m_DownloadHttpHandle);
                if (CURLE_OK == m_DownloadRepCode)
                {
                    break; // Exit loop on success
                }
                else
                {
                    OM_MEC_ERROR_PRINT << "[error] Request failed, attempt " << (attempt + 1) << ": " << curl_easy_strerror(static_cast<CURLcode>(m_DownloadRepCode));
                    usleep(1000000); // Retry after 1 second if failed
                }
            }

            // Handle response
            if (CURLE_OK != m_DownloadRepCode)
            {
                handleErrorResponse(m_DownloadRepCode);
                curl_easy_cleanup(m_DownloadHttpHandle);
                httpQueryResponseStr.clear();
                std::unique_lock<std::mutex> locker(m_MutexDownload);

                m_IsGetDownResponseSucess = false;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_SUCCESS_PRINT << "[seqNum]" << otaUpdateDownDataTemp.seqNum;
                }
                m_OtaSeqNum = otaUpdateDownDataTemp.seqNum;
                return;
            }

            // File storage logic
            std::string downloadTarRootPath = getConfiger().configerProjectPath.projectRoot + "/" + getConfiger().configerProjectPath.otaDirName;
            std::string tarFilePath = downloadTarRootPath + "/" + getConfiger().configerProjectPath.otaFileName;

            // Create directory if it doesn't exist
            if (!afl::FileUtil::isDirectory(downloadTarRootPath.c_str()))
            {
                afl::FileUtil::createRecursionDir(downloadTarRootPath.c_str());
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[success] Created directory: " << downloadTarRootPath;
                }
            }
            else
            {
                if (-1 == remove(tarFilePath.c_str()))
                {
                    OM_MEC_ERROR_PRINT << "[error] Failed to delete existing file!";
                }
                else
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[success] Successfully deleted existing file!";
                    }
                }
            }

            // Write to file
            std::fstream file(tarFilePath, std::ios::out);
            if (!file.is_open())
            {
                OM_MEC_ERROR_PRINT << "[error] Failed to open file for writing!";
                return;
            }
            file << httpQueryResponseStr;
            file.close();

            // Check file size and set status
            if (getFileSize(tarFilePath) > 100 * 1024 * 1024)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[success] File stored successfully!";
                }
                std::unique_lock<std::mutex> locker(m_MutexDownload);
                m_IsGetDownResponseSucess = true;
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[error] File size too small, please check the response content!";
                }
            }

            // Free the headers list
            curl_slist_free_all(headers);

        }
    }
    catch (const std::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[error] Exception: " << e.what();
        return;
    }
    catch (...)
    {
        OM_MEC_ERROR_PRINT << "[error] Unknown exception occurred.";
        return;
    }

}
long long OM_COMPONENT::getFileSize(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
    {
        return buffer.st_size; // Return file size
    }
    return -1; // Return -1 if the file does not exist
}
/**
 * @func: 心跳信息
 */
void OM_COMPONENT::publishDeviceHeartbeatData()
{
    if (!m_MqttConnected )
    {
        return;
    }

    if (!getConfiger().configerMec.configerEnable.Enable_Device_HeartBeat || !m_RegisterFlag)
    {
        return;
    }
    string hint = "push Device-HeartBeat info";
    HeartbeatData heartbeatData;
    heartbeatData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[heartbeat]" << hint << "[timeStamp]" << heartbeatData.timeStamp;
    }
    heartbeatData.seqNum = afl::util::Srand::srandStr(32);
    heartbeatData.rscuEsn = getConfiger().rscuEsn;
    heartbeatData.status = (RunStatusEnum)0;
    heartbeatData.statusEmpty = false;
    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Device_HeartBeat ,heartbeatData, hint);
}

bool OM_COMPONENT::parseRsuMapXml(const char *fname)
{
    pugi::xml_document xml_doc;
    if (!xml_doc.load_file(fname))
    {
        OM_MEC_ERROR_PRINT << "open xml file fasle:" << fname;
        return false;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_WARN_PRINT << "open xml file true:" << fname;
    }
    if (xml_doc.child("nodes").empty())
    {
        OM_MEC_ERROR_PRINT << "xml no nodes";
        return -1;
    }
    if (std::distance(xml_doc.child("nodes").begin(), xml_doc.child("nodes").end()) == 0)
    {
        OM_MEC_ERROR_PRINT << "xml nodes no node";
        return -1;
    }
    bool bool_have_optional_ = false;
    size_t nodes_size        = std::distance(xml_doc.child("nodes").begin(), xml_doc.child("nodes").end());
    pugi::xml_node iter_nodes = xml_doc.child("nodes").first_child();
    for (size_t i = 0; i < nodes_size; i++)
    {
        bool_have_optional_ = false;
        if (!iter_nodes.child("links").empty() &&
            std::distance(iter_nodes.child("links").begin(),iter_nodes.child("links").end()) > 0)
        {
            bool_have_optional_ = true;
        }
        if (bool_have_optional_)
        {
            size_t links_size = std::distance(iter_nodes.child("links").begin(), iter_nodes.child("links").end());
            pugi::xml_node iter_links = iter_nodes.child("links").first_child();
            for (size_t index_links = 0; index_links < links_size; index_links++)
            {
                size_t lanes_size        = iter_links.child("lanes").empty()
                                           ? 0
                                           : std::distance(
                                iter_links.child("lanes").begin(),
                                iter_links.child("lanes").end());
                pugi::xml_node iter_lane = iter_links.child("lanes").first_child();

                iter_links = iter_links.next_sibling();
            }
        }
        iter_nodes = iter_nodes.next_sibling();
    }
    return true;
}

void OM_COMPONENT::updateMecStatusInfo()
{
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first = getConfiger().configerMec.configerDb.DbMecDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_STATUS;

    DBUtils::getDBInfo(tableInfo, statusList);
    for(auto& mecStatus:statusList)
    {
        if(mecStatus.deviceID == getConfiger().rscuEsn)
        {
            bool isNeedUpdate = false;
            MsgDeviceStatus msgDeviceStatus;

            msgDeviceStatus.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            msgDeviceStatus.timeStampNeedUpdate = true;

            msgDeviceStatus.deviceID =  getConfiger().rscuEsn;
            msgDeviceStatus.deviceIDNeedUpdate = true;

            msgDeviceStatus.deviceType = DEVICE_TYPE_DB_MEC;
            msgDeviceStatus.deviceTypeNeedUpdate = true;

            msgDeviceStatus.deviceStatus = DEVICE_STATUS_DB_ON;
            msgDeviceStatus.deviceStatusNeedUpdate = true;

            msgDeviceStatus.active = DEVICE_ACTIVE_DB_ON;
            msgDeviceStatus.activeNeedUpdate = true;
            std::string softwareVersion = DBUtils::extractVersion(getConfiger().configerMec.configerOmCommon.softwareVersion);
            if(mecStatus.softwareVersion != softwareVersion)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]softwareVersion is not equal!" <<
                                       "[mecStatus-softwareVersion]" << mecStatus.softwareVersion <<
                                       "[softwareVersion]" << softwareVersion;
                }
                msgDeviceStatus.softwareVersion = softwareVersion;
                msgDeviceStatus.softwareVersionNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.pointNo != getConfiger().configerMec.configerOmCommon.pointNo )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]pointNo is not equal!" <<
                                       "[mecStatus-pointNo]" << mecStatus.pointNo <<
                                       "[pointNo]" << getConfiger().configerMec.configerOmCommon.pointNo;
                }
                msgDeviceStatus.pointNo = getConfiger().configerMec.configerOmCommon.pointNo;
                msgDeviceStatus.pointNoNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.pointName != getConfiger().configerMec.configerOmCommon.pointName )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]pointName is not equal!" <<
                                       "[mecStatus-pointName]" << mecStatus.pointName <<
                                       "[pointName]" << getConfiger().configerMec.configerOmCommon.pointName;
                }
                msgDeviceStatus.pointName = getConfiger().configerMec.configerOmCommon.pointName;
                msgDeviceStatus.pointNameNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.longitude != getConfiger().configerMec.configerOmCommon.longitude )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]longitude is not equal!" <<
                                       "[mecStatus-longitude]" << mecStatus.longitude <<
                                       "[longitude]" << getConfiger().configerMec.configerOmCommon.longitude;
                }
                msgDeviceStatus.longitude = getConfiger().configerMec.configerOmCommon.longitude;
                msgDeviceStatus.longitudeNeedUpdate = true;
                isNeedUpdate = true;
            }
            if(mecStatus.latitude != getConfiger().configerMec.configerOmCommon.latitude )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]latitude is not equal!" <<
                                       "[mecStatus-latitude]" << mecStatus.latitude <<
                                       "[latitude]" << getConfiger().configerMec.configerOmCommon.latitude;
                }
                msgDeviceStatus.latitude = getConfiger().configerMec.configerOmCommon.latitude;
                msgDeviceStatus.latitudeNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.altitude != getConfiger().configerMec.configerOmCommon.elevation )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]altitude is not equal!" <<
                                       "[mecStatus-altitude]" << mecStatus.altitude <<
                                       "[altitude]" << getConfiger().configerMec.configerOmCommon.elevation;
                }
                msgDeviceStatus.altitude = getConfiger().configerMec.configerOmCommon.elevation;
                msgDeviceStatus.altitudeNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.deviceIp != getConfiger().configerMec.configerOmCommon.addressIP )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]deviceIp is not equal!" <<
                                       "[mecStatus-deviceIp]" << mecStatus.deviceIp <<
                                       "[addressIP]" << getConfiger().configerMec.configerOmCommon.addressIP;
                }
                msgDeviceStatus.deviceIp =  getConfiger().configerMec.configerOmCommon.addressIP;
                msgDeviceStatus.deviceIpNeedUpdate = true;
                isNeedUpdate = true;
            }


            if(mecStatus.netMask != getConfiger().configerMec.configerOmCommon.netMask )
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]netMask is not equal!";
                }
                msgDeviceStatus.netMask =  getConfiger().configerMec.configerOmCommon.netMask;
                msgDeviceStatus.netMaskNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(mecStatus.gateway != getConfiger().configerMec.configerOmCommon.gateway )
            {

                OM_MEC_ERROR_PRINT << "[error]gateway is not equal!";
                msgDeviceStatus.gateway =  getConfiger().configerMec.configerOmCommon.gateway;
                msgDeviceStatus.gatewayNeedUpdate = true;
                isNeedUpdate = true;
            }

            if(isNeedUpdate)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]update db info";
                }
                std::pair<std::string, TABLE_TYPE> tableInfo;
                tableInfo.first =  getConfiger().configerMec.configerDb.DbMecDeviceStatusTableName;
                tableInfo.second = TABLE_TYEP_MEC_DEV_STATUS;
                DBUtils::updateDBInfo(tableInfo, msgDeviceStatus);
            }
        }
    }
}
void OM_COMPONENT::alarmMonitor()
{
    if (!m_MqttConnected|| !m_RegisterFlag)
    {
        return;
    }
    if(!getConfiger().configerMec.configerEnable.Enable_Alarm_Monitor )
    {
        return;
    }
    std::string hint;
    AlarmManagementData alarmManagementData;
    PerformanceData performanceData;
    alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
    if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
    {
        alarmManagementData.protocolVersion = PROTOCOL_VERSION;
    }
    else
    {
        alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
    }

    auto &runningInfo = performanceData.runningInfo;
    {
        // getRuningInfoFromDbMecStatusTable(runningInfo);
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[notice]get cpu-info ";
        }
        PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
        if(!PerformenceUtils::getCpuInfo(runningInfo.cpuInfo))
        {
            OM_MEC_ERROR_PRINT << "[error]get cpu-info failure!";
        }
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[notice]get mem-info ";
        }
        if(!PerformenceUtils::getMemInfo(runningInfo.memInfo))
        {
            OM_MEC_ERROR_PRINT << "[error]get mem-info failure!";
        }
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[notice]get disk-info ";
        }
        if(!PerformenceUtils::getDiskInfo(runningInfo.diskInfo))
        {
            OM_MEC_ERROR_PRINT << "[error]get disk-info failure!";
        }
        //gpu
         if(!PerformenceUtils::getGpuInfo(runningInfo.gpuInfo))
        {
            OM_MEC_ERROR_PRINT << "[error]get gpu-info failure!";
        }


        //cpu信息

        if(runningInfo.cpuInfo.temp > getConfiger().configerMec.configAlarmThresholdValue.TV_CPU_Tem)
        {
            hint = "cpu-temp-high , [temp]" +
                   std::to_string(runningInfo.cpuInfo.temp) + " [up]" +
                   std::to_string(getConfiger().configerMec.configAlarmThresholdValue.TV_CPU_Tem);
//            if(getConfiger().enableDebugPrint)
//            {
//                        OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//            }
            if(!m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag)
            {
                uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, getConfiger().rscuEsn,  getConfiger().rscuEsn );
                m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = alarmManagementData.timeStamp;
                m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredTimeStamp = nowTime;
                m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
            }
            else
            {
                uint64_t timeDiff = (alarmManagementData.timeStamp -  m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_ERROR_PRINT  << "[alarm][cpu-temp-high][timeDiff(s)]" << timeDiff;
                }
                if(timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
                {
                    alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                    m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = alarmManagementData.timeStamp;
                }
            }
            m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag = true;
            m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag = false;
        }
        else
        {
            if(m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag)
            {
                hint = "cpu-temp-high";
//                if(getConfiger().enableDebugPrint)
//                {
//                    OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//                }
                if(!m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag)
                {
                    alarmDisappeared(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, getConfiger().rscuEsn, getConfiger().rscuEsn,
                                     m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredTimeStamp);
                    m_AlarmTypeFlag.alarmHighTemperCPUFlag.timeStamp = 0;
                    m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredFlag = false;
                    m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                    m_AlarmTypeFlag.alarmHighTemperCPUFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                }
            }
        }
    }

    {
        //cpu负载
        if(runningInfo.cpuInfo.uti.size() > getConfiger().configerMec.configAlarmThresholdValue.TV_CPU_Uti)
        {
            float value = atof(runningInfo.cpuInfo.uti.c_str());
            if(value > getConfiger().configerMec.configAlarmThresholdValue.TV_CPU_Uti)
            {
                hint = "cpu-load-high, [uti]" +
                       std::to_string(value) + " [up]" +
                       std::to_string(getConfiger().configerMec.configAlarmThresholdValue.TV_CPU_Uti);
//                if(getConfiger().enableDebugPrint)
//                {
//                    OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
//                }
                if(!m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag)
                {
                    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                    alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                    m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = alarmManagementData.timeStamp;
                    m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredTimeStamp = nowTime;
                    m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                }
                else
                {
                    uint64_t timeDiff = (alarmManagementData.timeStamp -  m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_ERROR_PRINT  << "[alarm][cpu-load-high][timeDiff(s)]" << timeDiff;
                    }
                    if(timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
                    {
                        alarmOccurred(hint, MEC_ALARM_TYPE_HIGH_TEMPER_CPU, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                        m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = alarmManagementData.timeStamp;
                    }
                }

                m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag = true;
                m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag = false;
            }
            else
            {
                if(m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag)
                {
                    hint = "cpu-load-high";
//                    if(getConfiger().enableDebugPrint)
//                    {
//                        OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//                    }
                    if(!m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag)
                    {
                        alarmDisappeared(hint, MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU, getConfiger().rscuEsn, getConfiger().rscuEsn,
                                         m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredTimeStamp);
                        m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.timeStamp = 0;
                        m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredFlag = false;
                        m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorOccuredPublishedOnceFlag = true;
                        m_AlarmTypeFlag.alarmHighOccupancyCPUFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                    }
                }
            }
        }
    }

    {
        //mem可用空间
        if (runningInfo.memInfo.free <=  getConfiger().configerMec.configAlarmThresholdValue.TV_Mem_Free)
        {
            hint = "out-of-mem, [free]" +
                   std::to_string(runningInfo.memInfo.free) + " [up]" +
                   std::to_string(getConfiger().configerMec.configAlarmThresholdValue.TV_Mem_Free);
//            if(getConfiger().enableDebugPrint)
//            {
//                OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
//            }
            if(!m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag)
            {
                uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = alarmManagementData.timeStamp;
                m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredTimeStamp = nowTime;
                m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag = true;
            }
            else
            {
                uint64_t timeDiff = (alarmManagementData.timeStamp -  m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_ERROR_PRINT << "[alarm][out-of-mem][timeDiff(s)]" << timeDiff;
                }
                if(timeDiff >= getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
                {
                    alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                    m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = alarmManagementData.timeStamp;
                }
            }
            m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag = true;
            m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag = false;
        }
        else
        {
            if(m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag)
            {
                hint = "out-of-mem";
//                if(getConfiger().enableDebugPrint)
//                {
//                        OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//                }
                if(! m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag)
                {
                    alarmDisappeared(hint, MEC_ALARM_TYPE_OUT_OF_MEMORY, getConfiger().rscuEsn,  getConfiger().rscuEsn,
                                     m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredTimeStamp);

                    m_AlarmTypeFlag.alarmOutOfMemoryFlag.timeStamp = alarmManagementData.timeStamp;
                    m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredFlag = false;
                    m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorOccuredPublishedOnceFlag = true;
                    m_AlarmTypeFlag.alarmOutOfMemoryFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                }
            }
        }
    }
    {
        //disk可用空间
        // if(runningInfo.diskInfo.free <= getConfiger().configerMec.configAlarmThresholdValue.TV_Disk_Free)
        uint64_t rootAvailableCapacityBytes = 0;
        uint64_t workAvailableCapacityBytes = 0;
        for (auto mountPoint:runningInfo.diskInfo.mountPoints)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[mountPoint][name]" << mountPoint.mountPoint << " [availableCapacity(Byte)]" << mountPoint.availableCapacity;
            }
            if (mountPoint.mountPoint == PerformenceUtils::m_RootDirName)
            {
                rootAvailableCapacityBytes = mountPoint.availableCapacity;
            }
            if (mountPoint.mountPoint == PerformenceUtils::m_WorkDirName)
            {
                workAvailableCapacityBytes = mountPoint.availableCapacity;
            }
        }
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[rootAvailableCapacityBytes]" <<rootAvailableCapacityBytes << "[workAvailableCapacityBytes(Byte)]" << workAvailableCapacityBytes;
        }
        if(rootAvailableCapacityBytes <= getConfiger().configerMec.configAlarmThresholdValue.TV_Disk_Free ||
            workAvailableCapacityBytes <= getConfiger().configerMec.configAlarmThresholdValue.TV_Disk_Work_Free)
        {
            hint = "out-of-disk, [free]" +
                   std::to_string(runningInfo.diskInfo.free) + " [up]" +
                   std::to_string(getConfiger().configerMec.configAlarmThresholdValue.TV_Disk_Free);
//            if(getConfiger().enableDebugPrint)
//            {
//                OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
//            }
            if(!m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag)
            {
                uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = alarmManagementData.timeStamp;
                m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredTimeStamp = nowTime;
                m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag = true;
            }
            else
            {
                uint64_t timeDiff = (alarmManagementData.timeStamp -  m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_ERROR_PRINT  << "[alarm][out-of-disk][timeDiff(s)]" << timeDiff;
                }
                if(timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
                {
                    alarmOccurred(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE, getConfiger().rscuEsn,  getConfiger().rscuEsn);
                    m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = alarmManagementData.timeStamp;
                }
            }

            m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag = true;
            m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag = false;
        }
        else
        {
            if(m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag)
            {
                hint = "out-of-diskInfo";
//                if(getConfiger().enableDebugPrint)
//                {
//                        OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//                }
                if(!m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag)
                {
                    alarmDisappeared(hint, MEC_ALARM_TYPE_OUT_OF_DISK_SPACE, getConfiger().rscuEsn,  	 getConfiger().rscuEsn,
                                     m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredTimeStamp);
                    m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.timeStamp = alarmManagementData.timeStamp;
                    m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredFlag = false;
                    m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorOccuredPublishedOnceFlag = true;
                    m_AlarmTypeFlag.alarmOutOfDiskSpaceFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                }
            }
        }
    }
    //离线
    {
//        if(getConfiger().enableDebugPrint)
//        {
//            hint = "check sensor-device offline";
//            OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
//        }
        int sensorDeviceOfflineCount = 0;
        std::string sensorDeviceOfflineAddition = "";
        hint = "sensor-device-offline [device-ID]";
        if (getConfiger().configerMec.configerEnable.Enable_Use_DB_For_Monitor_Offline)
        {
            OM_MEC_WARN_PRINT  << "[use db]";
        	std::vector<MsgDeviceStatus> statusList;
        	std::pair<std::string, TABLE_TYPE> tableInfo;
        	tableInfo.first = m_MqttClientConfig.configerMec.configerDb.DbDeviceStatusTableName;
        	tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
        	DBUtils::getDBInfo(tableInfo, statusList);

	        for (auto status: statusList)
			{
	            if (mec::db::DEVICE_STATUS_DB_OFF == status.deviceStatus)
				{
	                if (getConfiger().enableDebugPrint)
					 {
	                    hint.append("|");
	                    hint.append(status.deviceID);
	                }

	                sensorDeviceOfflineCount++;
	                sensorDeviceOfflineAddition.append(status.deviceID);
	                sensorDeviceOfflineAddition.append(",");
	            }
	        }
        }
        else
        {
            OM_MEC_WARN_PRINT  << "[no use db]";
            hint = "sensor-device-offline [device-ID]";
            if (getConfiger().configerMec.enableUseHearbeatAsOffline)
            {
                OM_MEC_WARN_PRINT  << "[use heartbeat]";
                for (auto pair : topicCameraHeartBeatMap)
                {
                    //            std::string topic = pair.first;
                    std::string deviceID = pair.first;

                    if (!pair.second.second)
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            hint.append("|");
                            hint.append(deviceID);
                        }
                        sensorDeviceOfflineCount++;
                        sensorDeviceOfflineAddition.append(deviceID);
                        sensorDeviceOfflineAddition.append(",");
                    }
                }

                for (auto pair : topicRadarHeartBeatMap)
                {
                    //            std::string topic = pair.first;
                    std::string deviceID = pair.first;

                    if (!pair.second.second)
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            hint.append("|");
                            hint.append(deviceID);
                        }
                        sensorDeviceOfflineCount++;
                        sensorDeviceOfflineAddition.append(deviceID);
                        sensorDeviceOfflineAddition.append(",");
                    }
                }
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_WARN_PRINT  << "[us ping]";
                }
                std::unordered_map<std::string,  bool>  topicCameraPingMapTemp;
                std::unordered_map<std::string,  bool>  topicRadarPingMapTemp;
                topicCameraPingMapTemp = topicCameraPingMap;
                topicRadarPingMapTemp = topicRadarPingMap;

                for (auto pair:topicCameraPingMapTemp)
                {
                    if (!pair.second)
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            hint.append("|");
                            hint.append(pair.first);
                        }
                        sensorDeviceOfflineCount++;
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT  << "[notice]" << hint << "[sensorDeviceOfflineCount]" << sensorDeviceOfflineCount;
                        }
                        sensorDeviceOfflineAddition.append(pair.first);
                        sensorDeviceOfflineAddition.append(",");
                    }

                }
                for (auto pair:topicRadarPingMapTemp)
                {
                    if (!pair.second)
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            hint.append("|");
                            hint.append(pair.first);
                        }
                        sensorDeviceOfflineCount++;
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT  << "[notice]" << hint << "[sensorDeviceOfflineCount]" << sensorDeviceOfflineCount;
                        }
                        sensorDeviceOfflineAddition.append(pair.first);
                        sensorDeviceOfflineAddition.append(",");
                    }
                }
            }

        }
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[notice]" << hint << "[sensorDeviceOfflineCount]" << sensorDeviceOfflineCount;
        }

        if(sensorDeviceOfflineCount > 0)
        {
            hint = "sensor-device-offline";
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
            }
            if(!m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag)
            {
                uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                alarmOccurred(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, sensorDeviceOfflineAddition, sensorDeviceOfflineAddition);
                m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag = true;
                m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = alarmManagementData.timeStamp;
                m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredTimeStamp = nowTime;
            }
            else
            {
                uint64_t timeDiff = (alarmManagementData.timeStamp -  m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_ERROR_PRINT  << "[alarm][sensor-device-offline][timeDiff(s)]" << timeDiff;
                }
                if(timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
                {
                    alarmOccurred(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, sensorDeviceOfflineAddition, sensorDeviceOfflineAddition);
                    m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = alarmManagementData.timeStamp;
                }
            }

            m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag = true;
            m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag = false;
        }
        else
        {
            if(m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag)
            {
                hint = "sensor-device-offline";
//                if(getConfiger().enableDebugPrint)
//                {
//                        OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//                }
                if(!m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag)
                {
                    alarmDisappeared(hint, MEC_ALARM_TYPE_IRE_OFF_LINE, sensorDeviceOfflineAddition,  sensorDeviceOfflineAddition,
                                     m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredTimeStamp);
                    m_AlarmTypeFlag.alarmIreOffLineFlag.timeStamp = 0;
                    m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredFlag = false;
                    m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorOccuredPublishedOnceFlag = true;
                    m_AlarmTypeFlag.alarmIreOffLineFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                }
            }
        }
    }

    //授时
    {
        hint = "check timing";
//		if(getConfiger().enableDebugPrint)
//        {
//        	OM_MEC_DEBUG_PRINT  << "[alarm]" << hint;
//		}
        upPtpStatus(false);
    }
}

/**
 * @func 告警管理
 * @param deviceManagementData
 */
void OM_COMPONENT::publishAlarmManagementData(DeviceBaseInfoQueryData deviceBaseInfoQueryData, bool periodPushFlag)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!getConfiger().configerMec.configerEnable.Enable_Alarm || !m_RegisterFlag)
    {

        return;
    }

    string hint = "publish Alarm-Management info";
    AlarmManagementData alarmManagementData;
#if 0
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first = m_MqttClientConfig.configerMec.configerDb.DbMecDeviceAlarmTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_ALARM;
    DBUtils::getDBInfo(tableInfo, statusList);

    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[statusList-size]" <<statusList.size();
    }
    for(auto& v : statusList)
    {
        if(periodPushFlag)
        {
            alarmManagementData.seqNum = afl::util::Srand::srandStr(32);
        }
        else
        {
            alarmManagementData.seqNum = deviceBaseInfoQueryData.seqNum;
        }
        if(v.alarmStatus == ALARM_OCCURRED)
        {
            alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm]" << hint << "[timeStamp]" << alarmManagementData.timeStamp;
            }
            alarmManagementData.seqNum = afl::util::Srand::srandStr(32);
            alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
            if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
            {
                alarmManagementData.protocolVersion = PROTOCOL_VERSION;
            }
            else
            {
                alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
            }

            alarmManagementData.alarm.alarmLevel = v.alarmLevel;
            alarmManagementData.alarm.alarmStatus = v.alarmStatus;
            alarmManagementData.alarm.alarmRaisedTime = v.alarmRaisedTime;
            alarmManagementData.alarm.alarmChangedTime = v.alarmChangedTime;
            alarmManagementData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)v.alarmType;

            alarmManagementData.alarm.addition = v.addition;
            alarmManagementData.alarm.additionEmpty = false;
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
        }

    }
#endif
    std::map<int, AlarmManagementData> m_MecAlarmMapTemp;
    {
        m_MecAlarmMapTemp = m_MecAlarmMap;
        if(getConfiger().enableDebugPrint)
        {
            for(auto pair:m_MecAlarmMap)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm-map][type]" << pair.first << "[data]" << pair.second.to_string();
            }
        }
    }

    for(auto pair:m_MecAlarmMapTemp)
    {
        if(periodPushFlag)
        {
            alarmManagementData.seqNum = afl::util::Srand::srandStr(32);
        }
        else
        {
            alarmManagementData.seqNum = deviceBaseInfoQueryData.seqNum;
        }

        alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[alarm]" << hint << "[timeStamp]" << alarmManagementData.timeStamp;
        }
        alarmManagementData.seqNum = afl::util::Srand::srandStr(32);
        alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
        if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
        {
            alarmManagementData.protocolVersion = PROTOCOL_VERSION;
        }
        else
        {
            alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
        }

        alarmManagementData.alarm.alarmLevel = pair.second.alarm.alarmLevel;
        alarmManagementData.alarm.alarmStatus = pair.second.alarm.alarmStatus;
        alarmManagementData.alarm.alarmRaisedTime = pair.second.alarm.alarmRaisedTime;
        alarmManagementData.alarm.alarmChangedTime = pair.second.alarm.alarmChangedTime;
        alarmManagementData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)pair.second.alarm.alarmType;

        alarmManagementData.alarm.addition = pair.second.alarm.addition;
        alarmManagementData.alarm.additionEmpty = false;
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
    }

}
std::vector<double> OM_COMPONENT::extractValues(const std::string& data_str)
{
    std::vector<double> data_values;
    std::stringstream ss(data_str);
    std::string item;

    while (std::getline(ss, item, ','))
    {
        // 去掉百分号
        item.pop_back();
        data_values.push_back(std::stod(item));
    }

    return data_values;
}
double OM_COMPONENT::getMemoryUsagePercentage(const std::string& freeOutput)
{
    // 使用正则表达式匹配内存信息
    std::regex pattern(R"(Mem:\s+(\d+\.\d+)\w+\s+(\d+\.\d+)\w+\s+(\d+\.\d+)\w+)");
    std::smatch match;

    if (std::regex_search(freeOutput, match, pattern))
    {
        double totalMemory = std::stod(match[1]);
        double usedMemory = std::stod(match[2]);

        // 计算内存占用率
        return (usedMemory / totalMemory) * 100;
    } else {
        return -1.0;
    }
}

std::string OM_COMPONENT::executeCommand(const std::string& command)
{
    // 执行 shell 命令并获取输出
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return "";
    }

    char buffer[128];
    std::string output = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        output += buffer;
    }

    int status = pclose(pipe);
    if (status == -1)
    {
        return "";
    }

    return output;
}

/**
 * @func 查询设备版本
 * @param deviceManagementData
 */
void OM_COMPONENT::publishDeviceVersionData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData, bool queryFlag)
{
    if (!m_MqttConnected)
    {
        return;
    }

    if (!m_RegisterFlag)
    {
		return;		

    }
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Version || !m_RegisterFlag)
    {
        return;
    }

    string hint = "publish Device-Version info";
    DeviceVersionQueryAckData deviceVersionQueryAckData;
    deviceVersionQueryAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[device-version]" << hint << "[timeStamp]"
                           << deviceVersionQueryAckData.timeStamp;
    }
    if(queryFlag)
    {
        deviceVersionQueryAckData.seqNum = deviceBaseInfoQueryData.seqNum;
    }
    else
    {
        deviceVersionQueryAckData.seqNum = afl::util::Srand::srandStr(32);
    }
    deviceVersionQueryAckData.rscuEsn =  getConfiger().rscuEsn;
    if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
    {
        deviceVersionQueryAckData.protocolVersion = PROTOCOL_VERSION;
    }
    else
    {
        deviceVersionQueryAckData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
    }
    deviceVersionQueryAckData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
    deviceVersionQueryAckData.hardwareVersion =  getConfiger().configerMec.configerOmCommon.hardwareVersion;

    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Version, deviceVersionQueryAckData, hint);
}


/**
 * @func: 运行状态
 * @param deviceManagementData
 */
void OM_COMPONENT::publishRunningStatusData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData, DeviceBasicInfoDeviceTypeEnum deviceBasicInfoDeviceType)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Running_Status || !m_RegisterFlag)
    {
        return;
    }
    string hint = "publish Running-Status info";

    RunningStatusData runningStatusData;
    runningStatusData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    runningStatusData.seqNum = afl::util::Srand::srandStr(32);
    if(getConfiger().enableDebugPrint)
    {
    	OM_MEC_DEBUG_PRINT  << "[running-status]" << hint << "[timeStamp]" << runningStatusData.timeStamp;
    }
    runningStatusData.rscuEsn =  getConfiger().rscuEsn;
    if(!getConfiger().configerMec.configerOmCommon.regionId.empty())
    {
        runningStatusData.regionId = getConfiger().configerMec.configerOmCommon.regionId;
        runningStatusData.regionIdEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.longitude > 72 && getConfiger().configerMec.configerOmCommon.longitude < 136)
    {
        runningStatusData.longitude = getConfiger().configerMec.configerOmCommon.longitude;
        runningStatusData.longitudeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.latitude > 17 && getConfiger().configerMec.configerOmCommon.latitude < 54)
    {
        runningStatusData.latitude = getConfiger().configerMec.configerOmCommon.latitude;
        runningStatusData.latitudeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.elevation > 0)
    {
        runningStatusData.elevation = getConfiger().configerMec.configerOmCommon.elevation;
        runningStatusData.elevationEmpty = false;
    }

    runningStatusData.rscuStatus = RUN_STATUS_NORMAL;
    runningStatusData.rscuStatusEmpty = false;
    runningStatusData.active = NETWORK_STATUS_ONLINE;

    runningStatusData.rsuNum = 0;
    runningStatusData.rsuNumEmpty = false;

    int infoId = deviceBaseInfoQueryData.infoId;
    if(DEVICE_BASIC_INFO_SENSOR == deviceBasicInfoDeviceType)
    {
        infoId = CONNECTED_DEVICE_RUNNING_STATUS;
    }
    bool hasFaultData = false;
    switch (infoId)
    {
        case DEVICE_RUNNING_STATUS:  //1：MEC 运行状态信息；
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[running-status]" << hint << "[infoId]" << DEVICE_RUNNING_STATUS;
            }
            runningStatusData.sensorNum = 0;
            runningStatusData.sensorNumEmpty = false;
        }
            break;
        case CONNECTED_DEVICE_RUNNING_STATUS:  //3：接入 MEC 的设备运行状态信息；
        {

            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[running-status]" << hint << "[infoId]"
                                   << CONNECTED_DEVICE_RUNNING_STATUS;
            }
            std::vector<MsgDeviceStatus> statusList;
            std::pair<std::string, TABLE_TYPE> tableInfo;
            tableInfo.first = getConfiger().configerMec.configerDb.DbDeviceStatusTableName;
            tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;

            DBUtils::getDBInfo(tableInfo, statusList);

            for (auto status: statusList)
            {
//                OM_MEC_DEBUG_PRINT << "[running-status]" << "status:" << status.to_string();
                if (DEVICE_TYPE_DB_MEC == status.deviceType || 0 == status.deviceType)
                {
                    continue;
                }
                SensorStatusData sensorStatusData;
                sensorStatusData.sensorSn = status.deviceID;
                int deviceType = sensorStatusData.deviceType = status.deviceType;
                switch ( status.deviceStatus)
                {
                    case mec::db::DEVICE_STATUS_DB_ON:
                        sensorStatusData.status  = RUN_STATUS_NORMAL;
                        break;
                    case mec::db::DEVICE_STATUS_DB_OFF:
                        sensorStatusData.status  = RUN_STATUS_FAULT;
                        break;
                    default:
                        sensorStatusData.status  = RUN_STATUS_FAULT;
                        break;
                }
                FaultData faultData;

                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT  << "[running-status]" << "active:" << status.active;
                }
                switch ( status.active)
                {
                    case mec::db::DEVICE_ACTIVE_DB_ON:
                        sensorStatusData.active  = NETWORK_STATUS_ONLINE;
                        break;
                    case mec::db::DEVICE_ACTIVE_DB_OFF:
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_MEC_ERROR_PRINT  << "[running-status]" << "sensor-device offline";
                        }

                        sensorStatusData.active  = NETWORK_STATUS_OFFLINE;
                        faultData.deviceSn = status.deviceID;
                        faultData.deviceType = (DeviceType)status.deviceType;
                        switch (deviceType)
                        {
                            case OM_DEVICE_TYPE_CAMERA:
                                faultData.faultType = CAMERA_FAILURE;
                                faultData.faultTypeEmpty = false;
                                faultData.faultTime = afl::util::TimeStamp::now(true).millSeconds();
                                faultData.faultTimeEmpty = false;
                                faultData.faultDescription = "sensor-camera offline";
//                                if (getConfiger().enableDebugPrint)
//                                {
//                                    OM_MEC_DEBUG_PRINT  << "[running-status]"
//                                                       << "sensor-camera offline";
//                                }
                                faultData.faultDescriptionEmpty = false;
                                hasFaultData = true;
                                break;
                            case OM_DEVICE_TYPE_MMWRADAR:
                                faultData.faultType = MILLIMETER_WAVE_RADAR_FAILURE;
                                faultData.faultTypeEmpty = false;
                                faultData.faultTime = afl::util::TimeStamp::now(true).millSeconds();
                                faultData.faultTimeEmpty = false;
                                faultData.faultDescription = "sensor-wave-radar offline";
//                                if (getConfiger().enableDebugPrint)
//                                {
//                                    OM_MEC_DEBUG_PRINT  << "[running-status]"
//                                                       << "sensor-wave-radar offline";
//                                }
                                faultData.faultDescriptionEmpty = false;
                                hasFaultData = true;
                                break;
                            case OM_DEVICE_TYPE_LIDAR:
                                faultData.faultType = LASER_RADAR_FAILURE;
                                faultData.faultTypeEmpty = false;
                                faultData.faultTime = afl::util::TimeStamp::now(true).millSeconds();
                                faultData.faultTimeEmpty = false;
                                faultData.faultDescription = "sensor-lidar offline";
//                                if (getConfiger().enableDebugPrint)
//                                {
//                                    OM_MEC_DEBUG_PRINT  << "[running-status]"
//                                                       << "sensor-lidar offline";
//                                }
                                faultData.faultDescriptionEmpty = false;
                                hasFaultData = true;
                                break;
                            default:
                                faultData.faultType = FAULT_TYPE_OTHER;
                                faultData.faultTypeEmpty = false;
                                hasFaultData = false;
                                break;
                        }
                    }
                    break;
                    default:
                        sensorStatusData.deviceType  = NETWORK_STATUS_ONLINE;
                        break;
                }
                if(hasFaultData)
                {
                    runningStatusData.faultList.push_back(faultData);
                    runningStatusData.faultListEmpty = false;
                }

                runningStatusData.sensorStatusList.push_back(sensorStatusData);
            }
            if(runningStatusData.sensorStatusList.size() > 0)
            {
                runningStatusData.sensorStatusListEmpty = false;
            }
            runningStatusData.sensorNum = runningStatusData.sensorStatusList.size();
            runningStatusData.sensorNumEmpty = false;
        }
        break;
        default:
        {
            runningStatusData.sensorNum = 0;
            runningStatusData.sensorNumEmpty = false;
        }
        break;

    }
    switch (infoId)
    {
    case DEVICE_RUNNING_STATUS:  //1：MEC 运行状态信息；
        {
            if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Running_Status_Query_Mec_ACK)
            {
                return;
            }
        }
        break;
    case CONNECTED_DEVICE_RUNNING_STATUS:  //3：接入 MEC 的设备运行状态信息；
        {
            if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Running_Status_Query_Sensor_ACK)
            {
                return;
            }
        }
        break;
    default:
        break;
    }

    if(!hasFaultData)
    {
        runningStatusData.faultListEmpty = false;
    }
    runningStatusData.ack = true;
    runningStatusData.ackEmpty = false;
    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Running_Status ,runningStatusData, hint);
}
int OM_COMPONENT::transSensorType(int statusDeviceType)
{
    int deviceType;
    switch ( statusDeviceType)
    {
        case mec::db::DEVICE_TYPE_DB_CAMRA:
            deviceType   = OM_DEVICE_TYPE_CAMERA;
            break;
        case mec::db::DEVICE_TYPE_DB_MW_RADAR:
            deviceType  = OM_DEVICE_TYPE_MMWRADAR;
            break;
        case mec::db::DEVICE_TYPE_DB_RADAR:
            deviceType  = OM_DEVICE_TYPE_LIDAR;
            break;
        default:
            deviceType  = OM_DEVICE_TYPE_OTHERDEVICE;
            break;
    }

    return deviceType;
}
/**
 * @func 设备基础信息
 */
void OM_COMPONENT::publishDeviceBasicInfoData(DeviceBaseInfoQueryData& deviceBaseInfoQueryData,
                                                  DeviceBasicInfoDeviceTypeEnum deviceBasicInfoDeviceType,
                                                  BASIC_INFO_OPT_TYPE basicInfoOptType)
{
    if (!m_MqttConnected)
    {
        return;
    }

    string hint = "publish Device-Basic info";

    DeviceBasicInfoData deviceBasicInfoData;
    deviceBasicInfoData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    if(BASIC_INFO_OPT_TYPE_QUERY == basicInfoOptType)
    {
        deviceBasicInfoData.seqNum = deviceBaseInfoQueryData.seqNum;
    }
    else
    {
        deviceBasicInfoData.seqNum = afl::util::Srand::srandStr(32);
    }

    deviceBasicInfoData.rscuEsn     =  getConfiger().rscuEsn;
    if(getConfiger().configerMec.configerOmCommon.regionId.size() > 0)
    {
        deviceBasicInfoData.regionId   = getConfiger().configerMec.configerOmCommon.regionId;
        deviceBasicInfoData.regionIdEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.roadId > 0)
    {
        deviceBasicInfoData.roadId     = getConfiger().configerMec.configerOmCommon.roadId;
        deviceBasicInfoData.roadIdEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.roadName.size() > 0)
    {
        deviceBasicInfoData.roadName    = getConfiger().configerMec.configerOmCommon.roadName;
        deviceBasicInfoData.roadNameEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.roadType > 0)
    {
        deviceBasicInfoData.roadType = getConfiger().configerMec.configerOmCommon.roadType;
        deviceBasicInfoData.roadTypeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.crossId.size() > 0)
    {
        deviceBasicInfoData.crossId = getConfiger().configerMec.configerOmCommon.crossId;
        deviceBasicInfoData.crossIdEmpty = false;
    }

    if(getConfiger().configerMec.configerOmCommon.crossType > 0)
    {
        deviceBasicInfoData.crossType = getConfiger().configerMec.configerOmCommon.crossType;
        deviceBasicInfoData.crossTypeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.crossName.size() > 0)
    {
        deviceBasicInfoData.crossName = getConfiger().configerMec.configerOmCommon.crossName;
        deviceBasicInfoData.crossNameEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.linkId > 0)
    {
        deviceBasicInfoData.linkId    = getConfiger().configerMec.configerOmCommon.linkId;
        deviceBasicInfoData.linkIdEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.linkName.size() > 0)
    {
        deviceBasicInfoData.linkName    = getConfiger().configerMec.configerOmCommon.linkName;
        deviceBasicInfoData.linkNameEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.latitude > 0)
    {
        deviceBasicInfoData.latitude = getConfiger().configerMec.configerOmCommon.latitude;
        deviceBasicInfoData.latitudeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.longitude > 0)
    {
        deviceBasicInfoData.longitude = getConfiger().configerMec.configerOmCommon.longitude;
        deviceBasicInfoData.longitudeEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.elevation > 0)
    {
        deviceBasicInfoData.elevation = getConfiger().configerMec.configerOmCommon.elevation;
        deviceBasicInfoData.elevationEmpty = false;
    }
    deviceBasicInfoData.deviceType = OM_DEVICE_TYPE_RSCU;

    if(getConfiger().configerMec.configerOmCommon.supplier.size() > 0)
    {
        deviceBasicInfoData.supplier = getConfiger().configerMec.configerOmCommon.supplier;
        deviceBasicInfoData.supplierEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.owner.size() > 0)
    {
        deviceBasicInfoData.owner = getConfiger().configerMec.configerOmCommon.owner;
        deviceBasicInfoData.ownerEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.protocolVersion.size() > 0)
    {
        if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
        {
            deviceBasicInfoData.protocolVersion = PROTOCOL_VERSION;
            deviceBasicInfoData.protocolVersionEmpty = false;
        }
        else
        {
            deviceBasicInfoData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
            deviceBasicInfoData.protocolVersionEmpty = false;
        }
    }
    if(getConfiger().configerMec.configerOmCommon.imei.size() > 0)
    {
        deviceBasicInfoData.imei   = getConfiger().configerMec.configerOmCommon.imei;
        deviceBasicInfoData.imeiEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.imei.size() > 0)
    {
        deviceBasicInfoData.iccId   = getConfiger().configerMec.configerOmCommon.iccId;
        deviceBasicInfoData.iccIdEmpty = false;
    }
    deviceBasicInfoData.rscuStatus = RUN_STATUS_NORMAL;
    deviceBasicInfoData.active =  NETWORK_STATUS_ONLINE ;
    if(getConfiger().configerMec.configerOmCommon.transProtocal > 0)
    {
        deviceBasicInfoData.transProtocal = (TransProtocolType)getConfiger().configerMec.configerOmCommon.transProtocal;
        deviceBasicInfoData.transProtocalEmpty = false;
    }

    if(getConfiger().configerMec.configerOmCommon.softwareVersion.size() > 0)
    {
        deviceBasicInfoData.softwareVersion = getConfiger().configerMec.configerOmCommon.softwareVersion;
        deviceBasicInfoData.softwareVersionEmpty = false;
    }
    if(getConfiger().configerMec.configerOmCommon.hardwareVersion.size() > 0)
    {
        deviceBasicInfoData.hardwareVersion = getConfiger().configerMec.configerOmCommon.hardwareVersion;
        deviceBasicInfoData.hardwareVersionEmpty = false;
    }
    deviceBasicInfoData.rsuNum = 0;
    switch (deviceBasicInfoDeviceType)
    {
        case DEVICE_BASIC_INFO_MEC:
            deviceBasicInfoData.sensorNum = 0;
            break;
        case DEVICE_BASIC_INFO_SENSOR:
        {
            std::vector<MsgDeviceStatus> statusList;
            std::pair<std::string, TABLE_TYPE> tableInfo;
            tableInfo.first = getConfiger().configerMec.configerDb.DbDeviceStatusTableName;
            tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;
            DBUtils::getDBInfo(tableInfo, statusList);

            OmWorkParamConfiger  omWorkParamConfiger;
            WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
            int sensorDeviceNum = 0;

            for (auto status: statusList)
            {
                SensorData sensorData;
                //OM_MEC_DEBUG_PRINT << "status:" << status.to_string();
                sensorData.sensorSn = status.deviceID;
//                OM_MEC_DEBUG_PRINT    << "[sensorSn]" << sensorData.sensorSn ;
                switch ( status.deviceType)
                {
                    case mec::db::DEVICE_TYPE_DB_CAMRA:
                        sensorData.deviceType  = OM_DEVICE_TYPE_CAMERA;
                        sensorDeviceNum++;
                        break;
                    case mec::db::DEVICE_TYPE_DB_MW_RADAR:
                        sensorData.deviceType  = OM_DEVICE_TYPE_MMWRADAR;
                        sensorDeviceNum++;
                        break;
                    case mec::db::DEVICE_TYPE_DB_RADAR:
                        sensorData.deviceType  =  OM_DEVICE_TYPE_LIDAR;
                        sensorDeviceNum++;
                        break;
                    default:
                        sensorData.deviceType  =  OM_DEVICE_TYPE_OTHERDEVICE;
                        continue;
                        break;

                }
//                OM_MEC_DEBUG_PRINT    << "[deviceType]" << sensorData.deviceType ;
                bool isGetSensorDeviceLatNotEmpty  = true;  //获取的值非空，存在
                bool isGetSensorDeviceLonNotEmpty  = true;
                bool isGetSensorDeviceEleNotEmpty  = true;
//                OM_MEC_DEBUG_PRINT    << "[latitude]" << status.latitude;
                if(status.latitude > 17 && status.latitude < 54)
                {
//                    OM_MEC_ERROR_PRINT  << "[notice]status.latitude > 17 && status.latitude < 54!";
                    sensorData.latitude = status.latitude;
                    sensorData.latitudeEmpty = false;
                }
                else
                {
                    isGetSensorDeviceLatNotEmpty = true;
                }
//                OM_MEC_DEBUG_PRINT    << "[longitude]" << status.longitude;
                if(status.longitude > 72 && status.longitude < 136)
                {
//                    OM_MEC_ERROR_PRINT  << "[notice]status.longitude > 72 && status.longitude < 136)";
                    sensorData.longitude = status.longitude;
                    sensorData.longitudeEmpty = false;
                }
                else
                {
                    isGetSensorDeviceLonNotEmpty  = false;
                }
//                OM_MEC_DEBUG_PRINT    << "[altitude]" << status.altitude;
                if(status.altitude > 0)
                {
//                    OM_MEC_ERROR_PRINT  << "[notice]status.altitude > 0";
                    sensorData.elevation = status.altitude;
                    sensorData.elevationEmpty = false;
                }
                else
                {
                    isGetSensorDeviceEleNotEmpty  = false;
                }

                if(!isGetSensorDeviceLatNotEmpty||!isGetSensorDeviceLonNotEmpty||!isGetSensorDeviceEleNotEmpty)
                {
                    for(auto v: omWorkParamConfiger.sensorDeviceWorkParamList)
                    {
                        if(v.deviceEsn == status.deviceID)
                        {
                            if(!isGetSensorDeviceLatNotEmpty)
                            {
                                sensorData.latitude = v.deviceLatitude;
                                sensorData.latitudeEmpty = false;
                            }
                            if(!isGetSensorDeviceLatNotEmpty)
                            {
                                sensorData.longitude = v.deviceLongitude;
                                sensorData.longitudeEmpty = false;
                            }
                            if(!isGetSensorDeviceEleNotEmpty)
                            {
                                sensorData.elevation = v.deviceAltitude;
                                sensorData.elevationEmpty = false;
                            }
                            if(v.radarCrossId.length() > 0)
                            {
                                sensorData.crossId = v.radarCrossId;
                                sensorData.crossIdEmpty = false;
                            }
                            else
                            {
                                sensorData.crossIdEmpty = false;
                            }

                            break;
                        }
                    }
                }

                deviceBasicInfoData.sensorlist.push_back(sensorData);
                deviceBasicInfoData.sensorlistEmpty = false;
            }
            deviceBasicInfoData.sensorNum = sensorDeviceNum ;
            deviceBasicInfoData.sensorNumEmpty = false;
        }
            break;
        default:
            deviceBasicInfoData.sensorNum = 0;
            break;
    }
    if(BASIC_INFO_OPT_TYPE_QUERY == basicInfoOptType)
    {
        deviceBasicInfoData.ack = false;
        deviceBasicInfoData.ackEmpty = false;
    }
    else
    {
        deviceBasicInfoData.ack = true;
        deviceBasicInfoData.ackEmpty = false;
    }


    switch (basicInfoOptType)
    {
    case BASIC_INFO_OPT_TYPE_QUERY://查询
        switch (deviceBasicInfoDeviceType)
        {
        case DEVICE_BASIC_INFO_MEC:
            {
                if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Query_Mec_Ack)
                {
                    return;
                }
            }
            break;
        case DEVICE_BASIC_INFO_SENSOR:
            {
                if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Query_Sensor_Ack)
                {
                    return;
                }
            }
            break;
        default:
            break;
        }
        break;
    case BASIC_INFO_OPT_TYPE_TIMER://定时
        {
            if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Ack)
            {
                return;
            }
        }
        break;
    default:

        break;

    }


    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo, deviceBasicInfoData, hint);
}

std::string OM_COMPONENT::getIPFromString(const std::string& input)
{
    std::stringstream ss(input);
    std::string protocol, address, port;
    std::getline(ss, protocol, ':');
    std::getline(ss, address, ':');
    std::getline(ss, port);

    // Remove the leading "//" from the address
    address = address.substr(2);

    return address;
}
//配置查询
void OM_COMPONENT::subscribeConfigQueryData(const json & subscribeJson)
{
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Query_Down || !m_RegisterFlag)
    {
        return;
    }
    ConfigQueryData configQueryData;
    try
    {
        configQueryData = subscribeJson;
    } catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }

    if(m_MqttClientConfig.rscuEsn == configQueryData.rscuEsn)
    {
        if(configQueryData.actionName == "baseInfoEnquire")
        {

            publishConfigQueryAckData(configQueryData, CONFIG_QUERY_INFO_OPT_TYPE_QUERY_ACK);
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[error]rscuEsn no success!";
    }

}
//配置查询响应
void OM_COMPONENT::publishConfigQueryAckData(ConfigQueryData &configQueryData, CONFIG_QUERY_INFO_OPT_TYPE opt_type)
{
    if (!m_MqttConnected)
    {
        return;
    }

    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Query_Down_Ack || !m_RegisterFlag)
    {
        return;
    }
    std::string hint = "publish Config-Query-Ack info";

    ConfigQueryAckData configQueryAckData;
    configQueryAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    configQueryAckData.seqNum    = configQueryData.seqNum;
    configQueryAckData.rscuEsn   = getConfiger().rscuEsn;
    configQueryAckData.pointNo   = getConfiger().configerMec.configerOmCommon.pointNo;
    configQueryAckData.pointName = getConfiger().configerMec.configerOmCommon.pointName;
    configQueryAckData.longitude = getConfiger().configerMec.configerOmCommon.longitude;
    configQueryAckData.latitude     = getConfiger().configerMec.configerOmCommon.latitude;
    configQueryAckData.altitude = getConfiger().configerMec.configerOmCommon.elevation;
    configQueryAckData.height   = getConfiger().configerMec.configerOmCommon.height;
    configQueryAckData.siteType  = (SiteTypeEnum)getConfiger().configerMec.configerOmCommon.siteType;
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[config-query-ack]" << hint << "[timeStamp]" << configQueryAckData.timeStamp;
    }

    if(opt_type == CONFIG_QUERY_INFO_OPT_TYPE_QUERY_ACK)
    {
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down_Ack , configQueryAckData, hint);
    }
    if(opt_type == CONFIG_QUERY_INFO_OPT_TYPE_DEVICE_POS)
    {
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Position_Up , configQueryAckData, hint);
    }
}

void OM_COMPONENT::updateLogLevel(const std::string& newLogLevel)
{
    std::string filename = "/home/airos/os/setup.bash";
    std::ifstream file(filename);
    std::stringstream buffer;

    if (file.is_open())
    {
        std::string line;
        bool found = false;

        while (std::getline(file, line))
        {
            // 检查行是否包含 "GLOG_minloglevel"
            if (line.find("GLOG_minloglevel") != std::string::npos)
            {
                // 修改值
                size_t pos = line.find("=");
                if (pos != std::string::npos)
                {
                    line = line.substr(0, pos + 1) + newLogLevel; // 更新值
                    found = true;
                }
            }
            buffer << line << std::endl; // 将行写入缓冲区
        }
        file.close();

        // 如果找到了并修改了值，则写回文件
        if (found)
        {
            std::ofstream outFile(filename);
            if (outFile.is_open())
            {
                outFile << buffer.str(); // 写入缓冲区内容
                outFile.close();
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_SUCCESS_PRINT <<  "Log level updated to: " << newLogLevel;
                }
            }
            else
            {
                OM_MEC_ERROR_PRINT <<  "Unable to open file for writing.";
            }
        }
        else
        {
            OM_MEC_ERROR_PRINT <<  "GLOG_minloglevel not found in the file.";
        }
    } else {
        OM_MEC_ERROR_PRINT <<  "Unable to open file.";
    }
}
/**
 * @func: 配置修改
 * @param configQueryData
 */
void OM_COMPONENT::subscribeConfigUpdateData(const json & subscribeJson)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update || !m_RegisterFlag)
    {
        return;
    }
    std::string hint = "Config-Update";
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[hint]" << hint;
    }
    ConfigUpdateData configUpdateData;
    ConfigUpdateAckData configUpdateAckData;
    configUpdateAckData.timeStamp  = afl::util::TimeStamp::now(true).millSeconds();

    configUpdateAckData.rscuEsn = m_MqttClientConfig.rscuEsn;
    try
    {
        configUpdateData = subscribeJson;
    } catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        if( subscribeJson.find("seqNum") != subscribeJson.end())
        {
            configUpdateAckData.seqNum = subscribeJson["seqNum"];
            configUpdateAckData.status = STATUS_FAILURE;
        }
        else
        {
            OM_MEC_ERROR_PRINT << "[error]not find seqNum!";
            return;
        }
        if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
        }
         return ;
    }
    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
    configUpdateAckData.seqNum = configUpdateData.seqNum;
    if(configUpdateData.protocolVersion != PROTOCOL_VERSION)
    {
        configUpdateAckData.status = STATUS_FAILURE;
        hint = "[error]protocolversion is not V1.0";
        OM_MEC_ERROR_PRINT << hint;
        if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
        }
        return;
    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_ERROR_PRINT << "[ConfigUpdateData]" << configUpdateData.to_string();
    }
    if(configUpdateData.time == 0)
    {
        configUpdateAckData.status = STATUS_SUCCESS;

        processConfigUpdate(configUpdateData, configUpdateAckData);
    }
    else if(configUpdateData.time > 0)
    {
        if(configUpdateData.time  < nowTime)
        {
            configUpdateAckData.status = STATUS_FAILURE;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[error]time(" << configUpdateData.time << ") < nowtime(" << nowTime
                                   << "),no need update!";
            }
            hint = "[error]time < nowtime";
            if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
            {
                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
            }
        }
        else
        {
            configUpdateAckData.status = STATUS_SUCCESS;
            uint64_t updateInterTime = (configUpdateData.time - nowTime)/1000;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[notice]start timer, interval: " << updateInterTime << " s";
            }
            m_ConfigeUpdateTimer =  m_EventloopOmMec->addTimer(
                    std::bind(&OM_COMPONENT::processConfigUpdate, this, configUpdateData, configUpdateAckData), updateInterTime, true);
            configUpdateAckData.status = STATUS_SUCCESS;
            if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
            {
                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
            }
        }
    }
    else
    {
        configUpdateAckData.status = STATUS_FAILURE;
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[error]time < nowtime,no need update!";
        }
        hint = "[error]time < 0";
        if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
        }
    }
}

bool OM_COMPONENT::processConfigUpdate(ConfigUpdateData& configUpdateData,  ConfigUpdateAckData& configUpdateAckData)
{
    if(m_ConfigeUpdateTimer > 0)
    {
        m_EventloopOmMec->cancelTimer(m_ConfigeUpdateTimer);
    }
    if (!m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update_Ack)
    {
        return false;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[msg]" << configUpdateData.to_string();
    }
    if(m_MqttClientConfig.rscuEsn == configUpdateData.rscuEsn)
    {
        if(configUpdateData.actionName == "baseInfoEnquire")
        {
            getConfiger().configerMec.configerOmCommon.protocolVersion  = configUpdateData.protocolVersion;
            if(getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp == 0)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp = 0";
                }
                if(configUpdateData.runningInfoRate == 0)
                {
                    if(m_TimerPublishDeviceRunningStatusData > 0)
                    {
                       getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp  = configUpdateData.runningInfoRate;
                        saveConfiger();
                        m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceRunningStatusData);
                        m_TimerPublishDeviceRunningStatusData = -1;
                    }
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                    }
                }
                else if(configUpdateData.runningInfoRate > 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.runningInfoRate = 0";
                    }
                    DeviceBaseInfoQueryData deviceBaseInfoQueryData;
                    deviceBaseInfoQueryData.infoId = DEVICE_RUNNING_STATUS;
                    getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp = configUpdateData.runningInfoRate;
                    saveConfiger();
                    m_TimerPublishDeviceRunningStatusData = m_EventloopOmMec->addTimer(
                            std::bind(&OM_COMPONENT::publishRunningStatusData, this, deviceBaseInfoQueryData, DEVICE_BASIC_INFO_MEC),
                           getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp, true);
                }
                else
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                    }
                }

            }
            else if(getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp > 0)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp > 0";
                }
                if(configUpdateData.runningInfoRate == 0)
                {
                   getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp  = configUpdateData.runningInfoRate;
                    saveConfiger();
                    m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceRunningStatusData);
                    m_TimerPublishDeviceRunningStatusData = -1;
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.runningInfoRate = 0";
                    }
                }
                else if(configUpdateData.runningInfoRate > 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.runningInfoRate > 0";
                    }
                    if(getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp  != configUpdateData.runningInfoRate)
                    {
                        if(m_TimerPublishDeviceRunningStatusData > 0)
                        {
                            m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceRunningStatusData);
                            m_TimerPublishDeviceRunningStatusData = -1;
                        }
                       getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp  = configUpdateData.runningInfoRate;
                        saveConfiger();
                        DeviceBaseInfoQueryData deviceBaseInfoQueryData;
                        deviceBaseInfoQueryData.infoId = DEVICE_RUNNING_STATUS;
                        m_TimerPublishDeviceRunningStatusData = m_EventloopOmMec->addTimer(
                                std::bind(&OM_COMPONENT::publishRunningStatusData, this, deviceBaseInfoQueryData, DEVICE_BASIC_INFO_MEC),
                               getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp, true);
                    }
                    else
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "[notice]period RunningInfo equal, no need do!";
                        }
                    }
                }
                else
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]runningInfoRate < 0, no need do!";
                    }
                }
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                }
            }

            if(getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp == 0)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp = 0";
                }
                if(configUpdateData.heartRate == 0)
                {
                    if(m_TimerPublishDeviceHeartBeatData > 0)
                    {
                       getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp  = configUpdateData.heartRate;
                        saveConfiger();
                        m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceHeartBeatData);
                        m_TimerPublishDeviceHeartBeatData = -1;
                    }
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                    }
                }
                else if(configUpdateData.heartRate > 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.heartRate > 0";
                    }
                   getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp  = configUpdateData.heartRate;
                    saveConfiger();
                    m_TimerPublishDeviceHeartBeatData = m_EventloopOmMec->addTimer(
                            std::bind(&OM_COMPONENT::publishDeviceHeartbeatData, this),
                           getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp, true);
                }
                else
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                    }
                }

            }
            else if(getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp > 0)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp > 0";
                }
                if(configUpdateData.heartRate == 0)
                {
                   getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp  = configUpdateData.runningInfoRate;
                    saveConfiger();
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.heartRate = 0";
                    }
                    m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceHeartBeatData);
                    m_TimerPublishDeviceHeartBeatData = -1;
                }
                else if(configUpdateData.heartRate > 0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "configUpdateData.heartRate > 0";
                    }
                    if(getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp  != configUpdateData.heartRate)
                    {
                        if(m_TimerPublishDeviceHeartBeatData > 0)
                        {
                            m_EventloopOmMec->cancelTimer(m_TimerPublishDeviceHeartBeatData);
                            m_TimerPublishDeviceHeartBeatData = -1;
                        }
                        getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp  = configUpdateData.heartRate;
                        saveConfiger();
                        m_TimerPublishDeviceHeartBeatData = m_EventloopOmMec->addTimer(
                                std::bind(&OM_COMPONENT::publishDeviceHeartbeatData, this),
                               getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp, true);
                    }
                    else
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "[notice]period HearBeat equal, no need do!";
                        }
                    }
                }
                else
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[notice]heartRate < 0, no need do!";
                    }
                }
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[notice]no need do!";
                }
            }

            if(getConfiger().configerMec.configerEnable.Enable_Change_Cloud_Address)
            {
                if(configUpdateData.addressChg.size() > 0)
                {
                    std::string newUrl = configUpdateData.addressChg.substr(
                            configUpdateData.addressChg.find("//") == std::string::npos ? 0 :  configUpdateData.addressChg.find("//") + 2);

                    OmWorkParamConfiger  omWorkParamConfiger;
                    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
                    std::string maintenanceCloudUrl = omWorkParamConfiger.mecDeviceWorkParam.maintenanceCloudUrl;

                    std::string oldUrl = maintenanceCloudUrl.substr(
                            maintenanceCloudUrl.find("//") == std::string::npos ? 0 :  maintenanceCloudUrl.find("//") + 2);
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[newUrl]" << newUrl << "[oldurl]" << oldUrl;
                    }
                    if(newUrl != oldUrl)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "[notice] will update url! wait 2 seconds, kill om_cloud!";
                        }
                        omWorkParamConfiger.mecDeviceWorkParam.maintenanceCloudUrl = configUpdateData.addressChg;
                        WorkParam::updateWorkParamFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
                        m_EventloopOmMec->addTimer([this]()
                          {
                              std::string rebootOmCloudCmd = "kill -9 `ps -ef|grep om.dag|grep -v grep|awk '{print $2}'`";
                              int retRebootOmCloud = system(rebootOmCloudCmd.c_str());
                              if(retRebootOmCloud != 0)
                              {
                                  OM_MEC_ERROR_PRINT << "[error]reboot failure!";
                              }
                          }, 2, true);
                    }
                }
            }

            getConfiger().configerMec.configerOmCommon.addressIP        = configUpdateData.addressIP;
            getConfiger().configerMec.configerOmCommon.netMask         = configUpdateData.netMask;
            getConfiger().configerMec.configerOmCommon.gateway         = configUpdateData.gateway;
            std::string logLevelStr="0";
            switch(configUpdateData.logLevel)
            {
                case LOG_LEVEL_DEBUG:
                    logLevelStr="0";
                    break;
                case LOG_LEVEL_INFO:
                    logLevelStr="1";
                    break;
                case LOG_LEVEL_WARN:
                    logLevelStr="2";
                    break;
                case LOG_LEVEL_ERROR:
                    logLevelStr="3";
                    break;
                case LOG_LEVEL_NO_LOG:
                    logLevelStr="3";
                    break;
                default:
                    logLevelStr="0";
                    break;
            }
            updateLogLevel(logLevelStr);
            getConfiger().configerMec.configerOmCommon.logLevel        = configUpdateData.logLevel;

            saveConfiger();
            configUpdateAckData.status = STATUS_SUCCESS;
            std::string hint = "Config-Update-Ack";
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "[hint]" << hint;
            }

            if(configUpdateData.power != POWER_RESTART)
            {
                mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
            }
            else
            {
                switch(configUpdateData.power)
                {
                    case POWER_ON :
                        break;
                    case POWER_OFF:
                        break;
                    case POWER_RESTART:
                    {
//                        std::string rebootOmMecCmd = "kill -9 `ps -ef|grep om_mec.dag|grep -v grep|awk '{print $2}'`";
//                        std::string rebootOmCloudCmd = "kill -9 `ps -ef|grep om_cloud.dag|grep -v grep|awk '{print $2}'`";
//                        int retRebootOmMec = system(rebootOmCloudCmd.c_str());
//                        int retRebootOmCloud = system(rebootOmMecCmd.c_str());
//                        if(retRebootOmMec != 0 || retRebootOmCloud != 0)
//                        {
//                            OM_MEC_ERROR_PRINT << "[error]reboot failure!";
//                        }
                        OmWorkParamConfiger  omWorkParamConfiger;
                        WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_WARN_PRINT << "[omWorkParamConfiger]" << omWorkParamConfiger.to_string();
                        }
                        SshInfo sshInfo;
                        sshInfo.host = "127.0.0.1";
                        sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
                        sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
                        OM_MEC_ERROR_PRINT << "[user]" << sshInfo.user << "[password]" << sshInfo.password;
                        OM_MEC_ERROR_PRINT << "[user]" << sshInfo.user << "[password]" << sshInfo.password;
                        sshInfo.execCmd = "sleep 3 && reboot";
                        sshInfo.flagReboot = true;
                        if(!DBUtils::sshExec(sshInfo))
                        {
                            OM_MEC_ERROR_PRINT << "[error]reboot device failure!";
                            configUpdateAckData.status = STATUS_FAILURE;
                            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
                        }
                        else
                        {
                            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
                        }
                    }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[error]rscuEsn no success!";
        configUpdateAckData.status = STATUS_FAILURE;
        std::string hint = "Config-Update-Ack";
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[hint]" << hint;
        }
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
    }

    return true;
}

void OM_COMPONENT::pulishConfigUpdateData(ConfigUpdateData& configUpdateData)
{
    if (!m_MqttConnected)
    {
        return;
    }

    string hint = "Config-Update-Up";
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[hint]" << hint;
    }
    configUpdateData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    configUpdateData.rscuEsn = getConfiger().rscuEsn;
    configUpdateData.actionName = "baseInfoEnquire";
    if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
    {
        configUpdateData.protocolVersion =  PROTOCOL_VERSION;
    }
    else
    {
        configUpdateData.protocolVersion =  getConfiger().configerMec.configerOmCommon.protocolVersion;
    }


    configUpdateData.runningInfoRate =getConfiger().configerMec.configerPublishPeriod.periodRunningInfoUp;
    configUpdateData.heartRate =getConfiger().configerMec.configerPublishPeriod.periodHeartBeatUp;
    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);

	std::string maintenanceCloudUrl = omWorkParamConfiger.mecDeviceWorkParam.maintenanceCloudUrl;
	configUpdateData.addressChg = maintenanceCloudUrl;
    configUpdateData.addressIP = getConfiger().configerMec.configerOmCommon.addressIP;
    configUpdateData.netMask = getConfiger().configerMec.configerOmCommon.netMask;
    configUpdateData.gateway = getConfiger().configerMec.configerOmCommon.gateway;
    configUpdateData.logLevel = (LogLevelEnum)getConfiger().configerMec.configerOmCommon.logLevel;
    configUpdateData.time = 0;
    configUpdateData.power = (PowerEnum)0;
    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateData, hint);
}

void OM_COMPONENT::pulishConfigUpdateAckData(ConfigUpdateData& configUpdateData)
{
    string hint = "Config-Update-Ack";
	if(getConfiger().enableDebugPrint)
    {
    	OM_MEC_DEBUG_PRINT << "[hint]" << hint;
	}
    ConfigUpdateAckData configUpdateAckData;
    configUpdateAckData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    configUpdateAckData.seqNum = configUpdateData.seqNum;
    configUpdateAckData.rscuEsn = getConfiger().rscuEsn;
    configUpdateAckData.status = STATUS_SUCCESS;

    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack, configUpdateAckData, hint);
}
/**
 * @func：获取运行状态
 */
void OM_COMPONENT::getPerformanceData()
{
    string hint = "publish Performence info";

    PerformanceData performanceData;
    auto &runningInfo = performanceData.runningInfo;
    PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
    if(!PerformenceUtils::getCpuInfo(runningInfo.cpuInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get cpu-info failure!";
    }

    if(!PerformenceUtils::getMemInfo(runningInfo.memInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get mem-info failure!";
    }

    if(!PerformenceUtils::getDiskInfo(runningInfo.diskInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get disk-info failure!";
    }

    if(!PerformenceUtils::getGpuInfo(runningInfo.gpuInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get gpu-info failure!";
    }

    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    std::string nic = omWorkParamConfiger.mecDeviceWorkParam.nic;
    if(!PerformenceUtils::getNetInfo(runningInfo.netInfo, nic))
    {
        OM_MEC_ERROR_PRINT << "[error]get net-info failure!";
    }

    std::lock_guard<std::mutex> lock(m_MutexPerformanceData);
    {
        m_PerformanceData = performanceData;
    }
}


/**
 * @func 性能管理，运行状态
 * @param deviceManagementData
 */
void OM_COMPONENT::publishPerformenceData(DeviceBaseInfoQueryData deviceBaseInfoQueryData, bool periodPushFlag)
{
    if (!m_MqttConnected)
    {
        return;
    }
    if(!getConfiger().configerMec.configerEnable.Enable_Performence || !m_RegisterFlag)
    {
        return;
    }
    string hint = "publish Performence info";

    PerformanceData performanceData;
    performanceData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[running-info]" << hint << "[timeStamp]" << performanceData.timeStamp;
    }
    if(periodPushFlag)
    {
        performanceData.seqNum = afl::util::Srand::srandStr(32);
    }
    else
    {
        performanceData.seqNum = deviceBaseInfoQueryData.seqNum;
    }

    performanceData.rscuEsn = getConfiger().rscuEsn;
    if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
    {
        performanceData.protocolVersion =  PROTOCOL_VERSION;
    }
    else
    {
        performanceData.protocolVersion =  getConfiger().configerMec.configerOmCommon.protocolVersion;
    }

    auto &runningInfo = performanceData.runningInfo;
    PerformenceUtils::printInfo = getConfiger().enableDebugPrint;
    if(!PerformenceUtils::getCpuInfo(runningInfo.cpuInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get cpu-info failure!";
    }

    if(!PerformenceUtils::getMemInfo(runningInfo.memInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get mem-info failure!";
    }

    if(!PerformenceUtils::getDiskInfo(runningInfo.diskInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get disk-info failure!";
    }

     if(!PerformenceUtils::getGpuInfo(runningInfo.gpuInfo))
    {
        OM_MEC_ERROR_PRINT << "[error]get gpu-info failure!";
    }
    
    std::string nic = m_OmWorkParamConfiger.mecDeviceWorkParam.nic;
    OM_MEC_ERROR_PRINT << "[nic]" << nic;
    if(!PerformenceUtils::getNetInfo(runningInfo.netInfo, nic))
    {
        OM_MEC_ERROR_PRINT << "[error]get disk-info failure!";
    }
    // getRuningInfoFromDbMecStatusTable(runningInfo);
    if(runningInfo.cpuInfo.uti == "0" || runningInfo.diskInfo.total <= 1.0 || runningInfo.memInfo.total <= 1.0 || runningInfo.netInfo.txByte <= 1.0)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[running-info] no data from db!" ;
        }
        std::lock_guard<std::mutex> lock(m_MutexPerformanceData);
        {
            performanceData.runningInfo = m_PerformanceData.runningInfo;
        }
    }
    mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Performence, performanceData, hint);

}
bool OM_COMPONENT::getRuningInfoFromDbMecStatusTable(RunningInfo& runningInfo)
{
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first = m_MqttClientConfig.configerMec.configerDb.DbMecDeviceStatusTableName;
    tableInfo.second = TABLE_TYEP_MEC_DEV_STATUS;
    DBUtils::getDBInfo(tableInfo, statusList);
    for(auto& v : statusList)
    {
//        runningInfo.cpuInfo.load = v.cpuLoad;
        runningInfo.cpuInfo.loadUp = atof(v.cpuLoad.c_str());
        runningInfo.cpuInfo.loadUpEmpty = false;
        runningInfo.cpuInfo.temp = v.cpuTemp;
        runningInfo.cpuInfo.uti = v.cpuUti;


        runningInfo.memInfo.total = v.memTotal;
        runningInfo.memInfo.used = v.memUsed;
        runningInfo.memInfo.free = v.memFree;

        runningInfo.diskInfo.total = v.diskTotal;
        runningInfo.diskInfo.used = v.diskUsed;
        runningInfo.diskInfo.free = v.diskFree;
        runningInfo.diskInfo.tps = v.diskTps;
        runningInfo.diskInfo.write = v.diskWrite;
        runningInfo.diskInfo.read = v.diskRead;

        runningInfo.netInfo.rx = v.netRx;
        runningInfo.netInfo.tx = v.netTx;
        runningInfo.netInfo.rxByte = v.netRxByte;
        runningInfo.netInfo.txByte = v.netTxByte;
    }
    return true;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OM_COMPONENT:: getMqttConfigerMsgCloud()
{
    m_MqttClientConfig.configerMqttCloud.mqttBrokerUrl          = getConfiger().configerMqttCloud.mqttBrokerUrl;
    m_MqttClientConfig.configerMqttCloud.mqttUserName           = getConfiger().configerMqttCloud.mqttUserName;
    m_MqttClientConfig.configerMqttCloud.mqttPassword           = getConfiger().configerMqttCloud.mqttPassword;
    m_MqttClientConfig.configerMqttCloud.mqttClientId           = getConfiger().configerMqttCloud.mqttClientId;
    m_MqttClientConfig.configerMqttCloud.mqttKeepAliveInterval  = getConfiger().configerMqttCloud.mqttKeepAliveInterval;
    m_MqttClientConfig.configerMqttCloud.mqttSendQos            = getConfiger().configerMqttCloud.mqttSendQos;
    m_MqttClientConfig.configerMqttCloud.mqttSubScribeQos       = getConfiger().configerMqttCloud.mqttSubScribeQos;
    m_MqttClientConfig.configerMqttCloud.mqttCleanSession       = getConfiger().configerMqttCloud.mqttCleanSession;
    m_MqttClientConfig.configerMqttCloud.mqttReconnectInterval  = getConfiger().configerMqttCloud.mqttReconnectInterval;
    m_MqttClientConfig.configerMqttCloud.mqttSslVersion         = getConfiger().configerMqttCloud.mqttSslVersion;
    m_MqttClientConfig.configerMqttCloud.mqttVersion            = getConfiger().configerMqttCloud.mqttVersion;
    m_MqttClientConfig.configerMqttCloud.mqttConnectTimeOut     = getConfiger().configerMqttCloud.mqttConnectTimeOut;
    std::string projectPath = getConfiger().configerProjectPath.projectRoot;
    std::string tlsDirPath = projectPath + "/" + getConfiger().configerProjectPath.tlsDirName;
    afl::FileUtil::checkDirectory(projectPath);
    afl::FileUtil::checkDirectory(tlsDirPath);
    m_MqttClientConfig.configerProjectPath.tlsCAFilePath                  = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsCAFileName;
    m_MqttClientConfig.configerProjectPath.tlsClientKeyFilePath           = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientKeyFileName;
    m_MqttClientConfig.configerProjectPath.tlsClientPrivateKeyFilePath    = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientPrivateKeyFileName;
    m_MqttClientConfig.rscuEsn = getConfiger().rscuEsn;
    //订阅
    memset(&m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics, 0, sizeof(m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics));
    m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum = 0;
    if(getConfiger().configerEnable.Enable_Camera)
    {
        for(uint32_t i = 0; i < getConfiger().configerCamera.configerTopicRcId.RcIds.size(); i++)
        {
            std::string topicCameraProfix = getConfiger().configerCamera.configerTopic.Topic_Profix +  getConfiger().configerCamera.configerTopicRcId.RcIds[i];
            MqttTopicConfigerCamera topicConfiger;
            topicConfiger.Topic_Register_Ack         = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Register_Ack       ;
            topicConfiger.Topic_Config_Query         = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Query       ;
            topicConfiger.Topic_Config_Down          = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Down        ;
            topicConfiger.Topic_Restar               = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Restar             ;
            topicConfiger.Topic_Upgrade              = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade            ;
            topicConfiger.Topic_Upgrade_Cancel              = topicCameraProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade_Cancel            ;

            m_MqttClientConfig.configerCamera.topicUnMap[i] = topicConfiger;
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Register_Ack.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Config_Query.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Config_Down.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Restar.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Upgrade.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfiger.Topic_Upgrade_Cancel.c_str());

        }
    }
    if(getConfiger().configerEnable.Enable_Radar)
    {
        for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
        {
            std::string topicProfix = getConfiger().configerRadar.configerTopic.Topic_Profix +  DBUtils::getLastPartOfPath(getConfiger().configerRadar.configerTopicRadarID.radarIDs[i]);
            MqttTopicConfigerOmRadar topicConfigerOmRadar;

            topicConfigerOmRadar.Topic_Query_Log             = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Query_Log           ;
            topicConfigerOmRadar.Topic_Register_Ack          = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Register_Ack        ;
            topicConfigerOmRadar.Topic_Query_Config          = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Query_Config        ;
            topicConfigerOmRadar.Topic_Update_Config         = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Update_Config       ;
            topicConfigerOmRadar.Topic_Restore               = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Restore             ;
            topicConfigerOmRadar.Topic_Ota                   = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Ota                 ;
            topicConfigerOmRadar.Topic_Ota_Cancel                   = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Ota_Cancel                 ;
            topicConfigerOmRadar.Topic_Reboot                = topicProfix + getConfiger().configerRadar.configerTopic.Topic_Reboot              ;

            topicConfigerOmRadar.Topic_InternalExternalParams_Up_Ack           = topicProfix + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Up_Ack         ;
            topicConfigerOmRadar.Topic_InternalExternalParams_Query_Down           = topicProfix + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Query_Down         ;


            m_MqttClientConfig.configerRadar.topicUnMap[i] = topicConfigerOmRadar;
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Register_Ack.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Query_Log.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Query_Config.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Update_Config.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Restore.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Reboot.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota_Cancel.c_str());

        }
    }

    //mec
    if(getConfiger().configerEnable.Enable_Mec)
    {
            m_MqttClientConfig.configerMec.configerEnable = getConfiger().configerMec.configerEnable;
            std::string topicProfix = getConfiger().configerMec.configerTopic.Topic_Profix + getConfiger().rscuEsn;

            m_MqttClientConfig.configerMec.configerTopic.Topic_Device_Info_Query           =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_Info_Query        ;         // 设备信息查询信息
            m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo_Ack         =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_BaseInfo_Ack      ;     // 设备基础信息
            m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down           =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Query_Down        ;     //配置查询下发
            m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update               =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Update            ;       //配置更新下发
            m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Down                    =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Down                 ;            //远程升级下发消息
            m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel                  =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Cancel               ;            //远程升级取消消息
            m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot                      =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Reboot                   ;              //远程重启/关机下发

            m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query_Ack   =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Camera_InterExter_Param_Query_Ack                   ;              //相机参数查询响应
            m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up_Ack      =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Camera_InterExter_Param_Up_Ack                   ;              //相机参数上报响应


            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo_Ack.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Device_Info_Query.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Down.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot.c_str());

            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query_Ack.c_str());
            m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics[m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up_Ack.c_str());
    }

    for (uint32_t i = 0; i < m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum; i++)
    {
        m_MqttClientConfig.configerMqttCloud.mqttSubscribeQoss[i] =  m_MqttClientConfig.configerMqttCloud.mqttSubScribeQos;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_CLOUD_SUCCESS_PRINT << "[configer-cloud]\n" << m_MqttClientConfig.configerMqttCloud.to_string().c_str();
    }
    return true;
}
bool OM_COMPONENT::getMqttConfigerMsg()
{
    m_MqttClientConfig.configerMqtt.mqttBrokerUrl          = getConfiger().configerMqtt.mqttBrokerUrl;
    m_MqttClientConfig.configerMqtt.mqttUserName           = getConfiger().configerMqtt.mqttUserName;
    m_MqttClientConfig.configerMqtt.mqttPassword           = getConfiger().configerMqtt.mqttPassword;
    m_MqttClientConfig.configerMqtt.mqttClientId           = getConfiger().configerMqtt.mqttClientId;
    m_MqttClientConfig.configerMqtt.mqttKeepAliveInterval  = getConfiger().configerMqtt.mqttKeepAliveInterval;
    m_MqttClientConfig.configerMqtt.mqttSendQos            = getConfiger().configerMqtt.mqttSendQos;
    m_MqttClientConfig.configerMqtt.mqttSubScribeQos       = getConfiger().configerMqtt.mqttSubScribeQos;
    m_MqttClientConfig.configerMqtt.mqttCleanSession       = getConfiger().configerMqtt.mqttCleanSession;
    m_MqttClientConfig.configerMqtt.mqttReconnectInterval  = getConfiger().configerMqtt.mqttReconnectInterval;
    m_MqttClientConfig.configerMqtt.mqttSslVersion         = getConfiger().configerMqtt.mqttSslVersion;
    m_MqttClientConfig.configerMqtt.mqttVersion            = getConfiger().configerMqtt.mqttVersion;
    m_MqttClientConfig.configerMqtt.mqttConnectTimeOut     = getConfiger().configerMqtt.mqttConnectTimeOut;

    m_MqttClientConfig.configerProjectPath.projectRoot = getConfiger().configerProjectPath.projectRoot;
    m_MqttClientConfig.configerProjectPath.otaDirName = getConfiger().configerProjectPath.otaDirName;
    m_MqttClientConfig.configerProjectPath.otaFileName = getConfiger().configerProjectPath.otaFileName;
    std::string projectPath = getConfiger().configerProjectPath.projectRoot;
    std::string tlsDirPath = projectPath + "/" + getConfiger().configerProjectPath.tlsDirName;
    afl::FileUtil::checkDirectory(projectPath);
    afl::FileUtil::checkDirectory(tlsDirPath);
    m_MqttClientConfig.configerProjectPath.tlsCAFilePath                  = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsCAFileName;
    m_MqttClientConfig.configerProjectPath.tlsClientKeyFilePath           = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientKeyFileName;
    m_MqttClientConfig.configerProjectPath.tlsClientPrivateKeyFilePath    = tlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientPrivateKeyFileName;

    m_MqttClientConfig.configerMec.configerOmCommon.logLevel = getConfiger().configerMec.configerOmCommon.logLevel;
    m_MqttClientConfig.configerMec.configerOmCommon.addressIP =  getConfiger().configerMec.configerOmCommon.addressIP;
    m_MqttClientConfig.configerMec.configerOmCommon.netMask = getConfiger().configerMec.configerOmCommon.netMask;
    m_MqttClientConfig.configerMec.configerOmCommon.gateway = getConfiger().configerMec.configerOmCommon.gateway;

    m_MqttClientConfig.configerMec.configerEnable = getConfiger().configerMec.configerEnable;
    m_MqttClientConfig.rscuEsn = getConfiger().rscuEsn;
    m_MqttClientConfig.configerMec.configerDb.DbFilePath = getConfiger().configerMec.configerDb.DbFilePath;
    m_MqttClientConfig.configerMec.configerDb.DbDeviceStatusTableName = getConfiger().configerMec.configerDb.DbDeviceStatusTableName;
    m_MqttClientConfig.configerMec.configerDb.DbSynchronizeTableName  = getConfiger().configerMec.configerDb.DbSynchronizeTableName;

    //订阅
    memset(&m_MqttClientConfig.configerMqtt.mqttSubscribeTopics, 0, sizeof(m_MqttClientConfig.configerMqtt.mqttSubscribeTopics));
    m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum = 0;
    if(getConfiger().configerEnable.Enable_Mec)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CLOUD_SUCCESS_PRINT << "Get MEC Topic!";
        }
        std::string topicProfix = getConfiger().configerMec.configerTopic.Topic_Profix + m_MqttClientConfig.rscuEsn;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Device_Info_Query        = topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_Info_Query      ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo          = topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_BaseInfo        ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo_Ack      = topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_BaseInfo_Ack    ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down        = topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Query_Down      ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down_Ack    = topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Query_Down_Ack  ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update            = topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Update          ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update_Ack        = topicProfix + getConfiger().configerMec.configerTopic.Topic_Config_Update_Ack      ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Performence              = topicProfix + getConfiger().configerMec.configerTopic.Topic_Performence            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Running_Status           = topicProfix + getConfiger().configerMec.configerTopic.Topic_Running_Status         ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm                    = topicProfix + getConfiger().configerMec.configerTopic.Topic_Alarm                  ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Device_HeartBeat         = topicProfix + getConfiger().configerMec.configerTopic.Topic_Device_HeartBeat       ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Down                 = topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Down               ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Status               = topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Status             ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Version              = topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Version            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel_ACK           = topicProfix + getConfiger().configerMec.configerTopic.Topic_Ota_Cancel_ACK            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot                   = topicProfix + getConfiger().configerMec.configerTopic.Topic_Reboot                 ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot_Ack               = topicProfix + getConfiger().configerMec.configerTopic.Topic_Reboot_Ack             ;

        m_MqttClientConfig.configerMec.configerTopic.Topic_Timing                   = topicProfix + getConfiger().configerMec.configerTopic.Topic_Timing               ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Signal                   =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Signal               ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Sensor_Angle_Offset      =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Sensor_Angle_Offset  ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsi                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Rsi            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsc                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Rsc            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ssm                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Ssm            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rtcm               =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Rtcm           ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Bsm                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Bsm            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Vir                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Vir            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Pam                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Pam            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Map                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Map            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Spat               =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Spat           ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Rsm                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Rsm           ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Sam                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Sam           ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Ism                =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Ism            ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Scene_Perception         =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Scene_Perception     ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Position_Up         		=  topicProfix + getConfiger().configerMec.configerTopic.Topic_Position_Up     ;

        m_MqttClientConfig.configerMec.configerAngleOffset.channel_readers = getConfiger().configerMec.configerAngleOffset.channel_readers     ;
        m_MqttClientConfig.configerMec.configerSpatSrcData.channel_readers = getConfiger().configerMec.configerSpatSrcData.channel_readers     ;
        m_MqttClientConfig.configerMec.configerV2xData.channel_readers_generated = getConfiger().configerMec.configerV2xData.channel_readers_generated     ;
        m_MqttClientConfig.configerMec.configerV2xData.channel_readers_received = getConfiger().configerMec.configerV2xData.channel_readers_received     ;
        m_MqttClientConfig.configerMec.configerTrafficlightDetectData.channel_readers = getConfiger().configerMec.configerTrafficlightDetectData.channel_readers     ;

        m_MqttClientConfig.configerMec.configerTopic.Topic_Trafficlight_Detect         =  topicProfix + getConfiger().configerMec.configerTopic.Topic_Trafficlight_Detect     ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Abnormal_Behavior_Up     = topicProfix + getConfiger().configerMec.configerTopic.Topic_Abnormal_Behavior_Up  ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Abnormal_Behavior_Down   = topicProfix + getConfiger().configerMec.configerTopic.Topic_Abnormal_Behavior_Down  ;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Timing_Alarm_Inter = getConfiger().configerMec.configerTopic.Topic_Bs_Inter_Profix + getConfiger().configerMec.configerTopic.Topic_Timing_Alarm_Inter;
        m_MqttClientConfig.configerMec.configerTopic.Topic_AngleOffset_Alarm_Inter = getConfiger().configerMec.configerTopic.Topic_Bs_Inter_Profix + getConfiger().configerMec.configerTopic.Topic_AngleOffset_Alarm_Inter;
        m_MqttClientConfig.configerMec.configerTopic.Topic_MecSelfCheckResult_Up = topicProfix + getConfiger().configerMec.configerTopic.Topic_MecSelfCheckResult_Up;

        m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up = topicProfix + getConfiger().configerMec.configerTopic.Topic_Camera_InterExter_Param_Up;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query = topicProfix + getConfiger().configerMec.configerTopic.Topic_Camera_InterExter_Param_Query;
        m_MqttClientConfig.configerMec.configerTopic.Topic_Signal_Data_Status_Up = topicProfix + getConfiger().configerMec.configerTopic.Topic_Signal_Data_Status_Up;
    }

    //camera
    if(getConfiger().configerEnable.Enable_Camera)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CLOUD_SUCCESS_PRINT << "Get om-camera Topic!";
        }
        std::pair<std::string, CameraRegisterMapConfiger> registerPair;
        for(uint32_t i = 0; i < getConfiger().configerCamera.configerTopicRcId.RcIds.size(); i++)
        {
            std::string deviceId  = getConfiger().configerCamera.configerTopicRcId.RcIds[i];
            registerPair.first = deviceId;
            registerPair.second.isRegistered = false;
            m_CameraRegisterMap.insert(registerPair);

            std::string topicProfix = getConfiger().configerCamera.configerTopic.Topic_Profix +  deviceId;
            MqttTopicConfigerCamera topicConfigerOmCamera;
            topicConfigerOmCamera.Topic_Register             = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Register           ;
            topicConfigerOmCamera.Topic_Register_Ack         = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Register_Ack       ;
            topicConfigerOmCamera.Topic_Config_Query         = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Query       ;
            topicConfigerOmCamera.Topic_Config_Info_Query_Up         = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Info_Query_Up       ;


            topicConfigerOmCamera.Topic_Config_Query_Ack     = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Query_Ack   ;
            topicConfigerOmCamera.Topic_Config_Down          = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Down        ;
            topicConfigerOmCamera.Topic_Config_Down_ACK      = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Config_Down_ACK    ;
            topicConfigerOmCamera.Topic_Runstatus            = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Runstatus          ;
            topicConfigerOmCamera.Topic_Alarm                = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Alarm              ;
            topicConfigerOmCamera.Topic_Keepalive            = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Keepalive          ;
            topicConfigerOmCamera.Topic_Restar               = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Restar             ;
            topicConfigerOmCamera.Topic_Restar_Up            = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Restar_Up          ;
            topicConfigerOmCamera.Topic_Upgrade              = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade            ;
            topicConfigerOmCamera.Topic_Upgrade_Cancel       = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade_Cancel     ;
            topicConfigerOmCamera.Topic_Upgrade_Cancel_ACK   = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Upgrade_Cancel_ACK ;
            topicConfigerOmCamera.Topic_UpgradeStatus        = topicProfix + getConfiger().configerCamera.configerTopic.Topic_UpgradeStatus      ;
            topicConfigerOmCamera.Topic_Synchronize          = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Synchronize        ;

            topicConfigerOmCamera.Topic_Vehicle_Count_UP          = topicProfix + getConfiger().configerCamera.configerTopic.Topic_Vehicle_Count_UP        ;
            m_MqttClientConfig.configerCamera.topicUnMap[i] = topicConfigerOmCamera;
            if (getConfiger().configerMec.enableUseHearbeatAsOffline)
            {
                std::pair<int, bool> pairTemp;
                pairTemp.first = -1;
                pairTemp.second = false;

                topicCameraHeartBeatMap[getContentBetweenFirstAndSecondSlash(topicConfigerOmCamera.Topic_Keepalive)] = pairTemp;
            }
            else
            {
                topicCameraPingMap[deviceId] = false;
            }

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Register.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Keepalive.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Alarm.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Synchronize.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Config_Query_Ack.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Config_Info_Query_Up.c_str());

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Config_Down_ACK.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Runstatus.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_UpgradeStatus.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Upgrade_Cancel_ACK.c_str());

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Restar_Up.c_str());

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmCamera.Topic_Vehicle_Count_UP.c_str());

        }
    }

    //radar
    if(getConfiger().configerEnable.Enable_Radar)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CLOUD_SUCCESS_PRINT << "Get om-radar Topic!";
        }
        std::pair<std::string, RadarRegisterMapConfiger> registerPair;
        for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
        {
            std::string deviceId  = getConfiger().configerRadar.configerTopicRadarID.radarIDs[i];
            registerPair.first = deviceId;
            registerPair.second.isRegistered = false;
            m_RadarRegisterMap.insert(registerPair);

            std::string topicProfixOmRadar = getConfiger().configerRadar.configerTopic.Topic_Profix +  getConfiger().configerRadar.configerTopicRadarID.radarIDs[i];
            MqttTopicConfigerOmRadar topicConfigerOmRadar;

            topicConfigerOmRadar.Topic_Query_Log             = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Query_Log           ;
            topicConfigerOmRadar.Topic_Query_Log_Ack         = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Query_Log_Ack       ;
            topicConfigerOmRadar.Topic_Register              = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Register            ;
            topicConfigerOmRadar.Topic_Register_Ack          = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Register_Ack    ;
            topicConfigerOmRadar.Topic_Query_Config          = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Query_Config        ;
            topicConfigerOmRadar.Topic_Query_Config_Info_Up          = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Query_Config_Info_Up        ;

            topicConfigerOmRadar.Topic_Query_Config_Ack      = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Query_Config_Ack    ;
            topicConfigerOmRadar.Topic_Update_Config         = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Update_Config       ;
            topicConfigerOmRadar.Topic_Update_Config_Ack     = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Update_Config_Ack   ;
            topicConfigerOmRadar.Topic_Restore               = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Restore             ;
            topicConfigerOmRadar.Topic_Restore_ACK           = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Restore_ACK         ;
            topicConfigerOmRadar.Topic_Runstatus             = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Runstatus           ;
            topicConfigerOmRadar.Topic_Warning               = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Warning             ;
            topicConfigerOmRadar.Topic_Heartbeat             = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Heartbeat           ;
            topicConfigerOmRadar.Topic_Ota                   = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota                 ;
            topicConfigerOmRadar.Topic_Ota_Cancel            = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota_Cancel                 ;
            topicConfigerOmRadar.Topic_Ota_Cancel_ACK        = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota_Cancel_ACK                 ;
            topicConfigerOmRadar.Topic_Ota_ACK               = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Ota_ACK             ;
            topicConfigerOmRadar.Topic_Reboot                = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Reboot              ;
            topicConfigerOmRadar.Topic_Reboot_Ack            = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Reboot_Ack          ;
            topicConfigerOmRadar.Topic_Synchronize           = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_Synchronize         ;
            topicConfigerOmRadar.Topic_InternalExternalParams_Up           = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Up         ;
            topicConfigerOmRadar.Topic_InternalExternalParams_Query           = topicProfixOmRadar + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Query         ;

            m_MqttClientConfig.configerRadar.topicUnMap[i] = topicConfigerOmRadar;
            if (getConfiger().configerMec.enableUseHearbeatAsOffline)
            {
                std::pair<int, bool> pairTemp;
                pairTemp.first = -1;
                pairTemp.second = false;
                topicCameraHeartBeatMap[getContentBetweenFirstAndSecondSlash(topicConfigerOmRadar.Topic_Heartbeat)] = pairTemp;
            }
            else
            {
                topicRadarPingMap[deviceId] = false;
            }

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Register.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Heartbeat.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Query_Log_Ack.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Query_Config_Ack.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Query_Config_Info_Up.c_str());

            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Update_Config_Ack.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Restore_ACK.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Runstatus.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Warning.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota_ACK.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Ota_Cancel_ACK.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Reboot_Ack.c_str());
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_Synchronize.c_str());
//            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_InternalExternalParams_Up.c_str());
//            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(topicConfigerOmRadar.Topic_InternalExternalParams_Query.c_str());
        }
    }
    //mec自检
    if (getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CLOUD_SUCCESS_PRINT << "Get om-mec-self-check Topic!";
        }

        for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
        {
            std::string radarID = getConfiger().configerMec.configerTopicRadarID.radarIDs[i];
            std::string device_id_str;
            size_t last_slash = radarID.rfind('/');
            if (last_slash != std::string::npos) {
                // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
                device_id_str = radarID.substr(last_slash + 1);
            }
            m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix =   m_MqttClientConfig.rscuEsn + getConfiger().configerMec.configerTopicRadarID.radarIDs[i];

            if(getConfiger().enableDebugPrint)
            {
                OM_CLOUD_SUCCESS_PRINT << "[m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix]" << m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix ;
            }
            TopicCCIndexTm            topicCCIndexTm;
            //动态
            topicCCIndexTm.Topic_Trajectories     = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Trajectories +  m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_VehiclePass      = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_VehiclePass  + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_QueueUp          = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_QueueUp      + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_AreaState        = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_AreaState    + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Overflow         = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Overflow     + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Outlane          = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Outlane      + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Statistics       = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Statistics   + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Evaluations      = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Evaluations  + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Nonmotor         = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Nonmotor     + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_DeviceStatus     = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_DeviceStatus     + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            topicCCIndexTm.Topic_Pulse            = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Profix + getConfiger().configerMec.configerMqttTopicCCIndex.Topic_Pulse            + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            m_MecSelfCheckTmDeviceIdTopicMap[device_id_str] = topicCCIndexTm;
            //静态
            TopicCCIndexSt            topicCCIndexSt;
            // if (!m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.empty() && m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.back() == '/')
            // {
            //     m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack.pop_back();
            // }
            //20260403:去掉对查询的监控，只监控update
            // topicCCIndexSt.Topic_Static_Query_Ack = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
            //                                               m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Query_Ack + m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;
            //
            // if (!m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.empty()
            //         && m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.back() == '/')
            // {
            //     m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update_Ack.pop_back();
            // }
            topicCCIndexSt.Topic_Static_Update = m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Profix +
                                                            m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Static_Update +  m_MqttClientConfig.configerMec.configerMqttTopicCCIndex.Topic_Postfix;

            m_MecSelfCheckStDeviceIdTopicMap[device_id_str] = topicCCIndexSt;
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        for(auto pair:m_CameraRegisterMap)
        {
            OM_INTER_DEBUG_PRINT << "[esn]" << pair.first << "[flag]" << (pair.second.isRegistered ? "True" : "False");
        }
        for(auto pair:m_RadarRegisterMap)
        {
            OM_INTER_DEBUG_PRINT << "[esn]" <<pair.first << "[flag]" << (pair.second.isRegistered ? "true" :"false");
        }
    }

    for (uint32_t i = 0; i < m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum; i++)
    {
        m_MqttClientConfig.configerMqtt.mqttSubscribeQoss[i] =  m_MqttClientConfig.configerMqtt.mqttSubScribeQos;
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_INTER_DEBUG_PRINT << "[configer-inter]" << std::endl << m_MqttClientConfig.configerMqtt.to_string().c_str();
    }
    return true;
}

std::string OM_COMPONENT::extractDeviceID(const std::string& command, std::string regexProfix)
{
    std::string regexParam = regexProfix + "/([^ /]+)";
    std::regex regex(regexParam); // 正则表达式用于匹配 deviceID
    std::smatch match;

    if (std::regex_search(command, match, regex)) {
        return match[1]; // 返回匹配的 deviceID
    }
    return ""; // 如果没有匹配，返回空字符串
}
void OM_COMPONENT::mqttDispatchSubscripeMessageOmRadar(const std::string &topic, const afl::base::json & subScribeJson)
{
    if (!m_MqttConnected)
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt clinet has not connected!";
        return;
    }
    if (topic.empty())
    {
        OM_INTER_ERROR_PRINT << "topic empty!";
        return;
    }
    std::string hint;
    std::string topicPT;

    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(getConfiger().enableDebugPrint)
    {
        OM_INTER_DEBUG_PRINT << "[topic]" << topic << "[topicProfix]" << topicProfix;
    }
    std::string deviceEsn = extractDeviceID(topic, "radar");
    if(topic.find("/register")  != std::string::npos)
    {
        bool registerFlagTemp = false;
        if(topic.find("/ack")  != std::string::npos)
        {
            registerFlagTemp = true;
        }
        else
        {
            registerFlagTemp = false;
        }

        for(auto& radarPair: m_RadarRegisterMap)
        {
            if(deviceEsn == radarPair.first)
            {
                radarPair.second.isRegistered = registerFlagTemp;
                break;
            }
        }
    }

    if (getConfiger().configerMec.enableUseHearbeatAsOffline)
    {
        if(topic.find("/heartbeat") != std::string::npos )
        {
            for(auto& radarPair: m_RadarRegisterMap)
            {
                if(deviceEsn == radarPair.first)
                {
                    radarPair.second.isRegistered = true;

                    m_FlagReceiveCountHearbeatRadar++;
                    uint64_t m_FlagReceiveCountHearbeatRadarOld = m_FlagReceiveCountHearbeatRadar.load();
                    m_EventloopOmCamera->addTimer(
                            std::bind(&OM_COMPONENT::timerMonitorRadarHearBeatData, this, m_FlagReceiveCountHearbeatRadarOld, deviceEsn ),
                            getConfiger().configerMec.configerPublishPeriod.periodMonitorRadarHeartBeatUp + 2, false
                    );
                    processRadarHearBeat(topic);
                    break;
                }
            }
        }


    }

    if(topic.find("/upload/warning")  != std::string::npos)
    {
        uint64_t  timestamp = 0;

        RadarAlarm radarAlarm;
        try{
            radarAlarm = subScribeJson;
        }
        catch (json::exception& e)
        {
            OM_INTER_ERROR_PRINT << "[what]mqtt-cloud " << e.what() << " [json-exception-id]" << e.id;
            return ;
        }
        if(getConfiger().enableDebugPrint)
        {
            ALARM_OM_CAMERA_DEBUG_PRINT << radarAlarm.to_string_log();
        }
    }
    mqttPushMsg2BrokerCloud(topic, subScribeJson, hint);

    return;
}

bool OM_COMPONENT::timerMonitorRadarHearBeatData(uint64_t lastValue, std::string deviceEsn)
{
    uint64_t currentValue = m_FlagReceiveCountHearbeatRadar.load();
    if(currentValue > lastValue)
    {
        //有心跳
        if(m_FlagReceiveCountHearbeatRadar >= 10)
        {
            m_FlagReceiveCountHearbeatRadar = 0;
        }
    }
    else
    {
        //无心跳
        for(auto& radarPair: m_RadarRegisterMap)
        {
            if (deviceEsn == radarPair.first)
            {
                radarPair.second.isRegistered = false;
            }
        }
    }
}
void OM_COMPONENT::mqttDispatchSubscripeMessageOmCamera(const std::string &topic, const afl::base::json & subScribeJson)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     OM_INTER_ERROR_PRINT << "[error]mqtt clinet has not connected!";
    //     return;
    // }
    if (topic.empty())
    {
        OM_INTER_ERROR_PRINT << "topic empty!";
        return;
    }
    std::string hint;
    std::string topicPT;

    std::string topicProfix = topic.substr(0, topic.find("/"));
    std::string deviceEsn = extractDeviceID(topic);
    if(topic.find("/register") != std::string::npos)
    {
        bool registerFlagTemp = false;
        if(topic.find("/ack") != std::string::npos)
        {
            registerFlagTemp = true;
        }
        else
        {
            registerFlagTemp = false;
        }

        for(auto& cameraPair: m_CameraRegisterMap)
        {
            if(deviceEsn == cameraPair.first)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CAMERA_WARN_PRINT << "[notice]find this device! [esn]"<< deviceEsn << "[flag]" << (registerFlagTemp?"True":"False");
                }
                cameraPair.second.isRegistered = registerFlagTemp;
                break;
            }
        }
    }
    if (getConfiger().configerMec.enableUseHearbeatAsOffline)
    {
        if(topic.find("/keep-alive")  != std::string::npos)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_CAMERA_WARN_PRINT << "[notice]find topic[keep-alive]!";
            }
            for(auto& cameraPair: m_CameraRegisterMap)
            {
                if(deviceEsn == cameraPair.first)
                {
                    cameraPair.second.isRegistered = true;
                    m_FlagReceiveCountHearbeatCamera++;
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CAMERA_WARN_PRINT << "[m_FlagReceiveCountHearbeatCamera]" << m_FlagReceiveCountHearbeatCamera;
                    }
                    uint64_t m_FlagReceiveCountHearbeatCameraOld = m_FlagReceiveCountHearbeatCamera.load();
                    m_EventloopOmCamera->addTimer(
                            std::bind(&OM_COMPONENT::timerMonitorCameraHearBeatData, this, m_FlagReceiveCountHearbeatCameraOld, deviceEsn),
                            getConfiger().configerMec.configerPublishPeriod.periodMonitorCameraHeartBeatUp + 2, false
                    );
                    processCameraHearBeat(topic);
                    break;
                }
            }
        }
    }
    if(topic.find("/alarm")  != std::string::npos)
    {
        uint64_t  timestamp = 0;

        CameraAlarm cameraAlarm;
        try{
            cameraAlarm = subScribeJson;
        }
        catch (json::exception& e)
        {
            OM_MEC_ERROR_PRINT << "[what]mqtt-cloud " << e.what() << " [json-exception-id]" << e.id;
            return ;
        }
        if(getConfiger().enableDebugPrint)
        {
            ALARM_OM_CAMERA_DEBUG_PRINT << cameraAlarm.to_string_log();
        }
    }

    mqttPushMsg2BrokerCloud(topic, subScribeJson, hint);

    return;
}

bool OM_COMPONENT::processCameraHearBeat(std::string topic)
{
    std::string interface = "[camera-hearbeat]";
    if(topic.empty())
    {
        OM_CAMERA_ERROR_PRINT << interface << "[notice]camera topic empty!";
        return false;
    }

    for (auto it = topicCameraHeartBeatMap.begin(); it != topicCameraHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topic))
        {
            m_Eventloop->cancelTimer(it->second.first);
            it->second.first = -1;
            it->second.second = true;
            break;
        }
    }
    std::string hint;
    hint = "camera-hearbeat";

    int timerFd = m_Eventloop->addTimer(std::bind(&OM_COMPONENT::monitorCameraHeartBeat, this, topic),
                                        getConfiger().configerMec.configerPublishPeriod.periodMonitorCameraHeartBeatForAlarm, false);
    for (auto it = topicCameraHeartBeatMap.begin(); it != topicCameraHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topic))
        {
            it->second.first = timerFd;
            it->second.second = false;
            break;
        }
    }
    return true;
}


bool OM_COMPONENT::monitorCameraHeartBeat(std::string topicTemp)
{
    std::string interface = "[camera-hearbeat]";
    bool findFlag = false;
    for (auto it = topicCameraHeartBeatMap.begin(); it != topicCameraHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topicTemp))
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
    return true;
}
bool OM_COMPONENT::timerMonitorCameraHearBeatData(uint64_t lastValue, std::string deviceEsn)
{
    uint64_t currentValue = m_FlagReceiveCountHearbeatCamera.load();
    if(getConfiger().enableDebugPrint)
    {
        OM_CAMERA_DEBUG_PRINT  << "[currentValue]" << currentValue << "[lastValue]"  << lastValue;
    }
    if(currentValue > lastValue)
    {
        //有心跳
        if(m_FlagReceiveCountHearbeatCamera >= 10)
        {
            m_FlagReceiveCountHearbeatCamera = 0;
        }
    }
    else
    {
        //无心跳
        for(auto& cameraPair: m_CameraRegisterMap)
        {
            if (deviceEsn == cameraPair.first)
            {
                cameraPair.second.isRegistered = false;
            }
        }
    }
}

bool OM_COMPONENT::processRadarHearBeat(std::string topic)
{
    std::string interface = "[radar-hearbeat]";
    if(topic.empty())
    {
        OM_RADAR_ERROR_PRINT << interface << "[notice-radar]topic empty!";
        return false;
    }

    for (auto it = topicRadarHeartBeatMap.begin(); it != topicRadarHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topic))
        {
            m_Eventloop->cancelTimer(it->second.first);
            it->second.first = -1;
            it->second.second = true;  //是否收到心跳
            break;
        }
    }
    std::string hint;
    hint = "radar-hearbeat";


    int timerFd = m_Eventloop->addTimer(std::bind(&OM_COMPONENT::monitorRadarHeartBeat, this, topic),
                                        getConfiger().configerMec.configerPublishPeriod.periodMonitorRadarHeartBeatForAlarm, false);
    for (auto it = topicRadarHeartBeatMap.begin(); it != topicRadarHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topic))
        {
            it->second.first = timerFd;
            it->second.second = false;
            break;
        }
    }

    return true;
}


bool OM_COMPONENT::monitorRadarHeartBeat(std::string topicTemp)
{
    std::string interface = "[radar-heartbeat]";

    for (auto it = topicRadarHeartBeatMap.begin(); it != topicRadarHeartBeatMap.end(); ++it)
    {
        if (it->first == getContentBetweenFirstAndSecondSlash(topicTemp))
        {
            if(it->second.second)
            {
                m_Eventloop->cancelTimer(it->second.first);
                it->second.first = -1;
                it->second.second = false;
            }
            else
            {
                it->second.first = -1;
                it->second.second = false;
            }
            break;
        }
    }


    return true;
}
bool OM_COMPONENT::mqttPushStringMsg2Broker(string topic, string info, string hint)
{
    if(!info.empty())
    {
        if(!mqttPublishMsg(topic, info))
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud push " << hint.c_str() << " failure!";
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
bool OM_COMPONENT::mqttPushJsonMsg2Broker(string topic, json info, string hint)
{
    std::string pData;
    try{
        pData = info.dump();
    }
    catch (json::exception& e)
    {
        OM_CLOUD_ERROR_PRINT << "[what]mqtt-cloud " << e.what() << " [json-exception-id]" << e.id;
        return false;
    }
    if(!pData.empty())
    {
        if(!mqttPublishMsg(topic, pData))
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud  push " << hint.c_str() << " failure!";
            return false;
        }
    }else
    {
        return false;
    }
    return true;
}

void OM_COMPONENT::processAlarmSensorAngleOffset(const std::string &topic, const afl::base::json & subScribeJson)
{
    if (!m_MqttConnected)
    {
        OM_MEC_ERROR_PRINT << "[error]mqtt clinet has not connected!";
        return;
    }

    CameraDeviceAngleOffsetLogData cameraDeviceAngleOffsetLogData;
    try
    {
        cameraDeviceAngleOffsetLogData = subScribeJson;
    } catch (afl::base::json::exception &e)
    {
        OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return;
    }
	string hint = "sensor-Angle-Offset occured ";
//    if(getConfiger().enableDebugPrint)
//    {
//        OM_MEC_DEBUG_PRINT  << hint << "[cameraDeviceAngleOffsetLogData]"
//                           << cameraDeviceAngleOffsetLogData.to_string();
//    }
    alarmSensorAngleOffsetReceiv = true;
    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
    if(!m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredPublishedOnceFlag)
    {
        //是否推送一次
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[occured]sensor-Angle-Offset, publish once";
        }
        alarmOccurred(hint, MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET, getConfiger().rscuEsn, cameraDeviceAngleOffsetLogData.alarm.addition);
        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredPublishedOnceFlag = true;
        if(!cameraDeviceAngleOffsetLogData.alarm.additionEmpty)
        {
            m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorAddition = cameraDeviceAngleOffsetLogData.alarm.addition;
        }
//        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.timeStamp =  cameraDeviceAngleOffsetLogData.timeStamp;
        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.timeStamp =  nowTime;
        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredTimeStamp =  nowTime;
        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredFlag = true;
        m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorDisppearedPublishedOnceFlag = false;
        if(m_TimerMonitorAlarmSensorAngleOffset <= 0)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "sensor-Angle-Offset-timer start";
            }
            m_TimerMonitorAlarmSensorAngleOffset =   m_EventloopOmMec->addTimer(std::bind(&OM_COMPONENT::monitorAngleOffset, this ), (getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval - 1), true);
        }
    }
    else
    {
        //60s推送一次
        uint64_t timeDiff = (nowTime -  m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[sensor-Angle-Offset-timeDiff]" << timeDiff;
        }
        if((timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval) && !alarmSensorAngleOffsetTimerPublishOnceFlag)
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[sensor-Angle-Offset-timeDiff]time publish!";
            }
            alarmOccurred(hint, MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET, getConfiger().rscuEsn,  cameraDeviceAngleOffsetLogData.alarm.addition);
            m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.timeStamp = nowTime;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[sensor-Angle-Offset-timeDiff]time publish!";
            }
            alarmSensorAngleOffsetTimerPublishOnceFlag = true;
        }
        else
        {
            alarmSensorAngleOffsetTimerPublishOnceFlag = false;
        }
    }
}

void OM_COMPONENT::monitorAngleOffset()
{
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[alarmSensorAngleOffsetReceiv]start monitor angle-offset!";
    }
    if(alarmSensorAngleOffsetReceiv)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[alarmSensorAngleOffsetReceiv]true,will set false!";
        }
        alarmSensorAngleOffsetReceiv = false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[alarmSensorAngleOffsetReceiv]true,will set false!";
        }
        //告警消失
        if(m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredFlag)
        {
            string hint = "sensor-Angle-Offset disappeared";
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << hint;
            }
            if(!m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorDisppearedPublishedOnceFlag)
            {
                alarmDisappeared(hint, MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET, getConfiger().rscuEsn, m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorAddition ,
                                 m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredTimeStamp );
                m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.timeStamp = 0;
                m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredFlag = false;
                m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorOccuredPublishedOnceFlag = false;
                m_AlarmTypeFlag.alarmCameraAngleOffsetFlag.alarmErrorDisppearedPublishedOnceFlag = true;
                if(m_TimerMonitorAlarmSensorAngleOffset >  0)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT  << "[notice]cancel tiemer!";
                    }
                    m_EventloopOmMec->cancelTimer(m_TimerMonitorAlarmSensorAngleOffset);
                    m_TimerMonitorAlarmSensorAngleOffset = -1;
                }
            }

        }
    }
}

void OM_COMPONENT::processHttpPostRequest()
{
    m_videoAndPicturePath = getConfiger().configerMec.configerHttpServerMec.videoAndPicturePath;
    m_ftpUrl = getConfiger().configerMec.configerHttpServerMec.ftpUrl;
    m_postPath = getConfiger().configerMec.configerHttpServerMec.postPath;
    m_httpHost = getConfiger().configerMec.configerHttpServerMec.httpHost;
    m_httpPort = getConfiger().configerMec.configerHttpServerMec.httpPort;

    httplib::Server svr;
    if(!svr.is_valid())
    {
        OM_MEC_ERROR_PRINT << "[error]http server error";
        return;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_SUCCESS_PRINT << "http server start success";
        }
    }

    svr.Post(m_postPath, [&](const httplib::Request &req, httplib::Response &res)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "recv http post body:" << req.body.c_str();
        }
        afl::base::json j;
        httpPostBody hpb;
        try
        {
            j = afl::base::json::parse(req.body.c_str(), req.body.c_str() + req.body.size());
            hpb = j;
        }
        catch (...)
        {
            OM_MEC_ERROR_PRINT << "http post body, json parse error!";
            return;
        }

        if(!afl::FileUtil::isDirectory(m_videoAndPicturePath.c_str()))
        {
            OM_MEC_ERROR_PRINT << "picture and video path error!";
            return;
        }

        std::vector<std::string> urls;
        getUrlsBytime(m_videoAndPicturePath, urls, hpb.start_time, hpb.end_time, hpb.type, hpb.device_sn);

        std::vector<afl::base::json> urllist;
        afl::base::json jrspbody;
        if(hpb.type == "0")
        {
            for(uint32_t i = 0; i < urls.size(); i++)
            {
                afl::base::json jurl;
                string jkey = "video_ur" + std::to_string(i);
                jurl[jkey] = urls[i];
                urllist.push_back(jurl);
            }

            jrspbody["video_urls"] = urllist;
        }
        else
        {
            for(uint32_t i = 0; i < urls.size(); i++)
            {
                afl::base::json jurl;
                string jkey = "pic_ur" + std::to_string(i);
                jurl[jkey] = urls[i];
                urllist.push_back(jurl);
            }
            jrspbody["pic_urls"] = urllist;
        }

        std::string rspbody = jrspbody.dump();
        res.set_content(rspbody, "application/json;charset=UTF-8");
    });

    svr.listen(getConfiger().configerMec.configerHttpServerMec.httpHost.c_str(), getConfiger().configerMec.configerHttpServerMec.httpPort);
}

void OM_COMPONENT::getUrlsBytime(std::string directory, std::vector<std::string>& urls, int64_t start_time, int64_t end_time, std::string datatype, std::string devicesn)
{
    DIR* dir = opendir(directory.c_str());
    if(dir == NULL)
    {
        OM_MEC_ERROR_PRINT << "directory open failed: " << directory.c_str();
        return;
    }

    struct dirent* temp_dirent = NULL;
    char dot[3] = ".";
    char dotdot[6] = "..";
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "start directory searching : " << directory.c_str();
    }
    while((temp_dirent = readdir(dir)) != NULL)
    {
        if(strcmp(temp_dirent->d_name, dot) ==0 || strcmp(temp_dirent->d_name, dotdot) == 0)//去掉“。”和“。。”文件
        {
            continue;
        }

        if(temp_dirent->d_type != DT_DIR)//只处理文件夹
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "current is not a directory: " << temp_dirent->d_name;
            }
            continue;
        }
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "current is a directory: " << temp_dirent->d_name;
        }
        std::string currentdirname(temp_dirent->d_name);
        std::string timedirname = currentdirname;
        if(currentdirname.size() == sizeof("YYYY-MM-DD_HH-MM-SS") - 1)//只处理规定格式的文件夹
        {
            if((currentdirname.at(4) != '-') || (currentdirname.at(7) != '-')
               || (currentdirname.at(10) != '_') || (currentdirname.at(13) != '-') || (currentdirname.at(16) != '-'))
            {
                continue;
            }
            //YYYY-MM-DD_HH-MM-SS -> YYYY-MM-DD HH:MM:SS
            currentdirname.replace(10, 1, " ");
            currentdirname.replace(13, 1, ":");
            currentdirname.replace(16, 1, ":");

            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "current dir name to string time: " << currentdirname.c_str();
            }

            time_t utcsecond;
            afl::util::DateTime::stringToDataTime(currentdirname.c_str(), &utcsecond);

            //string time -> utc time
            if(utcsecond == -1)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "string time to utc time failed";
                }
                continue;
            }
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "string time to utc time success: " << utcsecond;
            }
            //文件夹所存视频/图片时间段为：以文件夹名字作为时间起点，长度为10分钟；判断与目标时间段有无重合时间段
            if((utcsecond > end_time/1000) || (utcsecond + 10*60 < start_time/1000))//不存在重合时间段
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "current directory time error!";
                }
                continue;
            }
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "start get video or picture!";
            }
            std::string subDirectory = directory + "/" + temp_dirent->d_name;

            if(!afl::FileUtil::isDirectory(subDirectory.c_str()))
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "sub directory open failed: " << subDirectory.c_str();
                }
                continue;
            }

            if(datatype == "0")//"0"video
            {
                std::string videoDirectory = subDirectory + "/ipcamera";
                std::string deviceDirectory = videoDirectory + "/" + devicesn;
                if(afl::FileUtil::isDirectory(videoDirectory.c_str()) && afl::FileUtil::isDirectory(deviceDirectory.c_str()))
                {
                    DIR* videodir = opendir(deviceDirectory.c_str());
                    if(videodir == NULL)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "video directory open failed: " << directory.c_str();
                        }
                    }
                    else
                    {
                        struct dirent* temp_videoflie = NULL;

                        while((temp_videoflie = readdir(videodir)) != NULL)
                        {
                            std::string tempvideoname(temp_videoflie->d_name);
                            if(tempvideoname.find(timedirname) != std::string::npos)
                            {
                                if(getConfiger().enableDebugPrint)
                                {
                                    OM_MEC_DEBUG_PRINT << "get video file: " << tempvideoname.c_str();
                                }
                                urls.push_back(m_ftpUrl + "/" + temp_dirent->d_name + "/ipcamera" + "/" + devicesn + "/" + tempvideoname);
                                break;
                            }
                        }
                    }
                    closedir(videodir);
                }
            }
            else//"1"picture
            {
                std::string pictureDirectory = subDirectory + "/img";
                std::string deviceDirectory = pictureDirectory + "/" + devicesn;
                if (afl::FileUtil::isDirectory(pictureDirectory.c_str()) &&
                    afl::FileUtil::isDirectory(deviceDirectory.c_str()))
                {
                    DIR *picturedir = opendir(deviceDirectory.c_str());
                    if (picturedir == NULL)
                    {
                        if (getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "picture directory open failed: " << directory.c_str();
                        }
                    } else {
                        struct dirent *temp_picturefile = NULL;

                        while ((temp_picturefile = readdir(picturedir)) != NULL)
                        {
                            if (strcmp(temp_picturefile->d_name, dot) == 0 ||
                                strcmp(temp_picturefile->d_name, dotdot) == 0)//去掉“。”和“。。”文件
                            {
                                continue;
                            }

                            std::string temppicturename(temp_picturefile->d_name);
                            auto pos = temppicturename.find_first_of(".");
                            if (getConfiger().enableDebugPrint)
                            {
                                OM_MEC_DEBUG_PRINT << "possible picture file: " << temppicturename.c_str()
                                                   << "(" << (int) temppicturename.size() << ", " << (int) pos << ")";
                            }
                            if (pos != std::string::npos &&
                                (temppicturename.size() - 1 - pos == 24))//YYYY-MM-DD_HH-MM-SS.jpeg一共24位
                            {
                                std::string partname = temppicturename.substr(pos + 1, 19);

                                if ((partname.at(4) == '-') && (partname.at(7) == '-')
                                    && (partname.at(10) == '_') && (partname.at(13) == '-') &&
                                    (partname.at(16) == '-'))
                                {
                                    partname.replace(10, 1, " ");
                                    partname.replace(13, 1, ":");
                                    partname.replace(16, 1, ":");
                                    if (getConfiger().enableDebugPrint)
                                    {
                                        OM_MEC_DEBUG_PRINT << "part file name to string time: " << partname.c_str();
                                    }

                                    struct tm res;
                                    afl::util::DateTime::stringToDataTime(partname.c_str(), &res);
                                    time_t utcsec = mktime(&res);
                                    if (utcsec >= start_time / 1000 && utcsec <= end_time / 1000)
                                    {
                                        urls.push_back(
                                                m_ftpUrl + "/" + temp_dirent->d_name + "/img" + "/" + devicesn + "/" +
                                                temppicturename);
                                        if (getConfiger().enableDebugPrint)
                                        {
                                            OM_MEC_DEBUG_PRINT << "get picture file: " << temppicturename.c_str();
                                        }
                                    } else {
                                        OM_MEC_ERROR_PRINT << "picture time error";
                                    }

                                } else {
                                    if (getConfiger().enableDebugPrint)
                                    {
                                        OM_MEC_DEBUG_PRINT << "partname error: " << partname.c_str();
                                    }
                                }
                            }
                        }
                    }
                    closedir(picturedir);
                } else {
                    if (getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "img directory(" << pictureDirectory.c_str() << ") or device directory("
                                           << deviceDirectory.c_str() << ") not found";
                    }
                }
            }
        }
    }
    closedir(dir);
}

void OM_COMPONENT::upPtpStatus(bool isPublish)
{
    if(!m_MqttConnected)
    {
        return;
    }

    if(!m_MqttClientConfig.configerMec.configerEnable.Enable_Timing || !m_RegisterFlag)
    {
        return;
    }
    std::string hint = "timing";
    // std::ifstream file("/var/ptppartlog.txt", std::ios::in);
    // int64_t lastTime = -9999;
    // int64_t timeDifference = -9999;
    // string timeDifferenceStr = "";
    // if(file.is_open())//打开成功
    // {
    //     string tmp;
    //     while(getline(file, tmp, '\n'))
    //     {
    //         string parttmp = tmp.substr(tmp.find_first_of(" "));

    //         auto a_pos = parttmp.find("phc offset");
    //         if(a_pos != string::npos)
    //         {
    //             //取授时时间
    //             string date = parttmp.substr(1, 2);
    //             string hour = parttmp.substr(4, 2);
    //             string min = parttmp.substr(7, 2);
    //             string sec = parttmp.substr(10, 2);

    //             time_t t = time(0);
    //             tm* now = localtime(&t);
    //             now->tm_mday = atoi(date.c_str());
    //             now->tm_hour = atoi(hour.c_str());
    //             now->tm_min = atoi(min.c_str());
    //             now->tm_sec = atoi(sec.c_str());
    //             lastTime = mktime(now);

    //             // string year = std::to_string(now->tm_year + 1900);
    //             // string mon = std::to_string(now->tm_mon + 1);
    //             // if(now->tm_mon < 9)
    //             // {
    //             //     mon = "0" + mon;
    //             // }
    //             //
    //             // string ptptime = year + "-" + mon + "-" + date + " " + hour + ":" + min + ":" + sec;
    //             // OM_MEC_DEBUG_PRINT << "ptp time: " << ptptime.c_str() << " size" << ptptime.length();
    //             // if(afl::util::DateTime::stringToDataTime(ptptime.c_str(), &lastTime))
    //             // {
    //             //     OM_MEC_DEBUG_PRINT << "get ptp time success" << lastTime;
    //             // }
    //             // else
    //             // {
    //             //     OM_MEC_DEBUG_PRINT << "get ptp time failed" << lastTime;
    //             // }

    //             //取授时时差
    //             timeDifferenceStr = parttmp.substr(a_pos + 10);
    //             if(getConfiger().enableDebugPrint)
    //             {
    //                 OM_MEC_DEBUG_PRINT  << "part1 ptp line: " << timeDifferenceStr.c_str();
    //             }
    //             uint32_t c_pos = timeDifferenceStr.find('s');
    //             timeDifferenceStr = timeDifferenceStr.substr(0, c_pos);
    //             if(getConfiger().enableDebugPrint)
    //             {
    //                 OM_MEC_DEBUG_PRINT  << "part2 ptp line: " << timeDifferenceStr;
    //             }
    //             string::size_type index = 0;
    //             while( (index = timeDifferenceStr.find(' ', index)) != string::npos)
    //             {
    //                 timeDifferenceStr.erase(index, 1);
    //             }

    //             if(timeDifferenceStr != "")
    //             {
    //                 timeDifference = stold(timeDifferenceStr);
    //             }
    //             if(getConfiger().enableDebugPrint)
    //             {
    //                 OM_MEC_DEBUG_PRINT  << "part3 ptp line: " << timeDifferenceStr.c_str();
    //             }
    //             break;
    //         }
    //     }
    //     file.clear();
    //     file.close();
    // }
    // else
    // {
    //     OM_MEC_ERROR_PRINT  << "[error]/var/ptppartlog.txt open failed";
    // }

    EquipmentTimingData equipmentTimingData;

    equipmentTimingData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    equipmentTimingData.seqNum =  afl::util::Srand::srandStr(32);
    equipmentTimingData.rscuEsn = m_MqttClientConfig.rscuEsn;
    uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
    if(m_currentSource.load() == 1)//ptp
    {
        equipmentTimingData.lastTime = m_ptpTime.load();
    }
    else//gps or other
    {
        equipmentTimingData.lastTime = m_gpsTime.load();
    }
    equipmentTimingData.timeNode = m_currentSource.load();
    equipmentTimingData.ptpDifference = m_ptpTimeDifference.load();
    equipmentTimingData.gpsDifference = m_gpsTimeDifference.load();
    // if(lastTime == -9999)
    // {

    //     equipmentTimingData.lastTime = m_last_lastTime;
    //     equipmentTimingData.timeDifference = m_last_timeDifference;
    // }
    // else
    // {
    //     equipmentTimingData.lastTime = ((uint64_t)lastTime)*1000 + now%1000;
    //     equipmentTimingData.timeDifference = timeDifference;
    //     m_last_lastTime = lastTime;
    //     m_last_timeDifference = timeDifference;

    // }
    // equipmentTimingData.timeNode = 1;


    std::string timingAddition = "";
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "[timeStamp]" << equipmentTimingData.timeStamp << "[lastTime]" << equipmentTimingData.lastTime << "[difference]" << (equipmentTimingData.timeStamp - equipmentTimingData.lastTime);
    }
    if(equipmentTimingData.lastTime == -9999  || std::abs(equipmentTimingData.gpsDifference) >
                getConfiger().configerMec.configAlarmThresholdValue.TV_Timing_Alarm)
    {
        m_AlarmTypeFlag.alarmMecErrorTimingFlag.alarmErrorOccuredFlag = true;
        if(equipmentTimingData.lastTime != -9999 )
        {
        	m_TimingCount++;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_WARN_PRINT << "[alarm][occurred]mec-timing-alarm [m_TimingCount]" << m_TimingCount;
            }
            timingAddition.append(getConfiger().rscuEsn);
            timingAddition.append(",");
		}
    }

    if(isPublish)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[alarm]Publish";
        }
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Timing, equipmentTimingData, hint);
    }

    if(m_TimingCount > getConfiger().configerMec.configAlarmThresholdValue.TV_Alarm_Timing_Count)
    {
        m_TimingCount = 0;
        hint = "timing-alarm, [count]" + std::to_string(m_TimingCount);
        if(!m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredPublishedOnceFlag)
        {
            uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_ERROR_PRINT  << "[alarm][occurred]timing-alarm, publish once";
            }
            alarmOccurred(hint, MEC_ALARM_TYPE_ERROR_TIMING,  timingAddition);
            m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredPublishedOnceFlag = true;
            m_AlarmTypeFlag.alarmErrorTimingFlag.timeStamp = equipmentTimingData.timeStamp;
            m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredTimeStamp = nowTime;
        }
        else
        {
            uint64_t timeDiff = (equipmentTimingData.timeStamp -  m_AlarmTypeFlag.alarmErrorTimingFlag.timeStamp)/m_PublishAlarmDataIntervalUnit;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_ERROR_PRINT  << "[alarm][timing-alarm][timeDiff(s)]" << timeDiff;
            }
            if(timeDiff >=getConfiger().configerMec.configerPublishPeriod.periodAlarmPublishInterval)
            {
                alarmOccurred(hint, MEC_ALARM_TYPE_ERROR_TIMING,  timingAddition);
                m_AlarmTypeFlag.alarmErrorTimingFlag.timeStamp = equipmentTimingData.timeStamp;
            }
        }

        m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredFlag = true;
        m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorDisppearedPublishedOnceFlag = false;
    }
    else
    {
        if(m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredFlag)
        {
            hint = "timing-alarm";
//            if(getConfiger().enableDebugPrint)
//            {
//                OM_MEC_DEBUG_PRINT  << "[disappeared]" << hint;
//            }
            if(!m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorDisppearedPublishedOnceFlag)
            {
                alarmDisappeared(hint, MEC_ALARM_TYPE_ERROR_TIMING, getConfiger().rscuEsn, timingAddition,
                                 m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredTimeStamp);
                m_AlarmTypeFlag.alarmErrorTimingFlag.timeStamp = 0;
                m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredFlag = false;
                m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorOccuredPublishedOnceFlag = true;
                m_AlarmTypeFlag.alarmErrorTimingFlag.alarmErrorDisppearedPublishedOnceFlag = true;
            }
        }
    }

}

/**
 *
 * @param hint  提示信息
 * @param mecAlarmTypeErrorCode 告警码
 * @param sensorDeviceNo  感知设备ESN
 * @param addition 提示字段
 * @param dbTableName  数据表名：不使用
 * @param tableType 数据表类型
 * @return
 */
bool OM_COMPONENT::alarmOccurred(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo,
                                 std::string addition,  std::string dbTableName, TABLE_TYPE tableType, uint64_t faultStartTime)
{
    hint = getAlarmTypeInfo(mecAlarmTypeErrorCode);
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT  << "[alarm][occurred][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
    }
    AlarmManagementData alarmManagementData;
    alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    alarmManagementData.seqNum  =  afl::util::Srand::srandStr(32);
    alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
    if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
    {
        alarmManagementData.protocolVersion = PROTOCOL_VERSION;
    }
    else
    {
        alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
    }

    alarmManagementData.alarm.alarmLevel = ALERT_CRITICAL;
    alarmManagementData.alarm.alarmStatus = ALARM_OCCURRED;
    alarmManagementData.alarm.alarmRaisedTime = afl::util::TimeStamp::now(true).millSeconds();
    alarmManagementData.alarm.alarmChangedTime = 0;
    alarmManagementData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
    if(!sensorDeviceNo.empty())
    {
        alarmManagementData.alarm.addition = sensorDeviceNo;
        alarmManagementData.alarm.additionEmpty = false;
    }
    if (mecAlarmTypeErrorCode < MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS )
    {
        if (getConfiger().configerMec.configerEnable.Enable_Alarm)
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
        }

    }
    else
    {
        // [新增] 检查并发送 MEC 自检结果上报
        int faultType = getSelfCheckFaultType(mecAlarmTypeErrorCode);
        if (faultType != 0) {
            // 故障产生：status=1, startTime=now, stopTime=0(或now)
            // uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
            if (getConfiger().configerMec.configerEnable.Enable_MecSelfCheck)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm][occurred][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
                sendMecSelfCheckMsg(mecAlarmTypeErrorCode, faultType, 1, faultStartTime, 0, addition);
            }
        }
    }

    {
        auto it = m_MecAlarmMap.find((int)alarmManagementData.alarm.alarmType);
        if (it != m_MecAlarmMap.end()) {

            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm-map][update][occurred][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
            }
            // 找到后修改内容
            it->second = alarmManagementData;
        } else {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT  << "[alarm-map][insert][occurred][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
            }
            //插入map中
            m_MecAlarmMap.emplace(alarmManagementData.alarm.alarmType, alarmManagementData);
        }
    }


    if(MEC_ALARM_TYPE_ERROR_TIMING == mecAlarmTypeErrorCode)
    {
        mqttPushMsg2Broker(m_MqttClientConfig.configerMec.configerTopic.Topic_Timing_Alarm_Inter, alarmManagementData, hint);
    }
    else if(MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET == mecAlarmTypeErrorCode)
    {
        mqttPushMsg2Broker(m_MqttClientConfig.configerMec.configerTopic.Topic_AngleOffset_Alarm_Inter, alarmManagementData, hint);
    }
    else
    {

    }
    return true;
}

bool OM_COMPONENT::alarmDisappeared(std::string hint, MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode, std::string sensorDeviceNo,
                                        std::string addition,  uint64_t alarmErrorOccuredTimeStamp,  std::string dbTableName, TABLE_TYPE tableType)
{
    m_EventloopOmMec->runInLoop([this, hint, mecAlarmTypeErrorCode, sensorDeviceNo, alarmErrorOccuredTimeStamp, dbTableName, tableType, addition]()
    {
        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT  << "[alarm][disappeared][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
        }
       AlarmManagementData alarmManagementData;
       alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
       alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
       alarmManagementData.seqNum  =  afl::util::Srand::srandStr(32);

       if(getConfiger().configerMec.configerOmCommon.protocolVersion != PROTOCOL_VERSION)
       {
           alarmManagementData.protocolVersion = PROTOCOL_VERSION;
       }
       else
       {
           alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
       }


       alarmManagementData.alarm.alarmLevel = ALERT_CRITICAL;
       alarmManagementData.alarm.alarmStatus = ALARM_DISAPPEARED;
       alarmManagementData.alarm.alarmRaisedTime = alarmErrorOccuredTimeStamp;
       alarmManagementData.alarm.alarmChangedTime =  afl::util::TimeStamp::now(true).millSeconds();
       alarmManagementData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
       if(!sensorDeviceNo.empty())
       {
           alarmManagementData.alarm.addition = sensorDeviceNo;
           alarmManagementData.alarm.additionEmpty = false;
       }
//        if(getConfiger().enableDebugPrint)
//        {
//            OM_MEC_DEBUG_PRINT  << "push channel data 2 om_cloud!";
//        }

        {
            auto it = m_MecAlarmMap.find((int)alarmManagementData.alarm.alarmType);
            if (it != m_MecAlarmMap.end())
            {
                // 找到后修改内容
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_ERROR_PRINT  << "[alarm-map][disappeared][type]" << mecAlarmTypeErrorCode << "[msg]" << hint;
                }
                m_MecAlarmMap.erase(it);
            }
        }
        if (mecAlarmTypeErrorCode < MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS)
        {
            mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
            mqttPushMsg2Broker(m_MqttClientConfig.configerMec.configerTopic.Topic_Timing_Alarm_Inter, alarmManagementData, hint);
        }
        else
        {

            // [新增] 检查并发送 MEC 自检结果上报
            int faultType = getSelfCheckFaultType(mecAlarmTypeErrorCode);
            if (faultType != 0)
            {
                // 故障消失：status=0, startTime=发生时间, stopTime=now
                uint64_t now = afl::util::TimeStamp::now(true).millSeconds();
                sendMecSelfCheckMsg(mecAlarmTypeErrorCode, faultType, 0, alarmErrorOccuredTimeStamp, now, addition);
            }
        }

//       std::vector<MsgDeviceStatus> statusList;
//       std::pair<std::string, TABLE_TYPE> tableInfo;
//       tableInfo.first =  dbTableName;
//       tableInfo.second = tableType;
//
//       DBUtils::getDBInfo(tableInfo, statusList);
//       for(auto& v : statusList)
//       {
//           AlarmManagementData alarmManagementData;
//           alarmManagementData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
//           alarmManagementData.rscuEsn =  getConfiger().rscuEsn;
//           alarmManagementData.seqNum  =  afl::util::Srand::srandStr(32);
//           alarmManagementData.protocolVersion = getConfiger().configerMec.configerOmCommon.protocolVersion;
//           alarmManagementData.alarm.alarmLevel = ALERT_CRITICAL;
//           alarmManagementData.alarm.alarmStatus = ALARM_DISAPPEARED;
//           alarmManagementData.alarm.alarmRaisedTime = v.alarmRaisedTime;
//           alarmManagementData.alarm.alarmChangedTime =  afl::util::TimeStamp::now(true).millSeconds();
//           alarmManagementData.alarm.alarmType = (MecAlarmTypeErrorCodeEnum)mecAlarmTypeErrorCode;
//           if(!sensorDeviceNo.empty())
//           {
//               alarmManagementData.alarm.addition = sensorDeviceNo;
//               alarmManagementData.alarm.additionEmpty = false;
//           }
//           OM_MEC_DEBUG_PRINT  << "push channel data 2 om_cloud!" ;
//           mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Alarm, alarmManagementData, hint);
//           mqttPushMsg2Broker(getConfiger().configerMec.configerTopic.Topic_Timing_Alarm_Inter, alarmManagementData, hint);
//       }

   });
    return true;
}
std::string OM_COMPONENT::getAlarmTypeInfo(MecAlarmTypeErrorCodeEnum mecAlarmTypeErrorCode)
{
    std::string hint = "";
    switch(mecAlarmTypeErrorCode)
    {
        case MEC_ALARM_TYPE_HIGH_TEMPER_CPU:
            hint = "cpu temp high";// CPU温度过高
            break;
        case MEC_ALARM_TYPE_HIGH_OCCUPANCY_CPU:          // CPU占用率过高
            hint = "cpu load high"; // CPU占用率过高
            break;
        case  MEC_ALARM_TYPE_HIGH_TEMPER_GPU:             // GPU温度过高
            hint = "gpu temp high"; // GPU温度过高
            break;
        case MEC_ALARM_TYPE_HIGH_OCCUPANCY_GPU:         // GPU占用率过高
            hint = "gpu load high"; // GPU占用率过高
            break;
        case MEC_ALARM_TYPE_OUT_OF_MEMORY:              // 内存不足
            hint = "mem out of "; // 内存不足
            break;
        case MEC_ALARM_TYPE_BAD_FIRMWARE:              // 固件损坏
            hint = "bad fireware"; // 固件损坏
            break;
        case MEC_ALARM_TYPE_STORAGE_BAD_BLOCK:          // 存储坏块
            hint = "bad bolck"; // 存储坏块
            break;
        case MEC_ALARM_TYPE_OUT_OF_DISK_SPACE:            // 磁盘空间不足
            hint = "disk space out of"; // 磁盘空间不足
            break;
        case MEC_ALARM_TYPE_IRE_OFF_LINE:                // 感知设备离线
            hint = "sensor device offline"; // 感知设备离线
            break;
        case MEC_ALARM_TYPE_THREAD_LOCK:               // 线程卡死
            hint = "thread lock"; // 线程卡死
            break;
        case MEC_ALARM_TYPE_CAMERA_ANGLE_OFFSET:        // 摄像机角度偏移
            hint = "camera angle offset"; // 摄像机角度偏移
            break;
        case MEC_ALARM_TYPE_ERROR_TIMING:              // 未搜索到授时服务器
            hint = "no find timing server "; // 未搜索到授时服务器
            break;
        case MEC_ALARM_TYPE_ERROR_VOLTAGE:             // 电压故障
            hint = "voltage error"; // 电压故障
            break;
        case MEC_ALARM_TYPE_OTHER :                      // 其他
            hint = "other"; // 其他
            break;
        // 新增的监控告警
        case MEC_ALARM_TYPE_MONITOR_SPAT_DATA_LOSS: hint = "【data loss】spat src "; break;
        case MEC_ALARM_TYPE_MONITOR_SENSOR_OBJ_DATA_LOSS: hint = "【data loss】sensor obj"; break;
        case MEC_ALARM_TYPE_MONITOR_V2X_CC_DATA_LOSS: hint = "【data loss】ccindex tc"; break;
        case MEC_ALARM_TYPE_MONITOR_RADAR_DYN_DATA_LOSS: hint = "【data loss】ccindex tm"; break;
        case MEC_ALARM_TYPE_MONITOR_RADAR_STAT_DATA_LOSS: hint = "【data loss】ccindex st"; break;
        case MEC_ALARM_TYPE_MONITOR_RADAR_RAW_DATA_LOSS: hint = "【data loss】radar raw data "; break;

        case MEC_ALARM_TYPE_MONITOR_CLOUD_LINK_ERROR: hint = "【link error】sensor cloud "; break;
        case MEC_ALARM_TYPE_MONITOR_CC_STAT_LINK_ERROR: hint = "【link error】ccindex st"; break;
        case MEC_ALARM_TYPE_MONITOR_CC_DYN_LINK_ERROR: hint = "【link error】ccindex tm"; break;
        case MEC_ALARM_TYPE_MONITOR_POINT_CLOUD_LINK_ERROR: hint = "【link error】cloud point "; break;
        default:

            break;
    }
    return hint;
}

//////////////////////////////////////////////////////////////////////////////////////

bool OM_COMPONENT::mqttInitConnOpts()
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
    m_MqttConnOpts.automaticReconnect = 1;
    if (getConfiger().enableDebugPrint)
    {
        OM_INTER_DEBUG_PRINT  << "mqtt-inter  finished Init ConnOpts";
    }
    return true;
}

void OM_COMPONENT::mqttConnectedCallback(void* context, char* cause) {
    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnected = true;
    OM_INTER_DEBUG_PRINT << "[success]mqtt-inter (re)connected!";

    // 在这里执行订阅，确保重连后也能重新订阅
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = mqttOnSubscribe;
    opts.onFailure = mqttOnSubscribeFailure;
    opts.context = thiz;

    if(thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum > 0) {
        MQTTAsync_subscribeMany(thiz->m_MqttClient,
                                thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum,
                                thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeTopics,
                                thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeQoss,
                                &opts);
    }
}

void OM_COMPONENT::mqttConnectedCallbackCloud(void* context, char* cause) {
    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = true;
    OM_INTER_DEBUG_PRINT << "[success]mqtt-cloud (re)connected!";

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = mqttOnSubscribeCloud;
    opts.onFailure = mqttOnSubscribeFailureCloud;
    opts.context = thiz;

    if(thiz->m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum > 0) {
        MQTTAsync_subscribeMany(thiz->m_MqttClientCloud,
                                thiz->m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum,
                                thiz->m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics,
                                thiz->m_MqttClientConfig.configerMqttCloud.mqttSubscribeQoss,
                                &opts);
    }
}
bool OM_COMPONENT::mqttInit()
{
    m_MqttClient = nullptr;
    m_MqttConnected = false;

    mqttInitConnOpts();
    std::string clientIdTemp = m_MqttClientConfig.configerMqtt.mqttClientId + ":" +
                               std::to_string(afl::util::TimeStamp::now(true).microSeconds());
    if(getConfiger().enableDebugPrint)
    {
        OM_INTER_DEBUG_PRINT  << "mqtt-inter [mqttBroker-Url]"
                           << m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str()
                           << " [client-id]" << clientIdTemp.c_str()
                           << " [username]" << m_MqttClientConfig.configerMqtt.mqttUserName.c_str()
                           << " [passwd]" << m_MqttClientConfig.configerMqtt.mqttPassword.c_str();
    }
    if (MQTTASYNC_SUCCESS != MQTTAsync_create(&m_MqttClient, m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str(),
                                              clientIdTemp.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL))
    {
        OM_INTER_ERROR_PRINT  << "[error]create connectiont failure!";
        return false;
    }

    MQTTAsync_setCallbacks(m_MqttClient, this, mqttConnlost, mqttSubscribeMsgArrvd, NULL);
    MQTTAsync_setConnected(m_MqttClient, this, mqttConnectedCallback);
    if (!mqttConnect())
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt-inter Connect Failure!";
        return false;
    }

    return true;
}

void OM_COMPONENT::mqttDeinit()
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

void OM_COMPONENT::mqttOnConnect(void *context, MQTTAsync_successData *response)
{
    OM_INTER_SUCCESS_PRINT << "[success]mqtt-inter connect success!";
    auto thiz = (OM_COMPONENT *) context;
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
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter subscribe error";
        }
    }
    if(thiz->m_TimerMqttReconnectInter > 0)
    {
        thiz->m_EventloopCloud->cancelTimer(thiz->m_TimerMqttReconnectInter);
        thiz->m_TimerMqttReconnectInter = -1;
    }
}

void OM_COMPONENT::mqttOnConnectFailure(void *context, MQTTAsync_failureData *response)
{
    if (response)
    {
        if (response->message)
        {
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter connect error [error-code]" << response->code << " [error-msg]" << response->message;
        } else
        {
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter connect error [error-code]" << response->code << " [error-msg]" << response->message;
        }
    } else
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt-inter connect error!";
    }

    auto thiz = (OM_COMPONENT *) context;
    thiz->mqttReconnect();
}


bool OM_COMPONENT::mqttConnect()
{
    int rc;
    if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(m_MqttClient, &m_MqttConnOpts)))
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt-inter connect error!";
        m_MqttConnected = false;
        return false;
    }
    return true;
}

bool OM_COMPONENT::mqttReconnect()
{
    OM_INTER_ERROR_PRINT << "[notice]mqtt-inter mqtt reconnect!";
    // if(m_TimerMqttReconnectInter < 0)
    // {
    //     m_TimerMqttReconnectInter = m_Eventloop->addTimer(std::bind(&OM_COMPONENT::mqttConnect, this), m_MqttClientConfig.configerMqtt.mqttReconnectInterval, true);
    // }
    // if(getConfiger().enableDebugPrint)
    // {
    //     OM_MQTT_DEBUG_PRINT << "[notice]mqtt-inter reconnect!";
    // }
    return true;
}

void OM_COMPONENT::mqttConnlost(void *context, char *cause)
{
    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnected = false;
    OM_INTER_ERROR_PRINT << "[error]mqtt-inter connect lost! [cause]" << cause;
    OM_MQTT_ERROR_PRINT << "[error]mqtt-inter connect lost! [cause]" << cause;
    thiz->mqttReconnect();
}

void OM_COMPONENT::mqttOnDisconnect(void *context, MQTTAsync_successData *response)
{
    OM_INTER_ERROR_PRINT << "[notice]mqtt-inter disconnect!";
    OM_MQTT_ERROR_PRINT << "[notice]mqtt-inter disconnect!";

    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnected = false;
}

void OM_COMPONENT::mqttOnSubscribe(void *context, MQTTAsync_successData *response)
{
    OM_INTER_SUCCESS_PRINT << "[success]mqtt-inter subscribe success!";
}

void OM_COMPONENT::mqttOnSubscribeFailure(void *context, MQTTAsync_failureData *response)
{
    OM_INTER_ERROR_PRINT << "[error]mqtt-inter subscribe failure!";
}

bool OM_COMPONENT::mqttPublishMsg(const std::string &topic, const std::string &msg)
{
    if (!m_MqttConnected)
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt-inter  has not connected!";
        return false;
    }

    if (msg.size() <= 0 || topic.empty())
    {
        OM_INTER_ERROR_PRINT << "[error]mqtt-inter len < 0 or topic empty!";
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
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter  Async Connect failure，will connect!";
            if (m_MqttConnected)
            {
                m_MqttConnected = false;
                // mqttReconnect();
            }
        } else
        {
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter  push msg error，[error-code]" << rc;
        }

        return false;
    }

    return true;
}

int OM_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int ret = 1;
    auto thiz = (OM_COMPONENT *) context;
    if (message->payloadlen)
    {

        if(thiz->getConfiger().enableDebugPrint)
        {
            OM_INTER_DEBUG_PRINT << "[notice]mqtt-inter " << "[topic]" << topicName << " [msg-len]" << message->payloadlen << " [msg]" << (char *)message->payload ;
        }

        afl::base::json j;
        try
        {
            j = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
        }
        catch (...)
        {
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter parse json failure!";
            MQTTAsync_freeMessage(&message);
            MQTTAsync_free(topicName);
            return ret;
        }

        std::string topic(topicName, topicLen);
        if (topic.size() <= 0)
        {
            OM_INTER_ERROR_PRINT << "[error]mqtt-inter topic empty!";
        } else
        {
            std::string hint;
            std::string topicProfix = topic.substr(0, topic.find("/"));
            OM_MQTT_DEBUG_PRINT << "[topic]" << topic;
            if(thiz->getConfiger().enableDebugPrint)
            {
                OM_INTER_DEBUG_PRINT << "[topic]" << topic << "[topicProfix]" << topicProfix;
            }
            if (topicProfix == "camera")
            {
                hint = "camera";
                thiz->m_EventloopOmCamera->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessageOmCamera(topic, j);
                 });
                // static int message_count_om_camera = 1; // 用于计数的静态变量
                // static int MAX_MESSAGE_COUNT_OM_CAMERA = 1000; // 设置您的最大限制
                //
                // // 根据消息计数判断是偶数还是奇数
                // if (message_count_om_camera % 2 == 0) // 偶数
                // {
                //     thiz->m_EventloopOmCamera->runInLoop([thiz, topic, j]()
                //      {
                //          thiz->mqttDispatchSubscripeMessageOmCamera(topic, j);
                //      });
                // }
                // else // 奇数
                // {
                //     thiz->m_EventloopOmCamera->runInLoop([thiz, topic, j]()
                //      {
                //          thiz->mqttDispatchSubscripeMessageOmCamera(topic, j);
                //      });
                // }
                // if (message_count_om_camera > MAX_MESSAGE_COUNT_OM_CAMERA)
                // {
                //     message_count_om_camera = 0; // 重置计数器
                // }
                // else
                // {
                //     message_count_om_camera++;
                // }

            }
            else if (topicProfix == "radar")
            {
                hint = "radar";

                thiz->m_EventloopOmRadar->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessageOmRadar(topic, j);
                 });
                // static int message_count_om_radar = 1; // 用于计数的静态变量
                // static int MAX_MESSAGE_COUNT_OM_RADAR = 1000; // 设置您的最大限制
                //
                // // 根据消息计数判断是偶数还是奇数
                // if (message_count_om_radar % 2 == 0) // 偶数
                // {
                //     thiz->m_EventloopOmRadar->runInLoop([thiz, topic, j]()
                //      {
                //          thiz->mqttDispatchSubscripeMessageOmRadar(topic, j);
                //      });
                // }
                // else // 奇数
                // {
                //     thiz->m_EventloopOmRadar->runInLoop([thiz, topic, j]()
                //      {
                //          thiz->mqttDispatchSubscripeMessageOmRadar(topic, j);
                //      });
                // }
                // if (message_count_om_radar > MAX_MESSAGE_COUNT_OM_RADAR)
                // {
                //     message_count_om_radar = 0; // 重置计数器
                // }
                // else
                // {
                //     message_count_om_radar++;
                // }

            }
        }
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return ret;
}

NetworkInfo OM_COMPONENT::getNetworkInfo(const std::string& interface_name)
{
    NetworkInfo info;
    std::string command = "ifconfig " + interface_name;
    std::string output;

    // 执行 ifconfig 命令并获取输出
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        OM_MEC_ERROR_PRINT << "Failed to execute command: " << command;
        return info;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        output += buffer;
    }
    pclose(pipe);

    // 解析 ifconfig 输出
    std::regex ip_regex(R"(inet\s+(\d+\.\d+\.\d+\.\d+))");
    std::regex gateway_regex(R"(inet\s+(\d+\.\d+\.\d+\.\d+).+\s+gw\s+(\d+\.\d+\.\d+\.\d+))");
    std::regex netmask_regex(R"(inet\s+\d+\.\d+\.\d+\.\d+\s+netmask\s+(\d+\.\d+\.\d+\.\d+))");

    std::smatch match;
    if (std::regex_search(output, match, ip_regex))
    {
        info.ip_address = match[1];
    }

    if (std::regex_search(output, match, gateway_regex))
    {
        info.ip_address = match[1];
        info.gateway = match[2];
    }

    if (std::regex_search(output, match, netmask_regex))
    {
        info.netmask = match[1];
    }

    return info;
}
std::string OM_COMPONENT::getGatewayAddress()
{
    // 执行 netstat 命令并获取输出
    FILE* pipe = popen("netstat -nr | grep ^0.0.0.0", "r");
    if (!pipe)
	{
        OM_MEC_ERROR_PRINT << "[error]popen error！";
        return ""; // 命令执行失败
    }

    char buffer[128];
    std::string output = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output += buffer;
        }
    }
    pclose(pipe);
    // 从输出中提取网关地址
    std::size_t startPos = output.find("0.0.0.0") + 8;
    std::size_t endPos = output.find("  ", startPos+8);
    std::string gateWayStr = output.substr(startPos, endPos - startPos);
    return trim(gateWayStr);
}
std::string OM_COMPONENT::trim(const std::string& str)
{
    std::string::const_iterator start = str.begin();
    while (start != str.end() && std::isspace(*start))
    {
        ++start;
    }

    std::string::const_iterator end = str.end();
    while (end != start && std::isspace(*(end - 1)))
    {
        --end;
    }

    return std::string(start, end);
}

//违法事件相关函数实现
bool OM_COMPONENT::startHttpServer()
{
    //启动时判断图片目录
    afl::FileUtil::checkDirectory(m_imageDir);
    DIR* dir = opendir(m_imageDir.c_str());
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            // 忽略 "." 和 ".."
            if (entry->d_name[0] == '.')
            {
                continue;
            }
            std::string filePath = std::string(m_imageDir) + entry->d_name;
            remove(filePath.c_str()); // 删除文件
        }
        closedir(dir);
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[success]Directory cleared!";
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "Could not open directory!";
    }

    httplib::Server svr;
    httplib::SocketOptions func = [&](socket_t sockfd)
    {
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            OM_MEC_ERROR_PRINT << "setsockopt(SO_REUSEADDR) failed!";
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        {
            OM_MEC_ERROR_PRINT << "setsockopt(SO_REUSEPORT) failed!";
        }

        struct linger linger_opt;
        linger_opt.l_onoff = 1;  // 启用 linger
        linger_opt.l_linger = 0;  // 设置 linger 时间为 0 秒
        if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0)
        {
            OM_MEC_ERROR_PRINT << "setsockopt(SO_LINGER) failed!";
        }
    };
    svr.set_socket_options(func);

    if(!svr.is_valid())
    {
        OM_MEC_ERROR_PRINT << "[error]camera event http server error";
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "camera event http server start success";
        }
    }

    svr.Post(m_httpPath, [&](const httplib::Request &req, httplib::Response &res)
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "recv camera event http post body:" << req.body.c_str();
        }

        if(req.files.size() < 0)
        {
            OM_MEC_DEBUG_PRINT << "xml and image file not found";
            res.set_content("failed", "text/plain;charset=UTF-8");
        }
        else
        {
            bool havejpg = false;
            if(req.files.size() > 1)
            {
                havejpg = true;
            }
            for(auto &file : req.files)
            {
                std::string param_name = file.first;
                std::string file_name = file.second.filename;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "receive name : " << param_name.c_str() << "receive file : " << file_name.c_str();
                }
                if(file.second.content.empty())
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "file content is empty!";
                    }
                    continue;
                }

                if(file_name.find("xml") != std::string::npos)
                {
                    std::string xmlContent = std::string(file.second.content.begin(), file.second.content.end());
                    pugi::xml_document doc;
                    pugi::xml_parse_result result = doc.load_string(xmlContent.c_str());
                    if (result)
                    {
                        processCameraEventXML(doc, havejpg);
                    }
                    // OM_MEC_DEBUG_PRINT << "Received XML data: " << xmlContent;
                }
                else if(file_name.find("jpg") != std::string::npos)
                {
                    std::string temp_fullname;
                    uint64_t tnow = afl::util::TimeStamp::now(true).millSeconds();
                    if(m_imageNameMap.find(file_name) == m_imageNameMap.end())
                    {
                        continue;
                    }
                    temp_fullname = m_imageNameMap[file_name];
                    std::string save_path = m_imageDir + temp_fullname;
                    std::ofstream outFile(save_path, std::ios::binary);
                    outFile.write(file.second.content.data(), file.second.content.size());
                    outFile.close();
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "Saved image: " << save_path;
                    }
                    m_imageTimeMap[temp_fullname] = tnow;
                }
                else
                {
                    //非xml、jpg文件不做处理
                }
            }

            res.set_content("success", "text/plain;charset=UTF-8");
        }

    });

    svr.listen(m_mecIp.c_str(), m_camerahttpPort);

    return true;
}

bool OM_COMPONENT::tcpinit()
{
    afl::net::InetAddress addrUp(m_tcpIp.c_str(), m_tcpPort);

    if (!m_TcpClient)
    {
        m_TcpClient = std::unique_ptr<afl::net::TcpClient>(new afl::net::TcpClient(m_EventloopOmMec.get(), addrUp));
    }
    m_TcpClient->setConnectionCallback(
            std::bind(&OM_COMPONENT::onTcpConnected, this, std::placeholders::_1));
    m_TcpClient->setMessageCallback(
            std::bind(&OM_COMPONENT::onTcpMessage, this, std::placeholders::_2));
    m_TcpClient->connect();
    m_TcpClient->enableRetry();
    return true;
}

void OM_COMPONENT::onTcpConnected(const afl::net::TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[success]camera event tcp connect success!";
        }
        m_CurrConn = conn;
        m_ConnetedFlag = true;
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[failed]camera event tcp connect failed!";
        m_ConnetedFlag = false;
        m_CurrConn = conn;
        m_CurrConn->shutdown();
//        m_TcpClient->reconnect();
    }
}

void OM_COMPONENT::onTcpMessage(afl::net::ByteBuffer *buffer)
{
    //暂无接收数据需处理
    return;
}

void OM_COMPONENT::processCameraEventXML(pugi::xml_document &doc, bool have_jpg)
{
    //缺少必要字段
    if(doc.child("EventNotificationAlert").empty()
        || doc.child("EventNotificationAlert").child("ANPR").empty()
        || doc.child("EventNotificationAlert").child("UUID").empty())
    {
        return;
    }

    //解析违法事件和图片信息，组包mqtt发给运维平台
    if(!doc.child("EventNotificationAlert").child("ANPR").child("illegalInfo").empty())
    {
        pugi::xml_node illegalInfo_node = doc.child("EventNotificationAlert").child("ANPR").child("illegalInfo");
        //违法事件信息包含必要字段
        if(!illegalInfo_node.child("illegalCode").empty() && !illegalInfo_node.child("illegalName").empty())
        {
            //摘取平台所需illegalInfo_node部分
            pugi::xml_document illegalinfo_doc;
            pugi::xml_node xdec = illegalinfo_doc.prepend_child("EventNotificationAlert");
            xdec.append_attribute("version").set_value("2.0");
            xdec.append_attribute("xmlns").set_value("http://www.isapi.org/ver20/XMLSchema");
            xdec.append_copy(illegalInfo_node);

            std::ostringstream oss;
            illegalinfo_doc.save(oss, "", pugi::format_default);
            std::string illegalinfo_xml = oss.str();
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "get illegalinfo xml:" << illegalinfo_xml.c_str();
            }

            //处理pictureinfolist部分
            m_imageNameMap.clear();
            std::vector<std::string> imagestr;
            std::string uuid = doc.child("EventNotificationAlert").child("UUID").child_value();
            std::string temp_name, temp_fullname;

            if(!doc.child("EventNotificationAlert").child("ANPR").child("pictureInfoList").empty() && have_jpg)
            {
                pugi::xml_node pictureinfolist_node = doc.child("EventNotificationAlert").child("ANPR").child("pictureInfoList");
                for(pugi::xml_node temp_node = pictureinfolist_node.first_child(); temp_node != NULL; temp_node = temp_node.next_sibling())
                {
                    if(temp_node.child("fileName").empty() || temp_node.child("absTime").empty())
                    {
                        continue;
                    }
                    temp_name = temp_node.child("fileName").child_value();
                    temp_fullname = uuid + "_" + temp_node.child("absTime").child_value() + "_" + temp_name;
                    m_imageNameMap[temp_name] = temp_fullname;
                    imagestr.push_back("ftp://" + m_mecIp + ":" + std::to_string(m_ftpPort) + m_imageDir + temp_fullname);
                }
            }

            //mqtt发送给运维平台
            if(m_MqttConnected && m_RegisterFlag)
            {
                upIllegalinfoXML2Cloud(illegalinfo_xml, imagestr);
            }

        }
    }

    //xml去掉pictureinfolist部分, tcp发送给算法
    // doc.child("EventNotificationAlert").child("ANPR").remove_child("pictureInfoList");
    std::ostringstream oss_ndoc;
    doc.save(oss_ndoc, "", pugi::format_default);
    std::string ndoc_xml = oss_ndoc.str();
    upCameraEventXML2Mec(ndoc_xml);

}

void OM_COMPONENT::upIllegalinfoXML2Cloud(std::string illegalinfo, std::vector<std::string> images)
{
    afl::base::json jmsg;
    jmsg["xmlFile"] = illegalinfo;
    jmsg["images"] = images;
    jmsg["location"] = m_jlocation;

    std::string msg;
    std::string hint = "Illegalinfo XML";
    msg = jmsg.dump();
    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_DEBUG_PRINT << "send camera event to mqtt:" << msg.c_str();
    }
    std::string topicPT = m_MqttClientConfig.configerMec.configerTopic.Topic_Abnormal_Behavior_Up;
    mqttPushStringMsg2BrokerCloud(topicPT, msg, hint);
}

void OM_COMPONENT::upCameraEventXML2Mec(std::string cameraevent)
{
    if(m_ConnetedFlag && m_CurrConn)
    {
        m_tcpBuf->retrieveAll();
        m_tcpBuf->write(m_tcpHeader);
        m_tcpBuf->write(cameraevent);
        m_tcpBuf->write(m_tcpTail);

        m_CurrConn->send(m_tcpBuf.get());
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "send camera event to mec success";
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "camera event send failed, tcp not connected!" ;
        return;
    }
}

//定时删除存储六十分钟以上的图片
void OM_COMPONENT::checkImagesTime()
{
    uint64_t tnow = afl::util::TimeStamp::now(true).millSeconds();
    for(std::map<std::string, uint64_t>::iterator iter = m_imageTimeMap.begin(); iter != m_imageTimeMap.end(); )
    {
        std::string file = m_imageDir + iter->first;
        if(afl::FileUtil::isFileExist(file.c_str()))
        {
            if(tnow - iter->second > 60*60*1000)
            {
                if(std::remove(file.c_str()) == 0)
                {
                    if(getConfiger().enableDebugPrint)
					{
                        OM_MEC_DEBUG_PRINT << "delete image file: " << file.c_str();
                    }
                }
                m_imageTimeMap.erase(iter++);
            }
            else
            {
                iter++;
            }
        }
        else
        {
            m_imageTimeMap.erase(iter++);
        }
    }
}

bool OM_COMPONENT::mqttInitConnOptsCloud()
{
    m_MqttConnOptsCloud = MQTTAsync_connectOptions_initializer;
    m_MqttConnOptsCloud.connectTimeout = m_MqttClientConfig.configerMqttCloud.mqttConnectTimeOut;
    m_MqttConnOptsCloud.keepAliveInterval = m_MqttClientConfig.configerMqttCloud.mqttKeepAliveInterval;
    m_MqttConnOptsCloud.cleansession = m_MqttClientConfig.configerMqttCloud.mqttCleanSession;
    m_MqttConnOptsCloud.MQTTVersion = m_MqttClientConfig.configerMqttCloud.mqttVersion;
    m_MqttConnOptsCloud.username = m_MqttClientConfig.configerMqttCloud.mqttUserName.c_str();
    m_MqttConnOptsCloud.password = m_MqttClientConfig.configerMqttCloud.mqttPassword.c_str();
    m_MqttConnOptsCloud.onSuccess = mqttOnConnectCloud;
    m_MqttConnOptsCloud.onFailure = mqttOnConnectFailureCloud;
    m_MqttConnOptsCloud.context = this;
    m_MqttConnOptsCloud.automaticReconnect = 1;
    return true;
}

bool OM_COMPONENT::mqttInitCloud()
{
    m_MqttClientCloud = nullptr;
    m_MqttConnectedCloud = false;

    mqttInitConnOptsCloud();
    std::string clientIdTemp = m_MqttClientConfig.configerMqttCloud.mqttClientId + ":" +
                               std::to_string(afl::util::TimeStamp::now(true).microSeconds());
    if(getConfiger().enableDebugPrint)
    {
        OM_CLOUD_DEBUG_PRINT << "mqtt-cloud [mqttBroker-Url]" << m_MqttClientConfig.configerMqttCloud.mqttBrokerUrl.c_str()
                             << " [client-id]" << clientIdTemp.c_str()
                             << " [username]" << m_MqttClientConfig.configerMqttCloud.mqttUserName.c_str()
                             << " [passwd]" << m_MqttClientConfig.configerMqttCloud.mqttPassword.c_str();
    }
    if (MQTTASYNC_SUCCESS != MQTTAsync_create(&m_MqttClientCloud, m_MqttClientConfig.configerMqttCloud.mqttBrokerUrl.c_str(),
                                              clientIdTemp.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL))
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud create connectiont failure!";
        return false;
    }

    MQTTAsync_setCallbacks(m_MqttClientCloud, this, mqttConnlostCloud, mqttSubscribeMsgArrvdCloud, NULL);
    MQTTAsync_setConnected(m_MqttClientCloud, this, mqttConnectedCallbackCloud);
    if (!mqttConnectCloud())
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud Connect Failure!";
        return false;
    }

    return true;
}

void OM_COMPONENT::mqttDeinitCloud()
{
    if (m_MqttClientCloud)
    {
        OM_MQTT_ERROR_PRINT << "[cloud]mqtt Deinit Cloud!";
        MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
        opts.onSuccess = mqttOnDisconnectCloud;
        opts.context = m_MqttClientCloud;
        MQTTAsync_disconnect(m_MqttClientCloud, &opts);

        int cnt = 0;
        while (m_MqttConnectedCloud && (cnt < 30))
        {
            usleep(10000L);
            cnt++;
        }

        MQTTAsync_destroy(&m_MqttClientCloud);
        m_MqttClientCloud = nullptr;
        m_MqttConnectedCloud = false;
        m_OmMecStatus.heartbeat_flag = false;
        m_OmMecStatus.register_flag = false;
    }
}

void OM_COMPONENT::mqttOnConnectCloud(void *context, MQTTAsync_successData *response)
{
    auto thiz = (OM_COMPONENT *) context;
    if(thiz->getConfiger().enableDebugPrint)
    {
        OM_CLOUD_SUCCESS_PRINT << "[success]mqtt-cloud connect success!";
    }

    thiz->m_MqttConnectedCloud = true;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = mqttOnSubscribeCloud;
    opts.onFailure = mqttOnSubscribeFailureCloud;
    opts.context = thiz;


    if(thiz->m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum > 0)
    {
        if (MQTTAsync_subscribeMany(thiz->m_MqttClientCloud, thiz->m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum,
                                    thiz->m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics,
                                    thiz->m_MqttClientConfig.configerMqttCloud.mqttSubscribeQoss,
                                    &opts) != MQTTASYNC_SUCCESS)
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud subscribe error";
        }
    }
    if(thiz->m_TimerMqttReconnectCloud > 0)
    {
        thiz->m_EventloopCloud->cancelTimer(thiz->m_TimerMqttReconnectCloud);
        thiz->m_TimerMqttReconnectCloud = -1;
    }
}

void OM_COMPONENT::mqttOnConnectFailureCloud(void *context, MQTTAsync_failureData *response)
{
    OM_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud connect error";
    if (response)
    {
        if (response->message)
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error [error-code]" << response->code << " [error-msg]" << response->message;
        } else
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error [error-code]" << response->code << " [error-msg]" << response->message;
        }
    } else
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error!";
    }

    auto thiz = (OM_COMPONENT *) context;
    thiz->mqttReconnectCloud();
}


bool OM_COMPONENT::mqttConnectCloud()
{
    int rc;
    if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(m_MqttClientCloud, &m_MqttConnOptsCloud)))
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error!";
        m_MqttConnectedCloud = false;
        return false;
    }
    return true;
}

bool OM_COMPONENT::mqttReconnectCloud()
{
    m_MqttReconnectCountCloud++;
    OM_CLOUD_ERROR_PRINT << "[notice]mqtt-cloud reconnect![count]" << m_MqttReconnectCountCloud;
    OM_MQTT_ERROR_PRINT << "[cloud][notice]mqtt-cloud reconnect![count]" << m_MqttReconnectCountCloud;
    // if(m_TimerMqttReconnectCloud < 0)
    // {
    //     m_TimerMqttReconnectCloud = m_EventloopCloud->addTimer(std::bind(&OM_COMPONENT::mqttConnectCloud, this), m_MqttClientConfig.configerMqttCloud.mqttReconnectInterval, true);
    // }

    return true;
}

void OM_COMPONENT::mqttConnlostCloud(void *context, char *cause)
{
    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = false;
    OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect lost! [cause]" << cause;
    OM_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud connect lost! [cause]" << cause;
    thiz->mqttReconnectCloud();
    thiz->m_OmMecStatus.heartbeat_flag = false;
    thiz->m_OmMecStatus.register_flag = false;
}

void OM_COMPONENT::mqttOnDisconnectCloud(void *context, MQTTAsync_successData *response)
{
    OM_CLOUD_ERROR_PRINT << "[notice]mqtt-cloud disconnect!";
    OM_MQTT_ERROR_PRINT << "[cloud][notice]mqtt-cloud disconnect!";
    auto thiz = (OM_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = false;
    thiz->m_OmMecStatus.heartbeat_flag = false;
    thiz->m_OmMecStatus.register_flag = false;
}

void OM_COMPONENT::mqttOnSubscribeCloud(void *context, MQTTAsync_successData *response)
{
    OM_CLOUD_SUCCESS_PRINT << "[success]mqtt-cloud subscribe success!";
}

void OM_COMPONENT::mqttOnSubscribeFailureCloud(void *context, MQTTAsync_failureData *response)
{
    OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud subscribe failure!";
    OM_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud subscribe failure!";
}

bool OM_COMPONENT::mqttPublishMsgCloud(const std::string &topic, const std::string &msg)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud  has not connected!";
    //     return false;
    // }

    if (msg.size() <= 0 || topic.empty())
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud len < 0 or topic empty!";
        return false;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    opts.context = m_MqttClientCloud;
    pubmsg.payload = (void *) msg.c_str();
    pubmsg.payloadlen = msg.length();
    pubmsg.qos = m_MqttClientConfig.configerMqttCloud.mqttSendQos;
    pubmsg.retained =  getConfiger().configerMqttCloud.mqttRetained;

    if ((rc = MQTTAsync_sendMessage(m_MqttClientCloud, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        if (rc == MQTTASYNC_DISCONNECTED)
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud Async Connect failure，will connect!";
            if (m_MqttConnectedCloud)
            {
                m_MqttConnectedCloud = false;
                // mqttReconnectCloud();
            }
        } else
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud push msg error，[error-code]" << rc;
        }

        return false;
    }

    return true;
}

int OM_COMPONENT::mqttSubscribeMsgArrvdCloud(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int ret = 1;
    auto thiz = (OM_COMPONENT *) context;
    if (message->payloadlen)
    {
        if(thiz->getConfiger().enableDebugPrint)
        {
            OM_CLOUD_DEBUG_PRINT << "[notice]cloud >>> om " << " [topic]" << topicName << "[msg]" << (char *) (message->payload);
        }
        afl::base::json j;
        try
        {
            j = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
        }
        catch (...)
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud  parse json failure!";
            MQTTAsync_freeMessage(&message);
            MQTTAsync_free(topicName);
            return ret;
        }

        std::string topic(topicName, topicLen);
        if (topic.size() <= 0)
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud topic empty!";
        } else
        {
//            thiz->m_Eventloop->runInLoop([thiz, topic, j]()
//                  {
//                      thiz->mqttDispatchSubscripeMessage(topic, j);
//                  });
            static int message_count = 1; // 用于计数的静态变量
            static int MAX_MESSAGE_COUNT = 1000; // 设置您的最大限制

            // 根据消息计数判断是偶数还是奇数
            if (message_count % 2 == 0) // 偶数
            {
                thiz->m_EventloopCloud->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessageCloud(topic, j);
                 });
            }
            else // 奇数
            {
                thiz->m_EventloopCloud->runInLoop([thiz, topic, j]()
                 {
                     thiz->mqttDispatchSubscripeMessageCloud(topic, j);
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
        }
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return ret;
}
void OM_COMPONENT::mqttDispatchSubscripeMessageCloud(const std::string &topic, const afl::base::json & subScribeJson)
{
    if (!m_MqttConnected)
    {
        OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud  has not connected!";
        return;
    }

    if (topic.empty())
    {
        OM_CLOUD_ERROR_PRINT << "mqtt-cloud topic empty!";
        return;
    }

    if(!topic.empty())
    {
        std::string topicProfix = topic.substr(0, topic.find("/"));
        if(topicProfix == "rscu")
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_CLOUD_DEBUG_PRINT << "[process]" << "cloud >>> mec !";
            }
            processSubscribeDataMecFromCloud(topic, subScribeJson);
        }
        else if (topicProfix == "camera")
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_CLOUD_DEBUG_PRINT << "[process]" << "cloud >>> camera !";
            }
            processSubscribeDataCameraFromCloud(topic, subScribeJson);
        }
        else if(topicProfix == "radar")
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_CLOUD_DEBUG_PRINT << "[process]" << "cloud >>> radar !";
            }
            processSubscribeDataRadarFromCloud(topic, subScribeJson);
        }
        else
        {
            OM_CLOUD_ERROR_PRINT << "[notice][process]" << "no support this message!";
        }

    }

    return;
}
bool OM_COMPONENT::processSubscribeDataRadarFromCloud(const std::string &topic, const afl::base::json & subScribeJson)
{

    std::string hint;
    std::string topicPT;
    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(topicProfix != "radar")
    {
        return false;
    }
      for (auto it = m_MqttClientConfig.configerRadar.topicUnMap.begin(); it != m_MqttClientConfig.configerRadar.topicUnMap.end(); ++it)
    {
        const MqttTopicConfigerOmRadar& mqttTopicConfigerOmRadarTemp = it->second;

        if(topic == mqttTopicConfigerOmRadarTemp.Topic_Register_Ack && m_MqttClientConfig.configerRadar.configerEnable.Enable_Register_Ack)
        {
            hint = "radar-register-ack";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Register_Ack;
            std::string deviceSn = extractDeviceID(topic, "radar");
            if(getConfiger().enableDebugPrint)
            {
                OM_CLOUD_WARN_PRINT << "[radar][deviceSn]" << deviceSn << "[topicPT]" << topicPT << "[hint]" << hint;
            }
            if(topic.find("/register")  != std::string::npos)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CLOUD_WARN_PRINT << "[radar][notice]topic find /register";
                }
                bool registerFlagTemp = false;
                if(topic.find("/ack")  != std::string::npos)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CLOUD_WARN_PRINT << "[radar][notice]topic find /ack";
                    }
                    registerFlagTemp = true;
                }
                else
                {
                    OM_CLOUD_ERROR_PRINT << "[radar][notice]topic no find /ack";
                    registerFlagTemp = false;
                }

                for(auto& radarPair: m_RadarRegisterMap)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CLOUD_WARN_PRINT << "[radar][esn]" << radarPair.first << "[flag]" << (radarPair.second.isRegistered? "true":"false");
                    }
                    if(deviceSn == radarPair.first)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_CLOUD_WARN_PRINT << "[radar][deviceSn]" << deviceSn << "find deviceSn!";
                        }
                        radarPair.second.isRegistered = registerFlagTemp;
                        break;
                    }
                }
            }
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Query_Log && m_MqttClientConfig.configerRadar.configerEnable.Enable_Query_Log)
        {
            hint = "query_log";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Query_Log;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Query_Config && m_MqttClientConfig.configerRadar.configerEnable.Enable_Query_Config)
        {
            hint = "query_config";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Query_Config;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Update_Config && m_MqttClientConfig.configerRadar.configerEnable.Enable_Update_Config)
        {
            hint = "update_config";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Update_Config;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Ota && m_MqttClientConfig.configerRadar.configerEnable.Enable_Ota)
        {
            hint = "ota";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Ota;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Ota_Cancel && m_MqttClientConfig.configerRadar.configerEnable.Enable_Ota)
        {
            hint = "ota";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Ota_Cancel;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Reboot && m_MqttClientConfig.configerRadar.configerEnable.Enable_Reboot)
        {
            hint = "reboot";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Reboot;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_Restore && m_MqttClientConfig.configerRadar.configerEnable.Enable_Restore)
        {
            hint = "restore";
            topicPT = mqttTopicConfigerOmRadarTemp.Topic_Restore;
            break;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_InternalExternalParams_Up_Ack && m_MqttClientConfig.configerRadar.configerEnable.Enable_InternalExternalParams_Up_Ack)
        {
            hint = "radar-param-up-ack";
            if(getConfiger().enableDebugPrint)
            {
                OM_CAMERA_WARN_PRINT << "[hint]" << hint << "[topic]" << topic << " [msg]" << subScribeJson.dump().c_str();
            }
            return true;
        }
        else if(topic == mqttTopicConfigerOmRadarTemp.Topic_InternalExternalParams_Query_Down && m_MqttClientConfig.configerRadar.configerEnable.Enable_InternalExternalParams_Query_Down)
        {
            hint = "radar-param-up-down";
            if(getConfiger().enableDebugPrint)
            {
                OM_CAMERA_WARN_PRINT << "[hint]" <<  hint << "[topic]" << topic << " [msg]" << subScribeJson.dump().c_str();
            }
            return true;
        }
        else
        {
            hint = "";
            topicPT = "";
        }
    }
    if(getConfiger().enableDebugPrint)
    {
        OM_RADAR_DEBUG_PRINT << "[topic]" << hint.c_str();
    }
    hint = "radar";
    if(!topicPT.empty())
    {
        if (!mqttPushMsg2Broker(topicPT, subScribeJson, hint))
        {
            OM_RADAR_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
            return false;
        }
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_RADAR_DEBUG_PRINT << "[topicPT][" << topicPT << "]";
        }
    }

    return true;
}
bool OM_COMPONENT::processSubscribeDataCameraFromCloud(const std::string &topic, const afl::base::json & subScribeJson)
{
    std::string hint;
    std::string topicPT;
    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(topicProfix != "camera")
    {
        return false;
    }
    for (auto it = m_MqttClientConfig.configerCamera.topicUnMap.begin(); it != m_MqttClientConfig.configerCamera.topicUnMap.end(); ++it)
    {
        const MqttTopicConfigerCamera& mqttTopicMecCameraTemp = it->second;
        if(topic == mqttTopicMecCameraTemp.Topic_Register_Ack && m_MqttClientConfig.configerCamera.configerEnable.Enable_Register_Ack)
        {
            hint = "camera-register-ack";
            topicPT = mqttTopicMecCameraTemp.Topic_Register_Ack;
            std::string deviceSn = extractDeviceID(topic);
            if(getConfiger().enableDebugPrint)
            {
                OM_CAMERA_WARN_PRINT << "[camera][deviceSn]" << deviceSn << "[topicPT]" << topicPT << "[hint]" << hint;
            }
            if(topic.find("/register")  != std::string::npos)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_CLOUD_WARN_PRINT << "[camera][notice]topic find /register";
                }
                bool registerFlagTemp = false;
                if(topic.find("/ack")  != std::string::npos)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CLOUD_WARN_PRINT << "[camera][notice]topic find /ack";
                    }
                    registerFlagTemp = true;
                }
                else
                {
                    OM_CLOUD_ERROR_PRINT << "[camera][notice]topic no find /ack";
                    registerFlagTemp = false;
                }

                for(auto& radarPair: m_CameraRegisterMap)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_CLOUD_WARN_PRINT << "[camera][esn]" << radarPair.first << "[flag]" << (radarPair.second.isRegistered? "true":"false");
                    }
                    if(deviceSn == radarPair.first)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_CLOUD_WARN_PRINT << "[camera][deviceSn]" << deviceSn << "find deviceSn!";
                        }
                        radarPair.second.isRegistered = registerFlagTemp;
                        break;
                    }
                }
            }

            break;
        }
        else if(topic == mqttTopicMecCameraTemp.Topic_Restar && m_MqttClientConfig.configerCamera.configerEnable.Enable_Restar)
        {
            hint = "reboot";
            topicPT = mqttTopicMecCameraTemp.Topic_Restar;
            break;
        }
        else if(topic == mqttTopicMecCameraTemp.Topic_Upgrade  && m_MqttClientConfig.configerCamera.configerEnable.Enable_Upgrade)
        {
            hint = "upgrade";
            topicPT = mqttTopicMecCameraTemp.Topic_Upgrade;
            break;
        }
        else if(topic == mqttTopicMecCameraTemp.Topic_Upgrade_Cancel  && m_MqttClientConfig.configerCamera.configerEnable.Enable_Upgrade_Cancel)
        {
            hint = "upgrade";
            topicPT = mqttTopicMecCameraTemp.Topic_Upgrade_Cancel;
            break;
        }
        else if(topic == mqttTopicMecCameraTemp.Topic_Config_Query  && m_MqttClientConfig.configerCamera.configerEnable.Enable_Config_Query)
        {
            hint = "config-query";
            topicPT = mqttTopicMecCameraTemp.Topic_Config_Query;
            break;
        }
        else if(topic == mqttTopicMecCameraTemp.Topic_Config_Down && m_MqttClientConfig.configerCamera.configerEnable.Enable_Config_Down)
        {
            hint = "config-down";
            topicPT = mqttTopicMecCameraTemp.Topic_Config_Down;
            break;
        }
        else
        {
            hint = "";
            topicPT = "";

        }
    }
    OM_CAMERA_DEBUG_PRINT << "[topic]" << hint.c_str();
    hint = "camera";
    if(!topicPT.empty())
    {
        if (!mqttPushMsg2Broker(topicPT, subScribeJson, hint))
        {
            OM_CAMERA_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
        }
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            OM_CAMERA_DEBUG_PRINT << "[topicPT][" << topicPT << "]";
        }
    }

    return true;
}

bool OM_COMPONENT::processSubscribeDataMecFromCloud(const std::string &topic, const afl::base::json & subScribeJson)
{
    std::string hint;
    std::string topicPT;
    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(topicProfix != "rscu")
    {
        return false;
    }

    if(m_MqttClientConfig.configerMec.configerEnable.Enable_Device_BaseInfo_Ack &&
            topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Device_BaseInfo_Ack)
    {
        //注册响应
        subscribeDeviceBaseInfo(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Device_Info_Query && m_RegisterFlag &&
        topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Device_Info_Query )
    {
        //设备信息查询
        subscribeDeviceInfoQuery(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Query_Down && m_RegisterFlag &&
             topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Query_Down)
    {
        //配置查询
        subscribeConfigQueryData(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Config_Update && m_RegisterFlag &&
             topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Config_Update)
    {
        //配置修改
        subscribeConfigUpdateData(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Reboot && m_RegisterFlag &&
             topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Reboot)
    {
        //设备重启
        subscribeRebootData(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Down && m_RegisterFlag &&
             topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Down)
    {
        //设备升级
        subscribeOtaDownData(subScribeJson);
    }
    else if (m_MqttClientConfig.configerMec.configerEnable.Enable_Ota_Cancel && m_RegisterFlag &&
             topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Ota_Cancel)
    {
        //取消升级
        subscribeOtaCancelData(subScribeJson);
    }
    else if( m_MqttClientConfig.configerMec.configerEnable.Enable_Camera_InternalExternalParams_Up_Ack && m_RegisterFlag &&
        topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up_Ack )
    {
        hint = "camera-param-up-ack";
        if(getConfiger().enableDebugPrint)
        {
            OM_CAMERA_WARN_PRINT << "[hint]" << hint << "[topic]" << topic << " [msg]" << subScribeJson.dump().c_str();
        }
        return true;
    }
    else if( m_MqttClientConfig.configerMec.configerEnable.Enable_Camera_InternalExternalParams_Query_Down && m_RegisterFlag &&
        topic == m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query_Ack )
    {
        hint = "camera-param-up-down";
        if(getConfiger().enableDebugPrint)
        {
            OM_CAMERA_WARN_PRINT << "[hint]" <<  hint << "[topic]" << topic << " [msg]" << subScribeJson.dump().c_str();
        }
        return true;
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[error] mqtt-cloud  no subscribe this msg!";
    }

    return true;
}

bool OM_COMPONENT::mqttPushStringMsg2BrokerCloud(string topic, string info, string hint)
{
    if(!info.empty())
    {
        if(!mqttPublishMsgCloud(topic, info))
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud push " << hint.c_str() << " failure!";
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
bool OM_COMPONENT::mqttPushJsonMsg2BrokerCloud(string topic, json info, string hint)
{
    std::string pData;
    try{
        pData = info.dump();
    }
    catch (json::exception& e)
    {
        OM_CLOUD_ERROR_PRINT << "[what]mqtt-cloud " << e.what() << " [json-exception-id]" << e.id;
        return false;
    }
    if(!pData.empty())
    {
        if(!mqttPublishMsgCloud(topic, pData))
        {
            OM_CLOUD_ERROR_PRINT << "[error]mqtt-cloud  push " << hint.c_str() << " failure!";
            return false;
        }
    }else
    {
        return false;
    }
    return true;
}


void OM_COMPONENT::publishRadarInterExterParam()
{
    if (!m_MqttConnectedCloud)
    {
        return ;
    }
    if (!getConfiger().configerRadar.configerEnable.Enable_InternalExternalParams_Up)
    {
        return ;
    }
    if(getConfiger().configerRadar.configerTopicRadarID.radarIDs.size() == m_RadarParamMapFromFile.size())
    {
        if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodUpRadarInterExterParam)
        {
            if(m_TimerPublishRadarInterExterParam > 0)
            {
                m_EventloopOmMec->cancelTimer(m_TimerPublishRadarInterExterParam);
                m_TimerPublishRadarInterExterParam = -1;
            }
        }

        return;
    }
    std::string hint = "radar-inter-exter-param";
    for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
    {
        std::string radarDeviceId = getConfiger().configerRadar.configerTopicRadarID.radarIDs[i];
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[radar-param][radarDeviceId]" << radarDeviceId;
        }
        bool isFindSameDeviceIdAndNoResgisted = false;
        for(auto pair :m_RadarRegisterMap)
        {
            if(pair.first == radarDeviceId)
            {
                if(!pair.second.isRegistered)
                {
                    isFindSameDeviceIdAndNoResgisted = true;
                    break;
                }
            }
        }
        if(isFindSameDeviceIdAndNoResgisted)
        {
            continue;
        }
        bool findFlagPushOnce = false;
        for(auto pairTemp : m_RadarParamMapFromFile)
        {
            if(pairTemp.first == radarDeviceId)
            {
                findFlagPushOnce = true;
                break;
            }
        }
        if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodUpRadarInterExterParam)
        {
            if(findFlagPushOnce)
            {
                continue;
            }
        }


        std::string topicProfixOmRadar = getConfiger().configerRadar.configerTopic.Topic_Profix +  radarDeviceId;
        std::string topicPT = topicProfixOmRadar +  getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Up;
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[topicPT]" << topicPT;
        }
        RadarCalibrationPublishData radarCalibrationPublishData;
        radarCalibrationPublishData.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
        radarCalibrationPublishData.seqNum = afl::util::Srand::srandStr(32);
        radarCalibrationPublishData.deviceID = radarDeviceId;
        std::string deviceEsn = "";
        for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
        {
            if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
            {
                if(radarDeviceId == v.deviceEsn)
                {
                    deviceEsn = v.deviceEsn;
                    radarCalibrationPublishData.latitude = v.deviceLatitude;
                    radarCalibrationPublishData.longitude = v.deviceLongitude;
                    radarCalibrationPublishData.northAngle = v.northAngle;
                    break;
                }
            }
        }
        radarCalibrationPublishData.ack = true;

        if(mqttPushMsg2BrokerCloud(topicPT,radarCalibrationPublishData, hint))
        {
            bool findFlagDeviceId = false;
            for(auto pairTemp:m_RadarParamMapFromFile)
            {
                if(pairTemp.first == radarDeviceId)
                {
                    findFlagDeviceId = true;
                    break;
                }
            }
            if(!findFlagDeviceId)
            {
                RadarParamMapFromFileConfiger radarParamMapFromFileConfiger;
                radarParamMapFromFileConfiger.deviceSn = deviceEsn;
                radarParamMapFromFileConfiger.isPubOnce = true;
                std::pair<std::string,RadarParamMapFromFileConfiger > radarParamPair;
                radarParamPair.first = radarDeviceId;
                radarParamPair.second = radarParamMapFromFileConfiger;
                m_RadarParamMapFromFile.insert(radarParamPair);
            }
        }
    }
}

bool OM_COMPONENT::queryRadarInterExterParam()
{
    // if (!m_MqttConnectedCloud)
    // {
    //     return false;
    // }
    if (!getConfiger().configerRadar.configerEnable.Enable_InternalExternalParams_Query)
    {
        return false;
    }
    string hint = "query radar Inter Exter Param";

    for(uint32_t i = 0; i < getConfiger().configerRadar.configerTopicRadarID.radarIDs.size(); i++)
    {
        std::string radarDeviceId = getConfiger().configerRadar.configerTopicRadarID.radarIDs[i];
        bool isFindSameDeviceIdAndNoResgisted = false;
        for(auto pair :m_RadarRegisterMap)
        {
            if(pair.first == radarDeviceId)
            {
                if(!pair.second.isRegistered)
                {
                    isFindSameDeviceIdAndNoResgisted = true;
                    break;
                }
            }
        }
        if(isFindSameDeviceIdAndNoResgisted)
        {
            continue;
        }

        RadarCalibrationQueryData radarCalibrationQueryData;
        radarCalibrationQueryData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
        radarCalibrationQueryData.deviceID = getConfiger().configerRadar.configerTopicRadarID.radarIDs[i];
        radarCalibrationQueryData.seqNum = afl::util::Srand::srandStr(32);
        std::string topicProfix = getConfiger().configerRadar.configerTopic.Topic_Profix + radarCalibrationQueryData.deviceID;
        std::string topicPT  = topicProfix + getConfiger().configerRadar.configerTopic.Topic_InternalExternalParams_Query           ;

        mqttPushMsg2BrokerCloud(topicPT,radarCalibrationQueryData, hint);
    }
    if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodQueryRadarInterExterParam)
    {
        if(m_TimerQueryRadarInterExterParam > 0)
        {
            m_EventloopOmMec->cancelTimer(m_TimerQueryRadarInterExterParam);
            m_TimerQueryRadarInterExterParam = -1;
        }
    }
    return true;
}

bool OM_COMPONENT::queryCameraInterExterParam()
{
    // if (!m_MqttConnectedCloud)
    // {
    //     return false;
    // }
    if (!getConfiger().configerMec.configerEnable.Enable_Camera_InternalExternalParams_Query)
    {
        return false;
    }

    string hint = "query Camera Inter Exter Param";

    for(uint32_t i = 0; i < getConfiger().configerCamera.configerTopicRcId.RcIds.size(); i++)
    {
        std::string cameraDeviceId = getConfiger().configerCamera.configerTopicRcId.RcIds[i];
        bool isFindSameDeviceIdAndNoResgisted = false;
        for(auto pair :m_CameraRegisterMap)
        {
            if(pair.first == cameraDeviceId)
            {
                if(!pair.second.isRegistered)
                {
                    isFindSameDeviceIdAndNoResgisted = true;
                    break;
                }
            }
        }
        if(isFindSameDeviceIdAndNoResgisted)
        {
            continue;
        }

        CameraCalibrationQueryData cameraCalibrationQueryData;
        cameraCalibrationQueryData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
        cameraCalibrationQueryData.deviceID = getConfiger().configerCamera.configerTopicRcId.RcIds[i];
        cameraCalibrationQueryData.seqNum = afl::util::Srand::srandStr(32);
        // std::string topicProfix = getConfiger().configerCamera.configerTopic.Topic_Profix + cameraCalibrationQueryData.deviceID;
        // std::string topicPT  = topicProfix + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Query           ;
        if(getConfiger().enableDebugPrint)
        {
            OM_MEC_DEBUG_PRINT << "[camera-param]" << m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query;
        }
        mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Query,cameraCalibrationQueryData, hint);
    }
    if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodQueryCameraInterExterParam)
    {
        if (m_TimerQueryCameraInterExterParam > 0) {
            m_EventloopOmMec->cancelTimer(m_TimerQueryCameraInterExterParam);
            m_TimerQueryCameraInterExterParam = -1;
        }
    }
    return true;
}
std::string OM_COMPONENT::extractIpFromFileName(const std::string& fileName)
{
    // 查找下划线的位置
    size_t underscorePos = fileName.find('_');
    // 查找文件扩展名的起始位置
    size_t dotPos = fileName.find_last_of('.');

    // 确保下划线和点都存在，并且下划线在文件扩展名前面
    if (underscorePos != std::string::npos && dotPos != std::string::npos && underscorePos < dotPos) {
        // 提取 IP 地址部分
        return fileName.substr(0, underscorePos);
    }
    return "";  // 如果没有找到有效的IP地址，返回空字符串
}
bool OM_COMPONENT::getCameraInterExterParamFromFile()
{
    if(!getConfiger().configerMec.configerEnable.Enable_Camera_InternalExternalParams_Up)
    {
        return false;
    }
    if(m_FlagGetCameraCalibrationAll)
    {

        if(m_TimerGetCameraInterExterParam > 0)
        {
            m_EventloopOmMec->cancelTimer(m_TimerGetCameraInterExterParam);
            m_TimerGetCameraInterExterParam = -1;
        }

        return false;
    }
    const std::string directoryPath = getConfiger().configerMec.configerCameraInterExterParam.path;
//    if(getConfiger().enableDebugPrint)
//    {
//        OM_MEC_DEBUG_PRINT << "[camera-param][dir] " << directoryPath;
//    }
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(directoryPath.c_str())) != nullptr)
    {
        while ((ent = readdir(dir)) != nullptr)
        {
            // Check if the file has a .json extension
            std::string fileName(ent->d_name);
            if (fileName.size() > 5 && fileName.substr(fileName.size() - 5) == ".json")
            {
                std::string filePath = directoryPath + "/" + fileName;
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_DEBUG_PRINT << "[camera-param][file-path]" << filePath;
                }
                bool findFlagCameraParamFilePath = false;
                for(auto pairFromMapTemp:m_CameraParamMapFromFile)
                {
                    if(pairFromMapTemp.second.cameraParamFilePath == filePath)
                    {
                        findFlagCameraParamFilePath = true;
                    }
                }
                if(findFlagCameraParamFilePath)
                {
//                    if(getConfiger().enableDebugPrint)
//                    {
//                        OM_MEC_DEBUG_PRINT << "[camera-param][success]find the Camera-Param-File-Path!";
//                    }
                    continue;
                }
                //get content
                std::ifstream file(filePath);
                if (!file.is_open())
                {
                    OM_MEC_ERROR_PRINT << "[error]Could not open the file: " << filePath;
                    continue;
                }
                std::string content((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
                if(!content.empty())
                {
                    //get ip from file-name: 172.20.65.20.json
//                    std::string fileNameTemp =   fileName.substr(0, fileName.size() - 5);
                    std::string deviceIpFromFromFileNameTemp = extractIpFromFileName(fileName);
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT << "[camera-ip-from-param-file]" << deviceIpFromFromFileNameTemp ;
                    }
                    std::string deviceSnTemp, deviceEsnTemp;
                    bool isFindDevice = false;
                    for(auto& v : m_OmWorkParamConfiger.sensorDeviceWorkParamList)
                    {
                        if(v.deviceType == WorkParamDeviceTypeCamera && !v.deviceSn.empty())
                        {
                            if(deviceIpFromFromFileNameTemp == v.deviceIp)
                            {
                                isFindDevice = true;
                                deviceSnTemp = v.deviceSn;
                                deviceEsnTemp = v.deviceEsn;
                                break;
                            }
                        }
                    }

                    if(!isFindDevice)
                    {
                        OM_MEC_DEBUG_PRINT << "[camera-ip-from-param-file]" << deviceIpFromFromFileNameTemp;
                        continue;
                    }

                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_DEBUG_PRINT <<"[camera-param-file-name]" << fileName << "[ip]" << deviceIpFromFromFileNameTemp << " [device-sn]" << deviceSnTemp << "[device-esn]" << deviceEsnTemp;
                    }
                    std::pair<std::string, CameraParamMapFromFileConfiger>  fileNameAndContent;
                    CameraParamMapFromFileConfiger cameraParamMapFromFileConfiger;
                    fileNameAndContent.first = deviceSnTemp;
                    bool findFlagDeviceSn = false;
                    for(auto pairFromMapTemp:m_CameraParamMapFromFile)
                    {
                        if(pairFromMapTemp.first == deviceSnTemp)
                        {
                            findFlagDeviceSn = true;
                        }
                    }
                    //no find, insert into m_CameraParamMapFromFile
                    if(!findFlagDeviceSn)
                    {

                        afl::base::json j;
                        try
                        {
                            j = afl::base::json::parse((char *) content.c_str(), (char *) content.c_str() + content.size());
                        }
                        catch (...)
                        {
                            OM_MEC_ERROR_PRINT << "[error]mqtt-inter parse json failure!";
                            continue;
                        }

                        cameraParamMapFromFileConfiger.deviceEsn = deviceEsnTemp;
                        cameraParamMapFromFileConfiger.isPubOnce = false;
                        j["serial_number"] = deviceEsnTemp;
                        if(getConfiger().enableDebugPrint)
                        {
                            OM_MEC_DEBUG_PRINT << "[camera-param][json]" << j.dump();
                        }
                        cameraParamMapFromFileConfiger.cameraParamJsonContent = j;
                        cameraParamMapFromFileConfiger.cameraParamFilePath = filePath;

                        fileNameAndContent.second = cameraParamMapFromFileConfiger;
                        m_CameraParamMapFromFile.insert(fileNameAndContent);
                    }

                }
            }
        }
        closedir(dir);
    } else {
        OM_MEC_ERROR_PRINT << "Could not open directory: " << directoryPath << std::endl;
        return false;
    }
    return true;
}

void OM_COMPONENT::publishCameraInterExterParam()
{
    if (!m_MqttConnectedCloud )
    {
        return ;
    }
    if (!m_RegisterFlag)
    {
        return ;
    }
    if (!getConfiger().configerMec.configerEnable.Enable_Camera_InternalExternalParams_Up)
    {
        OM_MEC_WARN_PRINT << "[notice] open the switch!";
        return ;
    }

    if(m_CameraParamMapFromFile.size() > 0)
    {

        if (getConfiger().enableDebugPrint)
        {
            OM_MEC_ERROR_PRINT << "[camera-param]Camera Param Map From File > 0";
        }
        CameraCalibrationPublishData m_CameraCalibrationPublishData;
        if(m_FlagGetCameraCalibrationAll)
        {
            if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodUpCameraInterExterParam)
            {
                if (m_TimerPublishCameraInterExterParam > 0) {
                    m_EventloopOmMec->cancelTimer(m_TimerPublishCameraInterExterParam);
                    m_TimerPublishCameraInterExterParam = -1;
                }
            }
            if (getConfiger().enableDebugPrint)
            {
                OM_MEC_ERROR_PRINT << "[camera-param]m_FlagGetCameraCalibrationAll is true!";
            }
           return ;
        }
        if(m_CameraParamMapFromFile.size() == getConfiger().configerRadar.configerTopicRadarID.radarIDs.size())
        {
            m_FlagGetCameraCalibrationAll = true;
        }
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_WARN_PRINT << "[notice][camera-param-size-file]" << m_CameraParamMapFromFile.size() << "[work-param-size]"  << getConfiger().configerRadar.configerTopicRadarID.radarIDs.size() ;
            }
        }
        m_CameraCalibrationPublishData.timestamp = afl::util::TimeStamp::now(true).millSeconds();
        m_CameraCalibrationPublishData.seqNum = afl::util::Srand::srandStr(32);
        m_CameraCalibrationPublishData.deviceID = getConfiger().rscuEsn;
        m_CameraCalibrationPublishData.ack = true;
        m_CameraCalibrationPublishData.ackEmpty = false;
        for (auto& cameraParamMapPair: m_CameraParamMapFromFile)
        {
            bool isFindSameDeviceIdAndNoResgisted = false;
            for(auto pair :m_CameraRegisterMap)
            {
                if(pair.first == cameraParamMapPair.second.deviceEsn)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        OM_MEC_WARN_PRINT << "[success]find this device!" << "[sn]" << cameraParamMapPair.first << " [esn]" << pair.first  << "[falg]"  << (pair.second.isRegistered?"true":"false");
                    }
                    if(!pair.second.isRegistered)
                    {
                        isFindSameDeviceIdAndNoResgisted = true;
                        break;
                    }
                }
            }
            // if(isFindSameDeviceIdAndNoResgisted)
            // {
            //     continue;
            // }
            if(!getConfiger().configerMec.configerSensorInterExterParamEnablePeriod.enablePeriodUpCameraInterExterParam)
            {
                if (cameraParamMapPair.second.isPubOnce)
                {
                    continue;
                }
            }
            
            CameraCalibrationData cameraCalibrationData;
            afl::base::json uploadJson;
            try
            {
                cameraCalibrationData = cameraParamMapPair.second.cameraParamJsonContent;
            } catch (json::exception &e)
            {
                OM_MEC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
                return ;
            }
            m_CameraCalibrationPublishData.internalParam.serialNumber = cameraCalibrationData.serial_number;
            m_CameraCalibrationPublishData.internalParam.cameraType = cameraCalibrationData.camera_type;
            m_CameraCalibrationPublishData.internalParam.reprojectionError = std::to_string(cameraCalibrationData.reprojection_error);
            m_CameraCalibrationPublishData.internalParam.calibrationResultFlag = cameraCalibrationData.calibration_result_flag;
            m_CameraCalibrationPublishData.internalParam.height = cameraCalibrationData.height;
            m_CameraCalibrationPublishData.internalParam.width = cameraCalibrationData.width;
            m_CameraCalibrationPublishData.internalParam.distortionModel = cameraCalibrationData.distortion_model;
            m_CameraCalibrationPublishData.internalParam.D = transVector2String(cameraCalibrationData.D);
            m_CameraCalibrationPublishData.internalParam.K = transVector2String(cameraCalibrationData.K);
            m_CameraCalibrationPublishData.internalParam.R = transVector2String(cameraCalibrationData.R);
            m_CameraCalibrationPublishData.internalParam.P = transVector2String(cameraCalibrationData.P);

            //
            MappingData mappingData;
            Position2D twoPosition;
            twoPosition.x = std::to_string(cameraCalibrationData.position_2D.x);
            twoPosition.y = std::to_string(cameraCalibrationData.position_2D.y);
            mappingData.twoPosition = twoPosition;
            Position3D threePosition;
            threePosition.x = std::to_string(cameraCalibrationData.position_3D.x);
            threePosition.y = std::to_string(cameraCalibrationData.position_3D.y);
            threePosition.z = std::to_string(cameraCalibrationData.position_3D.z);
            mappingData.threePosition = threePosition;

            mappingData.serialNumber = cameraParamMapPair.second.deviceEsn;
            std::string  deviceSnFromFile = cameraParamMapPair.first;
            std::string fullPathTemp, filenameTemp;
            if(m_CameraParamPathNameMap.size() <= 0)
            {
                OM_MEC_ERROR_PRINT << "[error]m_CameraParamPathNameMap.size() <= 0!";
                return;
            }

            for(auto pairTemp : m_CameraParamPathNameMap)
            {
                std::string fullPath = pairTemp.first;
                std::string fileName = pairTemp.second;
                std::string deviceSn = getSnFromFileName(fileName);
                if(getConfiger().enableDebugPrint)
                {
                    OM_MEC_WARN_PRINT << "[deviceSn]" << deviceSn << "[fullPath]" << fullPath << "[fileName]" << fileName;
                }
                // 输出文件路径和文件名
                if(deviceSnFromFile == deviceSn)
                {
                    mappingData.imageName = fileName;

                    std::string currentDirPath = getDirectoryPath(fullPath);
                    size_t pos = currentDirPath.find("common_config");

                    std::string result = "common_config/camera_param";
                    if (pos != std::string::npos)
                    {
                        result = currentDirPath.substr(pos);
                    }
                    else
                    {

                        size_t posTemp = getConfiger().configerMec.configerCameraInterExterParam.path.find("common_config");
                        if (pos != std::string::npos)
                        {
                            result = getConfiger().configerMec.configerCameraInterExterParam.path.substr(pos);
                        }
                        else
                        {

                            OM_MEC_ERROR_PRINT << "[error]The specified path segment was not found!";
                        }
                    }

                    if(!result.empty())
                    {
                        mappingData.imageUrl = extractFtpBaseUrl() + "/" + result;
                    }
                    else
                    {
                        mappingData.imageUrl = extractFtpBaseUrl() + "/"+result + getDirNameFromFileName(fileName);
                    }
                }
                else
                {
                    continue;
                }
            }

            m_CameraCalibrationPublishData.externalParam.mappingDataList.push_back(mappingData);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.childFrameId = "camera";
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.frameId = "world";
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.serialNumber = cameraParamMapPair.second.deviceEsn;
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.translation.x = std::to_string(cameraCalibrationData.transform.translation.x);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.translation.y = std::to_string(cameraCalibrationData.transform.translation.y);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.translation.z = std::to_string(cameraCalibrationData.transform.translation.y);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.rotation.x = std::to_string(cameraCalibrationData.transform.rotation.x);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.rotation.y = std::to_string(cameraCalibrationData.transform.rotation.y);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.rotation.z = std::to_string(cameraCalibrationData.transform.rotation.z);
            m_CameraCalibrationPublishData.externalParam.posePositionAngle.transform.rotation.w = std::to_string(cameraCalibrationData.transform.rotation.w);
            std::string hint = "InterExterParam";
            // std::string topicPT = getConfiger().configerCamera.configerTopic.Topic_Profix + cameraParamMapPair.second.deviceEsn + getConfiger().configerCamera.configerTopic.Topic_InternalExternalParams_Up;
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_WARN_PRINT << "[topic]" << m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up;
            }
            if(!m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up.empty())
            {
                if(mqttPushMsg2BrokerCloud(m_MqttClientConfig.configerMec.configerTopic.Topic_Camera_InterExter_Param_Up, m_CameraCalibrationPublishData, hint))
                {
                    cameraParamMapPair.second.isPubOnce = true;
                }
            }
        }
    }
    else
    {
        OM_MEC_ERROR_PRINT << "[camera-param]Camera Param Map From File <= 0";
    }
}

std::string OM_COMPONENT::transVector2String(std::vector<double> paramV)
{
    std::string transValue = "[";
    int index = 1;
    for(auto value:paramV)
    {
        transValue.append(std::to_string(value));
        if(index < paramV.size())
        {
            transValue.append(",");
        }
        index++;
    }
    transValue.append("]");
    return transValue;
}

std::string OM_COMPONENT::getDirNameFromFileName(const std::string& fileName)
{
    // 查找文件扩展名的起始位置
    size_t dotPos = fileName.find('.');

    // 确保下划线在文件扩展名前面
    if (dotPos != std::string::npos )
    {
        // 提取下划线之前的部分作为sn号
        return fileName.substr(0, dotPos);
    }
    return "";  // 如果没有找到有效的sn号，返回空字符串
}

std::string OM_COMPONENT::getDirectoryPath(const std::string& filePath)
{
    size_t lastSlashPosition = filePath.find_last_of('/'); // 找到最后一个斜杠的位置
    if (lastSlashPosition != std::string::npos)
    {
        return filePath.substr(0, lastSlashPosition); // 提取目录路径
    }
    return ""; // 如果找不到斜杠，返回空字符串
}

std::string OM_COMPONENT::extractFtpBaseUrl()
{
    std::string ftpUrlProfix = "";
    ftpUrlProfix.append("ftp://");
    ftpUrlProfix.append(m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpUserName);
    ftpUrlProfix.append(":");
    ftpUrlProfix.append(m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpPassword);
    ftpUrlProfix.append("@");
    ftpUrlProfix.append(m_OmWorkParamConfiger.mecDeviceWorkParam.mecIp);
    ftpUrlProfix.append(":");
    ftpUrlProfix.append(std::to_string(m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpPort));
    std::string ftpDir = m_OmWorkParamConfiger.omPtpLogParamConfiger.ftpDir;

    return ftpUrlProfix;

//    // 查找第一个“/”的位置
//    size_t pos = ftpUrl.find('/', 6); // 从“ftp://”后面开始查找
//    if (pos != std::string::npos) {
//        // 提取“ftp://username:password@host:port”部分
//        return ftpUrl.substr(0, pos);
//    }
//    return ftpUrl; // 如果没有找到，返回原始URL
}

std::string OM_COMPONENT::getSnFromFileName(const std::string& fileName)
{
    // 查找下划线的位置
    size_t underscorePos = fileName.find('_');
    // 查找文件扩展名的起始位置
    size_t dotPos = fileName.find('.');

    // 确保下划线在文件扩展名前面
    if (underscorePos != std::string::npos && dotPos != std::string::npos && underscorePos < dotPos)
    {
        // 提取下划线之前的部分作为sn号
        return fileName.substr(0, underscorePos);
    }
    return "";  // 如果没有找到有效的sn号，返回空字符串
}
bool OM_COMPONENT::findJpgFiles(const std::string& baseDir)
{
    if(!getConfiger().configerMec.configerEnable.Enable_Camera_InternalExternalParams_Up)
    {
        return false;
    }

    afl::FileUtil::checkDirectory(baseDir);
    DIR* dir = opendir(baseDir.c_str());

    if (dir == nullptr)
    {
        OM_MEC_ERROR_PRINT << "[camera-param]Error opening directory: " << baseDir;
        return false;
    }

    struct dirent* entry;
    // 遍历目录
    while ((entry = readdir(dir)) != nullptr)
    {
        // 跳过当前目录和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::string fullPath = baseDir + "/" + entry->d_name;

        // 检查是否是目录
        if (entry->d_type == DT_DIR)
        {
            // 如果是目录，则递归遍历
            findJpgFiles(fullPath);
        }
        // 检查是否是常规文件
        else if (entry->d_type == DT_REG)
        {
                std::string filename(entry->d_name);
//                OM_MEC_DEBUG_PRINT << "[camera-param][filename]" << filename;
                if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0)
                {
//                    OM_MEC_DEBUG_PRINT << "[[camera-param][path]" <<  fullPath << "[filename]" << filename;
                    std::pair<std::string, std::string> pairPathName;
                    pairPathName.first = fullPath;
                    pairPathName.second = filename;
                    m_CameraParamPathNameMap.insert(pairPathName);
                }
            }
    }
//    OM_MEC_WARN_PRINT << "[camera-param]scan dir(" << baseDir <<") success!";
    closedir(dir); // 关闭目录
    return true;
}
//信号灯自检
void  OM_COMPONENT::initProcessTrafficlightDetectData()
{
    auto reader = node_->CreateReader<airos::usecase::EventOutputResult>(
            m_MqttClientConfig.configerMec.configerTrafficlightDetectData.channel_readers,
            std::bind(&OM_COMPONENT::processTrafficlightDetectData, this, std::placeholders::_1));

}
//信号灯自检
void  OM_COMPONENT::processTrafficlightDetectData(const std::shared_ptr<const airos::usecase::EventOutputResult> &trafficlightDetectData)
{
    if(!getConfiger().configerMec.configerEnable.Enable_TrafficlightDetectData)
    {
        return ;
    }
    if (!m_MqttConnectedCloud)
    {
        return ;
    }
    if (!m_RegisterFlag)
    {
        return;
    }
    if (!trafficlightDetectData)
    {
        OM_MEC_ERROR_PRINT << "Received mec-data failed!";
        return ;
    }
    uint64_t nowMs = afl::util::TimeStamp::now(true).millSeconds();
    if (trafficlightDetectData->has_trafficlight_detect_data())
    {

        TrafficlightDetectData trafficlightDetectDataUp;
        auto trafficlight_detect_data = trafficlightDetectData->trafficlight_detect_data();
        if(m_TimerPublishTrafficlightDetectData > 0)
        {
            m_EventloopOmMec->cancelTimer(m_TimerPublishTrafficlightDetectData);
            m_TimerPublishTrafficlightDetectData = -1;
        }
        // if(m_TimerPublishTrafficlightDetectDataInit > 0)
        // {
        //     m_EventloopOmMec->cancelTimer(m_TimerPublishTrafficlightDetectDataInit);
        //     m_TimerPublishTrafficlightDetectDataInit = -1;
        // }
        TrafficlightDetectStatus tlds = (TrafficlightDetectStatus)trafficlight_detect_data.trafficlight_status();
        if(tlds != TrafficlightDetectStatus::TLS_NORMAL)//红绿灯产生异常
        {
            if(m_TLfaultMap.find(tlds) != m_TLfaultMap.end())
            {
                return;
            }

            m_TLfaultMap[tlds] = nowMs;

            trafficlightDetectDataUp.faultStatus = 1;
            trafficlightDetectDataUp.faultStartTime = trafficlight_detect_data.time_stamp();
            trafficlightDetectDataUp.faultStopTime = 0;
            trafficlightDetectDataUp.faultDescription = "tl fault " + getTLFaultDesc(tlds) + " begin.";
            trafficlightDetectDataUp.timeStamp = nowMs;
            trafficlightDetectDataUp.seqNum = afl::util::Srand::srandStr(32);
            trafficlightDetectDataUp.rscuEsn = m_MqttClientConfig.rscuEsn;
            trafficlightDetectDataUp.trafficlightStatus = (os::v2x::protocol::om::mec::TrafficlightDetectStatus)tlds;

            std::string topicPT =  m_MqttClientConfig.configerMec.configerTopic.Topic_Trafficlight_Detect;
            std::string hint = "trafficlight detect";
            if(!topicPT.empty())
            {
                if(!mqttPushMsg2BrokerCloud(topicPT, trafficlightDetectDataUp, hint))
                {
                    OM_MEC_ERROR_PRINT << "[error] push spat data failure!";
                }
                else
                {
                    // m_TimerPublishTrafficlightDetectData  = m_EventloopOmMec->addTimer(
                    //         std::bind(&OM_COMPONENT::publishTrafficlightDetectData, this),
                    //         getConfiger().configerMec.configerPublishPeriod.periodPublishTrafficlightDetectDataInterval, true);

                }
            }

        }
        else//异常消失
        {
            if(m_TLfaultMap.size() == 0)
            {
                return;
            }
            uint64_t normalTime = INT_MAX;
            for(auto& iter : m_TLfaultMap)
            {
                uint64_t tempTime = nowMs - iter.second;
                if(normalTime > tempTime)
                {
                    normalTime = tempTime;
                }
            }
            //正常持续时间超过某阈值才认为故障消失
            if(normalTime < getConfiger().spatNormalJudgeTime*1000)
            {
                return;
            }

            for(auto& iter : m_TLfaultMap)
            {

                trafficlightDetectDataUp.faultStatus = 0;
                trafficlightDetectDataUp.faultStartTime = iter.second;
                trafficlightDetectDataUp.faultStopTime = trafficlight_detect_data.time_stamp();
                trafficlightDetectDataUp.faultDescription = "tl fault " + getTLFaultDesc(iter.first) + " end.";
                trafficlightDetectDataUp.timeStamp = nowMs;
                trafficlightDetectDataUp.seqNum = afl::util::Srand::srandStr(32);
                trafficlightDetectDataUp.rscuEsn = m_MqttClientConfig.rscuEsn;
                trafficlightDetectDataUp.trafficlightStatus = (os::v2x::protocol::om::mec::TrafficlightDetectStatus)iter.first;

                std::string topicPT =  m_MqttClientConfig.configerMec.configerTopic.Topic_Trafficlight_Detect;
                std::string hint = "trafficlight detect";
                if(!topicPT.empty())
                {
                    if(!mqttPushMsg2BrokerCloud(topicPT, trafficlightDetectDataUp, hint))
                    {
                        OM_MEC_ERROR_PRINT << "[error] push spat data failure!";
                    }
                    else
                    {
                        // m_TimerPublishTrafficlightDetectData  = m_EventloopOmMec->addTimer(
                        //         std::bind(&OM_COMPONENT::publishTrafficlightDetectData, this),
                        //         getConfiger().configerMec.configerPublishPeriod.periodPublishTrafficlightDetectDataInterval, true);

                    }
                }
            }
            m_TLfaultMap.clear();
        }


    }
    else{
      OM_MEC_ERROR_PRINT << "[error] no trafficlight_detect_data !";
    }
}
//信号灯自检：推送检测数据
void OM_COMPONENT::publishTrafficlightDetectData()
{

    if(!getConfiger().configerMec.configerEnable.Enable_TrafficlightDetectData)
    {
        return ;
    }
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    if (!m_RegisterFlag)
    {
        return;
    }

    TrafficlightDetectData trafficlightDetectDataUp;
    trafficlightDetectDataUp.timeStamp = afl::util::TimeStamp::now(true).millSeconds();
    trafficlightDetectDataUp.seqNum = afl::util::Srand::srandStr(32);
    trafficlightDetectDataUp.rscuEsn = m_MqttClientConfig.rscuEsn;
    trafficlightDetectDataUp.trafficlightStatus = TLS_BLACK_FAILURE;  //正常

    std::string topicPT =  m_MqttClientConfig.configerMec.configerTopic.Topic_Trafficlight_Detect;
    std::string hint = "trafficlight detect";
    if(!topicPT.empty())
    {
        if(!mqttPushMsg2BrokerCloud(topicPT, trafficlightDetectDataUp, hint))
        {
            OM_MEC_ERROR_PRINT << "[error] push spat data failure!";
        }
    }

}
std::string OM_COMPONENT::getContentBetweenFirstAndSecondSlash(const std::string& input)
{
        size_t firstSlash = input.find('/');
        size_t secondSlash = input.find('/', firstSlash + 1);

        if (firstSlash != std::string::npos && secondSlash != std::string::npos)
        {
            return input.substr(firstSlash + 1, secondSlash - firstSlash - 1);
        }

        return ""; // 如果没有找到，返回空字符串
}

void  OM_COMPONENT::getTimingInfo()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        OM_MEC_ERROR_PRINT << "[error] timing udp socket create failed!";
        return;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;   // 监听所有网卡
    addr.sin_port = htons(19999);         // 监听端口 19999

    for(int i = 0; i < 5; i++)
    {
        if(bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
        {
            OM_MEC_ERROR_PRINT << "[error]timing udp socket bind error(try times: " << i << ")";
            if(i == 4)
            {
                return;
            }
            sleep(3000);//3s后再次尝试
        }
        else
        {
            break;
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MEC_WARN_PRINT << "timing udp listening on port 19999...";
    }
    char buf[1024];
    sockaddr_in client{};
    socklen_t len = sizeof(client);
    uint64_t gps_sec = 0;
    uint64_t gps_nsec = 0;
    uint64_t local_sec = 0;
    uint64_t local_nsec = 0;  
    uint64_t ptp_sec = 0;
    uint64_t ptp_nsec = 0;
    uint8_t source = 2;
    while(true) 
    {
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (sockaddr*)&client, &len);
        if(n < 0) 
        {
            continue;
        }
        if(n != 28)//固定长度28字节
        {
            OM_MEC_ERROR_PRINT << "timing info length error";
            continue;
        }
        gps_sec = (uint64_t)buf[0] + ((uint64_t)buf[1] << 8) + ((uint64_t)buf[2] << 16) + ((uint64_t)buf[3] << 24);
        gps_nsec = (uint64_t)buf[4] + ((uint64_t)buf[5] << 8) + ((uint64_t)buf[6] << 16) + ((uint64_t)buf[7] << 24);
        local_sec = (uint64_t)buf[8] + ((uint64_t)buf[9] << 8) + ((uint64_t)buf[10] << 16) + ((uint64_t)buf[11] << 24);
        local_nsec = (uint64_t)buf[12] + ((uint64_t)buf[13] << 8) + ((uint64_t)buf[14] << 16) + ((uint64_t)buf[15] << 24);
        ptp_sec = (uint64_t)buf[16] + ((uint64_t)buf[17] << 8) + ((uint64_t)buf[18] << 16) + ((uint64_t)buf[19] << 24);
        ptp_nsec = (uint64_t)buf[20] + ((uint64_t)buf[21] << 8) + ((uint64_t)buf[22] << 16) + ((uint64_t)buf[23] << 24);
        source = buf[24];
        m_currentSource.store(source);
        if(source == 0)
        {
            m_gpsTime.store(gps_sec*1000 + gps_nsec/(1000*1000));
            m_gpsTimeDifference.store((local_sec - gps_sec)*1000*1000*1000 + local_nsec - gps_nsec);
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "gpsTime:" << m_gpsTime.load() << ", gpsTimeDifference: " << m_gpsTimeDifference.load();
            }
        }
        else if(source == 1)
        {
            m_ptpTime.store(ptp_sec*1000 + ptp_nsec/(1000*1000));
            m_ptpTimeDifference.store((local_sec - ptp_sec)*1000*1000*1000 + local_nsec - ptp_nsec);
            if(getConfiger().enableDebugPrint)
            {
                OM_MEC_DEBUG_PRINT << "ptpTime:" << m_ptpTime.load() << ", ptpTimeDifference: " << m_ptpTimeDifference.load();
            }
        }
        else
        {

        }
    }

    close(sockfd);

}


std::string OM_COMPONENT::getTLFaultDesc(TrafficlightDetectStatus tlds)
{
    std::string tldsStr = "";
    switch (tlds)
    {
    case TrafficlightDetectStatus::TLS_BLACK_FAILURE:
        tldsStr = "black";
        break;
    case TrafficlightDetectStatus::TLS_LIGHT_COLOR_CONFLICT:
        tldsStr = "color conflict";
        break;        
    case TrafficlightDetectStatus::TLS_COUNTDOWN_MISMATCH_WITH_ON_SITE_CONDITIONS:
        tldsStr = "count mismatch with on site conditions";
        break;
    case TrafficlightDetectStatus::TLS_SECOND_FREEZING:
        tldsStr = "second freezing";
        break;
    case TrafficlightDetectStatus::TLS_SKIPPING_SECONDS:
        tldsStr = "skipping second";
        break; 
    case TrafficlightDetectStatus::TLS_COUNTDOWN_REVERTING:
        tldsStr = "count reverting";
        break;
    case TrafficlightDetectStatus::TLS_COUNTDOWN_NOT_EQUAL_TO_1_WHEN_CHANGING_LIGHTS:
        tldsStr = "count not equal to 1 when changing lights";
        break;        
    case TrafficlightDetectStatus::TLS_ALL_RED:
        tldsStr = "all red";
        break;
    case TrafficlightDetectStatus::TLS_ALL_GREEN:
        tldsStr = "all green";
        break;
    case TrafficlightDetectStatus::TLS_ALL_YELLOW:
        tldsStr = "all yellow";
        break;    
    default:
        break;
    }
    return tldsStr;
}
NAMESPACE_ENDED_OM