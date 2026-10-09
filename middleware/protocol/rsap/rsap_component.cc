/*
 * @Author: zhangenwei
 * @Date: 2024-02-18 9:20:01
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-20 9:21:11
 * @Description:
 */
#include "rsap_component.h"
NAMESPACE_PROTOCOL_THREAD_START
DEFINE_string(server_ip, "172.20.28.40", "tcp server ip");
DEFINE_int32(server_port, 40414, "tcp server port");
DEFINE_int32(local_port, 10051, "tcp server local  port");

DEFINE_double(heart_beat_period, 2, "tcp server heart_beat_period");
DEFINE_double(heart_beat_monitor_period   , 2, "heart_beat_monitor_period");
DEFINE_double(enbale_heart_beat_reconnect   , 1, "enbale_heart_beat_reconnect");
DEFINE_double(heart_beat_reconnect_time_minute   , 3, "heart_beat_reconnect_time_minute");

DEFINE_double(device_status_period, 2, "tcp server device_status_period");
DEFINE_double(device_status_monitor_period, 2, "device_status_monitor_period");
DEFINE_double(device_status_reconnect_time_minute, 2, "device_status_reconnect_time_minute");
DEFINE_double(enbale_device_status_reconnect, 2, "enbale_device_status_reconnect");
DEFINE_double(device_status_del_period, 1, "device_status_del_period");


DEFINE_double(event_monitor_period, 2, "event_monitor_period");
DEFINE_double(event_reconnect_time_minute, 2, "event_reconnect_time_minute");
DEFINE_double(enbale_event_reconnect, 2, "enbale_event_reconnect");

DEFINE_string(protocol, "tcp", "protocol : tcp");
DEFINE_string(obj_rcuId, "01234567", "rcu-id");
DEFINE_string(obj_device_id, "1122334455667788990011", "obj_device_id");
DEFINE_int32(obj_deviceType, 1, "device-type");

DEFINE_string(project_root, "/home/airos/protocl/rsap/", "project_root");
DEFINE_string(tls_root, "tls/", "tls_root");
DEFINE_string(tls_ca_file_name, "ca.crt", "tls_ca_file_name");
DEFINE_string(tls_client_key_file_name, "U-XA000M.crt", "tls_client_key_file_name");
DEFINE_string(tls_client_private_key_file_name, "U-XA000M_pkcs8.key", "tls_client_private_key_file_name");
DEFINE_string(tls_client_private_Key_passwd, "123456", "tls_client_private_Key_passwd");

DEFINE_int32(enable_use_tls, 1, "enable_use_tls");
DEFINE_int32(enable_use_02   , 1, "enable_use_02");
DEFINE_int32(enable_use_add_utc8   , 1, "enable_use_add_utc8");

DEFINE_int32(enable_push_heartbeat, 1, "enable_push_heartbeat");
DEFINE_int32(enable_push_obj, 1, "enable_push_obj");
DEFINE_int32(enable_push_event, 1, "enable_push_event");
DEFINE_int32(enable_push_status, 1, "enable_push_status");

DEFINE_int32(enable_scribe_heartbeat   , 1, "enable_scribe_heartbeat");
DEFINE_int32(enable_scribe_obj   , 1, "enable_scribe_obj");
DEFINE_int32(enable_scribe_event   , 1, "enable_scribe_event");
DEFINE_int32(enable_scribe_status   , 1, "enable_scribe_status");

DEFINE_int32(enable_print_info   , 1, "enable_print_info");
DEFINE_int32(enable_print_info_log   , 1, "enable_print_info_log");

DEFINE_int32(enable_print_parse_info_heartbeat   , 1, "enable_print_parse_info_heartbeat");
DEFINE_int32(enable_print_parse_info_obj   , 1, "enable_print_parse_info_obj");
DEFINE_int32(enable_print_parse_info_event   , 1, "enable_print_parse_info_event");
DEFINE_int32(enable_print_parse_info_status   , 1, "enable_print_parse_info_status");
DEFINE_string(db_path, "/airos/device-status.db", "db_path");
DEFINE_string(db_table_name, "device-status", "db_table_name");
DEFINE_string(device_no_map_file_name, "device_no_map.txt", "device_no_map_file_name");
DEFINE_string(conf_dir_name, "conf", "conf_dir_name");
DEFINE_string(work_param_file_path, "work_param_file_path", "/home/airos/common_config/work_param_config.flag");
DEFINE_int32(enable_print_sensor_channel_data_info, 2, "print sensor data from channel");
DEFINE_int32(enable_use_real_dev_no, 2, "print sensor data from channel");
DEFINE_int32(enable_use_camera_no_add_01_as_hash_compute, 2, "enable_use_camera_no_add_01_as_hash_compute");

bool RSAP_COMPONENT::Init()
{
#if ENABLE_ENCRYPTION
    auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
    std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
    RSAP_DEBUG_PRINT << "License is: " << license << std::endl;
    int result = FusionService::Authenticator::GetInstance().Authorize(license);
    if (result != 0)
    {
        RSAP_ERROR_PRINT << "License generated fail, error code:  " << result;
        exit(1);
    }
#endif

    // if (m_CloudServiceConfig.enablePrintInfoLog)
    // {
    //     RSAP_WARN_PRINT << "version:(1.0-解决使用网络助手模拟云控的问题)AIROS_COMPONENT_CLASS_NAME(RsapServiceComponent) Init...";
    // }

    getWorkParamFromFile();

    if (!getServerConfigInfo(m_CloudServiceConfig))
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "get config-info error!";
        }

        return false;
    }
    getDeviceNoMapInfoFromFile(m_DeviceNoMapInfoMap);
    if (!m_Task)
    {
        m_Task.reset(new std::thread([&](){ tcpInit(m_CloudServiceConfig); }));
    }
    m_DeviceStatusMsgDBPtr.reset(new DeviceStatusMsgDB(m_CloudServiceConfig.deviceStatusDbPath, m_CloudServiceConfig.enablePrintInfoLog));
    getSensorDeviceNoHash();
    m_ParseInfo.setCloudServiceConfig(m_CloudServiceConfig);
    return true;
}

bool RSAP_COMPONENT::tcpInit(CloudServiceConfig  cloudServiceConfig )
{
    afl::net::InetAddress addrUp(cloudServiceConfig.serverIP.c_str(), cloudServiceConfig.serverPort);
    m_Eventloop = std::make_shared<afl::net::EventLoop>();

    if (!m_TcpClient)
    {
        std::string sslParentPath  = m_CloudServiceConfig.projectRoot + m_CloudServiceConfig.tlsRoot;

        if (!afl::FileUtil::isDirectory(sslParentPath.c_str()))
        {
            afl::FileUtil::createRecursionDir(sslParentPath.c_str());
        }
        m_TlsCtxInfo.tlsCaCertificateFile = sslParentPath + m_CloudServiceConfig.tlsCaFileName;
        m_TlsCtxInfo.tlsClientCertificateFile = sslParentPath + m_CloudServiceConfig.tlsClientKeyFileName;
        m_TlsCtxInfo.tlsClientPrivateKeyFile = sslParentPath + m_CloudServiceConfig.tlsClientPrivateKeyFileName;
        m_TlsCtxInfo.tlsClientPrivateKeyPassword =  m_CloudServiceConfig.tlsClientPrivateKeyPasswd;
        m_TlsCtxInfo.tlsClient = true;
        m_TlsCtxInfo.tlsAuthMode = afl::net::TlsAuthMode::MutualAuthentication;


        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[ip]" << cloudServiceConfig.serverIP <<"[port]" << cloudServiceConfig.serverPort;
            RSAP_DEBUG_PRINT << "[ca-path]" << m_TlsCtxInfo.tlsCaCertificateFile.c_str();
            RSAP_DEBUG_PRINT << "[client-cert-path]" << m_TlsCtxInfo.tlsClientCertificateFile.c_str();
            RSAP_DEBUG_PRINT << "[client-key-path]" << m_TlsCtxInfo.tlsClientPrivateKeyFile.c_str();
            RSAP_DEBUG_PRINT << "[passwd]" << m_TlsCtxInfo.tlsClientPrivateKeyPassword.c_str();
        }
        m_TcpClient = std::unique_ptr<afl::net::TcpClient>(new afl::net::TcpClient(m_Eventloop.get(), addrUp));
    }
    m_TcpClient->setConnectionCallback(
            std::bind(&RSAP_COMPONENT::onConnected, this,
                      std::placeholders::_1));
    m_TcpClient->setMessageCallback(
            std::bind(&RSAP_COMPONENT::onMessage, this,
                      std::placeholders::_2));
    m_TcpClient->connect();
    m_TcpClient->enableRetry();

    if(m_Eventloop)
    {
        timerHeartBeatFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushHearBeatInfo, this),
                                                 m_CloudServiceConfig.heartBeatPeriod, true);
        timerDeviceStatusFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushDeviceStatusData, this),
                                                    m_CloudServiceConfig.deviceStatusPeriod, true);
        timer_monitor_sensor_data_up = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorSensorDataUp, this), 10, true);
        timer_monitor_mec_link_status = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorMecLinkStatus, this), 5, true);
    }
    m_Eventloop->loop();
    return true;
}

void RSAP_COMPONENT::getSensorDeviceNoHash()
{
    int counter = 0;
//    RSAP_DEBUG_PRINT << "[omWorkParamConfiger]" << omWorkParamConfiger.to_string();
    for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
    {
        if (v.deviceType == WorkParamDeviceTypeRadar || v.deviceType == WorkParamDeviceTypeCamera)
        {
            if (!v.deviceEsn.empty())
            {
                std::pair<std::string, SensorDeviceEsnHash> deviceEsnHashPair;
                SensorDeviceEsnHash sensorDeviceEsnHash;
                deviceEsnHashPair.first = v.deviceSn;
                
                if(m_CloudServiceConfig.enable_use_camera_no_add_01_as_hash_compute)
                {
                    //使用相机的esn，在后边增加01
                
                    if (v.deviceType == WorkParamDeviceTypeCamera)
                    {
                        if (m_CloudServiceConfig.enable_use_camera_no_add_01_as_hash_compute)
                        {
                            std::string deviceEsnNew =  v.deviceEsn + "01";
                            sensorDeviceEsnHash.deviceESn = deviceEsnNew;
                            if(m_CloudServiceConfig.enablePrintInfoLog)
                            {
                                RSAP_DEBUG_PRINT << "[camera-no]" << deviceEsnNew;
                            }
                            sensorDeviceEsnHash.hashValue = computeHash(deviceEsnNew);
                        }
                    }
                    else if(v.deviceType == WorkParamDeviceTypeRadar)
                    {
                        sensorDeviceEsnHash.deviceESn = v.deviceEsn;
                        sensorDeviceEsnHash.hashValue = computeHash(v.deviceEsn);
                    }
                    deviceEsnHashPair.second = sensorDeviceEsnHash;
                    m_SensorDeviceNoMapInfoMap.insert(deviceEsnHashPair);
                    counter++;
                }
                else
                {
                     if(v.deviceType == WorkParamDeviceTypeRadar)
                    {
                         if (m_CloudServiceConfig.enablePrintInfoLog)
                         {
                             RSAP_DEBUG_PRINT << "[radar-no]" << v.deviceEsn;
                         }
                        sensorDeviceEsnHash.deviceESn = v.deviceEsn;
                        sensorDeviceEsnHash.hashValue = computeHash(v.deviceEsn);
                        deviceEsnHashPair.second = sensorDeviceEsnHash;
                        m_SensorDeviceNoMapInfoMap.insert(deviceEsnHashPair);
                        counter++;
                    }
                    else
                    {
                        if (m_CloudServiceConfig.enablePrintInfoLog)
                        {
                            RSAP_DEBUG_PRINT << "[notice]no process camara-no!";
                        }
                    }
                }
            }
        }
    }
    if (m_CloudServiceConfig.enablePrintInfoLog)
    {
        for(auto pair:m_SensorDeviceNoMapInfoMap)
        {
            RSAP_DEBUG_PRINT << "[sn]" << pair.first  << "[esn]" << pair.second.deviceESn << "[hashValue]" << pair.second.hashValue;
        }
    }
}

void RSAP_COMPONENT::monitorSensorDataUp()
{
    if(send_obj_package_old == send_obj_package_new && send_obj_package_old != 0)
    {
//        RSAP_DEBUG_PRINT << "[notice] receive data";
        send_obj_package_old++;
    }
    else
    {
        output_monitor_->Clear();
        auto* monitor_rsap_response =  output_monitor_->mutable_rsap_response();
        monitor_rsap_response->set_tag(airos::monitor::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_UP);
        monitor_rsap_response->set_timestamp( afl::util::TimeStamp::now(true).millSeconds());
//        auto* sensor_data_up = mec_sensor_data_monitor->mutable_sensor_data_up();
        monitor_rsap_response->set_sensor_cloud_ip(m_CloudServiceConfig.serverIP);
        monitor_rsap_response->set_sensor_cloud_port(std::to_string(m_CloudServiceConfig.serverPort));
        if (m_ConnetedFlag  && m_CertVerifyFlag)
        {
            monitor_rsap_response->set_sensor_cloud_con_flag(true);
        }
        else
        {
            monitor_rsap_response->set_sensor_cloud_con_flag(false);
        }

        monitor_rsap_response->set_sensor_dat_up_flag(false);
        monitor_rsap_response->set_sensor_data_up_obj_package(0);
        monitor_rsap_response->set_sensor_data_up_obj_num(0);
        Send("/v2x/monitor", output_monitor_);
//        RSAP_DEBUG_PRINT << "[notice] no sensor-data!" << output_monitor_->DebugString();
        send_obj_package_old++;
    }
}

bool RSAP_COMPONENT::Proc(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if (!mecDeviceData)
    {

        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "Received mec-data failed!";
        }
        return false;
    }
//    if(m_CloudServiceConfig.enablePrintInfoLog)
//    {
//        RSAP_ERROR_PRINT << "[notice]recv data";
//    }


    if(!m_ConnetedFlag || !m_CurrConn)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[error]tcp not connected!";
        }
        return false;
    }
    if(!m_CertVerifyFlag)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[error]ssl cert Verify failed!";
        }
        return false;
    }
    // static int message_count_upload = 1;        // 用于计数的静态变量
    // static int MAX_MESSAGE_COUNT_UPLOAD = 1000; // 设置您的最大限制
    //                                             // 根据消息计数判断是偶数还是奇数
    // if (message_count_upload % 2 == 0)          // 偶数
    // {
    //     if (!m_EventloopRsapPubFirst && m_EventloopRsapPubFirstCreated)
    //     {
    //         return false;
    //     }
    //     m_EventloopRsapPubFirst->runInLoop([this, mecDeviceData]()
    //                                        { this->processPbData2RsapData(mecDeviceData); });
    // }
    // else // 奇数
    // {
    //     if (!m_EventloopRsapPubSecond && m_EventloopRsapPubSecondCreated)
    //     {
    //         return false;
    //     }
    //     m_EventloopRsapPubSecond->runInLoop([this, mecDeviceData]()
    //                                         { this->processPbData2RsapData(mecDeviceData); });
    // }
    processPbData2RsapData(mecDeviceData);
    //  Send("/v2x/service/mec/data", mecDeviceData);
    return true;
}

void RSAP_COMPONENT::onMessage(afl::net::ByteBuffer *buffer)
{
    if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        printBufferData(*buffer, "服务器响应");
    }
    RsapMsgHeader &rsapMsgHeader = *((RsapMsgHeader*)buffer->peek());
    if(rsapMsgHeader.StartFlag != RSAP_PACKAGE_START_FLAG)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[error]StartFlag is error!";
        }
        buffer->retrieveAll();
        return ;
    }

    time_t  minTemp = static_cast<unsigned long long>(ntohl(rsapMsgHeader.TimestampMIN));
    uint64_t millisecondsTemp = static_cast<unsigned long long>(ntohs(rsapMsgHeader.TimestampMS));
    uint64_t timeStampSum = minTemp * (60000) + millisecondsTemp;
    std::stringstream ss;
    ss << std::endl;
    ss << std::dec <<  std::setfill(' ') << std::setw(25) << "utc-0:" ;
    ss << std::dec <<  std::setfill('0')  <<   timeStampSum ;
    // 将时间戳转换为tm结构
    ss << std::endl << std::dec <<  std::setfill(' ') << std::setw(25) << "time-format:";
    time_t utc8Second =  (timeStampSum)/1000;
    tm* datetime = gmtime((const time_t *)&utc8Second);
    ss  <<  (datetime->tm_year + 1900) << '-'  // tm_year是从1900年开始的年数
        << std::setw(2) << std::dec <<  std::setfill('0')  << (datetime->tm_mon + 1) << '-'  // tm_mon是从0开始的月份数
        << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_mday << ' '  // tm_mday是月份中的第几天
        << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_hour << ':'  // tm_hour是24小时制的小时数
        << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_min << ':'  // tm_min是小时中的分钟数
        << std::setw(2) << std::dec <<  std::setfill('0')  << datetime->tm_sec  << ':'  // tm_sec是分钟中的秒数
        << std::setw(2) << std::dec <<  std::setfill('0')  << rsapMsgHeader.TimestampMS;
   if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[起始符]" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.StartFlag)
                  << std::endl <<"[数据单元长度]" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.DataLen[0])
                  << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.DataLen[1])
                  << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.DataLen[2])
                  << std::endl <<"[数据类别]" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.DataType)
                  << std::endl <<"[版本号]" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rsapMsgHeader.Version)
                  << std::endl <<"[时间戳-毫秒]" << ntohs(rsapMsgHeader.TimestampMS)
                  << std::endl <<"[时间戳-分钟]" << ntohl(rsapMsgHeader.TimestampMIN);

        RSAP_ERROR_PRINT << ss.str();
    }

    if(rsapMsgHeader.DataType == CLOUD2_HEARTBEAT_RES_DATA_TYPE)
    {

        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
            RSAP_DEBUG_PRINT << "[Data m_MsgRespFlag.heartBeatRespFlag ]" << m_MsgRespFlag.heartBeatRespFlag;
        }

        m_MsgRespFlag.heartBeatRespFlag = true;

    }
    else
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
        }
    }
    if(rsapMsgHeader.DataType == CLOUD2RCU_STATUS_RES_DATA_TYPE)
    {

        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
            RSAP_DEBUG_PRINT << "[Data m_MsgRespFlag.heartBeatRespFlag ]" << m_MsgRespFlag.statusRespFlag;
        }

        m_MsgRespFlag.statusRespFlag = true;

    }
    else
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
        }
    }
    if(rsapMsgHeader.DataType == CLOUD2RCU_EVENT_RES_DATA_TYPE)
    {

        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
            RSAP_DEBUG_PRINT << "[Data m_MsgRespFlag.eventRespFlag ]" << m_MsgRespFlag.eventRespFlag;
        }
        m_MsgRespFlag.eventRespFlag = true;

    }
    else
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[rsapMsgHeader.DataType]" << rsapMsgHeader.DataType;
        }
    }
    buffer->retrieveAll();
}
void RSAP_COMPONENT::getServerDisconnectedInfo(bool serverDis)
{
    RSAP_ERROR_PRINT << "[error]" << serverDis;
    m_ConnetedFlag = false;
    m_CertVerifyFlag = false;
    // pushMonitorMecLinkStatus(false);
    m_CurrConn->shutdown();
    m_TcpClient->reconnect();
}

void RSAP_COMPONENT::onConnected(const afl::net::TcpConnectionPtr &conn)
{
    RSAP_DEBUG_PRINT << "[sucess]tcp connect sucess!";
    if (conn->connected())
    {
        m_CurrConn = conn;
        if(!m_CloudServiceConfig.enabelUseTls)
        {
            m_ConnetedFlag = true;
            m_CertVerifyFlag = true;
            // pushMonitorMecLinkStatus(false);
            pushHearBeatInfo();
        }
        else
        {
            m_CurrConn->setConnectStateCallBack(std::bind(&RSAP_COMPONENT::getServerDisconnectedInfo, this, std::placeholders::_1));
            m_CurrConn->initTls(m_TlsCtxInfo);
            if(!m_CurrConn->tlsVerify())
            {
                RSAP_ERROR_PRINT << "[error]ssl cert Verify failed";
                m_CertVerifyFlag = false;
                // pushMonitorMecLinkStatus(false);
                return;
            }
            else
            {
                m_CertVerifyFlag = true;
                // pushMonitorMecLinkStatus(true);
            }

            if(!m_CurrConn->tlsConnect())
            {
                RSAP_ERROR_PRINT << "[error]ssl Connect failed";
                m_ConnetedFlag = false;
                // pushMonitorMecLinkStatus(false);
                return;
            }
            else
            {
                RSAP_DEBUG_PRINT << "[sucess]tcp-tls connect sucess!";
                m_ConnetedFlag = true;
                // pushMonitorMecLinkStatus(true);
                pushHearBeatInfo();
            }
        }
    } else
    {
        RSAP_ERROR_PRINT << "Connection to tcp Server disconnect!, reconnect to tcp Server!";
        m_ConnetedFlag = false;
        m_CertVerifyFlag = false;
        // pushMonitorMecLinkStatus(false);
        m_CurrConn = conn;
        m_CurrConn->shutdown();
        m_TcpClient->reconnect();
    }
}

void RSAP_COMPONENT::printBufferData(afl::net::ByteBuffer& buf, std::string hint)
{
   if(m_CloudServiceConfig.enablePrintInfoLog)
    {
//        RSAP_DEBUG_PRINT << "▽▽▽▽▽▽▽▽▽▽▽▽▽▽" <<  hint.c_str() << "- Raw data▽▽▽▽▽▽▽▽▽▽▽▽▽▽";
        std::ostringstream oss;
        for(uint32_t i = 0 ; i < buf.readableBytes(); i++)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint16_t>(*(buf.peek() + i));
            if (i < buf.readableBytes() - 1)
            {
                oss << " ";
            }
        }
        oss << std::endl;
       if(m_CloudServiceConfig.enablePrintInfoLog)
       {
           RSAP_DEBUG_PRINT << oss.str();
       }
//        RSAP_DEBUG_PRINT << "△△△△△△△△△△△△△△" << hint.c_str() << "- Raw data△△△△△△△△△△△△△△△";
    }
}
bool RSAP_COMPONENT::cancelTimer(int fd)
{
    m_Eventloop->cancelTimer(fd);
    return true;
}
bool RSAP_COMPONENT::monitorHeartBeatResp()
{
    if(m_MsgRespFlag.heartBeatRespFlag)
    {
        m_MsgRespFlag.heartBeatRPushedFlag = false;
        m_MsgRespFlag.heartBeatPushCount = 0;
        m_MsgRespFlag.heartBeatRespFlag = false;
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client recv hear-beat resp!";
            RSAP_DEBUG_PRINT << "[fd]" << timerHeartBeatMonitorFd;
        }

        cancelTimer(timerHeartBeatMonitorFd);
        cancelTimer(timerHeartBeatFd);
        timerHeartBeatFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushHearBeatInfo, this), m_CloudServiceConfig.heartBeatPeriod, true);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client cancle timer!";
        }
    }
    else
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
        RSAP_DEBUG_PRINT << "[timerHeartBeatMonitorFd]" << timerHeartBeatMonitorFd << "[timerHeartBeatFd]" << timerHeartBeatFd;
            RSAP_DEBUG_PRINT << "[note]client not  recv hear-beat resp![heartBeatPushCount]" << m_MsgRespFlag.heartBeatPushCount;
        }
        cancelTimer(timerHeartBeatMonitorFd);
        cancelTimer(timerHeartBeatFd);

        m_MsgRespFlag.heartBeatRPushedFlag = false;
        m_MsgRespFlag.heartBeatPushCount++;


        if(m_MsgRespFlag.heartBeatPushCount >= 3)
        {
//            cancelTimer(timerHeartBeatFd);
            if(m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[timerHeartBeatFd]" << timerHeartBeatFd;
            }
            m_CurrConn->shutdown();
            m_TcpClient->stop();
            m_TcpClient->disconnect();
            m_HeartBeatReconnectFlag = true;
            m_MsgRespFlag.heartBeatPushCount = 0;
            m_MsgRespFlag.heartBeatRPushedFlag = false;
            m_MsgRespFlag.heartBeatRespFlag = false;
            timeIntervalHeartBeat += (m_CloudServiceConfig.heartBeatReconnectTimeMinute * 60);
            if(m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[timeIntervalHeartBeat]" << timeIntervalHeartBeat;
            }
            timerHeartBeatIntervalFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::reconnectServerHeartBeat, this), timeIntervalHeartBeat, true);
        }
        else
        {
            if(m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[timerHeartBeatMonitorFd]" << timerHeartBeatMonitorFd << "[timerHeartBeatFd]" << timerHeartBeatFd;
            }
            cancelTimer(timerHeartBeatMonitorFd);
            cancelTimer(timerHeartBeatFd);
            pushHearBeatInfo();
        }
    }
    return true;
}
void RSAP_COMPONENT::reconnectServerHeartBeat()
{
    if(m_HeartBeatReconnectFlag)
    {
        m_TcpClient->reconnect();
        m_HeartBeatReconnectFlag = false;
        cancelTimer(timerHeartBeatIntervalFd);
        timerHeartBeatFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushHearBeatInfo, this), m_CloudServiceConfig.heartBeatPeriod, true);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[timerHeartBeatFd]" << timerHeartBeatFd;
        }
    }

}
void RSAP_COMPONENT::pushHearBeatInfo()
{
    if(!m_ConnetedFlag || !m_CurrConn)
    {
        RSAP_ERROR_PRINT << "[error]tcp not connected!";
//        m_TcpClient->reconnect();
        return;
    }
    if(! m_CertVerifyFlag)
    {
        RSAP_ERROR_PRINT << "[error]ssl cert Verify failed!";
        return;
    }
    if(!m_CloudServiceConfig.enablePushHeartbeat)
    {
        return;
    }

    afl::net::ByteBuffer bufferUp(12, 1024);
    bufferUp.retrieveAll();
    RsapMsgHeader rsapMsgHeader;
    rsapMsgHeader.StartFlag = RSAP_PACKAGE_START_FLAG;
    int len = 0;
    memcpy(rsapMsgHeader.DataLen , &len, sizeof(rsapMsgHeader.DataLen));

    rsapMsgHeader.DataType = RCU2CLOUD_HEARTBEAT_DATA_TYPE;
    rsapMsgHeader.Version = RCU2CLOUD_HEARTBEAT_VERSION;

    uint64_t nowMillSeconds = 0;
    if(m_CloudServiceConfig.enableUseAddUtc8)
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() + UTC8_MILLSECODND_DIFF_VALUE;
    }
    else
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() ;
    }
    rsapMsgHeader.TimestampMS = htons((nowMillSeconds) % (1000 * 60));
    rsapMsgHeader.TimestampMIN = htonl((nowMillSeconds) / (1000 * 60));

    bufferUp.write(&rsapMsgHeader, sizeof(RsapMsgHeader));
    printBufferData(bufferUp, "心跳");
    if(m_ConnetedFlag && m_CurrConn)
    {
        if(m_CloudServiceConfig.enablePushHeartbeat)
        {
            m_CurrConn->send(&bufferUp);
        }

        if(m_CloudServiceConfig.enablePrintParseInfoHeartbeat)
        {
            m_ParseInfo.parseHearBeat(&bufferUp);
        }

        if(m_CloudServiceConfig.enbaleHeartBeatReconnect)
        {
            m_MsgRespFlag.heartBeatRPushedFlag = true;
            timerHeartBeatMonitorFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorHeartBeatResp, this),
                                                       m_CloudServiceConfig.heartBeatMonitorPeriod, true);
        }

    }
    bufferUp.retrieveAll();
}
bool RSAP_COMPONENT::getDeviceConfigInfo()
{

    return true;
}

int RSAP_COMPONENT::getRandom(int startNum, int endNum)
{
    // 创建随机数生成器
    std::random_device rd;  // 获取随机数种子
    std::mt19937 gen(rd());  // 使用梅森旋转算法生成随机数
    std::uniform_int_distribution<> dis(startNum, endNum); // 定义范围 [50, 60]

    // 生成并输出随机数
    return dis(gen);
}
bool RSAP_COMPONENT::makeObjsUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if(!m_ConnetedFlag || !m_CurrConn)
    {
        RSAP_ERROR_PRINT << "[error]tcp not connected!";
//        m_TcpClient->reconnect();
        return false;
    }
    if(! m_CertVerifyFlag)
    {
        RSAP_ERROR_PRINT << "[error]ssl cert Verify failed!";
        return false;
    }
    if(!m_CloudServiceConfig.enablePushObj)
    {
        return false;
    }
    if(m_CloudServiceConfig.enable_print_sensor_channel_data_info)
    {
        RSAP_ERROR_PRINT <<  mecDeviceData->DebugString();
    }

    afl::net::ByteBuffer bufferUp(512, 8192);
    bufferUp.retrieveAll();
    RsapMsgHeader rsapMsgHeader;
    rsapMsgHeader.StartFlag = RSAP_PACKAGE_START_FLAG;
    /////////////////////////////////////////////////////////////////////////////////////////////
    rsapMsgHeader.DataType = RCU2CLOUD_OBJS_DATA_TYPE;
    /////////////////////////////////////////////////////////////////////////////////////////////
    rsapMsgHeader.Version = RCU2CLOUD_OBJS_VERSION;
    uint64_t nowMillSeconds = 0;
    /////////////////////////////////////////////////////////////////////////////////////////////
    if(m_CloudServiceConfig.enableUseAddUtc8)
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() + UTC8_MILLSECODND_DIFF_VALUE;
    }
    else
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() ;
    }

    rsapMsgHeader.TimestampMS = htons((nowMillSeconds)% (1000 * 60));
    rsapMsgHeader.TimestampMIN = htonl((nowMillSeconds) / (1000 * 60));

    /////////////////////////////////////////////////////////////////////////////////////////////
    afl::net::ByteBuffer buffer(512, 8192);
    buffer.retrieveAll();
    int len = 0;
    if (mecDeviceData->perception_obstacle_size() > 0)
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        output_monitor_->Clear();
        auto* monitor_rsap_response =  output_monitor_->mutable_rsap_response();
        monitor_rsap_response->set_tag(airos::monitor::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_UP);
        monitor_rsap_response->set_timestamp( afl::util::TimeStamp::now(true).millSeconds());
//        auto* sensor_data_up = mec_sensor_data_monitor->mutable_sensor_data_up();
        monitor_rsap_response->set_sensor_cloud_ip(m_CloudServiceConfig.serverIP);
        monitor_rsap_response->set_sensor_cloud_port(std::to_string(m_CloudServiceConfig.serverPort));
        monitor_rsap_response->set_sensor_cloud_con_flag(true);
        monitor_rsap_response->set_sensor_dat_up_flag(true);
        monitor_rsap_response->set_sensor_data_up_obj_package(mecDeviceData->header().total_package_num());
        send_obj_package_new = send_obj_package_old = mecDeviceData->header().total_package_num();
        monitor_rsap_response->set_sensor_data_up_obj_num(mecDeviceData->perception_obstacle().size());
        RSAP_DEBUG_PRINT << output_monitor_->DebugString();
        Send("/v2x/monitor", output_monitor_);
        ///////////////////////////////////////////////////////////////////////////////////////////
        const auto& obstacles = mecDeviceData->perception_obstacle();
        const uint32_t obstaclesSum = obstacles.size();
        ObjsDataHead objsDataHead;
        /////////////////////////////////////////////////////////////////////////////////////////////
        //channelId
        objsDataHead.channelId = CHANNEL_SOURCE_DATANG_ROADSIDE_PERCEPTION_DEVICE;
        /////////////////////////////////////////////////////////////////////////////////////////////
        //rcuId
        {
            memset(objsDataHead.rcuId, 0, sizeof(objsDataHead.rcuId));
            uint32_t needLen = std::min(m_CloudServiceConfig.objRcuId.size(),sizeof(objsDataHead.rcuId));
            std::string ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
            std::memcpy(objsDataHead.rcuId, ctbeStr.c_str(), ctbeStr.size());
        }

        /////////////////////////////////////////////////////////////////////////////////////////////
        //deviceType
        objsDataHead.deviceType = OBJ_DEVICE_TYPE_FUSIONRESULT;
        /////////////////////////////////////////////////////////////////////////////////////////////
        //deviceId
        auto header = mecDeviceData->header();
        if(objsDataHead.deviceType == OBJ_DEVICE_TYPE_UNKNOWN || objsDataHead.deviceType == OBJ_DEVICE_TYPE_FUSIONRESULT)
        {
          m_MecAllocDevNo = "00000000000000000000000";
          uint32_t needLen = std::min(m_MecAllocDevNo.size(),sizeof(objsDataHead.deviceId) * 2);
          memset(objsDataHead.deviceId, 0, sizeof(objsDataHead.deviceId));
          std::string ctbeStr = deviceCodeZip( m_MecAllocDevNo.substr(0, needLen));
          std::memcpy(objsDataHead.deviceId, ctbeStr.c_str(), ctbeStr.size());
        }
        else {
            std::string mec_real_dev_no = header.mec_no();
            if (m_CloudServiceConfig.enablePrintInfoLog) 
			{
              RSAP_DEBUG_PRINT << "[notice]mec_real_dev_no:" << mec_real_dev_no;
              RSAP_DEBUG_PRINT << "[device-no-map-size]"
                               << m_DeviceNoMapInfoMap.size();
            }
            if (!m_HasedGetAllocDevNo) 
			{
              for (auto m : m_DeviceNoMapInfoMap) 
			  {
                if (m_CloudServiceConfig.enablePrintInfoLog) 
				{
                  RSAP_DEBUG_PRINT << "[notice]" << m.first;
                }
                if (m.first == mec_real_dev_no) 
				{
                  m_MecAllocDevNo = m.second;
                  m_HasedGetAllocDevNo = true;

                  if (m_CloudServiceConfig.enablePrintInfoLog) 
				  {
                    RSAP_DEBUG_PRINT << "[notice]find real no!" << "[real-no]"
                                     << mec_real_dev_no << "[alloc-no]"
                                     << m_MecAllocDevNo;
                  }
                  break;
                }
              }
            }
            if (!m_HasedGetAllocDevNo) 
			{
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[notice]!m_HasedGetAllocDevNo";
                }
                m_MecAllocDevNo = m_CloudServiceConfig.objDeviceId;
            }
            uint32_t needLen = std::min(m_MecAllocDevNo.size(), sizeof(objsDataHead.deviceId) * 2);
            memset(objsDataHead.deviceId, 0, sizeof(objsDataHead.deviceId));
            std::string ctbeStr =deviceCodeZip(m_MecAllocDevNo.substr(0, needLen));
            std::memcpy(objsDataHead.deviceId, ctbeStr.c_str(), ctbeStr.size());
        }
        if(m_CloudServiceConfig.enableUseAddUtc8)
        {
            objsDataHead.timestampOfDevOut = boost::endian::native_to_big(header.timestamp_millisecond() - getRandom(73, 80) + UTC8_MILLSECODND_DIFF_VALUE);
            objsDataHead.timestampOfDetIn = boost::endian::native_to_big(header.timestamp_millisecond()- getRandom(55, 64) + UTC8_MILLSECODND_DIFF_VALUE);
            objsDataHead.timestampOfDetOut = boost::endian::native_to_big(header.timestamp_millisecond() + UTC8_MILLSECODND_DIFF_VALUE);
        }
        else
        {
            objsDataHead.timestampOfDevOut = boost::endian::native_to_big(header.timestamp_millisecond() );;
            objsDataHead.timestampOfDetIn = boost::endian::native_to_big(header.timestamp_millisecond() );
            objsDataHead.timestampOfDetOut = boost::endian::native_to_big(header.timestamp_millisecond() );
        }

        /////////////////////////////////////////////////////////////////////////////////////////////
        if(m_CloudServiceConfig.enableUse02)
        {
            objsDataHead.gnssType = COORDINATE_TYPE_GCJ02;
        }
        else
        {
            objsDataHead.gnssType = COORDINATE_TYPE_WGS84;
        }
        /////////////////////////////////////////////////////////////////////////////////////////////
        objsDataHead.targetsNum = htons(obstaclesSum);
        /////////////////////////////////////////////////////////////////////////////////////////////
        buffer.write(&objsDataHead, sizeof(ObjsDataHead));
        len += sizeof(ObjsDataHead);
        /////////////////////////////////////////////////////////////////////////////////////////////
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[obstaclesSum]" << obstaclesSum;
        }
        for(uint32_t i = 0; i < obstaclesSum; i++)
        {
            std::stringstream ss_check;

            ss_check << "[upload]" << (header.timestamp_millisecond() + UTC8_MILLSECODND_DIFF_VALUE) << ",";
            auto obstacle = obstacles[i];
            ObjHead objHead;
            /////////////////////////////////////////////////////////////////////////////////////////////
            //uuid
            {
                ss_check << obstacle.id_str() << ",";
                memset(objHead.uuid, 0, sizeof(objHead.uuid));
                std::string idTempStr = getIDStr(obstacle.id_str());
//                ss_check << idTempStr << ",";
                uint32_t needLen = std::min(idTempStr.size(),sizeof(objHead.uuid));
                std::string ctbeStr = (idTempStr.substr(0, needLen));
                std::memcpy(objHead.uuid, (void*)ctbeStr.c_str(), ctbeStr.size());
            }

            //objId
            objHead.objId = htons(i);
            /////////////////////////////////////////////////////////////////////////////////////////////
            //type
            int typeTemp;
            switch(obstacle.ptc_type())
            {
                case MEC_PTC_UNKNOWN:
                    typeTemp = OBJ_TYPE_UNKNOWN;
                    break;
                case MEC_PTC_MOTOR:
                    switch(obstacle.vehicle_type())
                    {
                        case VEHICLE_TYPE_AMBULANCE:  //警车
                        case VEHICLE_TYPE_POLICE_CAR: //消防车
                        case VEHICLE_TYPE_SEDAN:  //轿车
                        case VEHICLE_TYPE_VAN: //面包车
                            typeTemp = OBJ_TYPE_CAR;
                            break;
                        case VEHICLE_TYPE_FIRE_TRUCK:  //卡车
                        case VEHICLE_TYPE_TRUCK:
                            typeTemp = OBJ_TYPE_TRUCK;
                            break;
                        case VEHICLE_TYPE_BUS: //大巴车
                            typeTemp = OBJ_TYPE_BUS;
                            break;
                        default:
                            typeTemp = OBJ_TYPE_CAR;
                            break;
                    }
                    break;
                case MEC_PTC_NON_MOTOR:
                    switch(obstacle.non_vehicle_type())
                    {
                        case NON_VEHICLE_TYPE_UNKONWN:  //未知
                            typeTemp = OBJ_TYPE_UNKNOWN;
                            break;
                        case NON_VEHICLE_TYPE_BICYCLE:  //自行车
                            typeTemp = OBJ_TYPE_BICYCLE;
                            break;
                        case NON_VEHICLE_TYPE_MOTORCYCLE: //摩托车
                            typeTemp = OBJ_TYPE_MOTORBIKE;
                            break;
                        case NON_VEHICLE_TYPE_TRICYCLE: //三轮车
                            typeTemp = OBJ_TYPE_MOTORBIKE;
                            break;
                        default:
                            typeTemp = OBJ_TYPE_MOTORBIKE;
                            break;
                    }
                    break;
                case MEC_PTC_PEDESTRIAN:
                    typeTemp = OBJ_TYPE_PERSON;
                    break;
                case MEC_PTC_RSU:
                    typeTemp = OBJ_TYPE_OTHER;
                    break;
                default:
                    typeTemp = OBJ_TYPE_UNKNOWN;
                    break;
            }

            objHead.type = typeTemp;
            /////////////////////////////////////////////////////////////////////////////////////////////
            //status
            objHead.status = 1;
            /////////////////////////////////////////////////////////////////////////////////////////////
            objHead.len = htons(airos::base::MathUtil::convertM2CM(obstacle.length()));
            if(obstacle.has_width())
            {
                objHead.width =  htons(airos::base::MathUtil::convertM2CM(obstacle.width()));
            }
            else
            {
                objHead.width = 10000;
            }
            if(obstacle.has_height())
            {

                objHead.height =  htons(airos::base::MathUtil::convertM2CM(obstacle.height()));
            }
            else
            {
                objHead.height = 10000;
            }
            /////////////////////////////////////////////////////////////////////////////////////////////
            //lat lon ele
            auto positionGcs = obstacle.position_gcs();
            objHead.longitude =  htonl(airos::base::MathUtil::convertDegLatLonF2I(positionGcs.lon()));
            objHead.latitude = htonl(airos::base::MathUtil::convertDegLatLonF2I(positionGcs.lat()));
            ss_check << positionGcs.lon() << ",";
            ss_check << positionGcs.lat() << ",";
            ss_check << obstacle.ptc_type() << ",";
            switch(obstacle.ptc_type())
            {
                case MEC_PTC_MOTOR:
                    ss_check << obstacle.vehicle_type() << ",";
                    break;
                default:
                    ss_check << 4  << ",";
                    break;
            }

            objHead.locEast = 0;
            objHead.locNorth = 0;
            objHead.posConfidence = 0;
            if(positionGcs.has_ele())
            {
                objHead.elevation = htonl(airos::base::MathUtil::convertEleF2I(positionGcs.ele()));
            }
            else
            {
                objHead.elevation = 0;
            }

            objHead.elevConfidence = 0;
            /////////////////////////////////////////////////////////////////////////////////////////////
            objHead.speed = htons(airos::base::MathUtil::convertSpeedKMH2MS(obstacle.ptc_speed()) * 100);
            objHead.speedConfidence = 0;
            objHead.speedEast = 65535;
            objHead.speedEastConfidence = 0;
            objHead.speedNorth = 65535;
            objHead.speedNorthConfidence = 0;
            /////////////////////////////////////////////////////////////////////////////////////////////
            if(obstacle.has_ptc_heading())
            {
                objHead.heading  = htonl(obstacle.ptc_heading() * 10000);
                ss_check << obstacle.ptc_heading()  << ",";
            }
            else
            {
                objHead.heading = 0xFFFFFFFF;
            }

            objHead.headConfidence  = 0;
            /////////////////////////////////////////////////////////////////////////////////////////////
            objHead.accelVert = 0;
            objHead.accelVertConfidence = 0;
            /////////////////////////////////////////////////////////////////////////////////////////////
            if(obstacle.has_tracking())
            {
                objHead.trackedTimes = htonl(obstacle.tracking() * 1000);
            }
            else
            {
                objHead.trackedTimes = 0;
            }

            /////////////////////////////////////////////////////////////////////////////////////////////
            buffer.write(&objHead, sizeof(ObjHead));
            len += sizeof(ObjHead);
            ///////////////////////////////////////////////////////////////////////////////
            ObjHistLoc objHistLoc;
            objHistLoc.histLocNum = 0;
            buffer.write(&objHistLoc, sizeof(ObjHistLoc));
            len += sizeof(ObjHistLoc);
            ///////////////////////////////////////////////////////////////////////////////
            ObjPredLoc objPredLoc;
            objPredLoc.predLocNum = 0;
            buffer.write(&objPredLoc, sizeof(ObjPredLoc));
            len += sizeof(ObjPredLoc);
            ///////////////////////////////////////////////////////////////////////////////
            ObjLaneId objLaneId;
            if(obstacle.has_lane_no())
            {
                objLaneId.laneId = obstacle.lane_no();
            }
            else
            {
                objLaneId.laneId = 0;
            }

            objLaneId.filterInfoType = 0;
            buffer.write(&objLaneId, sizeof(ObjLaneId));
            len += sizeof(ObjLaneId);
            ///////////////////////////////////////////////////////////////////////////////
            //lenplateNo
            std::string plateNumStr = obstacle.plate_num();
            ObjPlateNum objPlateNum;
            objPlateNum.lenplateNum = plateNumStr.length();
            buffer.write(&objPlateNum, sizeof(ObjPlateNum));
            len += sizeof(ObjPlateNum);

            {
                auto ctbeStr = (obstacle.plate_num());
                buffer.write(ctbeStr);
            }

            len += objPlateNum.lenplateNum;
            ///////////////////////////////////////////////////////////////////////
            //plateType
            ObjsTail objsTail;
            objsTail.plateType = 0xFF;

            //plateColor
            int plateColorTemp;
            if(obstacle.has_plate_color())
            {
                switch(obstacle.plate_color())
                {
                    case PLATE_COLOR_TYPE_OTHER:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_NO_VALID;
                        break;
                    case PLATE_COLOR_TYPE_WHITE:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_BLUEBOTTOM_WHITEEXT;
                        break;
                    case PLATE_COLOR_TYPE_BLUE:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_BLUE;
                        break;
                        break;
                    case PLATE_COLOR_TYPE_YELLOW:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_YELLOW;
                        break;
                    case PLATE_COLOR_TYPE_BLACK:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_BLACK;
                        break;
                    case PLATE_COLOR_TYPE_GREEN:
                        plateColorTemp = LICENSE_PLATE_COLOR_TYPE_GREEN_AGRICULTURAL;
                        break;
                    default:
                        plateColorTemp =  LICENSE_PLATE_COLOR_TYPE_NO_VALID;
                        break;
                }
            }
            else
            {
                plateColorTemp = 0xFF;
            }

            objsTail.plateColor = plateColorTemp;

            //objColor
            int objColorTemp = 0;
            if(obstacle.has_obj_color())
            {
                switch(obstacle.obj_color())
                {
                    case OBJ_COLOR_TYPE_OTHER:
                        objColorTemp = VEHICLE_COLOR_NO_VALID;
                        break;
                    case OBJ_COLOR_TYPE_WHITE:
                        objColorTemp = VEHICLE_COLOR_WHITE;
                        break;
                    case OBJ_COLOR_TYPE_BLUE:
                        objColorTemp = VEHICLE_COLOR_BLUE;
                        break;
                    case OBJ_COLOR_TYPE_YELLOW:
                        objColorTemp = VEHICLE_COLOR_YELLOW;
                        break;
                    case OBJ_COLOR_TYPE_BLACK:
                        objColorTemp = VEHICLE_COLOR_BLACK;
                        break;
                    case OBJ_COLOR_TYPE_GREEN:
                        objColorTemp = VEHICLE_COLOR_GREEN;
                        break;
                    case OBJ_COLOR_TYPE_RED:
                        objColorTemp = VEHICLE_COLOR_RED;
                        break;
                    case OBJ_COLOR_TYPE_SILVER:
                        objColorTemp = VEHICLE_COLOR_SILVERWHITE;
                        break;
                    case OBJ_COLOR_TYPE_GRAY:
                        objColorTemp = VEHICLE_COLOR_GRAY;
                        break;
                    case OBJ_COLOR_TYPE_PINK:
                        objColorTemp = VEHICLE_COLOR_PINK;
                        break;
                    case OBJ_COLOR_TYPE_GOLD:
                        objColorTemp = VEHICLE_COLOR_OTHER;
                        break;
                    case OBJ_COLOR_TYPE_ORANGE:
                        objColorTemp = VEHICLE_COLOR_ORANGE;
                        break;
                    case OBJ_COLOR_TYPE_PURPLE:
                        objColorTemp = VEHICLE_COLOR_PURPLE;
                        break;
                    default:
                        objColorTemp = VEHICLE_COLOR_NO_VALID;
                        break;
                }
            }
            else
            {
                objColorTemp = 0xFF;
            }

            objsTail.objColor = objColorTemp;
            //funtionTimestamp
            if(m_CloudServiceConfig.enableUseAddUtc8)
            {
                objsTail.funtionTimestamp = boost::endian::native_to_big(header.timestamp_millisecond() + UTC8_MILLSECODND_DIFF_VALUE);
            }
            else
            {
                objsTail.funtionTimestamp = boost::endian::native_to_big(header.timestamp_millisecond()) ;
            }
            //统计感知设备个数：除了mec之外
            int deviceNumTemp = 0;
            std::string camera_no_str, radar_no_str, lidar_no_str;
            std::string no_use_real_dev_no_hash_value;  //不使用真实设备序列号
            if (!m_CloudServiceConfig.enable_use_real_dev_no)
            {
                //不使用真实的设备ESN，防止
                if (!m_SensorDeviceNoMapInfoMap.empty())
                {
                    // 生成随机数
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, m_SensorDeviceNoMapInfoMap.size() - 1);

                    // 获取随机位置
                    int randomIndex = dis(gen);

                    // 使用迭代器移动到随机位置
                    auto pairTemp = m_SensorDeviceNoMapInfoMap.begin();
                    std::advance(pairTemp, randomIndex);
                    if (deviceNumTemp == 0)
                    {
                        if(m_CloudServiceConfig.enablePrintInfoLog)
                        {
                            RSAP_DEBUG_PRINT << "[hash][no-real][sn]" << pairTemp->first << "[esn]" << pairTemp->second.deviceESn << "[hash]" << pairTemp->second.hashValue;
                            RSAP_DEBUG_PRINT << "[hash][no-real]find hash value!";
                        }
                        no_use_real_dev_no_hash_value = pairTemp->second.hashValue;
                        deviceNumTemp++;

                    }
                }

                //              if(deviceNumTemp == 0)
                //              {
                //                for(auto pairTemp: m_SensorDeviceNoMapInfoMap)
                //                {
                //                  RSAP_DEBUG_PRINT << "[pairTemp.first]" << pairTemp.first  << "[sn]" << pairTemp.first << "[esn]" << pairTemp.second.deviceESn << "[hash]" <<  pairTemp.second.hashValue ;
                //                  camera_no_str = pairTemp.second.hashValue;
                //                  deviceNumTemp++;
                //                  RSAP_DEBUG_PRINT << "find hash value!" ;
                //                  break;
                //                }
                //              }
                //              else
                //              {
                //                RSAP_DEBUG_PRINT << "[deviceNumTemp]" << deviceNumTemp;
                //              }
            }
            else
            {
                //真实设备序号
                // camera-no
                if (mecDeviceData->header().has_dev_no())
                {
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        for (auto pair : m_SensorDeviceNoMapInfoMap)
                        {
                            RSAP_DEBUG_PRINT << "[hash][real][sn]" << pair.first << "[esn]" << pair.second.deviceESn << "[hashValue]" << pair.second.hashValue;
                        }
                    }
                    for (auto pairTemp : m_SensorDeviceNoMapInfoMap)
                    {
                        if(m_CloudServiceConfig.enablePrintInfoLog)
                        {
                            RSAP_DEBUG_PRINT << "[hash][real][pairTemp.first]" << pairTemp.first << "[dev_no]" << mecDeviceData->header().dev_no();
                        }
                        if (pairTemp.first == mecDeviceData->header().dev_no())
                        {
                            if(m_CloudServiceConfig.enablePrintInfoLog)
                            {
                                RSAP_DEBUG_PRINT << "[hash][real][camera_no_str]" << mecDeviceData->header().dev_no() << "[sn]" << pairTemp.first << "[esn]" << pairTemp.second.deviceESn << "[hash]" << pairTemp.second.hashValue;
                                RSAP_DEBUG_PRINT << "[hash][real]find hash value!";
                            }
                            camera_no_str = pairTemp.second.hashValue;
                            deviceNumTemp++;
                            break;
                        }
                    }
                }
                // radar-no
                if (mecDeviceData->header().has_radar_no())
                {
                    for (auto pairTemp : m_SensorDeviceNoMapInfoMap)
                    {
                        if (pairTemp.first == mecDeviceData->header().radar_no())
                        {
                            if(m_CloudServiceConfig.enablePrintInfoLog)
                            {
                                RSAP_DEBUG_PRINT << "[hash][real][radar_no_str]" << mecDeviceData->header().radar_no() << "[sn]" << pairTemp.first << "[esn]" << pairTemp.second.deviceESn << "[hash]" << pairTemp.second.hashValue;
                            }
                            radar_no_str = pairTemp.second.hashValue;
                            deviceNumTemp++;
                            break;
                        }
                    }
                }

                // lidar-no
                if (mecDeviceData->header().has_lidar_no())
                {
                    for (auto pairTemp : m_SensorDeviceNoMapInfoMap)
                    {
                        if (pairTemp.first == mecDeviceData->header().lidar_no())
                        {
                            if(m_CloudServiceConfig.enablePrintInfoLog)
                            {
                                RSAP_DEBUG_PRINT << "[hash][real][lidar_no_str]" << mecDeviceData->header().lidar_no() << "[sn]" << pairTemp.first << "[esn]" << pairTemp.second.deviceESn << "[hash]" << pairTemp.second.hashValue;
                            }
                            lidar_no_str = pairTemp.second.hashValue;
                            deviceNumTemp++;
                            break;
                        }
                    }
                }
            }
            objsTail.deviceNum = deviceNumTemp * 2;
            if(m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[deviceNum]" << objsTail.deviceNum;
            }
            buffer.write(&objsTail, sizeof(objsTail));
            len += sizeof(objsTail);
            if (!m_CloudServiceConfig.enable_use_real_dev_no)
            {
                //不使用真实设备号
                if (!no_use_real_dev_no_hash_value.empty())
                {
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_DEBUG_PRINT << "[hash][no-real][no_use_real_dev_no_hash_value]" << no_use_real_dev_no_hash_value;
                    }
                    buffer.write(no_use_real_dev_no_hash_value);
                    len += no_use_real_dev_no_hash_value.length();
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_DEBUG_PRINT << "[hash][no-real][len]" << no_use_real_dev_no_hash_value.length() << "[no_use_real_dev_no_hash_value]" << no_use_real_dev_no_hash_value;
                    }
                    }
                else
                {
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_WARN_PRINT << "[hash][no-real][no_use_real_dev_no_hash_value] is empty!";
                    }
                }
            }
            else
            {
                // camera-no
                if (mecDeviceData->header().has_dev_no())
                {
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_DEBUG_PRINT << "[hash][real][dev_no]" << mecDeviceData->header().has_dev_no();
                    }
                    buffer.write(camera_no_str);
                    len += camera_no_str.length();
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_DEBUG_PRINT << "[hash][real][camera_no_hash]" << camera_no_str;
                    }
                }
                // radar-no
                if (mecDeviceData->header().has_radar_no())
                {
                    buffer.write(radar_no_str);
                    len += radar_no_str.length();
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_WARN_PRINT << "[hash][real][radar_no_hash]" << radar_no_str;
                    }
                }

                // lidar-no
                if (mecDeviceData->header().has_lidar_no())
                {
                    buffer.write(lidar_no_str);
                    len += lidar_no_str.length();
                    if(m_CloudServiceConfig.enablePrintInfoLog)
                    {
                        RSAP_WARN_PRINT << "[hash][real][lidar_no_hash]" << lidar_no_str;
                    }
                }
            }

//            for(auto pairTemp: m_SensorDeviceNoMapInfoMap)
//            {
//                RSAP_DEBUG_PRINT << "[esn]" << pairTemp.first << "[hash]" <<  pairTemp.second ;
//                buffer.write(pairTemp.second.hashValue);
//                len += pairTemp.second.hashValue.length();
//            }
            if(m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << ss_check.str();
            }
        }
        auto datalenTemp = boost::endian::native_to_big(len) >> 8;
        memcpy(rsapMsgHeader.DataLen , &datalenTemp, sizeof(rsapMsgHeader.DataLen));;
        bufferUp.write(&rsapMsgHeader, sizeof(RsapMsgHeader));
        bufferUp.write(buffer.peek(), buffer.readableBytes());
        if(m_ConnetedFlag && m_CurrConn)
        {
            printBufferData(bufferUp, "感知目标");
            if(m_CloudServiceConfig.enablePushObj)
            {
                m_CurrConn->send(&bufferUp);
            }
            if(m_CloudServiceConfig.enablePrintParseInfoObj)
            {
                m_ParseInfo.parseObjs(&bufferUp);
            }
        }
        bufferUp.retrieveAll();
    }
    buffer.retrieveAll();
    return true;
}

std::string RSAP_COMPONENT::computeHash(const std::string deviceId)
{
    unsigned int seed = 0; // 初始种子值

    for (char c : deviceId) {
        // 将字符转换为 32 位整数 (ASCII 值)，并与种子值组合
        unsigned int charValue = static_cast<unsigned int>(c); // 转换字符为整数值
        seed = seed * 31 + charValue; // 简单的 hash 逻辑：乘积 + 当前字符值
    }
    // RSAP_ERROR_PRINT << "[seed]" << seed;
   // 将seed转换为字符串
    std::string seedStr = std::to_string(seed);
    
    // 获取最后两个字符
	//现场会存在获取hash为单个字符，需要在前边补0
    std::string result;
    if (seedStr.length() >= 2) {
        // 如果字符串长度大于等于2，获取最后两个字符
        result = seedStr.substr(seedStr.length() - 2);
    } else {
        // 如果字符串长度小于2（理论上不会发生，因为seed是unsigned int），在前面补0
        result = std::string(2 - seedStr.length(), '0') + seedStr;
    }
    
    return result;
}

bool RSAP_COMPONENT::makeEventUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if(!m_ConnetedFlag || !m_CurrConn)
    {
        RSAP_ERROR_PRINT << "[error]tcp not connected!";
        return false;
    }
	if(! m_CertVerifyFlag)
    {
        RSAP_ERROR_PRINT << "[error]ssl cert Verify failed!";
        return false;
    }

    if(!m_CloudServiceConfig.enablePushEvent)
    {
        return false;
    }
    if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_WARN_PRINT <<  mecDeviceData->DebugString();
    }
    afl::net::ByteBuffer bufferUp(512, 8192);
    RsapMsgHeader rsapMsgHeader;
    //StartFlag
    rsapMsgHeader.StartFlag = RSAP_PACKAGE_START_FLAG;
    //DataType
    rsapMsgHeader.DataType = RCU2CLOUD_EVENT_DATA_TYPE;
    //Version
    rsapMsgHeader.Version = RCU2CLOUD_EVENT_VERSION;
    uint64_t nowMillSeconds = 0;
    if(m_CloudServiceConfig.enableUseAddUtc8)
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() + UTC8_MILLSECODND_DIFF_VALUE;
    }
    else
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() ;
    }

    rsapMsgHeader.TimestampMS = htons((nowMillSeconds)% (1000 * 60));
    rsapMsgHeader.TimestampMIN = htonl((nowMillSeconds) / (1000 * 60));
    afl::net::ByteBuffer buffer(512, 8192);
    buffer.retrieveAll();
    auto header = mecDeviceData->header();
    if (mecDeviceData->events_size() > 0)
    {
        auto events = mecDeviceData->events();
        const uint32_t eventsSum = events.size();
        for(uint32_t i = 0; i < eventsSum; i++)
        {
            EventDataHead eventDataHead;
            //channelId
            eventDataHead.channelId = CHANNEL_SOURCE_DATANG_ROADSIDE_PERCEPTION_DEVICE;
            //rcuId
            {
                memset(eventDataHead.rcuId, 0, sizeof(eventDataHead.rcuId));
                uint32_t needLen = std::min(m_CloudServiceConfig.objRcuId.size(),sizeof(eventDataHead.rcuId));
                auto ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
                std::memcpy(eventDataHead.rcuId, ctbeStr.c_str(), ctbeStr.size());
            }
            //eventType
            auto event = events[i];
            switch (event.event_type_mec())
            {
                case 1:
                    eventDataHead.eventType =  0;
                    break;
                case 2:
                    eventDataHead.eventType =  10;
                    break;
                case 3:
                    eventDataHead.eventType =  12;
                    break;
                case 4:
                    eventDataHead.eventType =  4;
                    break;
                case 10:
                    eventDataHead.eventType =  5;
                    break;
                case 11: eventDataHead.eventType =  1;
                    break;
                case 13: eventDataHead.eventType =  31;
                    break;
                case 14: eventDataHead.eventType =  7;
                    break;
                case 15: eventDataHead.eventType =  20;
                    break;
                case 20: eventDataHead.eventType =  21;
                    break;
                case 24: eventDataHead.eventType =  1;
                    break;
                //缺少： 15（20 占用公交车道） 16 17 18 19 20（21 连续变道 ） 21 22 23 24（1 倒车或逆行）25  33（14 行人闯红灯）34（28 紧急车辆）
                case 26: eventDataHead.eventType =  6;
                    break;
                case 27: eventDataHead.eventType =  8;
                    break;
                case 28: eventDataHead.eventType =  9;
                    break;
                case 29: eventDataHead.eventType =  11;
                    break;
                case 30: eventDataHead.eventType =  27;
                    break;
                case 31: eventDataHead.eventType =  30;
                    break;
                case 32: eventDataHead.eventType =  28;
                    break;
                case 33: eventDataHead.eventType =  14;
                    break;
                case 34: eventDataHead.eventType =  28;
                    break;
                default:
                    eventDataHead.eventType = 32;
                    continue;
                    break;
            }
            //confidence
            eventDataHead.confidence = 0xFF;

            auto positionGcs = event.position_gcs();
            //longitude
            eventDataHead.longitude = htonl(airos::base::MathUtil::convertDegLatLonF2I(positionGcs.lon()));
            //latitude
            eventDataHead.latitude = htonl(airos::base::MathUtil::convertDegLatLonF2I(positionGcs.lat()));
            //gnssType
            if(m_CloudServiceConfig.enableUse02)
            {
                eventDataHead.gnssType = COORDINATE_TYPE_GCJ02;
            }
            else
            {
                eventDataHead.gnssType = COORDINATE_TYPE_WGS84;
            }
            //timestamp
            if(m_CloudServiceConfig.enableUseAddUtc8)
            {
                eventDataHead.timestamp = boost::endian::native_to_big(header.timestamp_millisecond() + UTC8_MILLSECODND_DIFF_VALUE);

            }
            else
            {
                eventDataHead.timestamp = boost::endian::native_to_big(header.timestamp_millisecond());
            }

            //eventId
            {
                std::string idTempStr = getIDStr(event.id_str());
                memset(eventDataHead.eventId, 0, sizeof(eventDataHead.eventId));
                uint32_t needLen = std::min(idTempStr.size(), sizeof(eventDataHead.eventId));
                std::string ctbeStr = (idTempStr.substr(0, needLen));
                std::memcpy(eventDataHead.eventId, ctbeStr.c_str(), ctbeStr.size());
            }
            buffer.write(&eventDataHead, sizeof(EventDataHead));
            ////////////////////////////////////////////////////////////////////////////////////////////////
            //extsLen
            EventExts eventExts;
            eventExts.extsLen = 0;
            buffer.write(&eventExts, sizeof(EventExts));
            ////////////////////////////////////////////////////////////////////////////////////////////////
            EventDataTail eventDataTail;
            eventDataTail.targetIdsLen = 1;
            buffer.write(&eventDataTail, sizeof(EventDataTail));
            EventDataTargetIds eventDataTargetIds;

            // 复制转换后的字符串到objId数组中
            {
                std::string idTempStr = getIDStr(event.obj_id_str());
                memset(eventDataTargetIds.objId, 0, sizeof(eventDataTargetIds.objId));
                uint32_t needLen = std::min(idTempStr.size(), sizeof(eventDataTargetIds.objId));
                std::string ctbeStr = (idTempStr.substr(0, needLen));
                std::memcpy(eventDataTargetIds.objId, ctbeStr.c_str(), ctbeStr.size());
            }
            buffer.write(eventDataTargetIds.objId, sizeof(EventDataTargetIds));

            ////////////////////////////////////////////////////////////////////////////////////////////////
            int dataLen = buffer.readableBytes();
            auto datalenTemp = boost::endian::native_to_big(dataLen) >> 8;
            memcpy(rsapMsgHeader.DataLen , &datalenTemp, sizeof(rsapMsgHeader.DataLen));
            bufferUp.write(&rsapMsgHeader, sizeof(RsapMsgHeader));
            bufferUp.write(buffer.peek(), buffer.readableBytes());;
            if(m_ConnetedFlag && m_CurrConn)
            {
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    printBufferData(bufferUp, "感知事件");
                }
                if(m_CloudServiceConfig.enablePushEvent)
                {
                    m_CurrConn->send(&bufferUp);
                }
                if(m_CloudServiceConfig.enablePrintParseInfoEvent)
                {
                    m_ParseInfo.parseEvent(&bufferUp);
                    RSAP_DEBUG_PRINT << "-------------------" << i << "------------------------------";
                }

                if(m_CloudServiceConfig.enbaleEventReconnect)
                {
                    m_MsgRespFlag.eventPushedFlag = true;
                    timerEventMonitorFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorEventResp, this),
                                                                    m_CloudServiceConfig.eventMonitorPeriod, true);

                    cacheEventData(timerEventMonitorFd, mecDeviceData);
                }

            }
            bufferUp.retrieveAll();
            buffer.retrieveAll();
        }
    }
    return true;
}


bool RSAP_COMPONENT::cacheEventData(uint32_t monitorFd, const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    uint64_t timeMillKey = afl::util::TimeStamp::now(true).seconds();
    uint64_t timeMillNow = afl::util::TimeStamp::now(true).seconds();
    auto secondPair = std::make_pair(monitorFd, mecDeviceData);;
    // 将数据存入map中
    m_EventCache[timeMillKey] = secondPair;

    for(auto it = m_EventCache.begin(); it != m_EventCache.end(); it++)
    {
        if(timeMillNow - it->first > 10)
        {
            it = m_EventCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return true;
}
bool RSAP_COMPONENT::monitorEventResp()
{
    if(m_MsgRespFlag.eventRespFlag)
    {
        m_MsgRespFlag.eventPushedFlag = false;
        m_MsgRespFlag.eventPushCount = 0;
        m_MsgRespFlag.eventRespFlag = false;
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client recv hear-beat resp!";
            RSAP_DEBUG_PRINT << "[fd]%d" << timerEventMonitorFd;
        }


        cancelTimer(timerEventMonitorFd);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[note]client cancle timer!";
        }

    }
    else
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[fd]%d" << timerEventMonitorFd;
        }
        cancelTimer(timerEventMonitorFd);
        for(auto it = m_EventCache.begin(); it != m_EventCache.end(); it++)
        {
            if((int)it->second.first == timerEventMonitorFd)
            {
                makeEventUpData(it->second.second);
//                it = m_EventCache.erase(it);
            }
        }

        m_MsgRespFlag.eventPushedFlag = false;
        m_MsgRespFlag.eventPushCount++;
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client not  recv hear-beat resp![statusPushCount]" << m_MsgRespFlag.eventPushCount;
        }

        if(m_MsgRespFlag.eventPushCount >= 3)
        {
            for(auto it = m_EventCache.begin(); it != m_EventCache.end(); it++)
            {
                if((int)it->second.first == timerEventMonitorFd)
                {
                    it = m_EventCache.erase(it);
                }
            }
            m_CurrConn->shutdown();
            m_TcpClient->stop();
            m_TcpClient->disconnect();
            m_DeviceStatusReconnectFlag = true;
            m_MsgRespFlag.eventPushCount = 0;
            m_MsgRespFlag.eventPushedFlag = false;
            m_MsgRespFlag.eventRespFlag = false;
            timeIntervalEvent += (m_CloudServiceConfig.eventReconnectTimeMinute * 60);
            timerEventIntervalFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::reconnectServerEvent, this), timeIntervalEvent, true);
        }
    }
    return true;
}
void RSAP_COMPONENT::reconnectServerEvent()
{
    if(m_EventReconnectFlag)
    {
        m_TcpClient->reconnect();
        m_EventReconnectFlag = false;
        cancelTimer(timerEventIntervalFd);
//        timerDeviceStatusFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushDeviceStatusData, this), m_CloudServiceConfig.deviceStatusPeriod, true);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[timerDeviceStatusFd]" << timerDeviceStatusFd;
        }
    }

}

bool RSAP_COMPONENT::pushDeviceStatusData()
{
    if(!m_ConnetedFlag || !m_CurrConn)
    {
        RSAP_ERROR_PRINT << "[error]tcp not connected!";
        return false;
    }
    if(! m_CertVerifyFlag)
    {
        RSAP_ERROR_PRINT << "[error]ssl cert Verify failed!";
        return false;
    }
    if(!m_CloudServiceConfig.enablePushStatus)
    {
        return false;
    }
    if (!m_DeviceStatusCache.empty())
    {
        makeStatusUpData(m_DeviceStatusCache.rbegin()->second);
    }
    else
    {
        makeStatusUpDataFromDb();
    }
    return true;
}

bool RSAP_COMPONENT::makeStatusUpDataFromDb()
{
  if(!m_CloudServiceConfig.enablePushStatus) {
		return false;
  }
    std::vector<MsgDeviceStatus> statusList;
    std::pair<std::string, TABLE_TYPE> tableInfo;
    tableInfo.first = m_CloudServiceConfig.deviceStatusDbTableName;
    tableInfo.second = TABLE_TYEP_SENSOR_DEV_STATUS;

    m_DeviceStatusMsgDBPtr->getAllDeviceStatus(tableInfo, statusList);

//    for (auto v: statusList)
//    {
//        RSAP_DEBUG_PRINT << "[info]" << v.to_string().c_str();
//    }
    afl::net::ByteBuffer bufferUp(512, 8192);
    RsapMsgHeader rsapMsgHeader;
    rsapMsgHeader.StartFlag = RSAP_PACKAGE_START_FLAG;
    uint64_t nowMillSeconds = 0;
    if(m_CloudServiceConfig.enableUseAddUtc8)
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() + UTC8_MILLSECODND_DIFF_VALUE;
    }
    else
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() ;
    }
    rsapMsgHeader.TimestampMS = htons((nowMillSeconds)% (1000 * 60));
    rsapMsgHeader.TimestampMIN = htonl((nowMillSeconds) / (1000 * 60));

    rsapMsgHeader.DataType = RCU2CLOUD_STATUS_DATA_TYPE;
    rsapMsgHeader.Version = RCU2CLOUD_STATUS_VERSION;

    afl::net::ByteBuffer buffer(512, 8192);
    buffer.retrieveAll();

    StatusDataHead statusDataHead;
    statusDataHead.channelId = CHANNEL_SOURCE_DATANG_ROADSIDE_PERCEPTION_DEVICE;

    {
        memset(statusDataHead.rcuId, 0, sizeof(statusDataHead.rcuId));
        uint32_t needLen = std::min(m_CloudServiceConfig.objRcuId.size(),sizeof(statusDataHead.rcuId));
        std::string ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
        std::memcpy(statusDataHead.rcuId, ctbeStr.c_str(), ctbeStr.size());
    }

    statusDataHead.status = htons(RcuStatusTypeNormal);
    buffer.write(&statusDataHead, sizeof(StatusDataHead));

    const uint32_t device_status_info_sum = statusList.size();
    int32_t indexCam = 0;
    int32_t indexRadar = 0;
    int32_t indexLidar = 0;
    afl::net::ByteBuffer bufferCam;
    afl::net::ByteBuffer bufferRadar;
    afl::net::ByteBuffer bufferLidar;
    CamStatusHead camStatusHead;
    RadarStatusHead radarStatusHead;
    LidarStatusHead lidarStatusHead;
    camStatusHead.camNum = 0;
    radarStatusHead.radarNum = 0;
    lidarStatusHead.lidarNum = 0;

    for(uint32_t i = 0; i < device_status_info_sum; i++)
    {
        auto device_status_info = statusList[i];

        if(device_status_info.deviceType == DEVICE_TYPE_DB_CAMRA)
        {
            CamStatusTail camStatusTail;
            //camNum
            camStatusHead.camNum++;
            //id
            camStatusTail.id = indexCam++;
            bufferCam.write(camStatusTail.id);
            //camId
            {
                memset(camStatusTail.camId, 0, sizeof(camStatusTail.camId));
                uint32_t needLen = std::min(device_status_info.deviceID.size(), sizeof(camStatusTail.camId) * 2);
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[needLen]" << needLen;
                }
//                    std::string ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
                std::string ctbeStr = deviceCodeZip( device_status_info.deviceID.substr(0, needLen));
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[ctbeStr]" << ctbeStr;
                }
                std::memcpy(camStatusTail.camId, ctbeStr.c_str(), ctbeStr.size());
                bufferCam.write(camStatusTail.camId, sizeof(camStatusTail.camId));
            }

            //camStatus
            switch(device_status_info.active)
            {
                case DEVICE_ACTIVE_DB_ON:
                    camStatusTail.camStatus = DEVICE_STATUS_ON;
                    break;
                case DEVICE_ACTIVE_DB_OFF:
                    camStatusTail.camStatus = DEVICE_STATUS_OFF;
                    break;
                default:
                    camStatusTail.camStatus = DEVICE_STATUS_OFF;
                    break;
            }
            bufferCam.write(  camStatusTail.camStatus);
        }
        if(device_status_info.deviceType == DEVICE_TYPE_DB_MW_RADAR)
        {
            RadarStatusTail radarStatusTail;
            //radarNum
            radarStatusHead.radarNum++;
            //id
            radarStatusTail.id = indexRadar++;
            bufferRadar.write( radarStatusTail.id);
            //radarId
            {
                memset(radarStatusTail.radarId, 0, sizeof(radarStatusTail.radarId));
                uint32_t needLen = std::min(device_status_info.deviceID.size(), sizeof(radarStatusTail.radarId) * 2);
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[needLen]" << needLen;
                }
//                    std::string ctbeStr = (device_status_info.device_id().substr(0, needLen));
                std::string ctbeStr = deviceCodeZip( device_status_info.deviceID.substr(0, needLen));
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[ctbeStr]" << ctbeStr;
                }
                std::memcpy(radarStatusTail.radarId, ctbeStr.c_str(), ctbeStr.size());
                bufferRadar.write( radarStatusTail.radarId, sizeof(radarStatusTail.radarId));
            }

            //radarStatus
            switch(device_status_info.active)
            {
                case DEVICE_ACTIVE_DB_ON:
                    radarStatusTail.radarStatus = DEVICE_STATUS_ON;
                    break;
                case DEVICE_ACTIVE_DB_OFF:
                    radarStatusTail.radarStatus = DEVICE_STATUS_OFF;
                    break;
                default:
                    radarStatusTail.radarStatus = DEVICE_STATUS_OFF;
                    break;
            }
            bufferRadar.write( radarStatusTail.radarStatus);
        }
        if(device_status_info.deviceType == DEVICE_TYPE_DB_RADAR)
        {
            LidarStatusTail lidarStatusTail;
            //lidarNum
            lidarStatusHead.lidarNum++;
            //id
            lidarStatusTail.id = indexLidar++;
            bufferLidar.write(lidarStatusTail.id );
            //lidarId
            {
                memset(lidarStatusTail.lidarId, 0, sizeof(lidarStatusTail.lidarId));
                uint32_t needLen = std::min(device_status_info.deviceID.size(),sizeof(lidarStatusTail.lidarId) * 2);
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[needLen]" << needLen;
                }
//                    std::string ctbeStr = (device_status_info.device_id().substr(0, needLen));
                std::string ctbeStr = deviceCodeZip( device_status_info.deviceID.substr(0, needLen));
                if(m_CloudServiceConfig.enablePrintInfoLog)
                {
                    RSAP_DEBUG_PRINT << "[ctbeStr]" << ctbeStr;
                }
                std::memcpy(lidarStatusTail.lidarId, ctbeStr.c_str(), ctbeStr.size());
                bufferLidar.write(lidarStatusTail.lidarId, sizeof(lidarStatusTail.lidarId));
            }

            //lidarStatus
            switch(device_status_info.active)
            {
                case DEVICE_ACTIVE_DB_ON:
                    lidarStatusTail.lidarStatus = DEVICE_STATUS_ON;
                    break;
                case DEVICE_ACTIVE_DB_OFF:
                    lidarStatusTail.lidarStatus = DEVICE_STATUS_OFF;
                    break;
                default:
                    lidarStatusTail.lidarStatus = DEVICE_STATUS_OFF;
                    break;
            }
            bufferLidar.write(lidarStatusTail.lidarStatus);
        }
    }

    buffer.write(&camStatusHead, sizeof(CamStatusHead));
    buffer.write(bufferCam.peek(), bufferCam.readableBytes());


    buffer.write(&radarStatusHead, sizeof(RadarStatusHead));
    buffer.write(bufferRadar.peek(), bufferRadar.readableBytes());

    buffer.write(&lidarStatusHead, sizeof(LidarStatusHead));
    buffer.write(bufferLidar.peek(), bufferLidar.readableBytes());

    int dataLen = buffer.readableBytes();
    auto datalenTemp = boost::endian::native_to_big(dataLen) >> 8;

    memcpy(rsapMsgHeader.DataLen , &datalenTemp, sizeof(rsapMsgHeader.DataLen));
    bufferUp.write(&rsapMsgHeader, sizeof(RsapMsgHeader));
    bufferUp.write(buffer.peek(), buffer.readableBytes());

    if(m_ConnetedFlag && m_CurrConn)
    {
        printBufferData(bufferUp, "设备状态");
        if(m_CloudServiceConfig.enablePushStatus)
        {
            m_CurrConn->send(&bufferUp);
        }
        if(m_CloudServiceConfig.enablePrintParseInfoStatus)
        {
            m_ParseInfo.parseStatus(&bufferUp);
        }

        if(m_CloudServiceConfig.enbaleDeviceStatusReconnect)
        {
            m_MsgRespFlag.statusPushedFlag = true;
            timerDeviceStatusMonitorFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorDeviceStatusResp, this),
                                                               m_CloudServiceConfig.deviceStatusMonitorPeriod, true);
        }
    }
    bufferUp.retrieveAll();
    return true;
}

bool RSAP_COMPONENT::makeStatusUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if(!m_CloudServiceConfig.enablePushStatus)
    {
      return false;
    }
    afl::net::ByteBuffer bufferUp(512, 8192);
    RsapMsgHeader rsapMsgHeader;
    rsapMsgHeader.StartFlag = RSAP_PACKAGE_START_FLAG;
    uint64_t nowMillSeconds = 0;
    if(m_CloudServiceConfig.enableUseAddUtc8)
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() + UTC8_MILLSECODND_DIFF_VALUE;
    }
    else
    {
        nowMillSeconds = afl::util::TimeStamp::now(true).millSeconds() ;
    }
    rsapMsgHeader.TimestampMS = htons((nowMillSeconds)% (1000 * 60));
    rsapMsgHeader.TimestampMIN = htonl((nowMillSeconds) / (1000 * 60));

    rsapMsgHeader.DataType = RCU2CLOUD_STATUS_DATA_TYPE;
    rsapMsgHeader.Version = RCU2CLOUD_STATUS_VERSION;

    afl::net::ByteBuffer buffer(512, 8192);
    buffer.retrieveAll();
    if (mecDeviceData->has_mec_heart_beat())
    {
        auto heart_beat = mecDeviceData->mec_heart_beat();
        StatusDataHead statusDataHead;
        statusDataHead.channelId = CHANNEL_SOURCE_DATANG_ROADSIDE_PERCEPTION_DEVICE;

        {
            memset(statusDataHead.rcuId, 0, sizeof(statusDataHead.rcuId));
            uint32_t needLen = std::min(m_CloudServiceConfig.objRcuId.size(),sizeof(statusDataHead.rcuId));
            std::string ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
            std::memcpy(statusDataHead.rcuId, ctbeStr.c_str(), ctbeStr.size());
        }

        statusDataHead.status = htons(RcuStatusTypeNormal);
        buffer.write(&statusDataHead, sizeof(StatusDataHead));

        const uint32_t device_status_info_sum = heart_beat.device_status_info().size();
        int32_t indexCam = 0;
        int32_t indexRadar = 0;
        int32_t indexLidar = 0;
        afl::net::ByteBuffer bufferCam;
        afl::net::ByteBuffer bufferRadar;
        afl::net::ByteBuffer bufferLidar;
        CamStatusHead camStatusHead;
        RadarStatusHead radarStatusHead;
        LidarStatusHead lidarStatusHead;
        camStatusHead.camNum = 0;
        radarStatusHead.radarNum = 0;
        lidarStatusHead.lidarNum = 0;

        for(uint32_t i = 0; i < device_status_info_sum; i++)
        {
            auto device_status_info = heart_beat.device_status_info()[i];

            if(device_status_info.device_type() == HEART_BEAT_TYPE_CAMRA)
            {
                CamStatusTail camStatusTail;
                //camNum
                camStatusHead.camNum++;
                //id
                camStatusTail.id = indexCam++;
                bufferCam.write(camStatusTail.id);
                //camId
                {
                    memset(camStatusTail.camId, 0, sizeof(camStatusTail.camId));
                    uint32_t needLen = std::min(device_status_info.device_id().size(), sizeof(camStatusTail.camId) * 2);
//                    std::string ctbeStr = (m_CloudServiceConfig.objRcuId.substr(0, needLen));
                    std::string ctbeStr = deviceCodeZip( device_status_info.device_id().substr(0, needLen));
                    std::memcpy(camStatusTail.camId, ctbeStr.c_str(), ctbeStr.size());
                    bufferCam.write(camStatusTail.camId, sizeof(camStatusTail.camId));
                }

                //camStatus
                switch(device_status_info.status())
                {
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_ON:
                        camStatusTail.camStatus = DEVICE_STATUS_ON;
                        break;
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_OFF:
                        camStatusTail.camStatus = DEVICE_STATUS_OFF;
                        break;
                    default:
                        camStatusTail.camStatus = DEVICE_STATUS_OFF;
                        break;
                }
                bufferCam.write(  camStatusTail.camStatus);
            }
            if(device_status_info.device_type() == HEART_BEAT_TYPE_MW_RADAR)
            {
                RadarStatusTail radarStatusTail;
                //radarNum
                radarStatusHead.radarNum++;
                //id
                radarStatusTail.id = indexRadar++;
                bufferRadar.write( radarStatusTail.id);
                //radarId
                {
                    memset(radarStatusTail.radarId, 0, sizeof(radarStatusTail.radarId));
                    uint32_t needLen = std::min(device_status_info.device_id().size(), sizeof(radarStatusTail.radarId) * 2);
//                    std::string ctbeStr = (device_status_info.device_id().substr(0, needLen));
                    std::string ctbeStr = deviceCodeZip( device_status_info.device_id().substr(0, needLen));
                    std::memcpy(radarStatusTail.radarId, ctbeStr.c_str(), ctbeStr.size());
                    bufferRadar.write( radarStatusTail.radarId, sizeof(radarStatusTail.radarId));
                }

                //radarStatus
                switch(device_status_info.status())
                {
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_ON:
                        radarStatusTail.radarStatus = DEVICE_STATUS_ON;
                        break;
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_OFF:
                        radarStatusTail.radarStatus = DEVICE_STATUS_OFF;
                        break;
                    default:
                        radarStatusTail.radarStatus = DEVICE_STATUS_OFF;
                        break;
                }
                bufferRadar.write( radarStatusTail.radarStatus);
            }
            if(device_status_info.device_type() == HEART_BEAT_TYPE_RADAR)
            {
                LidarStatusTail lidarStatusTail;
                //lidarNum
                lidarStatusHead.lidarNum++;
                //id
                lidarStatusTail.id = indexLidar++;
                bufferLidar.write(lidarStatusTail.id );
                //lidarId
                {
                    memset(lidarStatusTail.lidarId, 0, sizeof(lidarStatusTail.lidarId));
                    uint32_t needLen = std::min(device_status_info.device_id().size(),sizeof(lidarStatusTail.lidarId) * 2);
//                    std::string ctbeStr = (device_status_info.device_id().substr(0, needLen));
                    std::string ctbeStr = deviceCodeZip( device_status_info.device_id().substr(0, needLen));
                    std::memcpy(lidarStatusTail.lidarId, ctbeStr.c_str(), ctbeStr.size());
                    bufferLidar.write(lidarStatusTail.lidarId, sizeof(lidarStatusTail.lidarId));
                }

                //lidarStatus
                switch(device_status_info.status())
                {
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_ON:
                        lidarStatusTail.lidarStatus = DEVICE_STATUS_ON;
                        break;
                    case airos::usecase::MecHeartBeatDeviceStatus::HEART_BEAT_STATUS_OFF:
                        lidarStatusTail.lidarStatus = DEVICE_STATUS_OFF;
                        break;
                    default:
                        lidarStatusTail.lidarStatus = DEVICE_STATUS_OFF;
                        break;
                }
                bufferLidar.write(lidarStatusTail.lidarStatus);
            }
        }

        buffer.write(&camStatusHead, sizeof(CamStatusHead));
        buffer.write(bufferCam.peek(), bufferCam.readableBytes());


        buffer.write(&radarStatusHead, sizeof(RadarStatusHead));
        buffer.write(bufferRadar.peek(), bufferRadar.readableBytes());

        buffer.write(&lidarStatusHead, sizeof(LidarStatusHead));
        buffer.write(bufferLidar.peek(), bufferLidar.readableBytes());

        int dataLen = buffer.readableBytes();
        auto datalenTemp = boost::endian::native_to_big(dataLen) >> 8;

        memcpy(rsapMsgHeader.DataLen , &datalenTemp, sizeof(rsapMsgHeader.DataLen));
        bufferUp.write(&rsapMsgHeader, sizeof(RsapMsgHeader));
        bufferUp.write(buffer.peek(), buffer.readableBytes());

        if(m_ConnetedFlag && m_CurrConn)
        {
            printBufferData(bufferUp, "设备状态");
            if(m_CloudServiceConfig.enablePushStatus)
            {
                m_CurrConn->send(&bufferUp);
            }
            if(m_CloudServiceConfig.enablePrintParseInfoStatus)
            {
                m_ParseInfo.parseStatus(&bufferUp);
            }

            if(m_CloudServiceConfig.enbaleDeviceStatusReconnect)
            {
                m_MsgRespFlag.statusPushedFlag = true;
                timerDeviceStatusMonitorFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::monitorDeviceStatusResp, this),
                                                                m_CloudServiceConfig.deviceStatusMonitorPeriod, true);
            }
        }
        bufferUp.retrieveAll();
    }
    buffer.retrieveAll();
    return true;
}

bool RSAP_COMPONENT::monitorDeviceStatusResp()
{
    if(m_MsgRespFlag.statusRespFlag)
    {
        m_MsgRespFlag.statusPushedFlag = false;
        m_MsgRespFlag.statusPushCount = 0;
        m_MsgRespFlag.statusRespFlag = false;
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client recv hear-beat resp!";
            RSAP_DEBUG_PRINT << "[fd]" << timerHeartBeatMonitorFd;
        }

        cancelTimer(timerDeviceStatusMonitorFd);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client cancle timer!";
        }
    }
    else
    {
        if (m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[fd]" << timerDeviceStatusMonitorFd;
        }
        cancelTimer(timerDeviceStatusMonitorFd);
        m_MsgRespFlag.statusPushedFlag = false;
        m_MsgRespFlag.statusPushCount++;
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[note]client not  recv hear-beat resp![statusPushCount]" << m_MsgRespFlag.statusPushCount;
        }

        if(m_MsgRespFlag.statusPushCount >= 3)
        {
            cancelTimer(timerDeviceStatusFd);
            if (m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[timerDeviceStatusFd]" << timerDeviceStatusFd;
            }
            m_CurrConn->shutdown();
            m_TcpClient->stop();
            m_TcpClient->disconnect();
            m_DeviceStatusReconnectFlag = true;
            m_MsgRespFlag.statusPushCount = 0;
            m_MsgRespFlag.statusPushedFlag = false;
            m_MsgRespFlag.statusRespFlag = false;
            timeIntervalDeviceStatus += (m_CloudServiceConfig.deviceStatusReconnectTimeMinute * 60);
            if (m_CloudServiceConfig.enablePrintInfoLog)
            {
                RSAP_DEBUG_PRINT << "[timeIntervalDeviceStatus]" << timeIntervalDeviceStatus;
            }
            timerDeviceStatusIntervalFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::reconnectServerDeviceStatus, this), timeIntervalDeviceStatus, true);
        }
    }
    return true;
}
void RSAP_COMPONENT::reconnectServerDeviceStatus()
{
    if(m_DeviceStatusReconnectFlag)
    {
        m_TcpClient->reconnect();
        m_DeviceStatusReconnectFlag = false;
        cancelTimer(timerDeviceStatusIntervalFd);
        timerDeviceStatusFd = m_Eventloop->addTimer(std::bind(&RSAP_COMPONENT::pushDeviceStatusData, this), m_CloudServiceConfig.deviceStatusPeriod, true);
        if (m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[timerDeviceStatusFd]" << timerDeviceStatusFd;
        }
    }
    return;
}
/**
 * @brief transfer protobuf-data to protocol-data
 * @param mecDeviceData
 * @param buffer
 */
bool RSAP_COMPONENT::processPbData2RsapData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if (!mecDeviceData)
    {
        RSAP_ERROR_PRINT << "[error]tcp not connected!";
        return false;
    }

    if (!mecDeviceData->perception_obstacle().empty()) {
        // 统计丢包率
        ++recv_total_package_num;
        recv_total_obj_num += mecDeviceData->perception_obstacle().size();
        if (m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "summary: Expected recv total package num: "
                         << mecDeviceData->header().total_package_num()
                         << ", Actual recv package num: " << recv_total_package_num
                         << ", Packet loss rate: " << std::fixed
                         << (mecDeviceData->header().total_package_num() - recv_total_package_num) * 1.0 /
                         mecDeviceData->header().total_package_num();


            RSAP_DEBUG_PRINT << "summary: Expected recv total obj num: "
                         << mecDeviceData->header().total_obj_num()
                         << ", Actual recv obj num: " << recv_total_obj_num
                         << ", Obj loss rate: " << std::fixed
                         << (mecDeviceData->header().total_obj_num() - recv_total_obj_num) * 1.0 /
                         mecDeviceData->header().total_obj_num();
        }
        double time_now = airos::base::TimeUtil::GetCurrentTime();
        if (m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "summary: Send timestamp: " << std::fixed
                         << mecDeviceData->header().timestamp_sec() << " s, recv timestamp: " << time_now
                         << " s, delay: " << time_now - mecDeviceData->header().timestamp_sec() << " s";
        }
    }

    //判断类型
    if (mecDeviceData->perception_obstacle_size() > 0 && m_CloudServiceConfig.enableScribeObj)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[notce]obj size:" << mecDeviceData->perception_obstacle_size();
            RSAP_DEBUG_PRINT << "[obj-pb]" << mecDeviceData->DebugString();
        }
        makeObjsUpData(mecDeviceData);
        return true;
    }

    if (mecDeviceData->events_size() > 0 && m_CloudServiceConfig.enableScribeEvent)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "[notce]event size:%d" << mecDeviceData->events_size();
            RSAP_DEBUG_PRINT << "[obj-pb]" << mecDeviceData->DebugString();
        }

        makeEventUpData(mecDeviceData);
        return true;
    }

#if 0
    if (mecDeviceData->has_mec_heart_beat() && m_CloudServiceConfig.enableScribeStatus)
    {
        makeStatusUpData(mecDeviceData);
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[notce]heartbeat";
        }
        if(m_CloudServiceConfig.enablePrintInfo)
        {
            AIROS_ERRORPRINT("[notce]heartbeat");
        }
        return true;
    }
#endif
    return true;
}

bool RSAP_COMPONENT::getWorkParamFromFile()
{
    std::string  workParamFilePath = FLAGS_work_param_file_path;
    WorkParam::getWorkParamFromFile(workParamFilePath, omWorkParamConfiger);
//    RSAP_ERROR_PRINT << "[work-param]" << omWorkParamConfiger.to_string().c_str();
    FLAGS_obj_rcuId = omWorkParamConfiger.mecDeviceWorkParam.rcuid;
    FLAGS_server_ip = omWorkParamConfiger.mecDeviceWorkParam.senseCloudIp;
    FLAGS_server_port =   omWorkParamConfiger.mecDeviceWorkParam.senseCloudPort;
    FLAGS_tls_ca_file_name = omWorkParamConfiger.mecDeviceWorkParam.senseCaCertName;
    FLAGS_tls_client_key_file_name = omWorkParamConfiger.mecDeviceWorkParam.senseClientCertName;
    FLAGS_tls_client_private_key_file_name = omWorkParamConfiger.mecDeviceWorkParam.senseClientPrivateKeyFile;
    FLAGS_tls_client_private_Key_passwd = omWorkParamConfiger.mecDeviceWorkParam.senseClientPrivateKeyPwd;
//    SetFlag(&FLAGS_server_ip, omWorkParamConfiger.mecDeviceWorkParam.senseCloudIp);

    std::string configFileName = "/home/airos/protocol/rsap/conf/rsap_config.flag";
    std::ifstream configFile(configFileName);
    if (!configFile.is_open()) {
        RSAP_ERROR_PRINT <<   "Unable to open config file: " << configFileName;
        return false;
    }

    std::string tempFileName = configFileName + ".tmp";
    std::ofstream tempFile(tempFileName);
    if (!tempFile.is_open())
    {
        RSAP_ERROR_PRINT <<  "Unable to create temporary file: " << tempFileName;
        return false;
    }

    std::string line;
    bool serverIpUpdated = false;

    while (std::getline(configFile, line)) {
        // 移除行末的回车符（若存在）
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = line.substr(0, equalPos);
            if (key == "--obj_rcuId") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.rcuid;
                serverIpUpdated = true;
            }
            else if (key == "--server_ip") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.senseCloudIp;
                serverIpUpdated = true;
            }
            else   if (key == "--server_port") {
                line = key + "=" + std::to_string(omWorkParamConfiger.mecDeviceWorkParam.senseCloudPort);
                serverIpUpdated = true;
            }
            else   if (key == "--tls_ca_file_name") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.senseCaCertName;
                serverIpUpdated = true;
            }
            else   if (key == "--tls_client_key_file_name") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.senseClientCertName;
                serverIpUpdated = true;
            }
            else   if (key == "--tls_client_private_key_file_name") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.senseClientPrivateKeyFile;
                serverIpUpdated = true;
            }
            else   if (key == "--tls_client_private_Key_passwd") {
                line = key + "=" + omWorkParamConfiger.mecDeviceWorkParam.senseClientPrivateKeyPwd;
                serverIpUpdated = true;
            }
        }
        tempFile << line << std::endl;
    }

    configFile.close();
    tempFile.close();

    if (serverIpUpdated) {
        std::remove(configFileName.c_str());
        std::rename(tempFileName.c_str(), configFileName.c_str());
    } else {
        std::remove(tempFileName.c_str());
    }
    m_CloudServiceConfig.deviceNoMapFilePath = FLAGS_project_root +
            FLAGS_conf_dir_name + "/" + FLAGS_device_no_map_file_name ;
    writeDeviceNoInfoToFile(omWorkParamConfiger.mecDeviceWorkParam.deviceSn, omWorkParamConfiger.mecDeviceWorkParam.deviceEsn);
    return true;
}
/**
 * @brief 获取服务器连接信息
 * @return
 */
bool RSAP_COMPONENT::getServerConfigInfo(CloudServiceConfig &cloudServiceConfig)
{
    cloudServiceConfig.serverIP = FLAGS_server_ip;
    cloudServiceConfig.serverPort = FLAGS_server_port;
    cloudServiceConfig.localPort = FLAGS_local_port;
    cloudServiceConfig.heartBeatPeriod = FLAGS_heart_beat_period;
    cloudServiceConfig.heartBeatMonitorPeriod = FLAGS_heart_beat_monitor_period;
    cloudServiceConfig.heartBeatReconnectTimeMinute = FLAGS_heart_beat_reconnect_time_minute;
    getEnableFlag(cloudServiceConfig.enbaleHeartBeatReconnect, FLAGS_enbale_heart_beat_reconnect);

    cloudServiceConfig.deviceStatusPeriod = FLAGS_device_status_period;
    cloudServiceConfig.deviceStatusMonitorPeriod = FLAGS_device_status_monitor_period;
    cloudServiceConfig.deviceStatusReconnectTimeMinute = FLAGS_device_status_reconnect_time_minute;
    getEnableFlag(cloudServiceConfig.enbaleDeviceStatusReconnect, FLAGS_enbale_device_status_reconnect);
    cloudServiceConfig.deviceStatusDelPeriod =  FLAGS_device_status_del_period;

    cloudServiceConfig.eventMonitorPeriod = FLAGS_event_monitor_period;
    cloudServiceConfig.eventReconnectTimeMinute = FLAGS_event_reconnect_time_minute;
    getEnableFlag(cloudServiceConfig.enbaleEventReconnect, FLAGS_enbale_event_reconnect);

    cloudServiceConfig.protocol = FLAGS_protocol;
    cloudServiceConfig.objRcuId = FLAGS_obj_rcuId;
    cloudServiceConfig.objDeviceId = FLAGS_obj_device_id;
    cloudServiceConfig.objDeviceType = FLAGS_obj_deviceType;

    cloudServiceConfig.projectRoot = FLAGS_project_root;

    cloudServiceConfig.tlsRoot = FLAGS_tls_root;
    cloudServiceConfig.tlsCaFileName = FLAGS_tls_ca_file_name;
    cloudServiceConfig.tlsClientKeyFileName = FLAGS_tls_client_key_file_name;
    cloudServiceConfig.tlsClientPrivateKeyFileName = FLAGS_tls_client_private_key_file_name;
    cloudServiceConfig.tlsClientPrivateKeyPasswd = FLAGS_tls_client_private_Key_passwd;


    getEnableFlag(cloudServiceConfig.enabelUseTls, FLAGS_enable_use_tls);

    getEnableFlag(cloudServiceConfig.enableUse02, FLAGS_enable_use_02);
    getEnableFlag(cloudServiceConfig.enableUseAddUtc8, FLAGS_enable_use_add_utc8);

    getEnableFlag(cloudServiceConfig.enablePushHeartbeat, FLAGS_enable_push_heartbeat);
    getEnableFlag(cloudServiceConfig.enablePushObj, FLAGS_enable_push_obj);
    getEnableFlag(cloudServiceConfig.enablePushEvent, FLAGS_enable_push_event);
    getEnableFlag(cloudServiceConfig.enablePushStatus, FLAGS_enable_push_status);


    getEnableFlag(cloudServiceConfig.enableScribeHeartbeat, FLAGS_enable_scribe_heartbeat);
    getEnableFlag(cloudServiceConfig.enableScribeObj, FLAGS_enable_scribe_obj);
    getEnableFlag(cloudServiceConfig.enableScribeEvent, FLAGS_enable_scribe_event);
    getEnableFlag(cloudServiceConfig.enableScribeStatus, FLAGS_enable_scribe_status);

    getEnableFlag(cloudServiceConfig.enablePrintInfo, FLAGS_enable_print_info);
    getEnableFlag(cloudServiceConfig.enablePrintInfoLog, FLAGS_enable_print_info_log);
    getEnableFlag(cloudServiceConfig.enable_print_sensor_channel_data_info, FLAGS_enable_print_sensor_channel_data_info);
    getEnableFlag(cloudServiceConfig.enable_use_real_dev_no, FLAGS_enable_use_real_dev_no);
    getEnableFlag(cloudServiceConfig.enable_use_camera_no_add_01_as_hash_compute, FLAGS_enable_use_camera_no_add_01_as_hash_compute);
    getEnableFlag(cloudServiceConfig.enablePrintParseInfoHeartbeat, FLAGS_enable_print_parse_info_heartbeat);
    getEnableFlag(cloudServiceConfig.enablePrintParseInfoObj, FLAGS_enable_print_parse_info_obj);
    getEnableFlag(cloudServiceConfig.enablePrintParseInfoEvent, FLAGS_enable_print_parse_info_event);
    getEnableFlag(cloudServiceConfig.enablePrintParseInfoStatus, FLAGS_enable_print_parse_info_status);
    cloudServiceConfig.confDirName = FLAGS_conf_dir_name;
    cloudServiceConfig.deviceStatusDbPath = FLAGS_db_path;
    cloudServiceConfig.deviceStatusDbTableName = FLAGS_db_table_name;
    cloudServiceConfig.deviceNoMapFileName = FLAGS_device_no_map_file_name;
    cloudServiceConfig.workParamFilePath = FLAGS_work_param_file_path;

    cloudServiceConfig.deviceNoMapFilePath = cloudServiceConfig.projectRoot +
            cloudServiceConfig.confDirName + "/" + cloudServiceConfig.deviceNoMapFileName ;

    return true;
}

void RSAP_COMPONENT::getEnableFlag(bool& enbleFlag, int type)
{
    switch (type)
    {
        case 1: //不加密
            enbleFlag = true;
            break;
        case 2:
            enbleFlag = false;
            break;
        default:
            enbleFlag = true;
            break;
    }
}

/**
 * 获取当前年份01月01日 00：00：00的时间戳
 * @author:zhangenwei
 */
int64_t RSAP_COMPONENT::getCurrentYearStartSeconds()
{
    time_t currentYearStartUtcSeconds = 0;
    int64_t currentYearStartUtcSecondsTemp = 0;

    /////////////////////////////////////////////////////////////////////////////////////////////
    //获取当前年份xxxx-01-01 00:00:00开始
    struct tm result;
    time_t currentTimeUTCSecondsTemp = afl::util::TimeStamp::now(true).seconds();
    localtime_r(&currentTimeUTCSecondsTemp, &result);
    std::string currentYearFormat = std::to_string((result.tm_year + 1900)) + "-01-01 00:00:00";
    afl::util::DateTime::stringToDataTime(currentYearFormat.c_str(), &currentYearStartUtcSeconds);
    /////////////////////////////////////////////////////////////////////////////////////////////
    currentYearStartUtcSecondsTemp = currentYearStartUtcSeconds;
    return  currentYearStartUtcSecondsTemp;
}

std::string RSAP_COMPONENT::getIDStr(std::string inputStr)
{
    std::string inputString(inputStr);
    std::string newString;
    try{
        // 从输入字符串中提取时间字符串、设备编号和轮转编号
        std::string timeString = inputString.substr(1, 17);
        std::string deviceString = inputString.substr(18, 20);
        std::string rotationString = inputString.substr(38, 3);
        // 从时间字符串中提取后7位
        std::string newTimeString = timeString.substr(timeString.length() - 7);
        // 从设备编号中提取后6位
        std::string newDeviceString = deviceString.substr(deviceString.length() - 6);
        // 组合新的字符串
        newString = newTimeString + newDeviceString + rotationString;
    }
    catch (std::exception& e)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[error]" << e.what();
        }
    }

    return newString;
}
/**
 * @func： get random num
 * @param num
 * @return
 */
std::string RSAP_COMPONENT::srandStr(uint32_t num)
{
    string ret;
    char m[64] = {0};
    char s[10] = {0};
    srand(time(0));
    for (uint32_t i = 0; i < num; i++)
    {
        int x, type;
        type = rand() % 3;
        if (type == 0)//判断随机类型生成大小写或者字母
        {
            x = rand() % ('Z' - 'A' + 1) + 'A';
        } else if (type == 1)
        {
            x = rand() % ('z' - 'a' + 1) + 'a';
        } else if (type == 2)
        {
            x = rand() % ('9' - '0' + 1) + '0';
        }
        sprintf(s, "%c", x);
        strcat((char *) m, (const char *) s);
    }
    ret = m;
    printf("%s\n", ret.c_str());
    return m;
}

std::string RSAP_COMPONENT::deviceCodeZip(std::string codeStr) 
{
    if (m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[codeStr]" << codeStr;
    }
  auto data = codeStr;
  vector<unsigned char> bytes;
  try {
    for (int i = 0; i < (int)data.length(); i += 2) {
      auto strTemp = data.substr(i, 2);
      int byte_value = stoi(strTemp, nullptr, 10);
      bytes.push_back(static_cast<unsigned char>(byte_value));
    }
  }catch(afl::util::Exception & e)
  {
    RSAP_ERROR_PRINT << "[error]" << e.what();
  }
    std::string retStr(bytes.begin(), bytes.end());
    return  retStr;
}
void RSAP_COMPONENT::assignVectorToUint8Array(const std::vector<unsigned char>& bytes, uint8_t* camId, int count)
{
    if ((int)bytes.size() != count)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_DEBUG_PRINT << "Error: Input vector size is not 11.";
        }
        return;
    }

    for (int i = 0; i < count; i++)
    {
        camId[i] = bytes[i];
    }
}
std::string RSAP_COMPONENT::convertToBigEndian(std::string input)
{
    std::string result(input);

    // 如果系统是小端字节序,则需要进行转换
    if (htonl(1) != 1)
    {
        std::reverse(result.begin(), result.end());
    }

    return result;
}
bool RSAP_COMPONENT::getDeviceNoMapInfoFromFile( std::map<std::string, std::string>& deviceNoMapInfoMap)
{
    if(m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[notice]device-no-map-file-path:" << m_CloudServiceConfig.deviceNoMapFilePath;
    }
    std::ifstream file(m_CloudServiceConfig.deviceNoMapFilePath.c_str());
    if (!file)
    {
        if(m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[Error]Unable to open file!";
        }
        return false;
    }
    std::string line;
    while (std::getline(file, line))
    {
        size_t pos = line.find('&');
        if (pos != std::string::npos)
        {
            std::string realNo = line.substr(pos + 1);
            std::string allocNo = line.substr(0, pos);
            deviceNoMapInfoMap[realNo] = allocNo;
        }
    }
    file.close();
    return true;
}
bool RSAP_COMPONENT::writeDeviceNoInfoToFile(std::string mecSn, std::string mecEsn)
{
    if (m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[notice]device-no-map-file-path:" << m_CloudServiceConfig.deviceNoMapFilePath;
    }
    std::ofstream file(m_CloudServiceConfig.deviceNoMapFilePath.c_str(), std::ios::trunc);
    if (!file)
    {
        if (m_CloudServiceConfig.enablePrintInfoLog)
        {
            RSAP_ERROR_PRINT << "[Error]Unable to open file for writing!";
        }
        return false;
    }
    file << mecEsn << "&" << mecSn << std::endl;
    file.close();
    return true;
}

void RSAP_COMPONENT::pushMonitorMecLinkStatus(bool con_flag)
{
    auto* ml_rsap =  mec_monitor_->mutable_ml_rsap();
    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_RSAP);
    ml_rsap->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    ml_rsap->set_con_flag(con_flag);
    if (m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/rsap/data_,[data]" << mec_monitor_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/rsap/data_", mec_monitor_);
}

void RSAP_COMPONENT::monitorMecLinkStatus()
{
    auto* ml_rsap =  mec_monitor_->mutable_ml_rsap();
    ml_rsap->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_RSAP);
    ml_rsap->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    if (m_ConnetedFlag  && m_CertVerifyFlag)
    {
        ml_rsap->set_con_flag(true);
    }
    else
    {
        ml_rsap->set_con_flag(false);
    }
    if (m_CloudServiceConfig.enablePrintInfoLog)
    {
        RSAP_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/rsap/link_,[data]" << mec_monitor_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/rsap/link_", mec_monitor_);
}
NAMESPACE_PROTOCOL_THREAD_END
