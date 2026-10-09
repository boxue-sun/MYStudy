/*
* @Author: zhangenwei
* @Date: 2024-02-18 9:20:01
* @LastEditors: zhangenwei
* @LastEditTime: 2024-02-20 9:21:11
* @Description:
*/
#include "om_monitor_component.h"
NAMESPACE_START_OM_COMPONENT_MONITOR

bool OM_MONITOR_COMPONENT::Init()
{
#if ENABLE_ENCRYPTION
    auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
    std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
    OM_MONITOR_DEBUG_PRINT << "License is: " << license << std::endl;
    int result = FusionService::Authenticator::GetInstance().Authorize(license);
    if (result != 0)
    {
        OM_MONITOR_ERROR_PRINT << "License generated fail, error code:  " << result;
        exit(1);
    }
#endif

    m_UdpBuffer = std::unique_ptr<afl::net::ByteBuffer>(new afl::net::ByteBuffer(128, 4096));
    m_Task.reset(new std::thread([&](){ initEventLoop(); }));

    doBaiscWork();
   return true;
}

bool OM_MONITOR_COMPONENT::initEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    m_EventloopIsRunning = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::monitorEventloopRunning, this), 4, true);
   m_Eventloop->loop();

   return true;
}

bool OM_MONITOR_COMPONENT::Proc(const std::shared_ptr<const  airos::monitor::MonitorResponse>& recv_monitor_)
{
//    OM_MONITOR_DEBUG_PRINT <<  recv_monitor_->DebugString();
    if (!recv_monitor_)
    {
        OM_MONITOR_ERROR_PRINT << "data empty!";
        return false;
    }
    int tag = 0;
    if (recv_monitor_->has_rsap_response())
    {
        if(!getConfiger().configerEanbleMonitor.Enable_MonitorRsapStatus)
        {
            return false;
        }
        const auto& rsap_response = recv_monitor_->rsap_response();
        try{
            tag = rsap_response.tag();
        }
        catch (...)
        {
            OM_MONITOR_ERROR_PRINT << "[error]" << "parse json failure!";
            return false;
        }
        switch (tag)
        {
            case airos::usecase::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_IN:
            {
                m_SensorDataStatusDataIn.tag = (om::monitor::QUERY_MSG_TAG)rsap_response.tag();
                m_SensorDataStatusDataIn.timestamp = rsap_response.timestamp();

                m_SensorDataStatusDataIn.data.sensor_data_in_port = rsap_response.sensor_data_in_port();
                m_SensorDataStatusDataIn.data.sensor_data_in_flag = rsap_response.sensor_data_in_flag();        // 是否接收到感知数据
                m_SensorDataStatusDataIn.data.sensor_data_in_obj_package = rsap_response.sensor_data_in_obj_package();                 // 感知目标2011大包个数
                m_SensorDataStatusDataIn.data.sensor_data_in_obj_num = rsap_response.sensor_data_in_obj_num();                     // 感知目标2011 总目标个数
            }
                break;
            case airos::usecase::MonitorMsgTag::MONITOR_TAG_SENSOR_DATA_UP:
            {
                m_SensorDataStatusDataUp.timestamp = rsap_response.timestamp();
                m_SensorDataStatusDataUp.data.sensor_cloud_ip = rsap_response.sensor_cloud_ip();                    // 感知运控的ip
                m_SensorDataStatusDataUp.data.sensor_cloud_port = rsap_response.sensor_cloud_port();                  // 感知运控的端口
                m_SensorDataStatusDataUp.data.sensor_cloud_con_flag = rsap_response.sensor_cloud_con_flag();                     // 感知上云模块连接云控是否正常
                m_SensorDataStatusDataUp.data.sensor_data_up_flag = rsap_response.sensor_dat_up_flag();                       // 感知数据上云是否成功
                m_SensorDataStatusDataUp.data.sensor_data_up_obj_package = rsap_response.sensor_data_up_obj_package();                 // 感知目标2011大包个数
                m_SensorDataStatusDataUp.data.sensor_data_up_obj_num = rsap_response.sensor_data_up_obj_num();                     // 感知目标2011 总目标个数
                break;
            }
            default:
                break;
        }
    }

    if (recv_monitor_->has_ccindex_response())
    {
        if(!getConfiger().configerEanbleMonitor.Enable_MonitorCcindexData)
        {
            return false;
        }
        const auto& ccindex_response = recv_monitor_->ccindex_response();
        try{
            tag = ccindex_response.tag();
        }
        catch (...)
        {
            OM_MONITOR_ERROR_PRINT << "[error]" << "parse json failure!";
            return false;
        }
        if(tag == airos::monitor::MonitorMsgTag::MONITOR_TAG_MONITOR_CCINDEX)
        {
            std::lock_guard<std::mutex> lock(m_CcindexStatusMonitor_Mutex);
            {
                m_CcindexStatusMonitor.tag = (om::monitor::QUERY_MSG_TAG)ccindex_response.tag();
                m_CcindexStatusMonitor.seqnum = ccindex_response.seqnum();
                m_CcindexStatusMonitor.timestamp = ccindex_response.timestamp();
                m_CcindexStatusMonitor.device_esn = ccindex_response.device_esn();
                auto ccindex_response_data = ccindex_response.data();
                m_CcindexStatusMonitor.data.con_flag = ccindex_response_data.con_flag();
                auto ccindex_tc_status = ccindex_response_data.ccindex_tc_status();
                m_CcindexStatusMonitor.data.ccindex_tc_status.tc_sub_flag = ccindex_tc_status.tc_sub_flag();
                m_CcindexStatusMonitor.data.ccindex_tc_status.tc_pub_flag = ccindex_tc_status.tc_pub_flag();
                m_CcindexStatusMonitor.data.ccindex_tm_status.clear();
                auto ccindex_tm_status_data_pb = ccindex_response_data.ccindex_tm_status();
                for(int i = 0; i < ccindex_tm_status_data_pb.size(); i++)
                {
                    auto ccindex_tm_status_pb = ccindex_tm_status_data_pb[i];
                    CcindexTmStatus ccindexTmStatus;
                    ccindexTmStatus.device_esn = ccindex_tm_status_pb.device_esn();
                    ccindexTmStatus.device_sn = ccindex_tm_status_pb.device_sn();
                    ccindexTmStatus.tm_pub_flag = ccindex_tm_status_pb.tm_pub_flag();
                    ccindexTmStatus.tm_sub_flag = ccindex_tm_status_pb.tm_sub_flag();
                    m_CcindexStatusMonitor.data.ccindex_tm_status.push_back(ccindexTmStatus);
                }
            }

        }
    }

    if (recv_monitor_->has_om_response())
    {
        if(!getConfiger().configerEanbleMonitor.Enable_MonitorCcindexData)
        {
            return false;
        }
        const auto& om_response = recv_monitor_->om_response();
        try{
            tag = om_response.tag();
        }
        catch (...)
        {
            OM_MONITOR_ERROR_PRINT << "[error]" << "parse json failure!";
            return false;
        }

        if(tag == airos::monitor::MonitorMsgTag::MONITOR_TAG_MONITOR_OM)
        {
            std::lock_guard<std::mutex> lock(m_OmStatusMonitor_Mutex);
            {
                m_OmStatusMonitor.tag = (om::monitor::QUERY_MSG_TAG)om_response.tag();
                m_OmStatusMonitor.seqnum = om_response.seqnum();
                m_OmStatusMonitor.timestamp = om_response.timestamp();
                m_OmStatusMonitor.device_esn = om_response.device_esn();
                auto om_response_data = om_response.data();
                m_OmStatusMonitor.data.con_flag = om_response_data.con_flag();
                auto om_mec_status = om_response_data.om_mec_status();
                m_OmStatusMonitor.data.om_mec_status.register_flag = om_mec_status.register_flag();
                m_OmStatusMonitor.data.om_mec_status.heartbeat_flag = om_mec_status.heartbeat_flag();
                //radar运维
                m_OmStatusMonitor.data.om_radars_status.clear();
                auto om_radars_status_data_pb = om_response_data.om_radars_status();
                for(int i = 0; i < om_radars_status_data_pb.size(); i++) {
                    auto om_radars_status_pb = om_radars_status_data_pb[i];
                    OmDeviceStatus omDeviceStatus;
                    omDeviceStatus.device_esn = om_radars_status_pb.device_esn();
                    omDeviceStatus.device_sn = om_radars_status_pb.device_sn();
                    omDeviceStatus.register_flag = om_radars_status_pb.register_flag();
                    omDeviceStatus.heartbeat_flag = om_radars_status_pb.heartbeat_flag();
                    m_OmStatusMonitor.data.om_radars_status.push_back(omDeviceStatus);
                }
                //camera运维
                m_OmStatusMonitor.data.om_camera_status.clear();
                auto om_camera_status_data_pb = om_response_data.om_camera_status();
                for(int i = 0; i < om_camera_status_data_pb.size(); i++) {
                    auto om_camera_status_pb = om_camera_status_data_pb[i];
                    OmDeviceStatus omDeviceStatus;
                    omDeviceStatus.device_esn = om_camera_status_pb.device_esn();
                    omDeviceStatus.device_sn = om_camera_status_pb.device_sn();
                    omDeviceStatus.register_flag = om_camera_status_pb.register_flag();
                    omDeviceStatus.heartbeat_flag = om_camera_status_pb.heartbeat_flag();
                    m_OmStatusMonitor.data.om_camera_status.push_back(omDeviceStatus);
                }
            }
        }
    }

   return true;
}

void OM_MONITOR_COMPONENT::readUdpData()
{
    ssize_t recv_len = 0;
    struct sockaddr_in m_ClientAddr;
    m_ClientSocklen = sizeof(m_ClientAddr);
    recv_len = recvfrom(m_Udpfd, m_UdpBuffer->beginWrite(), m_UdpBuffer->writableBytes(), 0,
                        (struct sockaddr *) &m_ClientAddr, &m_ClientSocklen);
    if (recv_len < 0)
    {
        OM_MONITOR_ERROR_PRINT << "[error]recv_len < 0!";
        return;
    }

    if (recv_len > 0)
    {
        m_UdpBuffer->hasWritten(recv_len);
        afl::base::json j;
        try
        {
            j = afl::base::json::parse(m_UdpBuffer->peek(), m_UdpBuffer->peek() + m_UdpBuffer->readableBytes());
        }
        catch (...)
        {
            m_UdpBuffer->retrieveAll();
            OM_MONITOR_ERROR_PRINT << "Get msg Json parse error!";
            return;
        }

        if (j.find("tag") == j.end()) {
            OM_MONITOR_ERROR_PRINT << "Can not find tag!!!";
            return;
        }

        int type = j["tag"];
//        OM_MONITOR_DEBUG_PRINT << "msg_tyep: " << type;
        switch (type)
        {
            case QUARY_TAG_DOCKER_STA:  //3011
                process_query_docker_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_AIROS_STA: //3012
                process_query_airos_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_DISK_STA: //3013
                process_query_disk_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_SYS_PERFORMANCE_STA: //3014
                process_query_sys_performance_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_RSAP_DATA_STA: //3021
                process_query_sensor_data_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_OM_DATA_STA: //3022
                process_query_om_status(m_ClientAddr, j);
                break;
            case QUARY_TAG_CCINDEX_DATA_STA: //3023
                process_query_ccindex_status(m_ClientAddr, j);
                break;
            default:
                break;
        }

        m_UdpBuffer->retrieveAll();
    }
}

bool OM_MONITOR_COMPONENT::getWorkParamFromFile()
{
    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
//    OM_MONITOR_DEBUG_PRINT << "[work-param]" << omWorkParamConfiger.to_string().c_str();
    getConfiger().rscuEsn = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
    m_SensorIpPortData.sensor_cloud_ip = omWorkParamConfiger.mecDeviceWorkParam.senseCloudIp;
    m_SensorIpPortData.sensor_cloud_port  = std::to_string(omWorkParamConfiger.mecDeviceWorkParam.senseCloudPort);
    YAML::Node config = YAML::LoadFile(getConfiger().needFilePathConfiger.mec_device_yaml_path);
    // 读取数据
    int local_port = config["local_port"].as<int>();
    m_SensorIpPortData.sensor_data_in_port = std::to_string(local_port);
//    getConfiger().enableDebugPrint = omWorkParamConfiger.hasedInit;
    saveConfiger();
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[m_SensorIpPortData]" << m_SensorIpPortData.to_string();
    }
    return true;
}
void OM_MONITOR_COMPONENT::doBaiscWork()
{
    getWorkParamFromFile();
}
bool contains(const std::vector<std::string>& vec, const std::string& str)
{
    return std::find(vec.begin(), vec.end(), str) != vec.end();
}
bool OM_MONITOR_COMPONENT::udpInit()
{
    m_Udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(m_Udpfd < 0)
    {
        OM_MONITOR_ERROR_PRINT << "Create socket error! reason:" << strerror(errno);
        return false;
    }

    // 设置 SO_REUSEADDR 以允许地址重用
    int reuse = 1;
    if (setsockopt(m_Udpfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        OM_MONITOR_ERROR_PRINT << "Set SO_REUSEADDR failed: " << strerror(errno);
        close(m_Udpfd);
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    std::string hostIp = "0.0.0.0";
    addr.sin_addr.s_addr = inet_addr(hostIp.c_str());
    addr.sin_port = htons(getConfiger().udpPort);
    OM_MONITOR_DEBUG_PRINT << "[ip]" << hostIp << "[port]" << getConfiger().udpPort;
    if(bind(m_Udpfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        OM_MONITOR_ERROR_PRINT << "Socket bind error! reason: " << strerror(errno);
        close(m_Udpfd);
        return false;
    }

    return true;
}
void OM_MONITOR_COMPONENT::monitorEventloopRunning()
{
    if(m_Eventloop)
    {
        if(m_Eventloop->isInLoopThread())
        {
            udpInit();
            m_UdpChnl =  std::unique_ptr<afl::net::Channel>(new afl::net::Channel(m_Eventloop.get(), m_Udpfd));
//        m_UdpChnl.reset(new afl::net::Channel(m_Eventloop, m_Udpfd));
            m_UdpChnl->setReadCallback(std::bind(&OM_MONITOR_COMPONENT::readUdpData, this) );
            m_UdpChnl->enableReading();
            // m_UdpChnl->enableWriting();
            if(getConfiger().enableDebugPrint) {
                OM_MONITOR_DEBUG_PRINT << "[sucess]udp callback set sucess!";
            }
            //监控docker状态
            if(getConfiger().configerEanbleMonitor.Enable_MonitorDockerStatus)
            {
                m_MonitorDockerStatusOnce = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_docker_status_once, this), 4, false);
                m_MonitorDockerStatus = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_docker_status, this),
                                                              getConfiger().configerMonitorPeriod.periodMonitorDockerStatus, true);
            }
            if(getConfiger().configerEanbleMonitor.Enable_MonitorDockerStatus)
            {
                m_MonitorAirosContainerTimeOnce = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_container_time_once, this), 4, false);
                m_MonitorAirosModulesStatusOnce = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_airos_modules_status_once, this), 4, true);
                m_MonitorAirosModulesStatus = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_airos_modules_status, this),
                                                                    getConfiger().configerMonitorPeriod.periodMonitorAirosModulesStatus, true);
            }

            //监控磁盘状态
            if(getConfiger().configerEanbleMonitor.Enable_MonitorDiskStatus)
            {
                m_MonitorDiskStatusOnce = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_disk_status_once, this), 4, false);
                m_MonitorDiskStatus = m_Eventloop->addTimer(std::bind(&OM_MONITOR_COMPONENT::period_query_disk_status, this),
                                                              getConfiger().configerMonitorPeriod.periodMonitorDiskStatus, true);
            }


            if(m_EventloopIsRunning > 0)
            {
                m_Eventloop->cancelTimer(m_EventloopIsRunning);
                m_EventloopIsRunning = -1;
            }
        }
    }

}

bool OM_MONITOR_COMPONENT::period_query_container_time_once()
{
    SshInfo sshInfo;
    sshInfo.host = "127.0.0.1";
    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    // const char *docker_cmd =
    //     "docker inspect --format '{{.Name}} {{.State.StartedAt}}' $(docker ps -q) | "
    //     "while read name time; do "
    //     "  echo \"$name $(date -d \"$time\" \"+%a %b %d %H:%M %Y\")\"; "
    //     "done";
    std::string docker_cmd = "docker inspect --format='{{.Created}}' " + m_AirosDockerName;
    sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
    sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
    sshInfo.execCmd = "echo " + sshInfo.password + " | " + docker_cmd;
    int maxRetries = 3;
    int retryCount = 0;  // 当前重试次数
    bool cmdExecFlag = false;
    while (retryCount < maxRetries)
    {
        if (DBUtils::sshExec(sshInfo))
        {
            // 成功执行，直接返回 true
            cmdExecFlag = true;
            break;
        }

        OM_MONITOR_ERROR_PRINT << "[error] Reboot device failure! Attempt " << (retryCount + 1) << " failed.\n";
        retryCount++;
    }

    if(!cmdExecFlag)
    {
        OM_MONITOR_ERROR_PRINT << "[error]reboot device failure!";
        return false;
    }
    else
    {
        m_ContainerTime = sshInfo.execRet;
        if (m_ContainerTime.size() > 0)
        {
            if (m_MonitorAirosContainerTimeOnce > 0)
            {
                m_Eventloop->cancelTimer(m_MonitorAirosContainerTimeOnce);
                m_MonitorAirosContainerTimeOnce = -1;
            }
        }

    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MONITOR_DEBUG_PRINT << "[execRet]" << sshInfo.execRet;
    }
    return true;

}


void OM_MONITOR_COMPONENT::period_query_docker_status_once()
{

    if(m_MonitorDockerStatusOnceCount == 4)
    {
        if(m_MonitorDockerStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorDockerStatusOnce);
            m_MonitorDockerStatusOnce = -1;
        }
    }

    if(period_query_docker_status())
    {
        if(m_MonitorDockerStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorDockerStatusOnce);
            m_MonitorDockerStatusOnce = -1;
        }
    }
    else
    {
        m_MonitorDockerStatusOnceCount++;
    }
}

bool OM_MONITOR_COMPONENT::period_query_docker_status()
{

    DockerStatus dockerStatus;
    SshInfo sshInfo;
    sshInfo.host = "127.0.0.1";
    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
    sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
    sshInfo.execCmd = "echo " + sshInfo.password + "| sudo -S docker stats --no-stream";
//    OM_MONITOR_ERROR_PRINT << "[sshInfo]" << sshInfo.to_string();
    int maxRetries = 3;
    int retryCount = 0;  // 当前重试次数
    bool cmdExecFlag = false;
    while (retryCount < maxRetries)
    {
        if (DBUtils::sshExec(sshInfo))
        {
            // 成功执行，直接返回 true
            cmdExecFlag = true;
            break;
        }

        OM_MONITOR_ERROR_PRINT << "[error] Reboot device failure! Attempt " << (retryCount + 1) << " failed.\n";
        retryCount++;
    }

    if(!cmdExecFlag)
    {
        OM_MONITOR_ERROR_PRINT << "[error]reboot device failure!";
        return false;
    }
    else
    {
        std::vector<ContainerInfo> airosContainers = extractAirosContainers(sshInfo.execRet);
        if (airosContainers.size() == 0)
        {
            dockerStatus.tag = QUERY_MSG_TAG::QUARY_TAG_DOCKER_STA;
            dockerStatus.seqnum = "period";
            dockerStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
            dockerStatus.device_esn = getConfiger().rscuEsn;
            dockerStatus.data.cpu = 999.999;
            dockerStatus.data.mem_percent = 999.999;
            dockerStatus.data.mem = 999.999;
            dockerStatus.data.run_flag = false;

            try{
                m_JsonDockerStatus = dockerStatus;
            }
            catch (json::exception &e)
            {
                OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
                return false;
            }
        }
        else
        {
            for (const auto &container : airosContainers)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MONITOR_DEBUG_PRINT << "[Container Name]" << std::left << std::setw(40)  <<  std::setfill(' ') << container.name
                                           << "[CPU Usage] "<< std::left << std::setw(10)  <<  std::setfill(' ') << container.cpuUsage
                                           << "% [Memory Usage]" << std::left << std::setw(10)  <<  std::setfill(' ') << container.memUsage << "MiB";
                }
                dockerStatus.tag = QUERY_MSG_TAG::QUARY_TAG_DOCKER_STA;
                dockerStatus.seqnum = "period";
                dockerStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
                dockerStatus.device_esn = getConfiger().rscuEsn;
                dockerStatus.data.cpu = container.cpuUsage;
                dockerStatus.data.mem_percent = container.memPercent;
                dockerStatus.data.mem = container.memUsage;
                dockerStatus.data.run_flag = true;

                try{
                    m_JsonDockerStatus = dockerStatus;
                }
                catch (json::exception &e)
                {
                    OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
                    return false;
                }
            }
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MONITOR_DEBUG_PRINT << "[execRet]" << sshInfo.execRet;
    }
    return true;

}

void OM_MONITOR_COMPONENT::period_query_disk_status_once()
{

    if(m_MonitorDiskStatusOnceCount == 4)
    {
        if(m_MonitorDiskStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorDiskStatusOnce);
            m_MonitorDiskStatusOnce = -1;
        }
    }

    if(period_query_disk_status())
    {
        if(m_MonitorDiskStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorDiskStatusOnce);
            m_MonitorDiskStatusOnce = -1;
        }
    }
    else
    {
        m_MonitorDiskStatusOnceCount++;
    }

}

bool OM_MONITOR_COMPONENT::period_query_disk_status()
{

    DiskStatus diskStatus;
    SshInfo sshInfo;
    sshInfo.host = "127.0.0.1";
    OmWorkParamConfiger  omWorkParamConfiger;
    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    sshInfo.user = omWorkParamConfiger.omPtpLogParamConfiger.localSshUserName;
    sshInfo.password = omWorkParamConfiger.omPtpLogParamConfiger.localSshPassword;
    sshInfo.execCmd = "echo " + sshInfo.password + "| sudo df -h /work /";

    int maxRetries = 3;
    int retryCount = 0;  // 当前重试次数
    bool cmdExecFlag = false;
    while (retryCount < maxRetries)
    {
        if (DBUtils::sshExec(sshInfo))
        {
            // 成功执行，直接返回 true
            cmdExecFlag = true;
            break;
        }

        OM_MONITOR_ERROR_PRINT << "[error] Reboot device failure! Attempt " << (retryCount + 1) << " failed.\n";
        retryCount++;
    }

    if(!cmdExecFlag)
    {
        OM_MONITOR_ERROR_PRINT << "[error]reboot device failure!";
        return false;
    }
    else
    {
        std::vector<DiskInfo> diskInfos = extractDiskInfos(sshInfo.execRet);
        if (diskInfos.size() == 0)
        {
            diskStatus.tag = QUERY_MSG_TAG::QUARY_TAG_DISK_STA;
            diskStatus.seqnum = "period";
            diskStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
            diskStatus.device_esn = getConfiger().rscuEsn;
            diskStatus.data.root_size = 999.999;
            diskStatus.data.root_avail = 999.999;
            diskStatus.data.root_avail_percent = 999.999;
            diskStatus.data.work_size = 999.999;
            diskStatus.data.work_avail = 999.999;
            diskStatus.data.work_avail_percent = 999.999;

            afl::base::json publishJson;
            try{
                m_JsonDiskStatus = diskStatus;
            }
            catch (json::exception &e)
            {
                OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
                return false;
            }
        }
        else
        {
            diskStatus.tag = QUERY_MSG_TAG::QUARY_TAG_DISK_STA;
            diskStatus.seqnum = "period";
            diskStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();
            diskStatus.device_esn = getConfiger().rscuEsn;
            for (const auto &disk : diskInfos)
            {
                if(getConfiger().enableDebugPrint)
                {
                    OM_MONITOR_DEBUG_PRINT << std::endl
                    <<"[fileSystem]" << std::left << std::setw(40)  <<  std::setfill(' ') << disk.fileSystem
                                           << " [size]" << std::left << std::setw(40)  <<  std::setfill(' ')  << disk.size << "G"
                                           << " [used]" << std::left << std::setw(40)  <<  std::setfill(' ')  << disk.used << "G"
                                           << " [avail]" << std::left << std::setw(40)  <<  std::setfill(' ') << disk.avail << "G"
                                           << " [use]" << std::left << std::setw(40)  <<  std::setfill(' ')  << disk.use << "%"
                                           << " [mountedOn]" << std::left << std::setw(40)  <<  std::setfill(' ') << disk.mountedOn;
                }

                if(disk.mountedOn == "/")
                {
                    diskStatus.data.root_size = disk.size;
                    diskStatus.data.root_avail = disk.avail;
                    diskStatus.data.root_avail_percent = 100 - disk.use;
                }
                if(disk.mountedOn == "/work")
                {
                    diskStatus.data.work_size = disk.size;
                    diskStatus.data.work_avail = disk.avail;
                    diskStatus.data.work_avail_percent = 100 - disk.use;
                }
            }

            
            try{
                m_JsonDiskStatus = diskStatus;
            }
            catch (json::exception &e)
            {
                OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
                return false;
            }
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        OM_MONITOR_DEBUG_PRINT << "[execRet]" << sshInfo.execRet;
    }
    return true;

}

void OM_MONITOR_COMPONENT::period_query_airos_modules_status_once()
{
    if(m_MonitorAirosMOdulesStatusOnceCount == 4)
    {
        if(m_MonitorAirosModulesStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorAirosModulesStatusOnce);
            m_MonitorAirosModulesStatusOnce = -1;
        }
    }

    if(period_query_airos_modules_status())
    {
        if(m_MonitorAirosModulesStatusOnce > 0)
        {
            m_Eventloop->cancelTimer(m_MonitorAirosModulesStatusOnce);
            m_MonitorAirosModulesStatusOnce = -1;
        }
    }
    else
    {
        m_MonitorAirosMOdulesStatusOnceCount++;
    }

}

bool OM_MONITOR_COMPONENT::period_query_airos_modules_status()
{
    AirosStatus airosStatus;
    airosStatus.tag = QUERY_MSG_TAG::QUARY_TAG_AIROS_STA;
    airosStatus.seqnum = "test";
    airosStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
    airosStatus.device_esn = getConfiger().rscuEsn;
    if(m_ContainerTime != " ")
    {
        // std::istringstream lineStream(m_ContainerTime);
        // std::string name, week, month, day, time, year;
        // lineStream >> name >> week >> month >> day >> time >> year;
        // airosStatus.data.container_name = name;
        // airosStatus.data.container_start_time = week + " " + month + " " + day + " " + time + " " + year;

        airosStatus.data.container_name = m_AirosDockerName;
        airosStatus.data.container_start_time = m_ContainerTime;
    }

    std::string command = "ps -eo pid,comm,lstart,etime,%cpu,%mem,cmd --sort=-%mem | grep -E 'mainboard|python'| head -n 5";
    std::vector<ProcessInfo> airosProcessInfos;
    int maxRetries = 3;
    int retryCount = 0;  // 当前重试次数
    bool cmdExecFlag = false;
    while (retryCount < maxRetries)
    {
        if (getProcessInfo(command, airosProcessInfos)) // 查询mainboard启动的模块
        {
            // 成功执行，直接返回 true
            cmdExecFlag = true;
            break;
        }

        OM_MONITOR_ERROR_PRINT << "[error] get airos-moduoles failure! Attempt " << (retryCount + 1) << " failed.\n";
        retryCount++;
    }

    if (!cmdExecFlag)
    {
        OM_MONITOR_ERROR_PRINT << "[error]reboot device failure!";
        return false;
    }
    else
    {
        for (const auto &processInfo : airosProcessInfos)
        {
            AirosMoudlesData airosMoudlesData;
            airosMoudlesData.pid = processInfo.pid;
            airosMoudlesData.cpu = processInfo.cpu;
            airosMoudlesData.mem = processInfo.mem;
            airosMoudlesData.start_time = processInfo.lstart;
            auto lastSlash = processInfo.cmd.find_last_of('/');
            auto lastDot = processInfo.cmd.find_last_of('.');
            if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash)
            {
                airosMoudlesData.name = processInfo.cmd.substr(lastSlash + 1, lastDot - lastSlash - 1);
            }
            if(airosMoudlesData.name != "")
            {
                airosStatus.data.modules_status.push_back(airosMoudlesData);
            }
        }
        std::string commandLaunchPs = "bash launch ps";
        std::vector<ProcessMoudlesNeedInfo> airosProcessMoudlesNeedInfos = getModulesNeedInfo(commandLaunchPs); // 查询所有需要启动的模块
        std::vector<std::string> otherModules;
        bool exist = false;
        int count = 0;
        for(auto m : m_AirosNeedRunModules)
        {
            for(unsigned int i = 0; i < airosStatus.data.modules_status.size(); i++)
            {
                count = 0;
                exist = false;
                if(airosStatus.data.modules_status[i].name != m)
                {
                    exist = false;
                    
                    continue;
                }
                else
                {
                    exist = true;
                    count = i;
                    break;
                }
                count = i;
            }

            if(exist == false && count == 0)
            {
                otherModules.push_back(m);
            }
        }
           
        bool flag = false;
        for(auto m : otherModules)
        {
            flag = false;
            for(auto nowModule : airosProcessMoudlesNeedInfos)
            {
                if(m == nowModule.moudle_name)
                {
                    AirosMoudlesData airosMoudlesData;
                    airosMoudlesData.pid = nowModule.pid;
                    airosMoudlesData.cpu = nowModule.cpu;
                    airosMoudlesData.mem = 999.999;
                    airosMoudlesData.name = nowModule.moudle_name;

                    std::time_t now = std::time(nullptr);
                    std::tm* localTime = std::localtime(&now);
                    char buffer[80];
                    std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localTime);

                    // 时间秒级
                    std::istringstream lineStream(buffer);
                    std::string week, month, day, time, year;
                    lineStream >>week >> month >> day >> time >> year;
                    airosMoudlesData.start_time = week + " " + month + " " + day + " " + nowModule.start + ":00" + " " + year;

                    airosStatus.data.modules_status.push_back(airosMoudlesData);
                    flag = true;
                }
            }

            if(flag == false)
            {
                // 其它未启动的模块
                AirosMoudlesData airosMoudlesData;
                airosMoudlesData.pid = "";
                airosMoudlesData.cpu = 999.999;
                airosMoudlesData.mem = 999.999;
                airosMoudlesData.name = m;
                airosMoudlesData.start_time = "";

                airosStatus.data.modules_status.push_back(airosMoudlesData);
            }
        }

        // for (auto i : airosStatus.data.modules_status)
        // {
        //     OM_MONITOR_DEBUG_PRINT << "module:" << i.to_string();
        // }

        try
        {
            m_JsonAirosModulesStatus = airosStatus;
            if (getConfiger().enableDebugPrint)
            {
                OM_MONITOR_DEBUG_PRINT << "[airosStatus]" << airosStatus.to_string();
            }
        }
        catch (json::exception &e)
        {
            OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            return false;
        }
    }

    return true;
}


bool OM_MONITOR_COMPONENT::process_query_sys_performance_status(struct sockaddr_in clientAddr, json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_SYS_PERFORMANCE_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }

    SysPerformanceStatus sysPerformanceStatus;
    airos::monitor::MonitorResponse monitorResponse;
    sysPerformanceStatus.tag = QUERY_MSG_TAG::QUARY_TAG_SYS_PERFORMANCE_STA;
    sysPerformanceStatus.seqnum = "period";
    sysPerformanceStatus.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
    sysPerformanceStatus.device_esn = getConfiger().rscuEsn;


    DockerStatus dockerStatus;
    m_JsonDockerStatus.get_to(dockerStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time)
    {
        if (((queryData.timestamp - dockerStatus.timestamp) / 1000) > getConfiger().configerMonitorPeriod.periodMonitorDockerStatus)
        {
            OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << dockerStatus.timestamp;
            dockerStatus.data.cpu = 999.999;
            dockerStatus.data.mem_percent = 999.999;
            dockerStatus.data.mem = 999.999;
            dockerStatus.data.run_flag = false;
        }
        else
        {
            sysPerformanceStatus.data.cpu = dockerStatus.data.cpu;
            sysPerformanceStatus.data.mem_percent = dockerStatus.data.mem_percent;
            sysPerformanceStatus.data.mem = dockerStatus.data.mem;
            sysPerformanceStatus.data.run_flag = dockerStatus.data.run_flag;
        }
    }
    else
    {
        sysPerformanceStatus.data.cpu = dockerStatus.data.cpu;
        sysPerformanceStatus.data.mem_percent = dockerStatus.data.mem_percent;
        sysPerformanceStatus.data.mem = dockerStatus.data.mem;
        sysPerformanceStatus.data.run_flag = dockerStatus.data.run_flag;
    }


    AirosStatus airosStatus;
    // 英文星期到中文的映射
    std::map<std::string, std::string> weekMap = {
        {"Mon", "星期一"}, {"Tue", "星期二"}, {"Wed", "星期三"}, 
        {"Thu", "星期四"}, {"Fri", "星期五"}, {"Sat", "星期六"}, {"Sun", "星期日"}
    };
    
    // 英文月份到中文的映射
    std::map<std::string, std::string> monthMap = {
        {"Jan", "一月"}, {"Feb", "二月"}, {"Mar", "三月"}, {"Apr", "四月"},
        {"May", "五月"}, {"Jun", "六月"}, {"Jul", "七月"}, {"Aug", "八月"},
        {"Sep", "九月"}, {"Oct", "十月"}, {"Nov", "十一月"}, {"Dec", "十二月"}
    };
    m_JsonAirosModulesStatus.get_to(airosStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time) {
        if (((queryData.timestamp - airosStatus.timestamp) / 1000) > getConfiger().configerMonitorPeriod.periodMonitorAirosModulesStatus) {
            OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << airosStatus.timestamp;
            airosStatus.data.container_name.clear();
            airosStatus.data.container_start_time.clear();
            airosStatus.data.modules_status.clear();
        } else {
            if (m_ContainerTime != " ") {
                OM_MONITOR_ERROR_PRINT << "[notice]m_ContainerTime != ";
                // std::istringstream lineStream(m_ContainerTime);
                // std::string name, week, month, day, time, year;
                // lineStream >> name >> week >> month >> day >> time >> year;
                //
                // // 转换星期和月份
                // std::string chineseWeek = weekMap[week];
                // std::string chineseMonth = monthMap[month];
                // sysPerformanceStatus.data.container_name = name;
                // sysPerformanceStatus.data.container_start_time = chineseWeek  + " " + chineseMonth  + " " + day + " " + time
                //     + " " + std::to_string(std::atoi(year.c_str()) + 1900);


                sysPerformanceStatus.data.container_name = m_AirosDockerName;
                sysPerformanceStatus.data.container_start_time = m_ContainerTime;
            }
            sysPerformanceStatus.data.modules_status = std::move(airosStatus.data.modules_status);
        }
    }
    else
    {
        if (m_ContainerTime != " ") {
            OM_MONITOR_ERROR_PRINT << "[notice]m_ContainerTime != ";
            // std::istringstream lineStream(m_ContainerTime);
            // std::string name, week, month, day, time, year;
            // lineStream >> name >> week >> month >> day >> time >> year;
            // // 转换星期和月份
            // std::string chineseWeek = weekMap[week];
            // std::string chineseMonth = monthMap[month];
            // sysPerformanceStatus.data.container_name = name;
            // sysPerformanceStatus.data.container_start_time = chineseWeek  + " " + chineseMonth  + " " + day + " " + time
            //         + " " + std::to_string(std::atoi(year.c_str()) + 1900);

            sysPerformanceStatus.data.container_name = m_AirosDockerName;
            sysPerformanceStatus.data.container_start_time = m_ContainerTime;
        }
        else
        {
            OM_MONITOR_ERROR_PRINT << "[m_ContainerTime]" << m_ContainerTime << "]";
        }
        sysPerformanceStatus.data.modules_status = std::move(airosStatus.data.modules_status);
    }

    DiskStatus diskStatus;
    m_JsonDiskStatus.get_to(diskStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time) {
        if (((queryData.timestamp - diskStatus.timestamp) / 1000) > getConfiger().configerMonitorPeriod.periodMonitorDiskStatus) {
            OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << diskStatus.timestamp;
            diskStatus.data.root_size = 999.999;
            diskStatus.data.root_avail = 999.999;
            diskStatus.data.root_avail_percent = 999.999;
            diskStatus.data.work_size = 999.999;
            diskStatus.data.work_avail = 999.999;
            diskStatus.data.work_avail_percent = 999.999;
        } else {
            sysPerformanceStatus.data.root_size = diskStatus.data.root_size;
            sysPerformanceStatus.data.root_avail = diskStatus.data.root_avail;
            sysPerformanceStatus.data.root_avail_percent = diskStatus.data.root_avail_percent;
            sysPerformanceStatus.data.work_size = diskStatus.data.work_size;
            sysPerformanceStatus.data.work_avail = diskStatus.data.work_avail;
            sysPerformanceStatus.data.work_avail_percent = diskStatus.data.work_avail_percent;
        }
    }
    else
    {
        sysPerformanceStatus.data.root_size = diskStatus.data.root_size;
        sysPerformanceStatus.data.root_avail = diskStatus.data.root_avail;
        sysPerformanceStatus.data.root_avail_percent = diskStatus.data.root_avail_percent;
        sysPerformanceStatus.data.work_size = diskStatus.data.work_size;
        sysPerformanceStatus.data.work_avail = diskStatus.data.work_avail;
        sysPerformanceStatus.data.work_avail_percent = diskStatus.data.work_avail_percent;
    }

    afl::base::json publishJson;
    try
    {
        publishJson = sysPerformanceStatus;
    }
    catch (json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }

    std::string querySeqNum = queryData.seqnum;
    if (publishJson.find("seqnum") != publishJson.end())
    {
        publishJson["seqnum"] = querySeqNum;
        std::string msg = publishJson.dump();
        if (getConfiger().enableDebugPrint)
        {
            OM_MONITOR_DEBUG_PRINT << "[sysPerformance-status][response]" << msg;
        }
        size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                             (struct sockaddr *)&clientAddr, sizeof(sockaddr_in));
        if (slen != msg.size())
        {
            OM_MONITOR_ERROR_PRINT << "[error]send sysPerformance-status msg error, reason:" << strerror(errno);
            return false;
        }
    }
    else
    {
        OM_MONITOR_ERROR_PRINT << "[error]no find seqNum field!";
    }

    return true;
}

std::vector<ContainerInfo> OM_MONITOR_COMPONENT::extractAirosContainers(const std::string &output)
{
    std::vector<ContainerInfo> containerList;
    std::istringstream stream(output);
    std::string line;

    // 跳过表头
    std::getline(stream, line);

    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        std::string separator;
        std::string containerId, name, cpuUsage, memUsage, memLimit, memPercent, netI, netO, blockI, blockO, pids;

        // 读取容器信息
        lineStream >> containerId >> name >> cpuUsage >> memUsage >> separator >> memLimit >> memPercent >> netI >> separator >> netO >> blockI >> separator >> blockO >> pids;
        // OM_MONITOR_ERROR_PRINT << "[name]" << name
        //                        << "[cpuUsage]" << cpuUsage
        //                        << "[memUsage]" << memUsage << "[memLimit]" << memLimit
        //                        << "[memPercent]" << memPercent
        //                        << "[netI]" << netI << "[netI]" << netO
        //                        << "[blockI]" << blockI << "[blockO]" << blockO
        //                        << "[pids]" << pids;

        OM_MONITOR_WARN_PRINT << "[Container Stats] "
                       << "Name: " << std::left << std::setw(40)  <<  std::setfill(' ') << name
                       << " | CPU: " << std::right << std::setw(10)  <<  std::setfill(' ') << std::fixed << std::setprecision(2) << cpuUsage << "%"
                       << " | Mem: " << std::setw(10) <<  std::setfill(' ') << memUsage << "MB/" << std::setw(6) <<  std::setfill(' ') << memLimit << "MB (" << std::setw(5) << memPercent << "%)"
                       << " | Net: " << std::setw(10) <<  std::setfill(' ') << netI << "MB↓/" << std::setw(6) <<  std::setfill(' ') << netO << "MB↑"
                       << " | IO: " << std::setw(10) <<  std::setfill(' ') << blockI << "MB↓/" << std::setw(6) <<  std::setfill(' ') << blockO << "MB↑"
                       << " | PIDs: " << pids;

        // 检查名称是否以airos开头
        if (name.find("airos") == 0)
        {
            // OM_MONITOR_DEBUG_PRINT << "[container-name]" << name;
            // 提取CPU和内存使用（去掉最后的%）
            ContainerInfo containerInfo;
            m_AirosDockerName =  containerInfo.name = name;

            containerInfo.cpuUsage = std::stod(cpuUsage.substr(0, cpuUsage.size() - 1));       // 去掉%并转换为double
            containerInfo.memUsage = std::stod(memUsage.substr(0, memUsage.find('/')));        // 只取使用部分
            containerInfo.memPercent = std::stod(memPercent.substr(0, memPercent.size() - 1)); // 去掉%并转换为double
            // 存储结果
            containerList.push_back(containerInfo);
        }
    }

    return containerList;
}
bool OM_MONITOR_COMPONENT::process_query_docker_status(struct sockaddr_in clientAddr, json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_DOCKER_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }

    DockerStatus dockerStatus;
    m_JsonDockerStatus.get_to(dockerStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time) {
        if (((queryData.timestamp - dockerStatus.timestamp) / 1000) > getConfiger().configerMonitorPeriod.periodMonitorDockerStatus) {
            OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << dockerStatus.timestamp;
            return false;
        }
    }

    std::string querySeqNum = queryData.seqnum;
    if(m_JsonDockerStatus.find("seqnum") != m_JsonDockerStatus.end())
    {
        m_JsonDockerStatus["seqnum"] = querySeqNum;
        std::string msg = m_JsonDockerStatus.dump();
        if(getConfiger().enableDebugPrint) {
            OM_MONITOR_DEBUG_PRINT << "[docker-status][response]" << msg;
        }
        size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                                (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if (slen != msg.size())
        {
            OM_MONITOR_ERROR_PRINT << "[error]send docker-status msg error, reason:" << strerror(errno);
            return false;
        }
    }
    else
    {
        OM_MONITOR_ERROR_PRINT << "[error]no find seqNum field!";
    }

    return true;
}

std::vector<DiskInfo> OM_MONITOR_COMPONENT::extractDiskInfos(const std::string &output)
{
    std::vector<DiskInfo> diskList;
    std::istringstream stream(output);
    std::string line;

    // 跳过表头
    std::getline(stream, line);

    while (std::getline(stream, line))
    {
        std::istringstream lineStream(line);
        std::string fileSystem, size, used, avail, use, mountedOn;

        // 读取容器信息
        lineStream >> fileSystem >> size >> used >> avail >> use >> mountedOn;

        DiskInfo diskInfo;
        diskInfo.fileSystem = fileSystem;
        if (size.find("T") != std::string::npos)
        {
            diskInfo.size = std::stod(size.substr(0, size.size() - 1)) * 1024; // 去掉G并转换为double
        }
        else
        {
            diskInfo.size = std::stod(size.substr(0, size.size() - 1)); // 去掉G并转换为double
        }
        if (used.find("T") != std::string::npos)
        {
            diskInfo.used = std::stod(used.substr(0, used.size() - 1)) * 1024; // 去掉G并转换为double
        }
        else
        {
            diskInfo.used = std::stod(used.substr(0, used.size() - 1)); // 去掉G并转换为double
        }
        if (avail.find("T") != std::string::npos)
        {
            diskInfo.avail = std::stod(avail.substr(0, avail.size() - 1)) * 1024; // 去掉G并转换为double
        }
        else
        {
            diskInfo.avail = std::stod(avail.substr(0, avail.size() - 1)); // 去掉G并转换为double
        }

        diskInfo.use = std::stod(use.substr(0, use.size() - 1)); // 去掉%并转换为double
        diskInfo.mountedOn = mountedOn;

        diskList.push_back(diskInfo);
    }

    return diskList;
}

bool OM_MONITOR_COMPONENT::process_query_disk_status(struct sockaddr_in clientAddr,json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_DISK_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }

    DiskStatus diskStatus;
    m_JsonDiskStatus.get_to(diskStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time) {
	    if(((queryData.timestamp - diskStatus.timestamp)/1000) > getConfiger().configerMonitorPeriod.periodMonitorDiskStatus)
	    {
	        OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << diskStatus.timestamp;
	        return false;
	    }

    }

    std::string querySeqNum = queryData.seqnum;
    if(m_JsonDiskStatus.find("seqnum") != m_JsonDiskStatus.end())
    {
        m_JsonDiskStatus["seqnum"] = querySeqNum;
        std::string msg = m_JsonDiskStatus.dump();
        if(getConfiger().enableDebugPrint)
        {
            OM_MONITOR_DEBUG_PRINT << "[disk-status][response]" << msg;
        }

        size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                                (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if (slen != msg.size())
        {
            OM_MONITOR_ERROR_PRINT << "[error]send disk-status msg error, reason:" << strerror(errno);
            return false;
        }
    }
    else
    {
        OM_MONITOR_ERROR_PRINT << "[error]no find seqNum field!";
    }

    return true;
}


bool OM_MONITOR_COMPONENT::getProcessInfo(std::string& command,  std::vector<ProcessInfo> &processes)
{
    // 使用popen执行命令并读取输出
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return false;
    }

    char buffer[128];
    std::string result;

    // 读取命令输出
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
    {
        result += buffer;
    }

    // 解析输出
    std::istringstream stream(result);
    std::string line;
    while (std::getline(stream, line))
    {
       // OM_MONITOR_ERROR_PRINT << "[line]" << line;
        std::istringstream lineStream(line);
        ProcessInfo process;
        std::string week, month, day, time, year, cmd, xx, args;

        lineStream >> process.pid >> process.comm >>week >> month >> day >> time >> year >> process.etime
                >> process.cpu >> process.mem >> cmd >> xx >> args;
                process.lstart=week+" "+ month + " " +day+" "+time+" "+year;
                process.cmd = cmd + " " + xx + " " + args;
        processes.push_back(process);
    }

    return true;
}
std::vector<ProcessMoudlesNeedInfo> OM_MONITOR_COMPONENT::getModulesNeedInfo(std::string& command)
{
    std::vector<ProcessMoudlesNeedInfo> processes;


    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cerr << "popen() failed!" << std::endl;
        return processes;
    }

    char buffer[128];
    std::string result;


    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
    {
        result += buffer;
    }


    std::istringstream stream(result);
    std::string line;

    while (std::getline(stream, line))
    {

        if (line.empty() || line == "--------------" || line.find("grep") != std::string::npos || line.find("bash") != std::string::npos) {
            continue;
        }

//        OM_MONITOR_ERROR_PRINT << "[line]" << line;
        std::istringstream lineStream(line);
        ProcessMoudlesNeedInfo process;


        if ( line.find("mainboard") != std::string::npos )
        {
            if (lineStream >> process.user >> process.pid >> process.ppid >> process.cpu >> process.start
                           >> process.tty >> process.cpuTime  >> process.command_name  >> process.command_type  >> process.moudle_name)
            {

                auto lastSlash = process.moudle_name.find_last_of('/');
                auto lastDot = process.moudle_name.find_last_of('.');
                if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash)
                {
                    process.moudle_name = process.moudle_name.substr(lastSlash + 1, lastDot - lastSlash - 1);
                }
            }

        }
        else if( line.find("python3") != std::string::npos)
        {
            if (lineStream >> process.user >> process.pid >> process.ppid >> process.cpu >> process.start
                           >> process.tty >> process.cpuTime  >> process.command_name  >> process.moudle_name)
            {

                auto lastSlash = process.moudle_name.find_last_of('/');
                auto lastDot = process.moudle_name.find_last_of('.');
                if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                    process.moudle_name = process.moudle_name.substr(lastSlash + 1, lastDot - lastSlash - 1);
                }
            }
        }
        else if( line.find("./bin/airos_app_framework") != std::string::npos)
        {
            if (lineStream >> process.user >> process.pid >> process.ppid >> process.cpu >> process.start
                           >> process.tty >> process.cpuTime  >> process.command_name  >> process.moudle_name)
            {

                auto lastSlash = process.moudle_name.find_last_of('/');
                auto lastDot = process.moudle_name.find_last_of('.');
                if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                    process.moudle_name = process.moudle_name.substr(lastSlash + 1, lastDot - lastSlash - 1);
                }
            }
        }
        processes.push_back(process);
//        OM_MONITOR_ERROR_PRINT << "[ProcessInfo]" << process.to_string();
    }

    return processes;
}

bool OM_MONITOR_COMPONENT::process_query_airos_status(struct sockaddr_in clientAddr,json& j)
{

    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    OM_MONITOR_DEBUG_PRINT << queryData.to_string();
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_AIROS_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }

    AirosStatus airosStatus;
    m_JsonAirosModulesStatus.get_to(airosStatus);
    if(getConfiger().configerEanbleMonitor.Enable_Judge_Quey_Time)
    {
        if (((queryData.timestamp - airosStatus.timestamp) / 1000) > getConfiger().configerMonitorPeriod.periodMonitorDiskStatus)
        {
            OM_MONITOR_ERROR_PRINT << "[timeout]" << "msg_time_ms=" << airosStatus.timestamp;
            return false;
        }
    }

    std::string querySeqNum = queryData.seqnum;
    if(m_JsonAirosModulesStatus.find("seqnum") != m_JsonAirosModulesStatus.end())
    {
         m_JsonAirosModulesStatus["seqnum"] = querySeqNum;
        std::string msg = m_JsonAirosModulesStatus.dump();
        if(getConfiger().enableDebugPrint) {
            OM_MONITOR_DEBUG_PRINT << "[querySeqNum]" << querySeqNum;
        }
        if(getConfiger().enableDebugPrint) {
            OM_MONITOR_DEBUG_PRINT << "[airos-modules][response]" << msg;
        }
        size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                                (struct sockaddr*)&clientAddr, sizeof(clientAddr));
        if (slen != msg.size())
        {
            OM_MONITOR_ERROR_PRINT << "[error]send airos-modules msg error, reason:" << strerror(errno);
            return false;
        }
    }
    else
    {
        OM_MONITOR_ERROR_PRINT << "[error]no find airos-modules seqNum field!";
    }

    return true;
}

bool OM_MONITOR_COMPONENT::process_query_om_status(struct sockaddr_in clientAddr,json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_OM_DATA_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }
    OmStatusResponse omStatusResponse;
    omStatusResponse.tag = QUERY_MSG_TAG::QUARY_TAG_OM_DATA_STA;
    omStatusResponse.seqnum = queryData.seqnum;
    omStatusResponse.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
    omStatusResponse.device_esn = getConfiger().rscuEsn;

    OmStatusResponse omStatusResponseTemp;
    std::lock_guard<std::mutex> lock(m_CcindexStatusMonitor_Mutex);
    {
        omStatusResponseTemp =  m_OmStatusMonitor;
    }
    OM_MONITOR_ERROR_PRINT << "[omStatusResponseTemp]" << omStatusResponseTemp.to_string();
    omStatusResponse.data.om_radars_status.clear();
    omStatusResponse.data.om_camera_status.clear();
    omStatusResponse.data.con_flag = omStatusResponseTemp.data.con_flag;
    omStatusResponse.data.om_mec_status = omStatusResponseTemp.data.om_mec_status;
    omStatusResponse.data.om_radars_status = omStatusResponseTemp.data.om_radars_status;
    omStatusResponse.data.om_camera_status = omStatusResponseTemp.data.om_camera_status;

    afl::base::json publishJson;
    try{
        publishJson = omStatusResponse;
    }
    catch (json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }

    std::string msg = publishJson.dump();
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[om_status][response]" << msg;
    }
    size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                         (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (slen != msg.size())
    {
        OM_MONITOR_ERROR_PRINT << "[error]send om-status msg error, reason:" << strerror(errno);
        return false;
    }
}
bool OM_MONITOR_COMPONENT::process_query_sensor_data_status(struct sockaddr_in clientAddr,json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_RSAP_DATA_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }


    m_SensorDataStatusData.tag = QUERY_MSG_TAG::QUARY_TAG_RSAP_DATA_STA;
    m_SensorDataStatusData.seqnum = queryData.seqnum;
    m_SensorDataStatusData.timestamp =  afl::util::TimeStamp::now(true).millSeconds();;
    m_SensorDataStatusData.device_esn = getConfiger().rscuEsn;

    //sensor-data-in-port
    if (!m_SensorDataStatusDataIn.data.sensor_data_in_port.empty())
    {
        m_SensorDataStatusData.data.sensor_data_in_port = m_SensorDataStatusDataIn.data.sensor_data_in_port;
    }
    else
    {
        m_SensorDataStatusData.data.sensor_data_in_port = m_SensorIpPortData.sensor_data_in_port ;
    }

    //sensor_data_in_obj_package
    if(m_SensorDataStatusData.data.sensor_data_in_obj_package  == m_SensorDataStatusDataIn.data.sensor_data_in_obj_package)
    {
        m_SensorDataStatusData.data.sensor_data_in_flag   = false;
        m_SensorDataStatusData.data.sensor_data_in_obj_package  = 0;
        m_SensorDataStatusData.data.sensor_data_in_obj_num  = 0;

        m_SensorDataStatusDataIn.data.sensor_data_in_obj_package = 0;
        m_SensorDataStatusDataIn.data.sensor_data_in_obj_num = 0;
    }
    else
    {
        m_SensorDataStatusData.data.sensor_data_in_flag = true;
        m_SensorDataStatusData.data.sensor_data_in_obj_package  = m_SensorDataStatusDataIn.data.sensor_data_in_obj_package;
        m_SensorDataStatusData.data.sensor_data_in_obj_num  = m_SensorDataStatusDataIn.data.sensor_data_in_obj_num;
    }

    if (!m_SensorDataStatusDataUp.data.sensor_cloud_ip.empty())
    {
        m_SensorDataStatusData.data.sensor_cloud_ip  =  m_SensorDataStatusDataUp.data.sensor_cloud_ip;
    }
    else
    {
        m_SensorDataStatusData.data.sensor_cloud_ip  = m_SensorIpPortData.sensor_cloud_ip ;
    }

    //sensor_cloud_port
    if (!m_SensorDataStatusDataUp.data.sensor_cloud_port.empty())
    {
        m_SensorDataStatusData.data.sensor_cloud_port  =  m_SensorDataStatusDataUp.data.sensor_cloud_port;
    }
    else
    {
        m_SensorDataStatusData.data.sensor_cloud_port  = m_SensorIpPortData.sensor_cloud_port ;
    }

    //sensor_cloud_con_flag
    m_SensorDataStatusData.data.sensor_cloud_con_flag  = m_SensorDataStatusDataUp.data.sensor_cloud_con_flag;

    //sensor_data_up_flag
    if ((m_SensorDataStatusData.timestamp - m_SensorDataStatusDataUp.timestamp) < 100000)
    {
        m_SensorDataStatusData.data.sensor_data_up_flag = m_SensorDataStatusDataUp.data.sensor_data_up_flag;
    }
    else
    {
        m_SensorDataStatusData.data.sensor_data_up_flag = false;
    }
    //sensor_data_up_obj_package
    m_SensorDataStatusData.data.sensor_data_up_obj_package = m_SensorDataStatusDataUp.data.sensor_data_up_obj_package;
    //sensor_data_up_obj_num
    m_SensorDataStatusData.data.sensor_data_up_obj_num = m_SensorDataStatusDataUp.data.sensor_data_up_obj_num;

    afl::base::json publishJson;
    try{
        publishJson = m_SensorDataStatusData;
    }
    catch (json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }

    m_SensorDataStatusData.data.sensor_data_in_flag = false;
    m_SensorDataStatusData.data.sensor_data_in_obj_package  = 0;
    m_SensorDataStatusData.data.sensor_data_in_obj_num  = 0;
    m_SensorDataStatusData.data.sensor_cloud_con_flag = false;
    m_SensorDataStatusData.data.sensor_data_up_obj_package = 0;
    m_SensorDataStatusData.data.sensor_data_up_obj_num = 0;
    std::string msg = publishJson.dump();
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[sensor-in&&sensor-up][response]" << msg;
    }
    size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                         (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (slen != msg.size())
    {
        OM_MONITOR_ERROR_PRINT << "[error]send sensor-in&&sensor-up msg error, reason:" << strerror(errno);
        return false;
    }
    return true;
}


bool OM_MONITOR_COMPONENT::process_query_ccindex_status(struct sockaddr_in clientAddr,json& j)
{
    QueryData queryData;
    try
    {
        queryData  = j;
    }
    catch(json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "json par error " << e.what();
        return false;
    }
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[rece-data]" << queryData.to_string();
    }
    if(queryData.tag != QUERY_MSG_TAG::QUARY_TAG_CCINDEX_DATA_STA)
    {
        OM_MONITOR_ERROR_PRINT << "tag error ";
        return false;
    }
    if(queryData.device_esn != getConfiger().rscuEsn)
    {
        OM_MONITOR_ERROR_PRINT << "device-esn error! [query-esn]" << queryData.device_esn << "[rscuEsn]" << getConfiger().rscuEsn;
        return false;
    }
    CcindexStatusResponse ccindexStatusResponse;
    CcindexStatusResponse ccindexStatusMonitorTemp;
    std::lock_guard<std::mutex> lock(m_CcindexStatusMonitor_Mutex);
    {
        ccindexStatusMonitorTemp =  m_CcindexStatusMonitor;
    }

    ccindexStatusResponse.data.ccindex_tm_status.clear();
    ccindexStatusResponse.tag = QUERY_MSG_TAG::QUARY_TAG_CCINDEX_DATA_STA;
    ccindexStatusResponse.seqnum = queryData.seqnum;
    ccindexStatusResponse.timestamp = afl::util::TimeStamp::now(true).millSeconds();
    ccindexStatusResponse.device_esn = getConfiger().rscuEsn;

    ccindexStatusResponse.data.con_flag = ccindexStatusMonitorTemp.data.con_flag;
    ccindexStatusResponse.data.ccindex_tc_status.tc_sub_flag = ccindexStatusMonitorTemp.data.ccindex_tc_status.tc_sub_flag;
    ccindexStatusResponse.data.ccindex_tc_status.tc_pub_flag = ccindexStatusMonitorTemp.data.ccindex_tc_status.tc_pub_flag;
    ccindexStatusResponse.data.ccindex_tm_status = ccindexStatusMonitorTemp.data.ccindex_tm_status;


    ccindexStatusResponse.data.ccindex_static_status.st_post_flag = ccindexStatusMonitorTemp.data.ccindex_static_status.st_post_flag;
    ccindexStatusResponse.data.ccindex_static_status.st_get_flag = ccindexStatusMonitorTemp.data.ccindex_static_status.st_get_flag;


    afl::base::json publishJson;
    try{
        publishJson = ccindexStatusResponse;
    }
    catch (json::exception &e)
    {
        OM_MONITOR_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }

    std::string msg = publishJson.dump();
    if(getConfiger().enableDebugPrint) {
        OM_MONITOR_DEBUG_PRINT << "[mec_radar_cloud][response]" << msg;
    }
    size_t slen = sendto(m_Udpfd, msg.c_str(), msg.size(), 0,
                         (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if (slen != msg.size())
    {
        OM_MONITOR_ERROR_PRINT << "[error]send mec_radar_cloud msg error, reason:" << strerror(errno);
        return false;
    }
    return true;
}

NAMESPACE_ENDED_OM_COMPONENT_MONITOR
