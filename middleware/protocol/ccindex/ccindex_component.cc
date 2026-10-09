/*********************************************************************************
 * @file		CCINDEX_COMPONENT
 * @brief		CCINDEX_COMPONENT belongs to CICTCI
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
#include "ccindex_component.h"
#include <cstdio>
#include <cstring>
NAMESPACE_START_RADAR_CCINDEX

// 把字节串（MQTT payload、json异常信息）转成可打印字符串用于日志：可见ASCII原样输出，
// 其它字节（含反斜杠）转成\xHH，这样非法UTF-8、控制字符、截断等都能看出来，也不会把日志文件变成二进制；
// 过长时只保留头尾
static std::string escapeForLog(const void* data, int len)
{
    const int kHead = 1024;
    const int kTail = 512;
    std::string out;
    const unsigned char* p = static_cast<const unsigned char*>(data);
    if (p == nullptr || len <= 0)
    {
        return out;
    }
    auto append = [&out, p](int begin, int end)
    {
        char hex[8];
        for (int i = begin; i < end; ++i)
        {
            if (p[i] >= 0x20 && p[i] < 0x7f && p[i] != '\\')
            {
                out += static_cast<char>(p[i]);
            }
            else
            {
                snprintf(hex, sizeof(hex), "\\x%02x", p[i]);
                out += hex;
            }
        }
    };
    if (len <= kHead + kTail)
    {
        append(0, len);
    }
    else
    {
        append(0, kHead);
        out += "...(" + std::to_string(len - kHead - kTail) + " bytes omitted)...";
        append(len - kTail, len);
    }
    return out;
}

int CCINDEX_COMPONENT::MAX_MESSAGE_COUNT_TC_CHANNEL = 1000; // 设置您的最大限制
bool CCINDEX_COMPONENT::Init()
{
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_MQTT_DEBUG_PRINT << "[notice]start  ccindex!";
    }
#if ENABLE_ENCRYPTION
    auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
    std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
    CCINDEX_INTER_DEBUG_PRINT << "License is: " << license << std::endl;
    int result = FusionService::Authenticator::GetInstance().Authorize(license);
    if (result != 0)
    {
        CCINDEX_INTER_ERROR_PRINT << "License generated fail, error code:  " << result;
        exit(1);
    }
#endif

    m_UdpBuffer = std::unique_ptr<afl::net::ByteBuffer>(new afl::net::ByteBuffer(128, 4096));
    m_Task.reset(new std::thread([&](){ initEventLoop(); }));
    m_TaskTrafficMetrics.reset(new std::thread([&](){ initEventLoopTrafficMetrics(); }));
    m_TaskTrafficMetricsPubFirst.reset(new std::thread([&](){ initEventLoopTrafficMetricsPubFirst(); }));
    m_TaskTrafficMetricsPubSecond.reset(new std::thread([&](){ initEventLoopTrafficMetricsPubSecond(); }));

//    m_TaskTrajectoriesTmstc.reset(new std::thread([&](){ initEventLoopTrajectoriesTmstc(); }));
//    m_TaskTrajectoriesDesaysv.reset(new std::thread([&](){ initEventLoopTrajectoriesDesaysv(); }));
//    m_TaskNoTrajectoriesOne.reset(new std::thread([&](){ initEventLoopNoTrajectoriesOne(); }));
//    m_TaskNoTrajectoriesTwo.reset(new std::thread([&](){ initEventLoopNoTrajectoriesTwo(); }));


    m_TaskTc.reset(new std::thread([&](){ initEventLoopTc(); }));
//    m_TaskTcPubFirst.reset(new std::thread([&](){ initEventLoopTcPubFirst(); }));
//    m_TaskTcPubSecond.reset(new std::thread([&](){ initEventLoopTcPubSecond(); }));
    m_TaskStatic.reset(new std::thread([&](){ initEventLoopStatic(); }));
    m_TaskStaticCloud.reset(new std::thread([&](){ initEventLoopCloud(); }));

    m_TaskUseCase.reset(new std::thread([&](){ initUseCase();}));
	
    // EventLoop构造时绑定当前线程，只能在各自线程里创建；这里等上面7个EventLoop都创建好，
    // 避免doBaiscWork()和随后到达的MQTT回调访问到还没创建的EventLoop（空指针）
    waitEventLoopsReady(7);
    doBaiscWork();
    return true;
}
bool CCINDEX_COMPONENT::initEventLoopTrajectoriesTmstc()
{
    m_EventloopTrajectoriesTmstc = std::make_shared<afl::net::EventLoop>();
    m_EventloopTrajectoriesTmstc->loop();
    return true;

}
bool CCINDEX_COMPONENT::initEventLoopTrajectoriesDesaysv()
{
    m_EventloopTrajectoriesDesaysv = std::make_shared<afl::net::EventLoop>();
    m_EventloopTrajectoriesDesaysv->loop();
    return true;

}
bool CCINDEX_COMPONENT::initEventLoopNoTrajectoriesOne()
{
    m_EventloopNoTrajectoriesOne = std::make_shared<afl::net::EventLoop>();
    m_EventloopNoTrajectoriesOne->loop();
    return true;

}
bool CCINDEX_COMPONENT::initEventLoopNoTrajectoriesTwo()
{
    m_EventloopNoTrajectoriesTwo = std::make_shared<afl::net::EventLoop>();
    m_EventloopNoTrajectoriesTwo->loop();
    return true;

}

// EventLoop只能在创建它的线程里析构（~EventLoop里会assertInLoopThread），线程退出后已无法满足。
// 组件只在进程退出时才析构，所以EventLoop和挂在上面的UDP Channel有意不释放，交给进程退出回收
static void leakAtExit(std::shared_ptr<afl::net::EventLoop>& loop)
{
    new std::shared_ptr<afl::net::EventLoop>(std::move(loop));
}

void CCINDEX_COMPONENT::shutdownWorkers()
{
    // 1. 置退出标志（之后MQTT回调直接丢弃新消息），让所有EventLoop退出loop()并join，之后不再有定时器/任务在执行
    //    注意：正在执行中的回调（如连不上的HTTP请求）要等它返回，EventLoop才会退出
    m_StopWorkers = true;
    std::shared_ptr<afl::net::EventLoop>* loops[] = {&m_Eventloop, &m_EventloopTrafficMetrics, &m_EventloopTrafficMetricsPubFirst,
                                                     &m_EventloopTrafficMetricsPubSecond, &m_EventloopTc, &m_EventloopTcPubFirst,
                                                     &m_EventloopTcPubSecond, &m_EventloopStatic, &m_EventloopCloud};
    for (auto* loop : loops)
    {
        if (*loop)
        {
            (*loop)->quit();
        }
    }
    auto joinTask = [](std::unique_ptr<std::thread>& task)
    {
        if (task && task->joinable())
        {
            task->join();
        }
    };
    for (auto* task : {&m_Task, &m_TaskTrafficMetrics, &m_TaskTrafficMetricsPubFirst, &m_TaskTrafficMetricsPubSecond,
                       &m_TaskTc, &m_TaskTcPubFirst, &m_TaskTcPubSecond, &m_TaskStatic, &m_TaskStaticCloud, &m_TaskUseCase})
    {
        joinTask(*task);
    }
    // 2. 唤醒并join上传队列线程（它们由m_EventloopTrafficMetrics上的定时器创建，此时已不会再创建）
    {
        std::lock_guard<std::mutex> lock(cv_m1);
    }
    cv1.notify_all();
    {
        std::lock_guard<std::mutex> lock(cv_m2);
    }
    cv2.notify_all();
    joinTask(m_TaskQueue1);
    joinTask(m_TaskQueue2);
    // 3. 断开MQTT，之后不再有回调访问EventLoop
    mqttDeinit();
    mqttDeinitCloud();
    // 4. EventLoop和UDP Channel不在这里析构，见leakAtExit的说明
    for (auto* loop : loops)
    {
        leakAtExit(*loop);
    }
    (void)m_UdpChnl.release();
}

void CCINDEX_COMPONENT::notifyEventLoopReady()
{
    std::lock_guard<std::mutex> lock(m_EventloopReadyMutex);
    ++m_EventloopReadyCount;
    m_EventloopReadyCv.notify_all();
}

void CCINDEX_COMPONENT::waitEventLoopsReady(int count)
{
    std::unique_lock<std::mutex> lock(m_EventloopReadyMutex);
    if (!m_EventloopReadyCv.wait_for(lock, std::chrono::seconds(10),
                                     [this, count]() { return m_EventloopReadyCount >= count; }))
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]wait eventloop ready timeout! [ready]" << m_EventloopReadyCount << " [expect]" << count;
        return;
    }
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[notice]all eventloop ready! [count]" << m_EventloopReadyCount;
    }
}

bool CCINDEX_COMPONENT::initEventLoop()
{
    m_Eventloop = std::make_shared<afl::net::EventLoop>();
    if(m_Eventloop)
    {
        m_timer_mec_radar_cloud_fd = m_Eventloop->addTimer(std::bind(&CCINDEX_COMPONENT::periodMonitorRadarCloudStatus, this), 5, true);
        m_TimerMonitorMqttConnected =  m_Eventloop->addTimer(std::bind(&CCINDEX_COMPONENT::monitorMqttConnectStatus, this),
                                                                getConfiger().configerPeriod.periodMonitorMqttConnect, true);
        m_TimerMecLinkStatusCcindexTm =  m_Eventloop->addTimer(std::bind(&CCINDEX_COMPONENT::monitorMecLinkStatusCcindexTm, this),
                                                                getConfiger().configerPeriod.periodMonitorMecLinkStatusCcindexTm, true);

        m_TimerMecLinkStatusCcindexSt =  m_Eventloop->addTimer(std::bind(&CCINDEX_COMPONENT::monitorMecLinkStatusCcindexSt, this),
                                                        getConfiger().configerPeriod.periodMonitorMecLinkStatusCcindexSt, true);

    }
    notifyEventLoopReady();
    m_Eventloop->loop();
    return true;
}
bool CCINDEX_COMPONENT::initEventLoopTrafficMetrics()
{
    m_EventloopTrafficMetrics = std::make_shared<afl::net::EventLoop>();
    notifyEventLoopReady();
    m_EventloopTrafficMetrics->loop();
    return true;
}

bool CCINDEX_COMPONENT::initEventLoopTrafficMetricsPubFirst()
{
    m_EventloopTrafficMetricsPubFirst = std::make_shared<afl::net::EventLoop>();
    notifyEventLoopReady();
    m_EventloopTrafficMetricsPubFirst->loop();
    return true;
}
bool CCINDEX_COMPONENT::initEventLoopTrafficMetricsPubSecond()
{
    m_EventloopTrafficMetricsPubSecond = std::make_shared<afl::net::EventLoop>();
    notifyEventLoopReady();
    m_EventloopTrafficMetricsPubSecond->loop();
    return true;
}

bool CCINDEX_COMPONENT::initEventLoopTc()
{
    m_EventloopTc = std::make_shared<afl::net::EventLoop>();
    if(getConfiger().configerEnableCCIndex.Enable_Tc)
    {
        if(!udpInit())
        {
            RADAR_TC_ERROR_PRINT << "udp init failure!";
        }
        //m_EventloopTc->addTimer(std::bind(&CCINDEX_COMPONENT::send_3s_level_radardata, this), 3, true);
        if(getConfiger().enableMec)//结合感知算法时，智路OS控制上报5m数据上报；不结合感知算法时透传5m数据
        {
            m_temptimer = m_EventloopTc->addTimer(std::bind(&CCINDEX_COMPONENT::temp_timer, this), 0.5, true);
        }
        else//结合感知算法时，接收到感知算法5s中数据时上报；不结合感知算法时，智路OS控制5s数据上报
        {
            m_EventloopTc->addTimer(std::bind(&CCINDEX_COMPONENT::send_5s_level_radardata, this), 1, true);
        }

    }
    notifyEventLoopReady();
    m_EventloopTc->loop();
    return true;
}
void CCINDEX_COMPONENT::periodMonitorRadarCloudStatus() 
{
    if(getConfiger().enableDebugPrint)
    {
        RADAR_TC_DEBUG_PRINT << "periodMonitorRadarCloudStatus!";
    }
    if (output_monitor_) {
        output_monitor_->Clear();
    }
    auto* ccindex_response =  output_monitor_->mutable_ccindex_response();

    ccindex_response->set_tag(airos::monitor::MonitorMsgTag::MONITOR_TAG_MONITOR_CCINDEX);
    ccindex_response->set_seqnum(afl::util::Srand::srandStr(32));
    ccindex_response->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    ccindex_response->set_device_esn(m_MqttClientConfig.rscuEsn);
    auto* ccindex_monitor_data = ccindex_response->mutable_data();
    ccindex_monitor_data->set_con_flag(m_MqttConnected);
    auto* ccindex_tc_status = ccindex_monitor_data->mutable_ccindex_tc_status();
    ccindex_tc_status->set_tc_pub_flag(m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_pub_flag);
    ccindex_tc_status->set_tc_sub_flag(m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_sub_flag);
    // repeated 字段，可以添加多个
    auto* ccindex_tm_status1 = ccindex_monitor_data->add_ccindex_tm_status();  // 新增一个
    ccindex_tm_status1->set_device_esn(m_CcindexStatusContainer.m_CcindexTmStatus1.deviceEsn);
    ccindex_tm_status1->set_device_sn(m_CcindexStatusContainer.m_CcindexTmStatus1.deviceSn);
    ccindex_tm_status1->set_tm_pub_flag(m_CcindexStatusContainer.m_CcindexTmStatus1.tm_pub_flag);
    ccindex_tm_status1->set_tm_sub_flag(m_CcindexStatusContainer.m_CcindexTmStatus1.tm_sub_flag);

    auto* ccindex_tm_status2 = ccindex_monitor_data->add_ccindex_tm_status();  // 新增一个
    ccindex_tm_status2->set_device_esn(m_CcindexStatusContainer.m_CcindexTmStatus2.deviceEsn);
    ccindex_tm_status2->set_device_sn(m_CcindexStatusContainer.m_CcindexTmStatus2.deviceSn);
    ccindex_tm_status2->set_tm_pub_flag(m_CcindexStatusContainer.m_CcindexTmStatus2.tm_pub_flag);
    ccindex_tm_status2->set_tm_sub_flag(m_CcindexStatusContainer.m_CcindexTmStatus2.tm_sub_flag);


    auto* ccindex_tm_status3 = ccindex_monitor_data->add_ccindex_tm_status();  // 新增一个
    ccindex_tm_status3->set_device_esn(m_CcindexStatusContainer.m_CcindexTmStatus3.deviceEsn);
    ccindex_tm_status3->set_device_sn(m_CcindexStatusContainer.m_CcindexTmStatus3.deviceSn);
    ccindex_tm_status3->set_tm_pub_flag(m_CcindexStatusContainer.m_CcindexTmStatus3.tm_pub_flag);
    ccindex_tm_status3->set_tm_sub_flag(m_CcindexStatusContainer.m_CcindexTmStatus3.tm_sub_flag);


    auto* ccindex_tm_status4 = ccindex_monitor_data->add_ccindex_tm_status();  // 新增一个
    ccindex_tm_status4->set_device_esn(m_CcindexStatusContainer.m_CcindexTmStatus4.deviceEsn);
    ccindex_tm_status4->set_device_sn(m_CcindexStatusContainer.m_CcindexTmStatus4.deviceSn);
    ccindex_tm_status4->set_tm_pub_flag(m_CcindexStatusContainer.m_CcindexTmStatus4.tm_pub_flag);
    ccindex_tm_status4->set_tm_sub_flag(m_CcindexStatusContainer.m_CcindexTmStatus4.tm_sub_flag);


    auto* ccindex_static_status = ccindex_monitor_data->mutable_ccindex_static_status();  // 新增一个
    ccindex_static_status->set_st_post_flag(m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag);
    ccindex_static_status->set_st_get_flag(m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag);
    Send("/v2x/monitor", output_monitor_);
}

//设置从整5分钟开始5min级定时器
void CCINDEX_COMPONENT::temp_timer()
{
    uint64_t nowtime = afl::util::TimeStamp::now(true).seconds();
    if (nowtime%300 == 0)
    {
        send_5m_level_radardata();
        m_EventloopTc->addTimer(std::bind(&CCINDEX_COMPONENT::send_5m_level_radardata, this), 60*5, true);
        m_EventloopTc->cancelTimer(m_temptimer);
        m_temptimer = -1;
    }
}
bool CCINDEX_COMPONENT::initEventLoopTcPubFirst()
{
    m_EventloopTcPubFirst = std::make_shared<afl::net::EventLoop>();
    m_EventloopTcPubFirst->loop();
    return true;
}

bool CCINDEX_COMPONENT::initEventLoopTcPubSecond()
{
    m_EventloopTcPubSecond = std::make_shared<afl::net::EventLoop>();
    m_EventloopTcPubSecond->loop();
    return true;
}

bool CCINDEX_COMPONENT::initEventLoopStatic()
{
    m_EventloopStatic = std::make_shared<afl::net::EventLoop>();
    notifyEventLoopReady();
    m_EventloopStatic->loop();
    return true;
}
bool CCINDEX_COMPONENT::initEventLoopCloud()
{
    m_EventloopCloud = std::make_shared<afl::net::EventLoop>();
    notifyEventLoopReady();
    m_EventloopCloud->loop();
    return true;
}

bool CCINDEX_COMPONENT::initUseCase()
{
    if(!getConfiger().enableMec)
    {
        return false;
    }

    // 使用 ReaderConfig 设置缓存队列
    apollo::cyber::ReaderConfig config;
    config.channel_name = "/v2x/usecase";
    config.pending_queue_size = 10;  // 设置缓存队列大小

    auto reader = node_->CreateReader<airos::usecase::EventOutputResult>(
        config,
        std::bind(&CCINDEX_COMPONENT::processProcessEventOutputResult, this, std::placeholders::_1));

    return true;
}


void  CCINDEX_COMPONENT::processProcessEventOutputResult(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData)
{
    if(!mecDeviceData->has_mec_trafficflow())
    {
        return;
    }
    //mec车道级流量按雷达车道级和流向级存储
    if(mecDeviceData->mec_trafficflow().cycle() == 300)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TC_DEBUG_PRINT << "Received mec 5min 2101:" << mecDeviceData->DebugString();
        }
        for(int i = 0; i < mecDeviceData->mec_trafficflow().trafficflow_list_size() ; i++)
        {
            auto tempmecflow = mecDeviceData->mec_trafficflow().trafficflow_list().Get(i);
            //车道级存储
            if(m_meclaneId_Map.find(tempmecflow.lane_no()) == m_meclaneId_Map.end())
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TC_DEBUG_PRINT << "could not found mec laneNO: " << tempmecflow.lane_no();
                }
                continue;
            }
            mecCreditControlInfo_5m_level mcci_5m_lane;
            mcci_5m_lane.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
            // mcci_5m_lane.mec_traffic_number.store(temp_number);
            // mcci_5m_lane.mec_traffic_flow.store(temp_flow);
            uint64_t temp_number = tempmecflow.volume3() + tempmecflow.volume4() + tempmecflow.volume5();
            double temp_flow = tempmecflow.pcu();
            mcci_5m_lane.mec_traffic_number.store(temp_number);
            mcci_5m_lane.mec_traffic_flow.store(temp_flow);

            std::string tempid = m_meclaneId_Map[tempmecflow.lane_no()];
            m_RecvMecInfo_5m_levelMap[tempid] = mcci_5m_lane;

            //流向级存储，多个车道属性时，直行流量占比为0.5，剩余0.5其他平分
            if(m_mecflowId_map.find(tempmecflow.lane_no()) == m_mecflowId_map.end())
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TC_DEBUG_PRINT << "could not found mec laneNO: " << tempmecflow.lane_no();
                }
                continue;
            }
            if(m_mecflowId_map[tempmecflow.lane_no()].typenum == 1)//只包含一个车道属性
            {
                std::string tempflow_id = "";
                if(m_mecflowId_map[tempmecflow.lane_no()].no_turnid != "")
                {
                    tempflow_id = m_mecflowId_map[tempmecflow.lane_no()].no_turnid;
                }
                else if(m_mecflowId_map[tempmecflow.lane_no()].leftid != "")
                {
                    tempflow_id = m_mecflowId_map[tempmecflow.lane_no()].leftid;
                }
                else if(m_mecflowId_map[tempmecflow.lane_no()].rightid != "")
                {
                    tempflow_id = m_mecflowId_map[tempmecflow.lane_no()].rightid;
                }
                else if(m_mecflowId_map[tempmecflow.lane_no()].u_turnid != "")
                {
                    tempflow_id = m_mecflowId_map[tempmecflow.lane_no()].u_turnid;
                }
                else
                {
                    continue;
                }
                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(temp_number);
                    mcci_5m_flow.mec_traffic_flow.store(temp_flow);
                    m_RecvMecInfo_5m_levelMap[tempflow_id] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id].mec_traffic_number.store(temp_number + m_RecvMecInfo_5m_levelMap[tempflow_id].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id].mec_traffic_flow.store(temp_flow + m_RecvMecInfo_5m_levelMap[tempflow_id].mec_traffic_flow.load());
                }

            }
            else if(m_mecflowId_map[tempmecflow.lane_no()].typenum == 2)//两个车道属性
            {
                std::vector<std::string> tempflow_id_vector;
                if(m_mecflowId_map[tempmecflow.lane_no()].no_turnid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].no_turnid);
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].leftid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].leftid);
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].rightid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].rightid);
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].u_turnid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].u_turnid);
                }

                int number_1 = temp_number/2;
                int number_2 = temp_number - number_1;
                int flow_1 = temp_flow/2;
                int flow_2 = temp_flow - flow_1;                    
                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id_vector[0]) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_1);
                    mcci_5m_flow.mec_traffic_flow.store(flow_1);
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_number.store(number_1 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_flow.store(flow_1 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_flow.load());
                }

                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id_vector[1]) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_2);
                    mcci_5m_flow.mec_traffic_flow.store(flow_2);
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_number.store(number_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_flow.store(flow_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_flow.load());
                }                    
            }
            else if(m_mecflowId_map[tempmecflow.lane_no()].typenum == 3)//三个车道属性
            {
                int number_1;
                int number_2;
                int number_3;
                int flow_1;
                int flow_2;
                int flow_3;
                std::vector<std::string> tempflow_id_vector;
                if(m_mecflowId_map[tempmecflow.lane_no()].no_turnid != "")
                {
                    number_1 =  temp_number/2;
                    number_2 = (temp_number - number_1)/2;
                    number_3 = temp_number - number_1 - number_2;
                    flow_1 = temp_flow/2;
                    flow_2 = (temp_flow - flow_1)/2;
                    flow_3 = temp_flow - flow_1 - flow_2;
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].no_turnid);
                }
                else
                {
                    number_1 =  temp_number/3;
                    number_2 = (temp_number - number_1)/2;
                    number_3 = temp_number - number_1 - number_2;
                    flow_1 = temp_flow/3;
                    flow_2 = (temp_flow - flow_1)/2;
                    flow_3 = temp_flow - flow_1 - flow_2;
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].leftid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].leftid);
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].rightid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].rightid);
                }
                if(m_mecflowId_map[tempmecflow.lane_no()].u_turnid != "")
                {
                    tempflow_id_vector.push_back(m_mecflowId_map[tempmecflow.lane_no()].u_turnid);
                }

                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id_vector[0]) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_1);
                    mcci_5m_flow.mec_traffic_flow.store(flow_1);
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_number.store(number_1 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_flow.store(flow_1 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[0]].mec_traffic_flow.load());
                }
                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id_vector[1]) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_2);
                    mcci_5m_flow.mec_traffic_flow.store(flow_2);
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_number.store(number_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_flow.store(flow_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[1]].mec_traffic_flow.load());
                } 
                if(m_RecvMecInfo_5m_levelMap.find(tempflow_id_vector[2]) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_2);
                    mcci_5m_flow.mec_traffic_flow.store(flow_2);
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[2]] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[2]].mec_traffic_number.store(number_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[2]].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[tempflow_id_vector[2]].mec_traffic_flow.store(flow_2 + m_RecvMecInfo_5m_levelMap[tempflow_id_vector[2]].mec_traffic_flow.load());
                }                    

            }
            else if(m_mecflowId_map[tempmecflow.lane_no()].typenum == 4)//四个车道属性
            {
                int number_1 = temp_number/2;
                int number_2 = (temp_number - number_1)/3;
                int number_3 = (temp_number - number_1 - number_2)/2;
                int number_4 = temp_number - number_1 - number_2 - number_3;
                int flow_1 = temp_flow/2;
                int flow_2 = (temp_flow - flow_1)/3;
                int flow_3 = (temp_flow - flow_1 - flow_2)/2;
                int flow_4 = temp_flow - flow_1 - flow_2 - flow_3;         
                //直行
                if(m_RecvMecInfo_5m_levelMap.find(m_mecflowId_map[tempmecflow.lane_no()].no_turnid) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_1);
                    mcci_5m_flow.mec_traffic_flow.store(flow_1);
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].no_turnid] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].no_turnid].mec_traffic_number.store(number_1 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].no_turnid].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].no_turnid].mec_traffic_flow.store(flow_1 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].no_turnid].mec_traffic_flow.load());
                }
                //左转
                if(m_RecvMecInfo_5m_levelMap.find(m_mecflowId_map[tempmecflow.lane_no()].leftid) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_2);
                    mcci_5m_flow.mec_traffic_flow.store(flow_2);
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].leftid] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].leftid].mec_traffic_number.store(number_2 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].leftid].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].leftid].mec_traffic_flow.store(flow_2 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].leftid].mec_traffic_flow.load());
                }
                //右转
                if(m_RecvMecInfo_5m_levelMap.find(m_mecflowId_map[tempmecflow.lane_no()].rightid) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_3);
                    mcci_5m_flow.mec_traffic_flow.store(flow_3);
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].rightid] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].rightid].mec_traffic_number.store(number_3 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].rightid].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].rightid].mec_traffic_flow.store(flow_3 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].rightid].mec_traffic_flow.load());
                }
                //掉头
                if(m_RecvMecInfo_5m_levelMap.find(m_mecflowId_map[tempmecflow.lane_no()].u_turnid) == m_RecvMecInfo_5m_levelMap.end())
                {
                    mecCreditControlInfo_5m_level mcci_5m_flow;
                    mcci_5m_flow.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
                    mcci_5m_flow.mec_traffic_number.store(number_4);
                    mcci_5m_flow.mec_traffic_flow.store(flow_4);
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].u_turnid] = mcci_5m_flow;
                }
                else//已存在，则累加
                {
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].u_turnid].mec_traffic_number.store(number_4 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].u_turnid].mec_traffic_number.load());
                    m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].u_turnid].mec_traffic_flow.store(flow_4 + m_RecvMecInfo_5m_levelMap[m_mecflowId_map[tempmecflow.lane_no()].u_turnid].mec_traffic_flow.load());
                }
            }
        }
    }
    else if(mecDeviceData->mec_trafficflow().cycle() == 5)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TC_DEBUG_PRINT << "Received mec 5s 2101:" << mecDeviceData->DebugString();
        }
        for(int i = 0; i < mecDeviceData->mec_trafficflow().trafficflow_list_size() ; i++)
        {
            auto tempmecflow = mecDeviceData->mec_trafficflow().trafficflow_list().Get(i);
            //车道级存储
            if(m_meclaneId_Map.find(tempmecflow.lane_no()) == m_meclaneId_Map.end())
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TC_DEBUG_PRINT << "could not found mec laneNO: " << tempmecflow.lane_no();
                }
                continue;
            }
            mecCreditControlInfo_5s_level mcci_5s_lane;
            mcci_5s_lane.updatetime.store(mecDeviceData->mec_trafficflow().time_stamp());
            mcci_5s_lane.mec_car_count.store(tempmecflow.classqueuenaturalnumber());
            mcci_5s_lane.mec_car_trans_count.store(tempmecflow.classqueueequivalentnumber());
            std::string tempid = m_meclaneId_Map[tempmecflow.lane_no()];
            m_RecvMecInfo_5s_levelMap[tempid] = mcci_5s_lane;
        }
        this->m_EventloopTc->runInLoop(std::bind(&CCINDEX_COMPONENT::send_5s_level_radardatabymec, this));         
    }
    else
    {

    }
    
}

bool CCINDEX_COMPONENT::Proc(const std::shared_ptr<const  os::v2x::device::CloudData>& recv_data)
{
//    CCINDEX_DEBUG_PRINT << "Received channel data!";
//    Send("/v2x/service/mec/data", mecDeviceData);
    return true;
}

bool CCINDEX_COMPONENT::getWorkParamFromFile()
{

    WorkParam::getWorkParamFromFile(getConfiger().configerWorkParam.workParamFilePath, omWorkParamConfiger);
    // if(getConfiger().enableDebugPrint)
    // {
    //     CCINDEX_DEBUG_PRINT << "[work-param]" << omWorkParamConfiger.to_string().c_str();
    // }

    bool hasErased = false;
    getConfiger().configerMqttCloud.mqttBrokerUrl = omWorkParamConfiger.mecDeviceWorkParam.ccindexCloudUrl;
    getConfiger().configerMqttCloud.mqttUserName = omWorkParamConfiger.mecDeviceWorkParam.ccindexMqttUsername;
    getConfiger().configerMqttCloud.mqttPassword = omWorkParamConfiger.mecDeviceWorkParam.ccindexMqttPasswd;
    getConfiger().configerProjectPath.tlsCAFileName = omWorkParamConfiger.mecDeviceWorkParam.ccindexCaCertName;
    getConfiger().configerProjectPath.tlsClientKeyFileName = omWorkParamConfiger.mecDeviceWorkParam.ccindexClientCertName;
    getConfiger().configerProjectPath.tlsClientPrivateKeyFileName = omWorkParamConfiger.mecDeviceWorkParam.ccindexClientPrivateKeyFile;
    getConfiger().configerProjectPath.tlsClientPrivateKeyPassword = omWorkParamConfiger.mecDeviceWorkParam.ccindexClientPrivateKeyPwd;

//    getConfiger().httpConfiger.httpCloudServerIp = omWorkParamConfiger.radarStaticHttpConfiger.httpCloudServerIp;
//    getConfiger().httpConfiger.httpCloudServerPort = omWorkParamConfiger.radarStaticHttpConfiger.httpCloudServerPort;
//    getConfiger().httpConfiger.mecMqttClientQueryConfigDataPeriod = omWorkParamConfiger.radarStaticHttpConfiger.mecMqttClientQueryConfigDataPeriod;
//    getConfiger().httpConfiger.httpHostServerIp = omWorkParamConfiger.radarStaticHttpConfiger.httpHostServerIp;
//    getConfiger().httpConfiger.httpHostServerPort = omWorkParamConfiger.radarStaticHttpConfiger.httpHostServerPort;
//    getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod = omWorkParamConfiger.radarStaticHttpConfiger.httpHostClientPostConfigUpdateDataPeriod;

    for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
    {
        if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty())
        {
            if(!hasErased)
            {
                getConfiger().configerTopicRadarID.radarIDs.clear();
                hasErased = true;
            }
            std::string topicPostfix = "/" +  v.vendor + "/" + v.category + "/" + v.radarCrossId + "/" + v.deviceEsn;
            getConfiger().configerTopicRadarID.radarIDs.push_back(topicPostfix);
            m_VendorStats[v.vendor]++;

            getConfiger().configerTopicRadarID.cross_id = v.radarCrossId; //丢弃不用
            getConfiger().configerTopicRadarID.vendor = v.vendor;  //厂商
            getConfiger().configerTopicRadarID.category = v.category; //设备
        }
    }
    getConfiger().rscuEsn = omWorkParamConfiger.mecDeviceWorkParam.deviceEsn;
    for(int i = 0; i < (int)omWorkParamConfiger.ccInexWorkParam.laneList.size(); i++)
    {
        std::string id = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].laneId;

        int phaseid = omWorkParamConfiger.ccInexWorkParam.laneList[i].phaseId;
        if(m_LaneOrFlowPhaseIdMap.find(phaseid) != m_LaneOrFlowPhaseIdMap.end())
        {
            m_LaneOrFlowPhaseIdMap[phaseid].push_back(id);
        }
        else
        {
            std::vector<std::string> tempvec;
            tempvec.push_back(id);
            m_LaneOrFlowPhaseIdMap[phaseid] = tempvec;
        }
        siglephaseInfo si;
        std::pair<int, siglephaseInfo> temppair(0, si);
        m_phaseInfoMap[phaseid] = temppair;
        //mec-雷达车道级路网映射
        m_meclaneId_Map[omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId] = id;
        //mec-雷达流向级路网映射
        int templen = omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId.size();
        FlowTypesId tempflowtypesid;
        if(templen < 4)
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TC_DEBUG_PRINT << "meclaneId error";
            }
            continue;
        }
        bool left = false;
        bool no_turn = false;
        bool right = false;
        bool u_turn = false; 
        int turn_type = 10;//对应adu::stb_map::LaneTurnType
        if(omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId[templen - 1] == '1')//左转
        {
            tempflowtypesid.leftid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId
                + "2";
            tempflowtypesid.typenum++;  
            left = true; 
        }
        if(omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId[templen - 2] == '1')//直行
        {
            tempflowtypesid.no_turnid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId
                + "1";
            tempflowtypesid.typenum++;
            no_turn = true;
        }
        if(omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId[templen - 3] == '1')//右转
        {
            tempflowtypesid.rightid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId
                + "3";
            tempflowtypesid.typenum++; 
            right = true;
        }
        if(omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId[templen - 4] == '1')//掉头
        {
            tempflowtypesid.u_turnid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId
                + "4";
            tempflowtypesid.typenum++; 
            u_turn = true;
        }
        m_mecflowId_map[omWorkParamConfiger.ccInexWorkParam.laneList[i].meclaneId] = tempflowtypesid;
        
        if(left == false && no_turn == true && right == false && u_turn == false)
        {
            turn_type = 1;
        }
        else if(left == true && no_turn == false && right == false && u_turn == false)
        {
            turn_type = 2;
        }
        else if(left == false && no_turn == false && right == true && u_turn == false)
        {
            turn_type = 3;            
        }
        else if(left == false && no_turn == false && right == false && u_turn == true)
        {
            turn_type = 4;
        }
        else if(left == true && no_turn == true && right == false && u_turn == false)
        {
            turn_type = 5;
        }
        else if(left == true && no_turn == false && right == false && u_turn == true)
        {
            turn_type = 6;
        }
        else if(left == false && no_turn == true && right == false && u_turn == true)
        {
            turn_type = 7;
        }
        else if(left == false && no_turn == true && right == true && u_turn == false)
        {
            turn_type = 8;
        }
         else if(left == true && no_turn == true && right == false && u_turn == true)
        {
            turn_type = 9;
        }
        else if(left == true && no_turn == false && right == true && u_turn == false)
        {
            turn_type = 11;
        }
        else if(left == true && no_turn == true && right == true && u_turn == false)
        {
            turn_type = 12;
        }
        else if(left == false && no_turn == false && right == true && u_turn == true)
        {
            turn_type = 13;
        }
        else if(left == true && no_turn == false && right == true && u_turn == true)
        {
            turn_type = 14;
        }
        else if(left == false && no_turn == true && right == true && u_turn == true)
        {
            turn_type = 15;
        }
        else if(left == true && no_turn == true && right == true && u_turn == true)
        {
            turn_type = 16;
        }
        else 
        {
        }
         
        radarCreditControlInfo_spatcycle_level rcciscl;
        rcciscl.islanelevel = true;
        rcciscl.crossid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid;
        rcciscl.branchid = omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId;
        rcciscl.laneid = omWorkParamConfiger.ccInexWorkParam.laneList[i].laneId;
        rcciscl.updatetime = 0;
        rcciscl.laneturntype = turn_type;
        m_RecvCreditControlInfo_spatcycle_levelMap[id] = rcciscl;

        radarCreditControlInfo_5m_level rcci5l;
        rcci5l.islanelevel = true;
        rcci5l.crossid = omWorkParamConfiger.ccInexWorkParam.laneList[i].crossid;
        rcci5l.branchid = omWorkParamConfiger.ccInexWorkParam.laneList[i].branchId;
        rcci5l.laneid = omWorkParamConfiger.ccInexWorkParam.laneList[i].laneId;
        rcci5l.updatetime = 0;
        rcci5l.laneturntype = turn_type;
        m_RecvCreditControlInfo_5m_levelMap[id] = rcci5l;
    }

    for(int i = 0; i < (int)omWorkParamConfiger.ccInexWorkParam.flowList.size(); i++)
    {
        std::string id = omWorkParamConfiger.ccInexWorkParam.flowList[i].crossid
                + omWorkParamConfiger.ccInexWorkParam.flowList[i].branchId
                + std::to_string(omWorkParamConfiger.ccInexWorkParam.flowList[i].turnType);
        int phaseid = omWorkParamConfiger.ccInexWorkParam.flowList[i].phaseId;
        if(m_LaneOrFlowPhaseIdMap.find(phaseid) != m_LaneOrFlowPhaseIdMap.end())
        {
            m_LaneOrFlowPhaseIdMap[phaseid].push_back(id);
        }
        else
        {
            std::vector<std::string> tempvec;
            tempvec.push_back(id);
            m_LaneOrFlowPhaseIdMap[phaseid] = tempvec;
        }
        siglephaseInfo si;
        std::pair<int, siglephaseInfo> temppair(0, si);
        m_phaseInfoMap[phaseid] = temppair;

        radarCreditControlInfo_5m_level rcci5l;
        rcci5l.islanelevel = false;
        rcci5l.crossid = omWorkParamConfiger.ccInexWorkParam.flowList[i].crossid;
        rcci5l.branchid = omWorkParamConfiger.ccInexWorkParam.flowList[i].branchId;
        rcci5l.flowtotype.push_back(omWorkParamConfiger.ccInexWorkParam.flowList[i].turnType);
        rcci5l.updatetime = 0;
        m_RecvCreditControlInfo_5m_levelMap[id] = rcci5l;
    }

    if(getConfiger().enableDebugPrint)
	{
        for (auto &iter: m_LaneOrFlowPhaseIdMap)
		{
            for (int i = 0; i < (int) iter.second.size(); i++)
			{
                RADAR_TC_DEBUG_PRINT << "phase id: " << iter.first << " lane or flow id: " << iter.second[i];
            }
        }

        for(auto &iter: m_meclaneId_Map)
        {
            RADAR_TC_DEBUG_PRINT << "mec laneid: " << iter.first << " radar lane id: " << iter.second;
        }

        for(auto &iter: m_mecflowId_map)
        {
            RADAR_TC_DEBUG_PRINT << "mec laneid: " << iter.first
            << " flow type count: " << iter.second.typenum
            << " radar flow left id: " << iter.second.leftid
            << " radar flow no_turn id: " << iter.second.no_turnid
            << " radar flow right id: " << iter.second.rightid
            << " radar flow u_turn id: " << iter.second.u_turnid;
        }


    }
    saveConfiger();

    return true;
}

void CCINDEX_COMPONENT::doBaiscWork()
{
    m_outBranchList = getConfiger().outBranchName;
    //初始化mqtt配置
    getWorkParamFromFile();
    getMqttConfigerMsg();
    if (!mqttInit())
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter Init failure!";
        return ;
    }

    getMqttConfigerMsgCloud();
    if (!mqttInitCloud())
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud Init failure!";
        return ;
    }
    if(getConfiger().configerEnableCCIndex.Enable_TrafficMetrics)
    {
//        m_TimerTrajectoriesMonitor = m_EventloopTrafficMetrics->addTimer(std::bind(&CCINDEX_COMPONENT::timerPublishTrajectoriesData, this),
//                                                           getConfiger().configerPeriod.trajectoriesPublishPeriod, true);
        m_TimerTrajectoriesMonitorStart = m_EventloopTrafficMetrics->addTimer(std::bind(&CCINDEX_COMPONENT::timerTrajectoriesMonitorStart, this),
                                                                getConfiger().configerPeriod.trajectoriesPublishPeriod, true);

    }

    if(getConfiger().configerEnableCCIndex.Enable_Static)
    {
        m_TimerConfigQueryMonitorPushOnce = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfigDataOnce, this),
                                                                  1, true);

        m_TimerConfigQueryMonitor = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfigData, this),
                                                          getConfiger().httpConfiger.mecMqttClientQueryConfigDataPeriod, true);
        if(getConfiger().httpConfiger.enablePushSeparate)
        {
            //定时更新
            m_TimerConfigUpdateMonitorTsmtc = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerUpdateConfigDataTsmtc, this),
                                                                          getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);

            m_TimerConfigUpdateMonitorDesaysv = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerUpdateConfigDataDesaysv, this),
                                                                            getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);
            //定时查询
            m_TimerConfigQuery2HttpServerTsmtc = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfig2HttpServerTsmtc, this),
                                                                        getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);
            m_TimerConfigQuery2HttpServerDesaysv = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfig2HttpServerDesaysv, this),
                                                                        getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);
        }
        else
        {
            m_TimerConfigUpdateMonitor = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerUpdateConfigData, this),
                                                                     getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);
            m_TimerConfigQuery2HttpServer = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfig2HttpServer, this),
                                                                        getConfiger().httpConfiger.httpHostClientPostConfigUpdateDataPeriod, true);
        }

    }

//    m_TaskHttpServer.reset(new std::thread([&](){ processCloudQuery(); }));//处理平台http Get
}

 bool CCINDEX_COMPONENT::monitorMqttConnectStatus()
{
    // bool mqttConnected = false;
    // bool mqttConnectedCloud = false;
    // if (MQTTAsync_isConnected(m_MqttClient))
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         CCINDEX_MQTT_DEBUG_PRINT << "[success]Connection is still active";
    //     }
    //     mqttConnected = true;
    // }
    // else
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         CCINDEX_MQTT_DEBUG_PRINT  << "[error]Connection is lost";
    //     }
    //     mqttConnected = false;
    // }
    // if (MQTTAsync_isConnected(m_MqttClientCloud))
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         CCINDEX_MQTT_DEBUG_PRINT << "[cloud][success]Connection is still active";
    //     }
    //     mqttConnectedCloud = true;
    // }
    // else
    // {
    //     if(getConfiger().enableDebugPrint)
    //     {
    //         CCINDEX_MQTT_DEBUG_PRINT  << "[cloud][error]Connection is lost";
    //     }
    //     mqttConnectedCloud = false;
    // }
    //
    // if(!mqttConnectedCloud)
    // {
    //     if (m_MqttClientCloud)
    //     {
    //         if(getConfiger().enableDebugPrint)
    //         {
    //             CCINDEX_MQTT_DEBUG_PRINT << "[cloud][error]Connection is lost, reconnect cloud!";
    //         }
    //         mqttReconnectCloud();
    //     }
    //     else
    //     {
    //         mqttInitCloud();
    //     }
    // }
    // if(!mqttConnected)
    // {
    //     if(m_MqttClient)
    //     {
    //         if(getConfiger().enableDebugPrint)
    //         {
    //             CCINDEX_MQTT_DEBUG_PRINT  << "[cloud][error]Connection is lost, reconnect cloud!";
    //         }
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


bool CCINDEX_COMPONENT::timerTrajectoriesMonitorStart()
{
    if (!m_MqttConnected)
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt clinet has not connected!";
        return false;
    }

    m_TaskQueue1.reset(new std::thread([&](){ uploadQueue1(); }));
    m_TaskQueue2.reset(new std::thread([&](){ uploadQueue2(); }));
    if(m_TimerTrajectoriesMonitorStart > 0)
    {
        m_EventloopTrafficMetrics->cancelTimer(m_TimerTrajectoriesMonitorStart);
        m_TimerTrajectoriesMonitorStart = -1;
    }

}
bool CCINDEX_COMPONENT::uploadQueue1()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(cv_m1);
        cv1.wait(lock, [&] { return queue1_ready || m_StopWorkers; }); // 等待通知
        if (m_StopWorkers)
        {
            return true;
        }
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>[queue1]revice notify queue1!";
        }
        std::map<uint64_t, std::unordered_map<std::string, std::vector<json>>> mapFromQueueTrajectoriesData;
        // 重置标志
        queue1_ready = false;
        // 消费队列 1 中的数据
        while (!queue1.empty())
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>!queue1.empty()!";
            }
            std::unordered_map<std::string, json> mapFromQueue;
            if (!queue1.pop(mapFromQueue))
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>!queue1.pop(mapFromQueue)!";
                continue;
            }
            for(auto& pairFromQueue : mapFromQueue)
            {
                std::string topic = pairFromQueue.first;
                json subScribeJson = pairFromQueue.second;
                try
                {
                    if(subScribeJson.find("head") != subScribeJson.end())
                    {
                        auto& headJson = subScribeJson["head"];
                        if(headJson.find("device_time") != headJson.end())
                        {
                            uint64_t deviceTime = 0;
                            try{
                                deviceTime = headJson["device_time"];
                            }catch (json::exception& e)
                            {
                                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]" << e.what();
                                return false;
                            }
                            uint64_t deviceTimeSecond = deviceTime/1000;
                            mapFromQueueTrajectoriesData[deviceTimeSecond][topic].push_back(subScribeJson);
                        }
                    }
                }
                catch(exception& e)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" << e.what();
                }
            }
        }
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>[notice]start merged Json";
        }
        json mergedJson;
        uint64_t nowUtcSecond = afl::util::TimeStamp::now(true).seconds();
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]merged Queue Trajectories Data!" << "[nowUtcSecond]"
                                             << nowUtcSecond << "[mapFromQueueTrajectoriesData-size]" << mapFromQueueTrajectoriesData.size();
        }
        for(const auto& outerMapPair: mapFromQueueTrajectoriesData)
        {
            uint64_t second = outerMapPair.first;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[nowUtcSecond]" << nowUtcSecond << "[cahce-second]" << second;
            }
            if(nowUtcSecond - second > getConfiger().configerPeriod.trajectoriesPublishTimeDiff)
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" <<" nowUtcSecond - second > 1";
                }
                continue;
            }
            else
            {
                const auto& innerMap = outerMapPair.second;
                for (const auto& innerMapPair : innerMap)
                {
                    std::string topicPT = innerMapPair.first;
                    auto& jsonArray = innerMapPair.second;
                    json mergedJson = jsonArray.at(0); // 用第一个 JSON 进行初始化
                    for(size_t i = 1; i < jsonArray.size(); ++i)
                    {
                        if (jsonArray[i].find("lanes") != jsonArray[i].end())
                        {
                            const json &currentData = jsonArray[i]["lanes"];
                            // 将 currentData 合并到 mergedJson 的 "data" 中
                            for (const auto &item: currentData)
                            {
                                mergedJson["lanes"].push_back(item);
                            }
                        }
                    }
                    if(mergedJson.find("head")!= mergedJson.end())
                    {
                        auto& headJson = mergedJson["head"];
                        headJson["platform_time"] = nowUtcSecond;
                        int objCount = 0, lanesCount = 0;
                        if(mergedJson.find("lanes")!= mergedJson.end())
                        {
                            auto& lanesJson = mergedJson["lanes"];
                            lanesCount = lanesJson.size();
                            for(size_t i = 0; i < lanesJson.size(); ++i)
                            {
                                auto& laneJson = lanesJson.at(i);
                                if (laneJson.find("trajectories") != laneJson.end())
                                {
                                    for(size_t j = 0; j < laneJson.size(); ++j)
                                    {
                                        objCount++;
                                    }
                                }
                            }
                        }
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-info][merged][index]" << "[topic]"
                                                              << topicPT << "[json-count]" << lanesCount
                                                              << "[obj-count]" << objCount;
                        }
                    }
                    json subScribeJson = mergedJson;
                    std::string hint = "trafficMetrics";
                    if(getConfiger().enableDebugPrint)
                    {
                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽queue1▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽▽";
                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "【trajectories-info】【upload】" << "【topic】" << topicPT
                                                         << " 【json】" << subScribeJson.dump().c_str();
                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "△△△△△△△△△△△△△△△△△△△△△△△queue1△△△△△△△△△△△△△△△△△△△△△△△";
                    }
                    if (!mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint))
                    {
                        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                    }
                }

            }
        }

    }
}

bool CCINDEX_COMPONENT::uploadQueue2()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(cv_m2);
        cv2.wait(lock, [&] { return queue2_ready || m_StopWorkers; });
        if (m_StopWorkers)
        {
            return true;
        }
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_ERROR_PRINT << "------>[queue2]revice notify queue2!";
        }
        std::map<uint64_t, std::unordered_map<std::string, std::vector<json>>> mapFromQueueTrajectoriesData;
        queue2_ready = false;
        while (!queue2.empty())
        {
            if(getConfiger().enableDebugPrint) {
                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "------>[queue2]revice notify queue2!";
            }
            std::unordered_map<std::string, json> mapFromQueue;
            if (!queue2.pop(mapFromQueue))
            {
                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "------>!queue2.pop(mapFromQueue)!";
                continue;
            }
            for(auto& pairFromQueue : mapFromQueue)
            {
                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "------>auto& pairFromQueue!";
                std::string topic = pairFromQueue.first;
                json subScribeJson = pairFromQueue.second;
                try
                {
                    if(subScribeJson.find("head") != subScribeJson.end())
                    {
                        auto& headJson = subScribeJson["head"];
                        if(headJson.find("device_time") != headJson.end())
                        {
                            uint64_t deviceTime = 0;
                            try{
                                deviceTime = headJson["device_time"];
                            }catch (json::exception& e)
                            {
                                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]" << e.what();
                            }
                            uint64_t deviceTimeSecond = deviceTime/1000;
                            mapFromQueueTrajectoriesData[deviceTimeSecond][topic].push_back(subScribeJson);
                        }
                    }
                }
                catch(exception& e)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" << e.what();
                }
            }
        }
        if(getConfiger().enableDebugPrint) {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>[notice]start merged Json";
        }
        json mergedJson;
        uint64_t nowUtcSecond = afl::util::TimeStamp::now(true).seconds();

        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]merged mapFromQueueTrajectoriesData!" << "[nowUtcSecond]"
                                              << nowUtcSecond << "[mapFromQueueTrajectoriesData-size]" << mapFromQueueTrajectoriesData.size();
        }
        for(const auto& outerMapPair: mapFromQueueTrajectoriesData)
        {
            uint64_t second = outerMapPair.first;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[nowUtcSecond]" << nowUtcSecond << "[cahce-second]" << second;
            }
            if(nowUtcSecond - second > getConfiger().configerPeriod.trajectoriesPublishTimeDiff)
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" <<" nowUtcSecond - second > 1";
                }
                continue;
            }
            else
            {
                const auto& innerMap = outerMapPair.second;
                for (const auto& innerMapPair : innerMap)
                {
                    std::string topicPT = innerMapPair.first;
                    auto& jsonArray = innerMapPair.second;
                    json mergedJson = jsonArray.at(0); // 用第一个 JSON 进行初始化
                    for(size_t i = 1; i < jsonArray.size(); ++i)
                    {
                        if (jsonArray[i].find("lanes") != jsonArray[i].end())
                        {
                            const json &currentData = jsonArray[i]["lanes"];
                            // 将 currentData 合并到 mergedJson 的 "data" 中
                            for (const auto &item: currentData)
                            {
                                mergedJson["lanes"].push_back(item);
                            }
                        }
                    }
                    json subScribeJson = mergedJson;
                    std::string hint = "trafficMetrics";
                    if (!mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint))
                    {
                        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                    }
                }

            }
        }

    }
}
std::string CCINDEX_COMPONENT::getRadarEsn(std::string topicMid)
{
    // 查找最后一个 '/' 的位置
    size_t pos = topicMid.find_last_of('/');
    if (pos != std::string::npos) {
        // 返回最后一个 '/' 后面的所有字符
        return topicMid.substr(pos + 1);
    }
    // 如果没有 '/'，返回原字符串
    return topicMid;
}

bool CCINDEX_COMPONENT::getMqttConfigerMsg()
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

    //订阅
    memset(&m_MqttClientConfig.configerMqtt.mqttSubscribeTopics, 0, sizeof(m_MqttClientConfig.configerMqtt.mqttSubscribeTopics));
    m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum = 0;
    m_MqttClientConfig.rscuEsn = getConfiger().rscuEsn;
    m_MqttClientConfig.configerEnableCCIndex = getConfiger().configerEnableCCIndex;

    m_MqttClientConfig.configerTopicCCIndex.Topic_Profix =  getConfiger().configerTopicCCIndex.Topic_Profix;
    m_MqttClientConfig.configerTopicRadarID.cross_id   = getConfiger().configerTopicRadarID.cross_id;
    m_MqttClientConfig.configerTopicRadarID.vendor   = getConfiger().configerTopicRadarID.vendor;
    m_MqttClientConfig.configerTopicRadarID.category   = getConfiger().configerTopicRadarID.category;
    m_MqttClientConfig.configerTopicCCIndex.Topic_Tc = getConfiger().configerTopicCCIndex.Topic_Tc + m_MqttClientConfig.rscuEsn;

    if(getConfiger().configerEnableCCIndex.Enable_Tc)
    {
        m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc.c_str()) ;
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_sub_flag = true;
    }


    for(uint32_t i = 0; i < getConfiger().configerTopicRadarID.radarIDs.size(); i++)
    {
        std::string deviceEsn = getRadarEsn(getConfiger().configerTopicRadarID.radarIDs[i]);
        if(i == 0 ){m_TS_Radar1.deviceEsn = deviceEsn;}
        else if(i == 1){m_TS_Radar2.deviceEsn = deviceEsn;}
        else if(i == 2){m_TS_Radar3.deviceEsn = deviceEsn;}
        else if(i == 3){m_TS_Radar4.deviceEsn = deviceEsn;}
        else if(i == 4){m_TS_Radar5.deviceEsn = deviceEsn;}
        else if(i == 5){m_TS_Radar6.deviceEsn = deviceEsn;}

        m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix =  m_MqttClientConfig.rscuEsn  + getConfiger().configerTopicRadarID.radarIDs[i];
        MqttTopicConfigerCCIndex            configerTopicCCIndex;
        //信控
        configerTopicCCIndex.Topic_Tc               =  getConfiger().configerTopicCCIndex.Topic_Tc           + m_MqttClientConfig.rscuEsn;
        //动态
        configerTopicCCIndex.Topic_Trajectories     = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Trajectories + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_VehiclePass      = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_VehiclePass  + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_QueueUp          = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_QueueUp      + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_AreaState        = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_AreaState    + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Overflow         = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Overflow     + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Outlane          = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Outlane      + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Statistics       = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Statistics   + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Evaluations      = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Evaluations  + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Nonmotor         = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Nonmotor     + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_DeviceStatus     = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_DeviceStatus     + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Pulse            = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Pulse            + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_SingleStatistics = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_SingleStatistics + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;

        //静态
        configerTopicCCIndex.Topic_Static_Query = m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Profix +
                                                  m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Query +   m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Static_Query_Ack = m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Profix +
                                                      m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Query_Ack + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;

        configerTopicCCIndex.Topic_Static_Update = m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Profix +
                                                   m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Update +   m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
        configerTopicCCIndex.Topic_Static_Update_Ack = m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Profix +
                                                       m_MqttClientConfig.configerTopicCCIndex.Topic_Static_Update_Ack +    m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;

        m_MqttClientConfig.topicUnMapCCIndex[i] = configerTopicCCIndex;
        if(getConfiger().configerEnableCCIndex.Enable_TrafficMetrics)
        {
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Trajectories.c_str());
            if (i == 0)
            {

                for (auto &v: omWorkParamConfiger.sensorDeviceWorkParamList)
                {
                    if (v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty() && v.deviceEsn == deviceEsn)
                    {
                        m_CcindexStatusContainer.m_CcindexTmStatus1.deviceEsn = deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus1.deviceSn = v.deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus1.tm_sub_flag = true;
                        break;
                    }
                }
            }
            else if (i == 1)
            {

                for (auto &v: omWorkParamConfiger.sensorDeviceWorkParamList)
                {
                    if (v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty() && v.deviceEsn == deviceEsn)
                    {
                        m_CcindexStatusContainer.m_CcindexTmStatus2.deviceEsn = deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus2.deviceSn = v.deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus2.tm_sub_flag = true;
                        break;
                    }
                }
            }
            else if(i == 2)
            {

                for(auto& v : omWorkParamConfiger.sensorDeviceWorkParamList)
                {
                    if(v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty() && v.deviceEsn  == deviceEsn)
                    {
                        m_CcindexStatusContainer.m_CcindexTmStatus3.deviceEsn = deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus3.deviceSn = v.deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus3.tm_sub_flag = true;
                        break;
                    }
                }
            }
            else  if(i == 3)
            {

                for (auto &v: omWorkParamConfiger.sensorDeviceWorkParamList)
                {
                    if (v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty() && v.deviceEsn == deviceEsn)
                    {
                        m_CcindexStatusContainer.m_CcindexTmStatus4.deviceEsn = deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus4.deviceSn = v.deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus4.tm_sub_flag = true;
                        break;
                    }
                }
            }
            else  if(i == 4)
            {

                for (auto &v: omWorkParamConfiger.sensorDeviceWorkParamList) {
                    if (v.deviceType == WorkParamDeviceTypeRadar && !v.deviceEsn.empty() && v.deviceEsn == deviceEsn)
                    {
                        m_CcindexStatusContainer.m_CcindexTmStatus5.deviceEsn = deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus5.deviceSn = v.deviceEsn;
                        m_CcindexStatusContainer.m_CcindexTmStatus5.tm_sub_flag = true;
                        break;
                    }
                }
            }
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_VehiclePass.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_QueueUp.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_AreaState.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Overflow.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Outlane.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Statistics.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Evaluations.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Nonmotor.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_DeviceStatus.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Pulse.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_SingleStatistics.c_str()) ;
        }

        if(getConfiger().configerEnableCCIndex.Enable_Static)
        {
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Static_Query_Ack.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Static_Update.c_str()) ;
        }
    }
    for (uint32_t i = 0; i < m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum; i++)
    {
        m_MqttClientConfig.configerMqtt.mqttSubscribeQoss[i] =  m_MqttClientConfig.configerMqtt.mqttSubScribeQos;
    }
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[configer-Inter]" << m_MqttClientConfig.configerMqtt.to_string().c_str();
    }
    return true;
}

bool CCINDEX_COMPONENT::getMqttConfigerMsgCloud()
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
    m_MqttClientConfig.configerProjectPath.tlsClientPrivateKeyPassword = getConfiger().configerProjectPath.tlsClientPrivateKeyPassword;
    m_MqttClientConfig.rscuEsn = getConfiger().rscuEsn;
    m_MqttClientConfig.configerEnableCCIndex = getConfiger().configerEnableCCIndex;

    //订阅信控指标
    memset(&m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics, 0, sizeof(m_MqttClientConfig.configerMqttCloud.mqttSubscribeTopics));
    m_MqttClientConfig.configerTopicCCIndex.Topic_Tc = getConfiger().configerTopicCCIndex.Topic_Tc + m_MqttClientConfig.rscuEsn;
    if(getConfiger().configerPrintSubMsgFromCloud.enableDebugPrintSubMsgTc)
    {
        m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc.c_str()) ;
    }
    if(getConfiger().configerPrintSubMsgFromCloud.enableDebugPrintSubMsgTm)
    {
        m_MqttClientConfig.configerTopicRadarID.cross_id   = getConfiger().configerTopicRadarID.cross_id;
        m_MqttClientConfig.configerTopicRadarID.vendor   = getConfiger().configerTopicRadarID.vendor;
        m_MqttClientConfig.configerTopicRadarID.category   = getConfiger().configerTopicRadarID.category;
        m_MqttClientConfig.configerTopicCCIndex.Topic_Tc = getConfiger().configerTopicCCIndex.Topic_Tc + m_MqttClientConfig.rscuEsn;
        //订阅动态数据
        for(uint32_t i = 0; i < getConfiger().configerTopicRadarID.radarIDs.size(); i++)
        {
            m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix =  m_MqttClientConfig.rscuEsn  + getConfiger().configerTopicRadarID.radarIDs[i];
            MqttTopicConfigerCCIndex            configerTopicCCIndex;
            configerTopicCCIndex.Topic_Tc               =  getConfiger().configerTopicCCIndex.Topic_Tc           + m_MqttClientConfig.rscuEsn;
            configerTopicCCIndex.Topic_Trajectories     = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Trajectories + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_VehiclePass      = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_VehiclePass  + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_QueueUp          = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_QueueUp      + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_AreaState        = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_AreaState    + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Overflow         = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Overflow     + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Outlane          = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Outlane      + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Statistics       = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Statistics   + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Evaluations      = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Evaluations  + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Nonmotor         = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Nonmotor     + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_DeviceStatus     = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_DeviceStatus + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_Pulse            = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_Pulse        + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;
            configerTopicCCIndex.Topic_SingleStatistics = m_MqttClientConfig.configerTopicCCIndex.Topic_Profix + getConfiger().configerTopicCCIndex.Topic_SingleStatistics + m_MqttClientConfig.configerTopicCCIndex.Topic_Postfix;

    //        m_MqttClientConfig.topicUnMapCCIndex[i] = configerTopicCCIndex;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Trajectories.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_VehiclePass.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_QueueUp.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_AreaState.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Overflow.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Outlane.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Statistics.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Evaluations.c_str() ) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Nonmotor.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_DeviceStatus.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_Pulse.c_str()) ;
            m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum++] = (char *) strdup(configerTopicCCIndex.Topic_SingleStatistics.c_str()) ;
        }

    }
    for (uint32_t i = 0; i < m_MqttClientConfig.configerMqttCloud.mqttRealSubscribeTopicNum; i++)
    {
        m_MqttClientConfig.configerMqttCloud.mqttSubscribeQoss[i] =  m_MqttClientConfig.configerMqttCloud.mqttSubScribeQos;
    }
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_DEBUG_PRINT << "[configer-cloud]" << m_MqttClientConfig.configerMqttCloud.to_string().c_str();
    }
    return true;
}

bool CCINDEX_COMPONENT::timerPublishTrajectoriesData()
{
    if (!m_MqttConnected)
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt clinet has not connected!";
        return false;
    }
    std::string hint = "trafficMetrics";
    std::map<uint64_t, std::unordered_map<std::string, std::vector<json>>> trajectoriesDataJsonRecivByUtcTemp;
    std::lock_guard<std::mutex> lock(m_TrajectoriesMutex);
    {
        trajectoriesDataJsonRecivByUtcTemp = m_TrajectoriesDataJsonRecivByUtc;
        m_TrajectoriesDataJsonRecivByUtc.clear();
    }
    if (!trajectoriesDataJsonRecivByUtcTemp.empty())
    {
        if(getConfiger().enableDebugPrint)
        {
            int mapIndex = 0;
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "============================================>";

            for(const auto& outerMapPair: trajectoriesDataJsonRecivByUtcTemp)
            {
                uint64_t second = outerMapPair.first;
                mapIndex +=1;
                const auto& innerMap = outerMapPair.second;
                for (const auto& innerMapPair : innerMap)
                {
                    size_t lastSlashPos = innerMapPair.first.find_last_of('/');
                    std::string deviceId;
                    if (lastSlashPos != std::string::npos)
                    {
                        deviceId = innerMapPair.first.substr(lastSlashPos + 1); // 提取子字符串
                    }
                    int objCount = 0;
                    for(const auto& v:innerMapPair.second)
                    {
                        objCount += v.size();
                    }
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-info][cache][index]" << mapIndex << "[second]" << second << "[topic]" << deviceId << "[json-count]" << innerMapPair.second.size() << "[obj-count]" << objCount;
                }
            }
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "============================================>";

        }
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]m_ConfigUpdateDataJson is empty!";
        }
        return false;
    }

    json mergedJson;
    uint64_t nowUtcSecond = afl::util::TimeStamp::now(true).seconds();
    if(getConfiger().enableDebugPrint)
    {
        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[nowUtcSecond]" << nowUtcSecond;
    }
    for(const auto& outerMapPair: trajectoriesDataJsonRecivByUtcTemp)
    {
        uint64_t second = outerMapPair.first;
        if(nowUtcSecond - second >= 1)
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" <<" nowUtcSecond - second > 2";
            }
            continue;
        }
        else
        {
            const auto& innerMap = outerMapPair.second;
            for (const auto& innerMapPair : innerMap)
            {
                std::string topicPT = innerMapPair.first;
                auto& jsonArray = innerMapPair.second;
                json mergedJson = jsonArray.at(0); // 用第一个 JSON 进行初始化
                for(size_t i = 1; i < jsonArray.size(); ++i)
                {
                    if (jsonArray[i].find("lanes") != jsonArray[i].end())
                    {
                        const json &currentData = jsonArray[i]["lanes"];
                        // 将 currentData 合并到 mergedJson 的 "data" 中
                        for (const auto &item: currentData)
                        {
                            mergedJson["lanes"].push_back(item);
                        }
                    }
                }
                if(mergedJson.find("head")!= mergedJson.end())
                {
                    auto& headJson = mergedJson["head"];
                    headJson["platform_time"] = nowUtcSecond;
                    int objCount = 0, lanesCount = 0;
                    if(mergedJson.find("lanes")!= mergedJson.end())
                    {
                        auto& lanesJson = mergedJson["lanes"];
                        lanesCount = lanesJson.size();
                        for(size_t i = 0; i < lanesJson.size(); ++i)
                        {
                            auto& laneJson = lanesJson.at(i);
                            if (laneJson.find("trajectories") != laneJson.end())
                            {
                                for(size_t j = 0; j < laneJson.size(); ++j)
                                {
                                    objCount++;
                                }
                            }
                        }
                    }
                    if(getConfiger().enableDebugPrint)
                    {
                        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-info][merged][index]" << "[topic]" << topicPT << "[json-count]" << lanesCount << "[obj-count]" << objCount;
                    }
                }
                json subScribeJson = mergedJson;

                static int message_count_trajectories_channel = 1; // 用于计数的静态变量
                static int MAX_MESSAGE_COUNT_TRAJECTORIES_CHANNEL = 1000; // 设置您的最大限制

                // 根据消息计数判断是偶数还是奇数
                if (message_count_trajectories_channel % 2 == 0) // 偶数
                {
                    m_EventloopTrafficMetricsPubFirst->runInLoop([this, topicPT, subScribeJson, hint]()
                       {
                              if (!this->mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint))
                              {
                                  RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                              }
                       });
                }
                else // 奇数
                {
                    m_EventloopTrafficMetricsPubSecond->runInLoop([this, topicPT, subScribeJson, hint]()
                   {
                       if (!this->mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint))
                       {
                           RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                       }
                   });
                }
                if (message_count_trajectories_channel > MAX_MESSAGE_COUNT_TRAJECTORIES_CHANNEL)
                {
                    message_count_trajectories_channel = 0; // 重置计数器
                }
                else
                {
                    message_count_trajectories_channel++;
                }
            }

        }
    }
}

bool CCINDEX_COMPONENT::determineOperationType(const std::string& path)
{
    bool isTrajectoriesTopic = false;
    if (path.find("trafficMetrics/trajectories/") == 0)
    {
        isTrajectoriesTopic = true;
    }
    else
    {
        isTrajectoriesTopic = false;
    }

    return isTrajectoriesTopic;
}
bool CCINDEX_COMPONENT::processTrajectoriesData(std::string topic, const json& subScribeJson)
{
    // 新增：发布 CloudData 到 CyberRT（独立功能，放在前面）
    publishCloudDataMqtt(topic, subScribeJson);

    auto& headJson = subScribeJson["head"];
    try{
        std::string deviceId = topic;
        static int s = 0;
        if(subScribeJson.find("head") != subScribeJson.end())
        {
            if(headJson.find("platform_time") != headJson.end())
            {
                uint64_t platformTime = 0;
                try{
                    platformTime = headJson["platform_time"];
                }catch (json::exception& e)
                {
                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]" << e.what();
                }
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-info][sub1][json]" << subScribeJson.dump();
                }
                uint64_t platformTimeSecond = platformTime/1000;
                std::lock_guard<std::mutex> lock(m_TrajectoriesMutex);
                {
                    m_TrajectoriesDataJsonRecivByUtc[platformTimeSecond][topic].push_back(subScribeJson);
                }
            }
        }
        else {
            RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]no find head";
            return false;
        }
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------------------------------->";
            int mapIndex = 0;
            for(const auto& outerMapPair: m_TrajectoriesDataJsonRecivByUtc)
            {
                uint64_t second = outerMapPair.first;
                mapIndex +=1;
                const auto& innerMap = outerMapPair.second;
                for (const auto& innerMapPair : innerMap)
                {
                    size_t lastSlashPos = innerMapPair.first.find_last_of('/');
                    std::string deviceId;
                    if (lastSlashPos != std::string::npos)
                    {
                        deviceId = innerMapPair.first.substr(lastSlashPos + 1); // 提取子字符串
                    }
                    int objCount = 0;
                    for(const auto& v:innerMapPair.second)
                    {
                        objCount += v.size();
                    }
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-info][sub][index]" << mapIndex << "[second]" << second << "[topic]" << deviceId << "[json-count]" << innerMapPair.second.size() << "[obj-count]" << objCount;
                }
            }
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------------------------------->";
        }
    }catch(exception& e)
    {
        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[error]" << e.what();
    }


    return true;
}
/////////////////////////////////////////////////////
void CCINDEX_COMPONENT::mqttDispatchSubscribeMessageTrajectories(const std::string &topic, const afl::base::json & subScribeJson)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt clinet has not connected!";
    //     return;
    // }

    if (topic.empty())
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "topic empty!";
        return;
    }
    std::string topicProfix = topic.substr(0, topic.find("/"));
    std::string deviceId = topic.substr(topic.find_last_of("/"));
    std::string hint;
    std::string topicPT;

    if(subScribeJson.find("head") != subScribeJson.end())
    {

        auto& headJson = subScribeJson["head"];
        if(headJson.find("device_time") != headJson.end())
        {
            static uint64_t lastUtcTIme = -1;
            uint64_t deviceTime = 0;
            try{
                uint64_t timeTemp = headJson["device_time"];
                deviceTime = timeTemp/ 1000;
            }catch(json::exception& e)
            {
                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]" << e.what();
                return ;
            }

            static bool cv1NotifyOnce = false;
            static bool cv2NotifyOnce = false;
            if(deviceTime % 2 ==0)
            {
                if(!cv2NotifyOnce)
                {
                    m_EventloopTrafficMetrics->runInLoop([&]()
                    {
                        std::lock_guard<std::mutex> lock(cv_m2);
                        queue2_ready = true;
                        cv2.notify_all();
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "cv2.notify_one();" ;
                        }
                    });
                    cv1NotifyOnce = false;
                    cv2NotifyOnce = true;
                }
                std::unordered_map<std::string, json> mapData;
                mapData[topic] = subScribeJson;
                queue1.push(mapData);
                if(getConfiger().enableDebugPrint)
                {
                    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>【trajectories-info】 push into queue1 " << "[nowtime]" << nowTime << "【deviceTime】" << deviceTime << " 【deviceId】" << deviceId  << " 【subScribeJson】" << subScribeJson.dump();
                }
            }
            else
            {
                if(!cv1NotifyOnce)
                {
                    m_EventloopTrafficMetrics->runInLoop([&]()
                   {
                       std::lock_guard<std::mutex> lock(cv_m1);
                       queue1_ready = true;
                       cv1.notify_all();
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "cv1.notify_one();" ;
                        }
                   });
                    cv1NotifyOnce = true;
                    cv2NotifyOnce = false;
                }

                std::unordered_map<std::string, json> mapData;
                mapData[topic] = subScribeJson;
                queue2.push(mapData);
                if(getConfiger().enableDebugPrint)
                {
                    uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "------>【trajectories-info】 push into queue2 " << "[nowTime]" <<  nowTime << "【deviceTime】" << deviceTime << " 【deviceId】" << deviceId  << " 【subScribeJson】" << subScribeJson.dump();
                }
            }
        }
    }

    return;
}

void CCINDEX_COMPONENT::mqttDispatchSubscribeMessage(const std::string &topic, const afl::base::json & subScribeJson)
{
    // if (!m_MqttConnectedCloud)
    // {
    //   RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt clinet has not connected!";
    //     return;
    // }

    if (topic.empty())
    {
      RADAR_TRAFFIC_METRICS_ERROR_PRINT << "topic empty!";
        return;
    }

    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(getConfiger().enableDebugPrint)
    {
        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[topicProfix]" << topicProfix << "[topic]" << topic;
    }
    std::string hint;
    std::string topicPT;
    //动态
//    if(topicProfix == "trafficMetrics")
//    {
//        if(determineOperationType(topic))
//        {
//            if(processTrajectoriesData(topic, subScribeJson))
//            {
//                if(getConfiger().enableDebugPrint)
//                {
//                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[success]process Trajectories-Data success!";
//                }
//                return ;
//            }
//            else
//            {
//                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]process Trajectories-Data failure!";
//                return ;
//            }
//        }
//        topicPT = topic;
//        hint = "trafficMetrics";
//    }
//    else
//    {
//        if(getConfiger().enableDebugPrint)
//        {
//            CCINDEX_INTER_DEBUG_PRINT << "[noitce]no need this msg!";
//        }
//        return ;
//    }

    topicPT = topic;
    if(!topicPT.empty())
    {
        static int message_count_trafficMetrics_channel = 1; // 用于计数的静态变量
        static int MAX_MESSAGE_COUNT_TAFFIC_METRICS_CHANNEL = 1000; // 设置您的最大限制
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "!topic empty!";
        }
        // 根据消息计数判断是偶数还是奇数
        if (message_count_trafficMetrics_channel % 2 == 0) // 偶数
        {

            m_EventloopTrafficMetricsPubFirst->runInLoop([this, topicPT, subScribeJson, hint]()
           {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "m_EventloopTrafficMetricsPubFirst!";
                }
                this->mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint);
           });
        }
        else // 奇数
        {
            m_EventloopTrafficMetricsPubSecond->runInLoop([this, topicPT, subScribeJson, hint]()
             {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "m_EventloopTrafficMetricsPubSecond!";
                }
                this->mqttPushMsg2BrokerCloud(topicPT, subScribeJson, hint);
             });
        }
        if (message_count_trafficMetrics_channel > MAX_MESSAGE_COUNT_TAFFIC_METRICS_CHANNEL)
        {
            message_count_trafficMetrics_channel = 0; // 重置计数器
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[message_count_trafficMetrics_channel]" << message_count_trafficMetrics_channel;
            }
        }
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[message_count_trafficMetrics_channel]" << message_count_trafficMetrics_channel;
            }
            message_count_trafficMetrics_channel++;
        }
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[topicPT][" << topicPT << "]";
          RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[topicPT][" << topicPT << "]";
        }
    }

    return;
}
int printMqttTlsError(const char *str, size_t len, void *u)
{
    CCINDEX_INTER_ERROR_PRINT << "[ssl error]" << str;
    return 0;
}
bool CCINDEX_COMPONENT::mqttInitConnOpts()
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

    return true;
}

void CCINDEX_COMPONENT::mqttConnectedCallback(void* context, char* cause)
{
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnected = true;
    CCINDEX_INTER_DEBUG_PRINT << "[success]mqtt-inter (re)connected!";

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

void CCINDEX_COMPONENT::mqttConnectedCallbackCloud(void* context, char* cause)
{
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = true;
    CCINDEX_CLOUD_DEBUG_PRINT << "[success]mqtt-cloud (re)connected!";

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
bool CCINDEX_COMPONENT::mqttInit()
{
    m_MqttClient = nullptr;
    m_MqttConnected = false;

    mqttInitConnOpts();
    std::string clientIdTemp = m_MqttClientConfig.rscuEsn + m_MqttClientConfig.configerMqtt.mqttClientId + ":" +
                               std::to_string(afl::util::TimeStamp::now(true).microSeconds());
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[mqtt-inter]"
                            << "[mqttBroker-Url]" << m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str()
                            << " [client-id]" << clientIdTemp.c_str()
                            << " [username]" << m_MqttClientConfig.configerMqtt.mqttUserName.c_str()
                            << " [passwd]" << m_MqttClientConfig.configerMqtt.mqttPassword.c_str();
    }
    if (MQTTASYNC_SUCCESS != MQTTAsync_create(&m_MqttClient, m_MqttClientConfig.configerMqtt.mqttBrokerUrl.c_str(),
                                              clientIdTemp.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL))
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter create connectiont failure!";
        return false;
    }

    MQTTAsync_setCallbacks(m_MqttClient, this, mqttConnlost, mqttSubscribeMsgArrvd, NULL);
    MQTTAsync_setConnected(m_MqttClient, this, mqttConnectedCallback);
    if (!mqttConnect())
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter Connect Failure!";
        return false;
    }

    return true;
}

void CCINDEX_COMPONENT::mqttDeinit()
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
        //
        m_CcindexStatusContainer.m_CcindexMonitor.con_flag = false;
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_sub_flag = false;
        // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;


    }
}

void CCINDEX_COMPONENT::mqttOnConnect(void *context, MQTTAsync_successData *response)
{
    CCINDEX_INTER_DEBUG_PRINT << "[success]mqtt-inter connect success!";
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnected = true;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    opts.onSuccess = mqttOnSubscribe;
    opts.onFailure = mqttOnSubscribeFailure;
    opts.context = thiz;

    if(thiz->getConfiger().enableDebugPrint)
    {
        std::stringstream ss;
        for (uint32_t i = 0; i < thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum; i++)
        {
            ss << std::left  << "SubscribeQos[" <<  thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeQoss[i] << "] " << "SubscribeTopic[" << i << "]: " << thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeTopics[i] << std::endl;
        }
        CCINDEX_INTER_DEBUG_PRINT << ss.str();
    }
    if(thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum > 0)
    {
        if (MQTTAsync_subscribeMany(thiz->m_MqttClient, thiz->m_MqttClientConfig.configerMqtt.mqttRealSubscribeTopicNum,
                                    thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeTopics,
                                    thiz->m_MqttClientConfig.configerMqtt.mqttSubscribeQoss,
                                    &opts) != MQTTASYNC_SUCCESS)
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter subscribe error";
        }
    }
    if(thiz->m_TimerMqttReconnectInter > 0)
    {
        thiz->m_EventloopCloud->cancelTimer(thiz->m_TimerMqttReconnectInter);
        thiz->m_TimerMqttReconnectInter = -1;
    }
}

void CCINDEX_COMPONENT::mqttOnConnectFailure(void *context, MQTTAsync_failureData *response)
{
    if (response)
    {
        if (response->message)
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter connect error [error-code]" << response->code << "[error-msg]" << response->message;
        } else
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter connect error [error-code]" << response->code << " [error-msg]" << response->message;
        }
    } else
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter connect error!";
    }

    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->mqttReconnect();
}


bool CCINDEX_COMPONENT::mqttConnect()
{
    int rc;
    if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(m_MqttClient, &m_MqttConnOpts)))
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter connect error!";
        m_MqttConnected = false;
        return false;
    }
    return true;
}

bool CCINDEX_COMPONENT::mqttReconnect()
{
    CCINDEX_INTER_ERROR_PRINT << "[notice]mqtt-inter reconnect!";
    // if(m_TimerMqttReconnectInter < 0)
    // {
    //     m_TimerMqttReconnectInter = m_Eventloop->addTimer(std::bind(&CCINDEX_COMPONENT::mqttConnect, this), m_MqttClientConfig.configerMqtt.mqttReconnectInterval, true);
    // }
    // if(getConfiger().enableDebugPrint)
    // {
    //     CCINDEX_MQTT_DEBUG_PRINT << "[notice]mqtt-inter reconnect!";
    // }
    return true;
}

void CCINDEX_COMPONENT::mqttConnlost(void *context, char *cause)
{
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnected = false;
    CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter connect lost! [cause]" << cause;
    CCINDEX_MQTT_ERROR_PRINT << "[error]mqtt-inter connect lost! [cause]" << cause;
    thiz->mqttReconnect();
}

void CCINDEX_COMPONENT::mqttOnDisconnect(void *context, MQTTAsync_successData *response)
{
    CCINDEX_INTER_ERROR_PRINT << "[notice]mqtt-inter disconnect!";
    CCINDEX_MQTT_ERROR_PRINT << "[notice]mqtt-inter disconnect!";
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnected = false;
}

void CCINDEX_COMPONENT::mqttOnSubscribe(void *context, MQTTAsync_successData *response)
{
    CCINDEX_INTER_SUCCESS_PRINT << "[success]" << "mqtt-inter subscribe success!";
}

void CCINDEX_COMPONENT::mqttOnSubscribeFailure(void *context, MQTTAsync_failureData *response)
{
    CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter subscribe failure!";
}

bool CCINDEX_COMPONENT::mqttPublishMsg(const std::string &topic, const std::string &msg)
{
    // if (!m_MqttConnected)
    // {
    //     CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter  has not connected!";
    //     return false;
    // }

    if (msg.size() <= 0 || topic.empty())
    {
        CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter len < 0 or topic empty!";
        return false;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    opts.context = m_MqttClient;
    pubmsg.payload = (void *) msg.c_str();
    pubmsg.payloadlen = msg.length();
    pubmsg.qos = m_MqttClientConfig.configerMqtt.mqttSendQos;
    pubmsg.retained = getConfiger().configerMqtt.mqttRetained;

    if ((rc = MQTTAsync_sendMessage(m_MqttClient, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        if (rc == MQTTASYNC_DISCONNECTED)
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter Async Connect failure，will connect!";
            if (m_MqttConnected)
            {
                m_MqttConnected = false;
                // mqttReconnect();
            }
        } else
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter push msg error，[error-code]" << rc;
        }

        return false;
    }

    return true;
}
//根据topic获取
bool CCINDEX_COMPONENT::getMecCheckCcindexTmTag(std::string& topic)
{

 // 1. 提取第二部分的内容 (例如 "vehiclePass", "trajectories" 等)
    std::string type_str;
    size_t first_slash = topic.find('/');
    size_t second_slash = topic.find('/', first_slash + 1);

    // 确保找到了两个 '/'，保证格式正确
    if (first_slash != std::string::npos && second_slash != std::string::npos) {
        // substr(起始位置, 长度)
        type_str = topic.substr(first_slash + 1, second_slash - first_slash - 1);
    } else {
        // 格式不对，直接返回或记录错误
        return false;
    }
    // ================= 新增：提取最后一个 '/' 之后的内容 =================
    std::string device_id_str;
    size_t last_slash = topic.rfind('/');
    if (last_slash != std::string::npos) {
        // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
        device_id_str = topic.substr(last_slash + 1);
    }
    // =====================================================================
    // 2. 定义默认 Tag (可选)
    airos::monitor_mec::MonitorMecTag target_tag;
    bool is_match = true;

    // 3. 字符串匹配并赋值对应的 Enum
    if (type_str == "trajectories") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_TRAJECTORIES; // 3028
    }
    else if (type_str == "vehiclePass") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_VEHICLEPASS; // 3029
    }
    else if (type_str == "queueUp") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_QUEUEUP; // 3030
    }
    else if (type_str == "areaState") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_AREASTATE; // 3031
    }
    else if (type_str == "overflow") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_OVERFLOW; // 3032
    }
    else if (type_str == "outlane") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_OUTLANE; // 3033
    }
    else if (type_str == "statistics") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_STATISTICS; // 3034
    }
    else if (type_str == "evaluations") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_EVALUATIONS; // 3035
    }
    else if (type_str == "nonmotor") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_NONMOTOR; // 3036
    }
    else if (type_str == "deviceStatus") {
        target_tag = airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TM_DEVICESTATUS; // 3037
    }
    else {
        // 未知的类型
        is_match = false;
    }

    // 4. 如果匹配成功，调用函数
    if (is_match) {
        pushMonitorMecDataStatusTm(target_tag, topic, device_id_str);
    }
    return true;
}

int CCINDEX_COMPONENT::mqttSubscribeMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int ret = 1;
    auto thiz = (CCINDEX_COMPONENT *) context;
    if (thiz->m_StopWorkers)
    {
        // 进程正在退出（EventLoop已停）：新消息直接丢弃，不再处理/转发
        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topicName);
        return 1;
    }
    if (message->payloadlen)
    {
        if(topicLen <= 0)
        {
            MQTTAsync_freeMessage(&message);
            MQTTAsync_free(topicName);
            return ret;
        }
        std::string topic(topicName, topicLen);
        std::string topicProfix = topic.substr(0, topic.find("/"));
        uint64_t nowTime = afl::util::TimeStamp::now(true).millSeconds();
        if(thiz->getConfiger().enableDebugPrint)
        {
            CCINDEX_MQTT_DEBUG_PRINT << "[topic]" << topic;
        }
        if(topicProfix == "trafficMetrics" )
        {

            // {
            //     auto* md_ccindex_tc =  thiz->mec_monitor_->mutable_md_ccindex_tc();
            //     md_ccindex_tc->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC);
            //     md_ccindex_tc->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
            //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
            // }
            thiz->getMecCheckCcindexTmTag(topic);
            if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_TrafficMetrics)
            {
                afl::base::json trafficMetricsJson;
                try
                {
                    trafficMetricsJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                }
                catch (const std::exception& e)
                {
                    // 坏消息直接丢弃：释放后必须返回1。返回0会让paho用已释放的指针重投，导致double free
                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure! [topic]" << topic
                                                      << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
                    MQTTAsync_freeMessage(&message);
                    MQTTAsync_free(topicName);
                    return 1;
                }
                if(thiz->getConfiger().enableDebugPrint)
                {
                    CCINDEX_INTER_DEBUG_PRINT << "[nowTime]" << nowTime << "[topicProfix] "  << topicProfix << " [topicName] " << topicName << " [topicLen] " << topicLen << "[json]" << trafficMetricsJson.dump();
                }
                thiz->processTrafficMetricsData(context, topic, trafficMetricsJson);
            }

        }
        else if(topicProfix == "upload" )
        {
            // {
            //     auto* md_ccindex_tm =  thiz->mec_monitor_->mutable_md_ccindex_tm();
            //     md_ccindex_tm->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TM);
            //     md_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
            //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
            // }
            //mec自检：动态数据
            thiz->pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TC);
            if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Tc)
            {
                std::string msg((char*)message->payload, message->payloadlen);
                thiz->processTcData(context, topic, msg);
            }

        }
        else if(topicProfix == "static")
        {
            // {
            //     auto* md_ccindex_st =  thiz->mec_monitor_->mutable_md_ccindex_st();
            //     md_ccindex_st->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_ST);
            //     md_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            //     CCINDEX_MQTT_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/monitor/mec,[data]" << thiz->mec_monitor_->ShortDebugString();
            //     thiz->Send("/v2x/monitor/mec", thiz->mec_monitor_);
            // }
            std::string device_id_str;
            size_t last_slash = topic.rfind('/');
            if (last_slash != std::string::npos) {
                // 提取最后一个 '/' 之后的所有字符 (例如 "{device_id}")
                device_id_str = topic.substr(last_slash + 1);
            }
            if (topic.find("static") != std::string::npos &&
                topic.find("query") != std::string::npos &&
                topic.find("ack") != std::string::npos)
            {
                //mec自检：静态数据
               thiz->pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_ST_QUERY_ACK, topic, device_id_str);
            }

            if (topic.find("static") != std::string::npos &&
                topic.find("update") != std::string::npos )
            {
                //mec自检：静态数据
                thiz->pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_ST_UPDATE, topic, device_id_str);
            }
            if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Static)
            {
                afl::base::json staticJson;
                try {
                    staticJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                }
                catch (const std::exception& e)
                {
                    // 坏消息直接丢弃：释放后返回1。返回0会让paho不停重投同一条坏消息
                    RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter " << "parse json failure! [topic]" << topic
                                                      << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
                    MQTTAsync_freeMessage(&message);
                    MQTTAsync_free(topicName);
                    return 1;
                }
                if(thiz->getConfiger().enableDebugPrint)
                {
                    CCINDEX_INTER_DEBUG_PRINT << "[nowTime]" << nowTime << "[topicProfix] "  << topicProfix << " [topicName] " << topicName << " [topicLen] " << topicLen << "[json]" << staticJson.dump();
                }
                thiz->processStaticData(context, topic, staticJson);
            }
        }
        else
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]this msg not need! [topic]" << topic;
        }
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return ret;
}

std::string CCINDEX_COMPONENT::getContentBetweenFirstAndSecondSlash(const std::string& input)
{
    size_t firstSlash = input.find('/');
    size_t secondSlash = input.find('/', firstSlash + 1);

    if (firstSlash != std::string::npos && secondSlash != std::string::npos)
    {
        return input.substr(firstSlash + 1, secondSlash - firstSlash - 1);
    }

    return ""; // 如果没有找到，返回空字符串
}
bool CCINDEX_COMPONENT::processTrafficMetricsData(void *context, std::string topic, afl::base::json j)
{
    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[topic]" << topic ;
    std::string deviceEsn = getRadarEsn(topic);
    std::string topicSecond = getContentBetweenFirstAndSecondSlash(topic);
    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[topicSecond]" << topicSecond;

    // 脉冲数据单独处理：发布到独立channel供信号灯模块消费，不走云控转发流程
    if (topicSecond == "pulse")
    {
        publishRadarPulse(topic, j);
        return true;
    }
    else if (topicSecond == "SingleStatistics")
    {
        // 雷达实时统计数据单独处理：发布到独立channel供信号灯模块消费，不走云控转发流程
        publishRadarStatistic(topic, j);
        return true;
    }
    else    //过滤掉脉冲/统计数据后，其他动态数据才走云控转发流程
    {
        // 新增：发布到channel上去
        publishCloudDataMqtt(topic, j);
    }

    bool TrafficMetrics_data_flag = false;    // 用于判断是否有发送动态数据到云控
    this->m_TrafficMetrics_Flag = false;
    auto thiz = (CCINDEX_COMPONENT *) context;
    
    if (topicSecond == "trajectories" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Trajectories)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Trajectories";
        }
        return false;
    }
    else if (topicSecond == "vehiclePass" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_VehiclePass)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_VehiclePass";
        }
        return false;
    }
    else if (topicSecond == "queueUp" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_QueueUp)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_QueueUp";
        }
        return false;
    }
    else if (topicSecond == "areaState" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_AreaState)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_AreaState";
        }
        return false;
    }
    else if (topicSecond == "overflow" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Overflow)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Overflow";
        }
        return false;
    }
    else if (topicSecond == "outlane" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Outlane)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Outlane";
        }
        return false;
    }
    else if (topicSecond == "statistics" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Statistics)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Statistics";
        }
        return false;
    }
    else if (topicSecond == "evaluations" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Evaluations)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Evaluations";
        }
        return false;
    }
    else if (topicSecond == "nonmotor" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Nonmotor)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Nonmotor";
        }
        return false;
    }
    else if (topicSecond == "deviceStatus" && !thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Device_Status)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]no Enable_Device_Statu";
        }
        return false;
    }


    if (topic.size() <= 0)
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[error]mqtt-inter topic empty!";
    }
    else
    {
        if(getConfiger().configerEnableCCIndex.Enable_Trajectories_Count)
        {
            if(topicSecond == "trajectories")
            {
                if(deviceEsn == m_TS_Radar1.deviceEsn)
                {
                    m_TS_Radar1.trajectories_recv_count ++;
                }
                else  if(deviceEsn == m_TS_Radar2.deviceEsn)
                {
                    m_TS_Radar2.trajectories_recv_count ++;
                }
                else  if(deviceEsn == m_TS_Radar3.deviceEsn)
                {
                    m_TS_Radar3.trajectories_recv_count ++;
                }
                else  if(deviceEsn == m_TS_Radar4.deviceEsn)
                {
                    m_TS_Radar4.trajectories_recv_count ++;
                }
                else  if(deviceEsn == m_TS_Radar5.deviceEsn)
                {
                    m_TS_Radar5.trajectories_recv_count ++;
                }
                else  if(deviceEsn == m_TS_Radar6.deviceEsn)
                {
                    m_TS_Radar6.trajectories_recv_count ++;
                }
                else{}
            }
            if (topicSecond == "vehiclePass") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.vehiclePass_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.vehiclePass_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.vehiclePass_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.vehiclePass_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.vehiclePass_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.vehiclePass_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】vehiclePass";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_VehiclePass) {
                    return false;
                }

            } else if (topicSecond == "queueUp") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.queueUp_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.queueUp_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.queueUp_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.queueUp_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.queueUp_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.queueUp_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】queueUp";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_QueueUp) {
                    return false;
                }


            } else if (topicSecond == "areaState") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.areaState_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.areaState_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.areaState_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.areaState_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.areaState_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.areaState_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】areaState";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_AreaState) {
                    return false;
                }

            } else if (topicSecond == "overflow") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.overflow_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.overflow_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.overflow_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.overflow_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.overflow_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.overflow_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】overflow";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Overflow) {
                    return false;
                }

            } else if (topicSecond == "outlane") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.outlane_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.outlane_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.outlane_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.outlane_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.outlane_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.outlane_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】outlane";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Outlane) {
                    return false;
                }

            } else if (topicSecond == "statistics") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.statistics_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.statistics_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.statistics_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.statistics_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.statistics_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.statistics_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】statistics";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Statistics) {
                    return false;
                }

            } else if (topicSecond == "evaluations") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.evaluations_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.evaluations_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.evaluations_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.evaluations_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.evaluations_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.evaluations_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】evaluations";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Evaluations) {
                    return false;
                }

            } else if (topicSecond == "nonmotor") {
                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                    m_TS_Radar1.nonmotor_recv_count++;
                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                    m_TS_Radar2.nonmotor_recv_count++;
                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                    m_TS_Radar3.nonmotor_recv_count++;
                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                    m_TS_Radar4.nonmotor_recv_count++;
                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                    m_TS_Radar5.nonmotor_recv_count++;
                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                    m_TS_Radar6.nonmotor_recv_count++;
                } else {}
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】nonmotor";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Nonmotor) {
                    return false;
                }
            } else if (topicSecond == "deviceStatus") {
                if(thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【topic】device_status";
                }
                if (!thiz->getConfiger().configerEnableCCIndex.Enable_Device_Status) {
                    return false;
                }
            } else {
                return false;
            }
            if (deviceEsn == m_TS_Radar1.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar1.to_string_recv();
            } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar2.to_string_recv();
            } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar3.to_string_recv();
            } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar4.to_string_recv();
            } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar5.to_string_recv();
            } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar6.to_string_recv();
            } else {}
        }



        if(!getConfiger().enableTransTrajectoriesDirect)
        {
            if(thiz->getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice] no enable TransTrajectories Direct";
            }
            thiz->mqttDispatchSubscribeMessage(topic, j);
//			if (topicSecond == "trajectories")
//            {
//
//                if (topic.find("desaysv") != std::string::npos)
//                {
//
//                    if(getConfiger().configerEnableCCIndex.Enable_Trajectories_Desaysv)
//                    {
//                        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories]desaysv";
//                        thiz->m_EventloopTrafficMetrics->runInLoop([thiz, topic, j]()
//                           {
//                               RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories]desaysv";
//                               thiz->mqttDispatchSubscribeMessage(topic, j);
//                           });
//                    }
//                }
//                else
//                {
//                    if(getConfiger().configerEnableCCIndex.Enable_Trajectories_Tsmtc) {
//
//                        RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories]tsmtc";
//                        thiz->m_EventloopTrafficMetrics->runInLoop([thiz, topic, j]()
//                           {
//                               RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories]tsmtc";
//                               thiz->mqttDispatchSubscribeMessageTrajectories(topic, j);
//                           });
//                    }
//
//                }
//            }
//            else
//            {
//                    RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[trajectories-no]enable  Direct";
//                    std::string hint = "trafficMetrics";
//
//                    static int message_count_trajectories_no = 1; // 用于计数的静态变量
//                    static int MAX_MESSAGE_COUNT_TRAJECTORIES_NO = 1000; // 设置您的最大限制
//                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【index】" << message_count_trajectories_no << "【MAX_MESSAGE_COUNT_TRAJECTORIES_NO】" << MAX_MESSAGE_COUNT_TRAJECTORIES_NO;
//                    // 根据消息计数判断是偶数还是奇数
//
//                    if (message_count_trajectories_no % 2 == 0) // 偶数
//                    {
//                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 1";
//                        if(thiz->m_EventloopTrafficMetricsPubFirst)
//                        {
//                            thiz->m_EventloopTrafficMetricsPubFirst->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                                                                               {
//                                                                                   RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 1";
//                                                                                   thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                                                                                   TrafficMetrics_data_flag = true;
//                                                                               });
//                        }
//                    }
//                    else // 奇数
//                    {
//                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 2";
//                        if(thiz->m_EventloopTrafficMetricsPubSecond)
//                        {
//                            thiz->m_EventloopTrafficMetricsPubSecond->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                                                                                {
//                                                                                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 2";
//                                                                                    thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                                                                                    TrafficMetrics_data_flag = true;
//                                                                                });
//                        }
//
//                    }
//                    if (message_count_trajectories_no >= MAX_MESSAGE_COUNT_TRAJECTORIES_NO)
//                    {
//                        RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】message_count_trafficMetrics_no >= MAX_MESSAGE_COUNT_TAFFIC_METRICS_NO";
//                        message_count_trajectories_no = 0; // 重置计数器
//                    }
//                    else
//                    {
//                        message_count_trajectories_no++;
//                    }
//                }
        }
        else
        {
            //直接透传
            if(thiz->getConfiger().enableDebugPrint)
            {
                RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[notice]enable TransTrajectories Direct";
            }
            std::string hint = "trafficMetrics";
            thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//            // 根据消息计数判断是偶数还是奇数
//            if (topicSecond == "trajectories")
//            {
//                if (topic.find("desaysv") != std::string::npos)
//                {
//                    if(thiz->m_EventloopTrajectoriesDesaysv)
//                    {
//                        thiz->m_EventloopTrajectoriesDesaysv->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                       {
//                           RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop desaysv";
//                           thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                           TrafficMetrics_data_flag = true;
//                       });
//                    }
//                }
//                else
//                {
//                    if(thiz->m_EventloopTrajectoriesTmstc)
//                    {
//                        thiz->m_EventloopTrajectoriesTmstc->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                        {
//                            RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop Tmstc";
//                            thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                            TrafficMetrics_data_flag = true;
//                        });
//                    }
//
//                }
//            }
//            else
//            {
//                static int message_count_no_trafficMetrics = 1; // 用于计数的静态变量
//                static int MAX_MESSAGE_COUNT_NO_TAFFIC_METRICS = 1000; // 设置您的最大限制
//                RADAR_TRAFFIC_METRICS_WARN_PRINT << "【message_count_no_trafficMetrics】" << message_count_no_trafficMetrics
//                            << "【MAX_MESSAGE_COUNT_TAFFIC_METRICS】" << MAX_MESSAGE_COUNT_NO_TAFFIC_METRICS;
//                if (message_count_no_trafficMetrics % 2 == 0) // 偶数
//                {
//                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 1";
//                    if(thiz->m_EventloopNoTrajectoriesOne)
//                    {
//                        thiz->m_EventloopNoTrajectoriesOne->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                       {
//                           RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 1";
//                           thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                           TrafficMetrics_data_flag = true;
//                       });
//                    }
//
//                }
//                else // 奇数
//                {
//                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 2";
//                    if(thiz->m_EventloopNoTrajectoriesTwo)
//                    {
//                        thiz->m_EventloopNoTrajectoriesTwo->runInLoop([thiz, topic, j, hint, &TrafficMetrics_data_flag]()
//                        {
//                            RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】runInLoop 2";
//                            thiz->mqttPushMsg2BrokerCloud(topic, j, hint);
//                            TrafficMetrics_data_flag = true;
//                        });
//                    }
//
//                }
//                if (message_count_no_trafficMetrics >= MAX_MESSAGE_COUNT_NO_TAFFIC_METRICS)
//                {
//                    RADAR_TRAFFIC_METRICS_WARN_PRINT << "【notice】message_count_no_trafficMetrics >= MAX_MESSAGE_COUNT_NO_TAFFIC_METRICS";
//                    message_count_no_trafficMetrics = 0; // 重置计数器
//                }
//                else
//                {
//                    message_count_no_trafficMetrics++;
//                }
//            }
        }

    }

    // 更新标志位
    this->m_TrafficMetrics_Flag = TrafficMetrics_data_flag;
    return true;

}
bool CCINDEX_COMPONENT::processTcData(void *context, std::string topic, std::string msg)
{

    bool tc_data_flag = false;    // 用于判断是否有发送指标数据到云控
    this->m_TcFlag = false;
    auto thiz = (CCINDEX_COMPONENT *) context;

    if (topic.size() <= 0)
    {
        RADAR_TC_ERROR_PRINT << "[error]topic empty!";
    }
    else
    {
        thiz->mqttDispatchSubscribeMessageProto(topic, (void*)msg.c_str(), msg.size());
        tc_data_flag = true;
//        static int message_count_upload = 1; // 用于计数的静态变量
//        static int MAX_MESSAGE_COUNT_UPLOAD = 1000; // 设置您的最大限制
//
//        // 根据消息计数判断是偶数还是奇数
//        if (message_count_upload % 2 == 0) // 偶数
//        {
//            thiz->m_EventloopTc->runInLoop([thiz, topic, msg, &tc_data_flag]()
//             {
//                 thiz->mqttDispatchSubscribeMessageProto(topic, (void*)msg.c_str(), msg.size());
//                 tc_data_flag = true;
//             });
//        }
//        else // 奇数
//        {
//            thiz->m_EventloopTc->runInLoop([thiz, topic, msg, &tc_data_flag]()
//             {
//                 thiz->mqttDispatchSubscribeMessageProto(topic, (void*)msg.c_str(), msg.size());
//                 tc_data_flag = true;
//             });
//        }
//        if (message_count_upload > MAX_MESSAGE_COUNT_UPLOAD)
//        {
//            message_count_upload = 0; // 重置计数器
//        }
//        else
//        {
//            message_count_upload++;
//        }
    }
    
    // 更新标志位
    this->m_TcFlag = tc_data_flag;
    return true;
}

bool CCINDEX_COMPONENT::processStaticData(void *context,std::string topic, afl::base::json j)
{

    // 新增：发布到channel上去  CyberRT
    publishCloudDataHttp("/static/device/config", "application/json", j);

    auto thiz = (CCINDEX_COMPONENT *) context;

    switch (thiz->determineOperationTypeStatic(topic))
    {
        case RadarStaticQueryAck: {
            if (thiz->getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[success]find [topic]" << topic;
            }
            ConfigUpdateResponseData configUpdateResponseData;
            if (thiz->processConfigQeuryDataAck(topic, j))
            {
                if (thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_DEBUG_PRINT << "[success]process config-query-ack success!";
                }
                return true;
            } else {
                RADAR_STATIC_ERROR_PRINT << "[error]process config-query-ack failure!";
                return false;
            }
        }
            break;

        case RadarStaticUpdate: {
            //更新信息
            if (thiz->getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[success]find topic ![topic]" << topic;
            }
            if (thiz->processConfigUpdateDataAck(topic, j))
            {
                if (thiz->getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_DEBUG_PRINT << "[success]process config-udpate success!";
                }
                return true;
            } else {
                RADAR_STATIC_ERROR_PRINT << "[error]process config-update failure!";
                return false;
            }
        }
            break;
        default:
            RADAR_STATIC_ERROR_PRINT << "[error]this msg not need!";
            break;
    }
    return true;
}

bool CCINDEX_COMPONENT::mqttPushStringMsg2Broker(string topic, string info, string hint)
{
    if(!info.empty())
    {
        if(!mqttPublishMsg(topic, info))
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter push " << hint.c_str() << " failure!";
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
bool CCINDEX_COMPONENT::mqttPushJsonMsg2Broker(string topic, json info, string hint)
{
    std::string pData;
    try{
        pData = info.dump();
    }
    catch (json::exception& e)
    {
        CCINDEX_INTER_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }
    if(!pData.empty())
    {
        if(getConfiger().enableDebugPrint)
        {
            CCINDEX_INTER_ERROR_PRINT << "[notice]mqtt-inter  ccindex >>>>> cloud [topic]" << topic  << "[data]" << std::endl << pData;
        }
        if(!mqttPublishMsg(topic, pData))
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter  push " << hint.c_str() << " failure!";
            return false;
        }
    }else
    {
        return false;
    }
    return true;
}


////////////////////////////////////////////////////////////////////

bool CCINDEX_COMPONENT::mqttPushStringMsg2BrokerCloud(string topic, string info, string hint)
{
    if(!info.empty())
    {
        if(!mqttPublishMsgCloud(topic, info))
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter push " << hint.c_str() << " failure!";
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}
bool CCINDEX_COMPONENT::mqttPushJsonMsg2BrokerCloud(string topic, json info, string hint)
{
    std::string pData;
    try{
        pData = info.dump();
    }
    catch (json::exception& e)
    {
        CCINDEX_INTER_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
        return false;
    }
    if(!pData.empty())
    {

        if(!mqttPublishMsgCloud(topic, pData))
        {
            CCINDEX_INTER_ERROR_PRINT << "[error]mqtt-inter  push " << hint.c_str() << " failure!";
            return false;
        }
    }else
    {
        return false;
    }
    return true;
}
void CCINDEX_COMPONENT::mqttDispatchSubscribeMessageProtoCloud(std::string topic, void* data, int size)
{
    adu::st::TrafficInfos tis;
    std::vector<int> remove_tis_index;
    if(!tis.ParseFromArray(data, size))
    {
        //CCINDEX_CLOUD_DEBUG_PRINT << "Recv radar Tc msg but proto parse fail";
		std::cerr << "Received data: " << size << std::endl;
        std::cerr << "Debug string: " << tis.DebugString() << std::endl;
        return;
    }

    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_DEBUG_PRINT << "parse radar proto:" << tis.DebugString();
        for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
        {
            auto temp_info = tis.traffic_info().Get(tempi);
            std::string tempinfoStr = temp_info.DebugString();
            size_t pos = tempinfoStr.find("timestamp_sec");
            if(pos == std::string::npos)
            {

                CCINDEX_CLOUD_DEBUG_PRINT << "timestamp_sec not found";
                continue;
            }

            std::string tempStr = tempinfoStr.substr(pos);
            std::string tempPrintStr;
            for(char tempchar : tempStr)
            {
                if(tempchar == '}')
                {
                    continue;
                }

                if(tempchar == '\n')
                {
                    tempPrintStr = tempPrintStr + ';';
                }
                else
                {
                    tempPrintStr = tempPrintStr + tempchar;
                }
            }

            CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:" << tempPrintStr;
        }
    }
    if(getConfiger().enableDebugPrint)
    {
        for(int i = 0; i < tis.traffic_info_size(); i++)
        {
            auto temp_info = tis.traffic_info().Get(i);
            if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_1_SECOND)//车道级/流向级，雷达过来一秒钟级数据
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_1_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_3_SECOND)//流向级/车道级，雷达过来三秒钟级数据存储，不给平台透传
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_3_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_SECOND)//车道级，雷达过来五秒钟级数据透传给平台
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_5_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_MINUTE)//车道级/流向级，雷达过来的五分钟级数据需添加“浪费时间”后透传给平台
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_5_MINUTE]";
            }
        }
    }
}

int printMqttTlsErrorCloud(const char *str, size_t len, void *u)
{
    CCINDEX_CLOUD_ERROR_PRINT << "[ssl error]" << str;
    return 0;
}
bool CCINDEX_COMPONENT::mqttInitConnOptsCloud()
{
    m_MqttConnOptsCloud = MQTTAsync_connectOptions_initializer;
    m_MqttConnOptsCloud.connectTimeout      = m_MqttClientConfig.configerMqttCloud.mqttConnectTimeOut;
    m_MqttConnOptsCloud.keepAliveInterval   = m_MqttClientConfig.configerMqttCloud.mqttKeepAliveInterval;
    m_MqttConnOptsCloud.cleansession        = m_MqttClientConfig.configerMqttCloud.mqttCleanSession;
    m_MqttConnOptsCloud.MQTTVersion         = m_MqttClientConfig.configerMqttCloud.mqttVersion;
    m_MqttConnOptsCloud.username            = m_MqttClientConfig.configerMqttCloud.mqttUserName.c_str();
    m_MqttConnOptsCloud.password            = m_MqttClientConfig.configerMqttCloud.mqttPassword.c_str();
    m_MqttConnOptsCloud.automaticReconnect = 1;
    m_MqttConnOptsCloud.onSuccess = mqttOnConnectCloud;
    m_MqttConnOptsCloud.onFailure = mqttOnConnectFailureCloud;
    m_MqttConnOptsCloud.context = this;

    std::string sMqttCommunicationMethod = getConfiger().configerMqttCloud.mqttBrokerUrl;
    std::string m_ModuleDirPath = getConfiger().configerProjectPath.projectRoot;
    if (!afl::FileUtil::isDirectory(m_ModuleDirPath.c_str()))
    {
        afl::FileUtil::createRecursionDir(m_ModuleDirPath.c_str());
    }
    std::string m_TlsDirPath = m_ModuleDirPath + "/" + getConfiger().configerProjectPath.tlsDirName;
    if (!afl::FileUtil::isDirectory(m_TlsDirPath.c_str()))
    {
        afl::FileUtil::createRecursionDir(m_TlsDirPath.c_str());
    }

    m_TrustCAFilePath 		= m_TlsDirPath + "/" + getConfiger().configerProjectPath.tlsCAFileName;
    m_KeyFilePath 			= m_TlsDirPath + "/" + getConfiger().configerProjectPath.tlsClientKeyFileName;
    m_PrivateKeyFilePath 	= m_TlsDirPath + "/" +  getConfiger().configerProjectPath.tlsClientPrivateKeyFileName;
    bool bTrustCAFile = afl::FileUtil::isFileExist(m_TrustCAFilePath.c_str());
    bool bKeyFile = afl::FileUtil::isFileExist(m_KeyFilePath.c_str());
    bool bPrivateKeyFile = afl::FileUtil::isFileExist(m_PrivateKeyFilePath.c_str());
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_DEBUG_PRINT << std::endl
                            << "[CA]" << m_TrustCAFilePath.c_str() << std::endl
                            << "[Client Cert]" << m_KeyFilePath.c_str() << std::endl
                            << "[Client Key]" << m_PrivateKeyFilePath.c_str();
    }
    if (sMqttCommunicationMethod.npos != sMqttCommunicationMethod.find("ssl"))
    {
        if (bTrustCAFile && bKeyFile && bPrivateKeyFile)
        {
            //add ssl
            static MQTTAsync_SSLOptions sslopts = MQTTAsync_SSLOptions_initializer;
            m_MqttConnOptsCloud.ssl = &sslopts;
            //m_MqttConnOptsCloud.ssl->struct_version = 0;
            m_MqttConnOptsCloud.ssl->sslVersion =  m_MqttClientConfig.configerMqttCloud.mqttSslVersion;
            //m_MqttConnOptsCloud.ssl->enableServerCertAuth = 0;
            //m_MqttConnOptsCloud.ssl->verify = 0;
            m_MqttConnOptsCloud.ssl->ssl_error_cb = printMqttTlsErrorCloud;
            m_MqttConnOptsCloud.ssl->trustStore = m_TrustCAFilePath.c_str();
            m_MqttConnOptsCloud.ssl->keyStore = m_KeyFilePath.c_str();
            m_MqttConnOptsCloud.ssl->privateKey = m_PrivateKeyFilePath.c_str();
            m_MqttConnOptsCloud.ssl->privateKeyPassword = m_MqttClientConfig.configerProjectPath.tlsClientPrivateKeyPassword.c_str();
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "[success]mqtt tls init success! ";
            }
        }
        else	//no cert or key file
        {
            CCINDEX_CLOUD_ERROR_PRINT << "[错误]使用了tls加密，请在GoHighReporter.cfg文件中，BasicConfiger下的mqttAddress值中ssl修改为tcp!";
//            exit(0);
        }
    }
    return true;
}

bool CCINDEX_COMPONENT::mqttInitCloud()
{
    m_MqttClientCloud = nullptr;
    m_MqttConnectedCloud = false;

    mqttInitConnOptsCloud();
    std::string clientIdTemp = m_MqttClientConfig.configerMqttCloud.mqttClientId + ":" +
                               std::to_string(afl::util::TimeStamp::now(true).microSeconds());
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_DEBUG_PRINT << "[mqtt-cloud]"
                            << "[mqttBroker-Url]" << m_MqttClientConfig.configerMqttCloud.mqttBrokerUrl.c_str()
                             << " [client-id]" << clientIdTemp.c_str()
                             << " [username]" << m_MqttClientConfig.configerMqttCloud.mqttUserName.c_str()
                             << " [passwd]" << m_MqttClientConfig.configerMqttCloud.mqttPassword.c_str();
    }
    if (MQTTASYNC_SUCCESS != MQTTAsync_create(&m_MqttClientCloud, m_MqttClientConfig.configerMqttCloud.mqttBrokerUrl.c_str(),
                                              clientIdTemp.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL))
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud create connectiont failure!";
        return false;
    }

    MQTTAsync_setCallbacks(m_MqttClientCloud, this, mqttConnlostCloud, mqttSubscribeMsgArrvdCloud, NULL);
    MQTTAsync_setConnected(m_MqttClientCloud, this, mqttConnectedCallbackCloud);
    if (!mqttConnectCloud())
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud Connect Failure!";
        return false;
    }

    return true;
}

void CCINDEX_COMPONENT::mqttDeinitCloud()
{
    if (m_MqttClientCloud)
    {
        CCINDEX_MQTT_ERROR_PRINT << "[cloud]mqtt Deinit Cloud!";
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
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
    }
}

void CCINDEX_COMPONENT::mqttOnConnectCloud(void *context, MQTTAsync_successData *response)
{
    auto thiz = (CCINDEX_COMPONENT *) context;
    if(thiz->getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_SUCCESS_PRINT << "[success]mqtt-cloud connect success!";
    }
    thiz->m_MqttConnectedCloud = true;
    //thiz->pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, true);
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
            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud subscribe error";
        }
    }

    if(thiz->m_TimerMqttReconnectCloud > 0)
    {
        thiz->m_EventloopCloud->cancelTimer(thiz->m_TimerMqttReconnectCloud);
        thiz->m_TimerMqttReconnectCloud = -1;
    }
}

void CCINDEX_COMPONENT::mqttOnConnectFailureCloud(void *context, MQTTAsync_failureData *response)
{
    CCINDEX_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud connect error";
    if (response)
    {
        if (response->message)
        {
            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error [error-code]" << response->code << " [error-msg]" << response->message;

        } else
        {
            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error [error-code]" << response->code << " [error-msg]" << response->message;
        }
    } else
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error!";
    }

    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->mqttReconnectCloud();
}


bool CCINDEX_COMPONENT::mqttConnectCloud()
{
    int rc;
    if (MQTTASYNC_SUCCESS != (rc = MQTTAsync_connect(m_MqttClientCloud, &m_MqttConnOptsCloud)))
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect error!";
        m_MqttConnectedCloud = false;
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
        return false;
    }
    return true;
}

bool CCINDEX_COMPONENT::mqttReconnectCloud()
{
    // m_MqttReconnectCountCloud++;
    CCINDEX_CLOUD_ERROR_PRINT << "[notice]mqtt-cloud reconnect![count]" << m_MqttReconnectCountCloud;
    // CCINDEX_MQTT_ERROR_PRINT << "[cloud][notice]mqtt-cloud reconnect![count]" << m_MqttReconnectCountCloud;
    // if(m_TimerMqttReconnectCloud < 0)
    // {
    //     m_TimerMqttReconnectCloud = m_EventloopCloud->addTimer(std::bind(&CCINDEX_COMPONENT::mqttConnectCloud, this), m_MqttClientConfig.configerMqttCloud.mqttReconnectInterval, true);
    // }
//    if(m_MqttReconnectCountCloud <  getConfiger().configerMqtt.mqttReconnectMaxCount)
//    {
//        m_EventloopCloud->addTimer(std::bind(&CCINDEX_COMPONENT::mqttConnectCloud, this), m_MqttClientConfig.configerMqttCloud.mqttReconnectInterval, false);
//    }
//    else
//    {
//        if(m_MqttClientCloud)
//        {
//#if 0
//            MQTTAsync_destroy(&m_MqttClientCloud);
//            usleep(100000L);
//            m_MqttClientCloud = nullptr;
//            if(!mqttInitCloud())
//            {
//                CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud init failure!";
//                return false;
//            }
//#endif
//            mqttConnectCloud();
//        }
//    }

    return true;
}

void CCINDEX_COMPONENT::mqttConnlostCloud(void *context, char *cause)
{
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = false;
    //thiz->pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
    CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud connect lost! [cause]" << cause;
    CCINDEX_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud connect lost! [cause]" << cause;
    thiz->mqttReconnectCloud();
}

void CCINDEX_COMPONENT::mqttOnDisconnectCloud(void *context, MQTTAsync_successData *response)
{
    CCINDEX_CLOUD_ERROR_PRINT << "[notice]mqtt-cloud disconnect!";
    CCINDEX_MQTT_ERROR_PRINT << "[cloud][notice]mqtt-cloud disconnect!";
    auto thiz = (CCINDEX_COMPONENT *) context;
    thiz->m_MqttConnectedCloud = false;
    //thiz->pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
}

void CCINDEX_COMPONENT::mqttOnSubscribeCloud(void *context, MQTTAsync_successData *response)
{
    CCINDEX_CLOUD_SUCCESS_PRINT << "[success]mqtt-cloud subscribe success!";
}

void CCINDEX_COMPONENT::mqttOnSubscribeFailureCloud(void *context, MQTTAsync_failureData *response)
{
    CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud subscribe failure!";
    CCINDEX_MQTT_ERROR_PRINT << "[cloud][error]mqtt-cloud subscribe failure!";
}

bool CCINDEX_COMPONENT::mqttPublishMsgCloud(const std::string &topic, const std::string &msg)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud has not connected!";
    //     return false;
    // }
    std::string topicProfix = topic.substr(0, topic.find("/"));
    if(topicProfix == "upload" )
    {
        pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_CCINDEX_TC);
    }
    if (msg.size() <= 0 || topic.empty())
    {
        CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud len < 0 or topic empty!";
        return false;
    }

    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    opts.context = m_MqttClientCloud;
    pubmsg.payload = (void *) msg.c_str();
    pubmsg.payloadlen = msg.length();
    pubmsg.qos = m_MqttClientConfig.configerMqttCloud.mqttSendQos;
    pubmsg.retained = m_MqttClientConfig.configerMqttCloud.mqttRetained;

    if ((rc = MQTTAsync_sendMessage(m_MqttClientCloud, topic.c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        if (rc == MQTTASYNC_DISCONNECTED)
        {
            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud Async Connect failure，will connect!";
            if (m_MqttConnectedCloud)
            {
                m_MqttConnectedCloud = false;
                //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
                // mqttReconnectCloud();
            }
        } else
        {
            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud push msg error，[error-code]" << rc;
        }

        return false;
    }

    return true;
}

int CCINDEX_COMPONENT::mqttSubscribeMsgArrvdCloud(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int ret = 1;
    auto thiz = (CCINDEX_COMPONENT *) context;
    if (thiz->m_StopWorkers)
    {
        // 进程正在退出（EventLoop已停）：新消息直接丢弃，不再处理/转发
        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topicName);
        return 1;
    }
    if (message->payloadlen)
    {
        std::string topic(topicName, topicLen);
        std::string topicProfix = topic.substr(0, topic.find("/"));
        if(thiz->getConfiger().enableDebugPrint)
        {
            CCINDEX_CLOUD_DEBUG_PRINT << "[notice]cloud >>> ccindex" << " [topic]" << topicName << "[msg]" << std::string((char *) message->payload, message->payloadlen);
        }
        if(topicProfix != "upload")
        {
            CCINDEX_CLOUD_DEBUG_PRINT << "[error]mqtt-cloud topic profix is not upload!";
            MQTTAsync_freeMessage(&message);
            MQTTAsync_free(topicName);
            return 1;
        }
        if(topicProfix == "upload")
        {
            std::string topic(topicName, topicLen);
            if (topic.size() <= 0)
            {
                if(thiz->getConfiger().enableDebugPrint)
                    CCINDEX_CLOUD_DEBUG_PRINT << "[Inter][error]topic empty!";
            }
            else
            {
                std::string msg((char*)message->payload, message->payloadlen);
                //把具体的消息处理放到主线程里，和定时器在一个线程，避免出现两个线程同时修改一个vector/map
//                thiz->m_Eventloop->runInLoop([thiz, topic, msg]()
//                  {
//                      thiz->mqttDispatchSubscripeMessageProto(topic, (void*)msg.c_str(), msg.size());
//                  });
                static int message_count_upload = 1; // 用于计数的静态变量
                static int MAX_MESSAGE_COUNT_UPLOAD = 1000; // 设置您的最大限制

                // 根据消息计数判断是偶数还是奇数
                if (message_count_upload % 2 == 0) // 偶数
                {
                    thiz->m_Eventloop->runInLoop([thiz, topic, msg]()
                                                 {
                                                     thiz->mqttDispatchSubscripeMessageProto(topic, (void*)msg.c_str(), msg.size());
                                                 });
                }
                else // 奇数
                {
                    thiz->m_Eventloop->runInLoop([thiz, topic, msg]()
                                                 {
                                                     thiz->mqttDispatchSubscripeMessageProto(topic, (void*)msg.c_str(), msg.size());
                                                 });
                }
                if (message_count_upload > MAX_MESSAGE_COUNT_UPLOAD)
                {
                    message_count_upload = 0; // 重置计数器
                }
                else
                {
                    message_count_upload++;
                }
            }
        }
        else   if(topicProfix == "trafficMetrics" )
        {
            if(thiz->m_MqttClientConfig.configerEnableCCIndex.Enable_Trajectories)
            {
                afl::base::json trafficMetricsJson;
                try
                {
                    trafficMetricsJson = afl::base::json::parse((char *) message->payload, (char *) message->payload + message->payloadlen);
                }
                catch (const std::exception& e)
                {
                    // 坏消息直接丢弃：释放后必须返回1。返回0会让paho用已释放的指针重投，导致double free
                    CCINDEX_CLOUD_ERROR_PRINT << "[Cloud]radar-cloud <<<<<<< clod[error] parse json failure! [topic]" << topic
                                              << " [payloadlen]" << message->payloadlen << " [what]" << escapeForLog(e.what(), strlen(e.what())) << " [payload]" << escapeForLog(message->payload, message->payloadlen);
                    MQTTAsync_freeMessage(&message);
                    MQTTAsync_free(topicName);
                    return 1;
                }
                if (thiz->getConfiger().enableDebugPrint)
                {
                    CCINDEX_CLOUD_DEBUG_PRINT << "[trafficMetrics]radar-cloud <<<<<<< clod[msg]" << trafficMetricsJson.dump().c_str();
                }

            }
        }
        else
        {

        }
//
//        if (topic.size() <= 0)
//        {
//            CCINDEX_CLOUD_ERROR_PRINT << "[error]mqtt-cloud topic empty!";
//        } else
//        {
//            std::string msg((char*)message->payload, message->payloadlen);
//            static int message_count_cloud = 1; // 用于计数的静态变量
//            static int MAX_MESSAGE_COUNT_CLOUD = 1000; // 设置您的最大限制
//
//            // 根据消息计数判断是偶数还是奇数
//            if (message_count_cloud % 2 == 0) // 偶数
//            {
//                thiz->m_EventloopCloud->runInLoop([thiz, topic, msg]()
//                 {
//                     thiz->mqttDispatchSubscribeMessageProtoCloud(topic, (void*)msg.c_str(), msg.size());
//                 });
//            }
//            else // 奇数
//            {
//                thiz->m_EventloopCloud->runInLoop([thiz, topic, msg]()
//                 {
//                     thiz->mqttDispatchSubscribeMessageProtoCloud(topic, (void*)msg.c_str(), msg.size());
//                 });
//            }
//            if (message_count_cloud > MAX_MESSAGE_COUNT_CLOUD)
//            {
//                message_count_cloud = 0; // 重置计数器
//            }
//            else
//            {
//                message_count_cloud++;
//            }
//        }
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return ret;
}
void CCINDEX_COMPONENT::mqttDispatchSubscripeMessageProto(std::string topic, void* data, int size)
{
    adu::st::TrafficInfos tis;
    std::vector<int> remove_tis_index;
    if(!tis.ParseFromArray(data, size))
    {
        CCINDEX_CLOUD_ERROR_PRINT << "Recv radar Tc msg but proto parse fail";
        return;
    }
    
    m_recv_cloud_tc_count ++;
    if(m_recv_cloud_tc_count == INT_MAX)
    {   
        m_recv_cloud_tc_count = 0;
    }
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_CLOUD_DEBUG_PRINT << "recv cloud tc count:" << m_recv_cloud_tc_count;

    }
//    CCINDEX_CLOUD_DEBUG_PRINT << "parse radar proto:" << tis.DebugString();
    for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
    {
        auto temp_info = tis.traffic_info().Get(tempi);
        std::string tempinfoStr = temp_info.DebugString();
        size_t pos = tempinfoStr.find("timestamp_sec");
        if(pos == std::string::npos)
        {
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "timestamp_sec not found";
            }
            continue;
        }

        std::string tempStr = tempinfoStr.substr(pos);
        std::string tempPrintStr;
        for(char tempchar : tempStr)
        {
            if(tempchar == '}')
            {
                continue;
            }

            if(tempchar == '\n')
            {
                tempPrintStr = tempPrintStr + ';';
            }
            else
            {
                tempPrintStr = tempPrintStr + tempchar;
            }
        }
        if(getConfiger().configerPrintSubMsgFromCloud.enableDebugPrintSubMsgTc)
        {
            CCINDEX_CLOUD_DEBUG_PRINT <<  tempPrintStr;
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        for(int i = 0; i < tis.traffic_info_size(); i++)
        {
            auto temp_info = tis.traffic_info().Get(i);
            if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_1_SECOND)//车道级/流向级，雷达过来一秒钟级数据
            {

                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_1_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_3_SECOND)//流向级/车道级，雷达过来三秒钟级数据存储，不给平台透传
            {

                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_3_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_SECOND)//车道级，雷达过来五秒钟级数据透传给平台
            {

                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_5_SECOND]";
            }
            else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_MINUTE)//车道级/流向级，雷达过来的五分钟级数据需添加“浪费时间”后透传给平台
            {
                CCINDEX_CLOUD_DEBUG_PRINT << "parse cloud proto:[TrafficInfo_TimeType_TIME_5_MINUTE]";
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
//静态数据
int CCINDEX_COMPONENT::determineOperationTypeStatic(const std::string& path)
{
    int subscribeMsgType = 0;
    if (path.find("static/device/config/update/") == 0)
    {
        subscribeMsgType = RadarStaticUpdate;
    } else if (path.find("static/device/config/query/ack/") == 0)
    {
        subscribeMsgType =  RadarStaticQueryAck;
    }
    else
    {
        subscribeMsgType =  RadarStaticUnkown; // 如果不匹配任何已知路径
    }
}

bool CCINDEX_COMPONENT::processConfigUpdateDataAck(std::string topic, const json& subScribeJson)
{
    try{
        std::string deviceId;
        if(getDeviceIdFromSubscribeData(topic, deviceId))
        {
//            //将json转换为结构体
//            /////////////////////////////////////////////
//            ConfigUpdateData configData;
//            try
//            {
//                configData = subScribeJson;
//            }
//            catch (afl::base::json::exception &e)
//            {
//                RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
//                return false;
//            }
//            if(getConfiger().enableDebugPrint)
//            {
//                RADAR_STATIC_DEBUG_PRINT << "[deviceId]" << deviceId;
//            }
//            try{
//                if(insertOrUpdateConfigUpdateData(m_ConfigUpdateData, deviceId, configData))
//                {
//                    if(getConfiger().enableDebugPrint)
//                    {
//                        RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Update-Config-Data success!";
//                    }
//                }
//                else
//                {
//                    RADAR_STATIC_ERROR_PRINT << "[error]insert Or Update Update-Config-Data  error!";
//                    return false;
//                }
//            }catch(exception& e)
//            {
//                RADAR_STATIC_DEBUG_PRINT << "[error]" << e.what();
//            }
            /////////////////////////////////////////////
            try{
                if(getConfiger().httpConfiger.enablePushSeparate)
                {
                    if(getConfiger().enableDebugPrint)
                    {
                        RADAR_STATIC_DEBUG_PRINT << "[topic]" << topic;
                    }
                    //分开推送
                    if(topic.find(getConfiger().httpConfiger.vensorTsmtc) != std::string::npos)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_STATIC_DEBUG_PRINT << "[tsmtc]update json";
                        }
                        if(insertOrUpdateConfigQueryDataJson(m_ConfigUpdateDataJsonTsmtc, deviceId, subScribeJson))
                        {
                            if(getConfiger().enableDebugPrint)
                            {
                                RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Update-Config-Data Json success!";
                            }
                        }
                        else
                        {
                            RADAR_STATIC_ERROR_PRINT << "[error]insert Or Update Update-Config-Data Json  error!";
                            return false;
                        }
                    }

                    if(topic.find(getConfiger().httpConfiger.vensorDesaysv) != std::string::npos)
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_STATIC_DEBUG_PRINT << "[desaysv]update json";
                        }
                        if(insertOrUpdateConfigQueryDataJson(m_ConfigUpdateDataJsonDesaysv, deviceId, subScribeJson))
                        {
                            if(getConfiger().enableDebugPrint)
                            {
                                RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Update-Config-Data Json success!";
                            }
                        }
                        else
                        {
                            RADAR_STATIC_ERROR_PRINT << "[error]insert Or Update Update-Config-Data Json  error!";
                            return false;
                        }
                    }

                }
                else
                {
                    if(insertOrUpdateConfigQueryDataJson(m_ConfigUpdateDataJson, deviceId, subScribeJson))
                    {
                        if(getConfiger().enableDebugPrint)
                        {
                            RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Update-Config-Data Json success!";
                        }
                    }
                    else
                    {
                        RADAR_STATIC_ERROR_PRINT << "[error]insert Or Update Update-Config-Data Json  error!";
                        return false;
                    }
                }

                }catch(exception& e)
                {
                    RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
                    return false;
                }
            }
            else
            {
                RADAR_STATIC_ERROR_PRINT << "[error]get device-id failure!";
                return false;
            }
    }catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }

    return true;
}
bool CCINDEX_COMPONENT::processConfigQeuryDataAck(std::string topic, const json& subScribeJson)
{
    try{
        //查询响应数据
        std::string deviceId;
        if(getDeviceIdFromSubscribeData(topic, deviceId, RadarStaticQueryAck))
        {
            //将json转换为结构体
            /////////////////////////////////////////////
//            ConfigQueryData configData;
//            try
//            {
//                configData = subScribeJson;
//            }
//            catch (afl::base::json::exception &e)
//            {
//                RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
//                return false;
//            }
//            if(insertOrUpdateConfigQueryData(m_ConfigQueryData, deviceId, configData))
//            {
//                if(getConfiger().enableDebugPrint)
//                {
//                    RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Query Config Data success!";
//                }
//            }
//            else
//            {
//                RADAR_STATIC_ERROR_PRINT << "[Inter][error]insert Or Update Query Config Data  error!";
//                return false;
//            }
            /////////////////////////////////////////////
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[deviceId]" << deviceId;
            }
            if(insertOrUpdateConfigQueryDataJson(m_ConfigQueryDataJson, deviceId, subScribeJson))
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_DEBUG_PRINT << "[success]insert Or Update Config Data Json success!";
                }
            }
            else
            {
                RADAR_STATIC_ERROR_PRINT << "[error]insert Or Update Config Data Json  failure!";
                return false;
            }
        }
        else
        {
            RADAR_STATIC_ERROR_PRINT << "[Inter][error]get device id  error!";
            return false;
        }
    } catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }

    return true;
}


bool CCINDEX_COMPONENT::insertOrUpdateConfigQueryData(std::unordered_map<std::string, ConfigQueryData>& configDataMap, std::string key, const ConfigQueryData& data)
{
    try{
        // 方法 1: 使用 find 检查是否存在
        auto it = configDataMap.find(key);
        if (it != configDataMap.end())
        {
            // 更新已存在的元素
            it->second = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "Updated: Key = " << key << std::endl;
            }
        }
        else
        {
            // 插入新元素
            configDataMap[key] = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "Inserted: Key = " << key << std::endl;
            }
        }
    } catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }

    return true;
}
bool CCINDEX_COMPONENT::insertOrUpdateConfigQueryDataJson(std::unordered_map<std::string, json>& configDataMap, std::string key, const json& data)
{
    try{
        // 方法 1: 使用 find 检查是否存在
        auto it = configDataMap.find(key);
        if (it != configDataMap.end())
        {
            // 更新已存在的元素
            it->second = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[Updated]Key = " << key << std::endl;
            }
        }
        else
        {
            // 插入新元素
            configDataMap[key] = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[Inserted]Key = " << key << std::endl;
            }
        }
    }catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }

    return true;
}

bool CCINDEX_COMPONENT::insertOrUpdateConfigUpdateData(std::unordered_map<std::string, ConfigUpdateData>& configDataMap, std::string key, const ConfigUpdateData& data)
{
    try {
        // 方法 1: 使用 find 检查是否存在
        auto it = configDataMap.find(key);
        if (it != configDataMap.end())
        {
            // 更新已存在的元素
            it->second = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[Updated]Key = " << key << std::endl;
            }
        }
        else
        {
            // 插入新元素
            configDataMap[key] = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[Inserted]Key = " << key << std::endl;
            }
        }
    }catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }

    return true;
}
bool CCINDEX_COMPONENT::insertOrUpdateConfigUpdateDataJsonStatic(std::unordered_map<std::string, json>& configDataMap, std::string key, const json& data)
{
    try
    {
        // 方法 1: 使用 find 检查是否存在
        auto it = configDataMap.find(key);
        if (it != configDataMap.end())
        {
            // 更新已存在的元素
            it->second = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "Updated: Key = " << key << std::endl;
            }
        }
        else
        {
            // 插入新元素
            configDataMap[key] = data;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "Inserted: Key = " << key << std::endl;
            }
        }
    }catch(exception& e)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]" << e.what();
        return false;
    }


    return true;
}


bool CCINDEX_COMPONENT::getDeviceIdFromSubscribeData(const std::string& input, std::string& device_id, RADAR_STATIC_MESSAGE_TYPE optType)
{
    // 定义分隔符
    const std::string delimiter = "/";
    std::vector<std::string> tokens;

    // 使用字符串流分割字符串
    std::istringstream ss(input);
    std::string token;

    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }
    int topicLen = 9;
    int deviceIdPos = 8;
    switch(optType)
    {
        case RadarStaticQueryAck:
            topicLen = 10;
            deviceIdPos = 9;
            break;

        case RadarStaticUpdate:
            topicLen = 9;
            deviceIdPos = 8;
            break;
        default:
            RADAR_STATIC_ERROR_PRINT << "[error]this msg not need!";
            break;
    }
    // 检查长度并提取 {device_id}
    if (tokens.size() >= topicLen)
    {
        device_id = tokens[deviceIdPos];
        return true;
    } else {
        RADAR_STATIC_ERROR_PRINT << "[error]get deviceId failure!";
        return false;
    }
    return true;
}
bool CCINDEX_COMPONENT::mqttPublishMsgRetries(std::string topic, json payload, std::string hint)
{

    for (int attempt = 1; attempt <= getConfiger().configerMqtt.mqttPublishMaxRetries; ++attempt)
    {
        if (mqttPushJsonMsg2Broker(topic, payload, hint))
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[success]Publish success!";
            }
            return true;  // 成功发布消息，返回true
        }
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[error] Publish " << hint << " data failed on attempt " << attempt << "!";
            }
        }
    }
    return false;  // 所有重试均失败，返回false
}
std::string CCINDEX_COMPONENT::extractIPAddress(const std::string& url)
{
    std::string ip;

    // Check if the address starts with "ssl://"
    if (url.find("ssl://") == 0)
    {
        // Remove the "ssl://" part
        std::string ipPort = url.substr(6);
        // Find the position of the colon
        std::size_t pos = ipPort.find(':');
        if (pos != std::string::npos)
        {
            // Extract IP address
            ip = ipPort.substr(0, pos);
        }
        else
        {
            // If no colon, the whole string is the IP
            ip = ipPort;
        }
    } else {
        // If it doesn't start with "ssl://", find the colon directly
        std::size_t pos = url.find(':');
        if (pos != std::string::npos)
        {
            // Extract IP address
            ip = url.substr(0, pos);
        }
        else
        {
            // If no colon, the whole string is the IP
            ip = url;
        }
    }

    return ip;
}
bool CCINDEX_COMPONENT::processNumberConfigData(json& mergedJson)
{
    if (mergedJson.empty())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]The map is empty!" << std::endl;
        return false;
    }

    // 检查第一个 JSON 对象中是否存在 "data" 字段
    if (mergedJson.find("data") == mergedJson.end())
    {
        RADAR_STATIC_ERROR_PRINT << "The first JSON object does not contain 'data'!" << std::endl;
        return false;
    }

    // 遍历数据结构并修改 branch_no 和 lane_no
    for (auto& place : mergedJson["data"])
    {
        for (auto& branch : place["branches"])
        {
            // 修改 branch_no
            uint64_t branchNum =branch["branch_no"].get<int>();

            uint64_t branchProfixNum = branchNum /10000;
            uint64_t branchPostNum = branchNum % 10000;
            uint64_t branchNo = 0;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[branchNum] " << branchNum  << " [branchProfixNum] " << branchProfixNum << " [branchPostNum] " << branchPostNum;
            }
            branch["branch_no"] = branchNo = branchProfixNum * 100000000 + 5110000 + branchPostNum;
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT  << "[branchNo] " << branchNo<< " [branchNum] " << branchNum  << " [branchProfixNum] " << branchProfixNum << " [branchPostNum] " << branchPostNum;
            }
            for (auto& lane : branch["lanes"])
            {
                // 修改 lane_no
                uint64_t laneNum = lane["lane_no"].get<uint64_t>() ;
                uint64_t laneProfixNum = laneNum /1000000;
                uint64_t lanePostNum = laneNum % 1000000;
                uint64_t laneNo = 0;
                lane["lane_no"] = laneNo = laneProfixNum * 10000000000 + 511000000 + lanePostNum;
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_DEBUG_PRINT  << "[laneNo] " << laneNo<< " [laneNum] " << laneNum  << " [laneProfixNum] " << laneProfixNum << " [lanePostNum] " << lanePostNum;
                }
            }
        }
    }
    return true;
}

bool CCINDEX_COMPONENT::timerQueryConfig2HttpServer()
{
    if(!m_HttpClientPtr)
    {
        std::string httpCloudeHostPort = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                                         std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtr = std::unique_ptr<httplib::Client>(new httplib::Client(httpCloudeHostPort));
    }

    if(!m_HttpClientPtr->is_valid())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]radar_static http client error";
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_DEBUG_PRINT << "[success]radar_static http client start success";
        }
    }

//    if (!m_ConfigUpdateDataJson.empty())
//    {
//        if(getConfiger().enableDebugPrint)
//        {
//            for (const auto& pair : m_ConfigUpdateDataJson)
//            {
//                RADAR_STATIC_DEBUG_PRINT << "[Key]" << pair.first << "[Value]" << pair.second.dump() << std::endl; // Pretty print JSON with indentation
//            }
//        }
//    }
//    else
//    {
//        RADAR_STATIC_DEBUG_PRINT << "[error]m_ConfigUpdateDataJson is empty!";
//        return false;
//    }
//
//    if (m_ConfigUpdateDataJson.begin()->second.find("replace") == m_ConfigUpdateDataJson.begin()->second.end())
//    {
//        RADAR_STATIC_ERROR_PRINT << "The first JSON object does not contain 'replace'!";
//        return false;
//    }


    httplib::Headers headers = {
//            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientGetPath = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                           + "&vendor=" + getConfiger().configerTopicRadarID.vendor
                           +  "&category=" +  getConfiger().categoryUrlValue;
    if(getConfiger().enableDebugPrint)
    {
        RADAR_STATIC_DEBUG_PRINT << "[m_HttpClientGetPath]" << m_HttpClientGetPath << std::endl; // Pretty print JSON with indentation
    }

    // 发送 GET 请求
    httplib::Result result = m_HttpClientPtr->Get(m_HttpClientGetPath.c_str(), headers);
    if (result)
    { // 检查是否有响应
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = true;
        if (result->status == 200) {
            // 成功

            if(getConfiger().enableDebugPrint) {
                RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Status: " << result->status;
                RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_DEBUG_PRINT << ss.str();
                RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Body: " << result->body;
            }
        } else {
            // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
            // 非200情况（如404/500等）
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_ERROR_PRINT << "[GET-Response]Unexpected Status: " << result->status;
                RADAR_STATIC_ERROR_PRINT << "[GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_DEBUG_PRINT << ss.str();
                RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Body: " << result->body;
            }
        }
//        if(getConfiger().enableDebugPrint)
//        {
//            RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Status: " << result->status ;
//            RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Headers:";
//            std::stringstream ss;
//            for (const auto& header : result->headers)
//            {
//                ss << "  " << header.first << ": " << header.second << std::endl;
//            }
//            RADAR_STATIC_DEBUG_PRINT << ss.str();
//            RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Body: " << result->body;
//        }
    }
    else { // 处理错误
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
        auto err = result.error();
        RADAR_STATIC_ERROR_PRINT << "[GET-Response]Error Code: " << static_cast<int>(err);
        RADAR_STATIC_ERROR_PRINT << "[GET-Response]Error Message: " << httplib::to_string(err) ;
    }


}

//定时查询： mec 去服务器查询  tsmtc
bool CCINDEX_COMPONENT::timerQueryConfig2HttpServerTsmtc()
{

    if(!m_HttpClientPtrQueryTsmtc)
    {
        m_HttpCloudeHostIpPortQueryTsmtc = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                                         std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtrQueryTsmtc = std::unique_ptr<httplib::Client>(new httplib::Client(m_HttpCloudeHostIpPortQueryTsmtc));
    }

    if(!m_HttpClientPtrQueryTsmtc->is_valid())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]radar_static http client error";
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_DEBUG_PRINT << "[success]radar_static http client start success";
        }
    }


    httplib::Headers headers =
            {
//            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientGetPathTsmtc = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                           + "&vendor=" + getConfiger().httpConfiger.vensorTsmtc
                           +  "&category=" +  getConfiger().categoryUrlValue;
    if(getConfiger().enableDebugPrint)
    {
        RADAR_STATIC_GET_DEBUG_PRINT << "[get-tsmtc]======>[url]" << m_HttpCloudeHostIpPortQueryTsmtc << m_HttpClientGetPathTsmtc << std::endl; // Pretty print JSON with indentation
    }

    // 发送 GET 请求
    m_HttpClientPtrQueryTsmtc->set_connection_timeout(2, 0);
    m_HttpClientPtrQueryTsmtc->set_read_timeout(5, 0);
    m_HttpClientPtrQueryTsmtc->set_write_timeout(5, 0);
    httplib::Result result = m_HttpClientPtrQueryTsmtc->Get(m_HttpClientGetPathTsmtc.c_str(), headers);
    if (result)
    { // 检查是否有响应
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = true;
        m_MecLinkStatucCcindexTmTsmtc = true;
        if (result->status == 200)
        {
            // 成功
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-tsmtc][GET-Response]Status: " << result->status;
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-tsmtc][GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_GET_DEBUG_PRINT << ss.str();
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-tsmtc][GET-Response]Body: " << result->body;
            }
        }
        else
        {
            // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
            // 非200情况（如404/500等）
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_GET_ERROR_PRINT << "[get-tsmtc][GET-Response]Unexpected Status: " << result->status;
                RADAR_STATIC_GET_ERROR_PRINT << "[get-tsmtc][GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_GET_ERROR_PRINT << ss.str();
                RADAR_STATIC_GET_ERROR_PRINT << "[get-tsmtc][GET-Response]Body: " << result->body;
            }
        }
    }
    else
    { // 处理错误
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
        m_MecLinkStatucCcindexTmTsmtc = false;
        auto err = result.error();
        RADAR_STATIC_GET_ERROR_PRINT << "[get-tsmtc][GET-Response]Error Code: " << static_cast<int>(err);
        RADAR_STATIC_GET_ERROR_PRINT << "[get-tsmtc][GET-Response]Error Message: " << httplib::to_string(err) ;
    }
}
//定时查询： mec 去服务器查询  desaysv
bool CCINDEX_COMPONENT::timerQueryConfig2HttpServerDesaysv()
{
    if(!m_HttpClientPtrQueryDesaysv)
    {
        m_HttpCloudeHostIpPortQueryDesaysv = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                               std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtrQueryDesaysv = std::unique_ptr<httplib::Client>(new httplib::Client(m_HttpCloudeHostIpPortQueryDesaysv));
    }

    if(!m_HttpClientPtrQueryDesaysv->is_valid())
    {
        RADAR_STATIC_GET_ERROR_PRINT << "[error]radar_static http client error";
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_GET_ERROR_PRINT << "[success]radar_static http client start success";
        }
    }


    httplib::Headers headers =
    {
//            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientGetPathDesaysv = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                           + "&vendor=" + getConfiger().httpConfiger.vensorDesaysv
                           +  "&category=" +  getConfiger().categoryUrlValue;
    if(getConfiger().enableDebugPrint)
    {
        RADAR_STATIC_GET_DEBUG_PRINT << "[get-desaysv]======>[url]" << m_HttpCloudeHostIpPortQueryDesaysv << m_HttpClientGetPathDesaysv << std::endl; // Pretty print JSON with indentation
    }

    // 发送 GET 请求
    httplib::Result result = m_HttpClientPtrQueryDesaysv->Get(m_HttpClientGetPathDesaysv.c_str(), headers);
    if (result)
    { // 检查是否有响应
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = true;
        m_MecLinkStatucCcindexTmDesaysv = true;
        if (result->status == 200)
        {
            // 成功
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-desaysv][GET-Response]Status: " << result->status;
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-desaysv][GET-Response]Status: " << result->status;
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-desaysv][GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_GET_DEBUG_PRINT << ss.str();
                RADAR_STATIC_GET_DEBUG_PRINT << "[get-desaysv][GET-Response]Body: " << result->body;
            }
        }
        else
        {
            // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
            // 非200情况（如404/500等）
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_ERROR_PRINT << "[get-desaysv][GET-Response]Unexpected Status: " << result->status;
                RADAR_STATIC_DEBUG_PRINT << "[get-desaysv][GET-Response]Headers:";
                std::stringstream ss;
                for (const auto& header : result->headers)
                {
                    ss << "  " << header.first << ": " << header.second << std::endl;
                }
                RADAR_STATIC_GET_ERROR_PRINT << ss.str();
                RADAR_STATIC_GET_ERROR_PRINT << "[get-desaysv][GET-Response]Body: " << result->body;
            }
        }
    }
    else
    { // 处理错误
        m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_get_flag = false;
        m_MecLinkStatucCcindexTmDesaysv = false;
        auto err = result.error();
        RADAR_STATIC_GET_ERROR_PRINT << "[get-desaysv][GET-Response]Error Code: " << static_cast<int>(err);
        RADAR_STATIC_GET_ERROR_PRINT << "[get-desaysv][GET-Response]Error Message: " << httplib::to_string(err) ;
    }
}
bool CCINDEX_COMPONENT::timerUpdateConfigData()
{
    if(!m_HttpClientPtr)
    {
        std::string httpCloudeHostPort = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtr = std::unique_ptr<httplib::Client>(new httplib::Client(httpCloudeHostPort));
    }

    if(!m_HttpClientPtr->is_valid())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]radar_static http client error";
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, false);
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_DEBUG_PRINT << "[success]radar_static http client start success";
        }
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, true);
    }

    if (!m_ConfigUpdateDataJson.empty())
    {
        if(getConfiger().enableDebugPrint)
        {
            for (const auto& pair : m_ConfigUpdateDataJson)
            {
                RADAR_STATIC_DEBUG_PRINT << "[Key]" << pair.first << "[Value]" << pair.second.dump() << std::endl; // Pretty print JSON with indentation
            }
        }
    }
    else
    {
        RADAR_STATIC_ERROR_PRINT << "[error]m_ConfigUpdateDataJson is empty!";
        return false;
    }

    if (m_ConfigUpdateDataJson.begin()->second.find("replace") == m_ConfigUpdateDataJson.begin()->second.end())
    {
        RADAR_STATIC_ERROR_PRINT << "The first JSON object does not contain 'replace'!";
        return false;
    }

    int replace;
    try
    {
        replace = m_ConfigUpdateDataJson.begin()->second["replace"];
        if (replace == 0)
        {
            replace = 0;
        }
    } catch (json::exception &e)
    {
        RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
        return false;
    }
    httplib::Headers headers = {
//            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientPostPath = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                          + "&vendor=" + getConfiger().httpConfiger.vensorDesaysv
                          +  "&category=" + getConfiger().categoryUrlValue + "&replace=" + std::to_string(replace);
    if(getConfiger().enableDebugPrint)
    {
        RADAR_STATIC_DEBUG_PRINT << "[m_HttpClientPostPath]" << m_HttpClientPostPath << std::endl; // Pretty print JSON with indentation
        RADAR_STATIC_DEBUG_PRINT << "[notice]begin get data";
        RADAR_STATIC_DEBUG_PRINT << "[m_HttpClientPostPath]" << m_HttpClientPostPath << std::endl;
    }

    // 遍历所有 ConfigData
    json mergedJson; // 存放合并结果的 JSON 对象
    if(mergeJsonConfigData(m_ConfigUpdateDataJson, mergedJson))
    {
        std::string body;
        try
        {
            body = mergedJson.dump();
        } catch (json::exception &e)
        {
            RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
            return false;
        }

        // 检查路径是否有效
        if (m_HttpClientPostPath.length() == 0)
        {
            RADAR_STATIC_ERROR_PRINT << "Request path is empty!" << std::endl;
            return false;  // 路径无效，退出
        }
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[url]" << m_HttpClientPostPath.c_str() << std::endl;
            }

        }
        // 检查请求体
        if (body.empty())
        {
            RADAR_STATIC_ERROR_PRINT << "Request body is empty!" << std::endl;
            return -1;  // 请求体为空，退出
        }
        else
        {
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[body]" << body << std::endl;
            }
        }

        // 发送 POST 请求
        httplib::Result result = m_HttpClientPtr->Post(m_HttpClientPostPath.c_str(), headers, body, m_ContentType.c_str());
        if (result)
        { // 检查是否有响应
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = true;
            if (result->status == 200) {
                // 成功

                if(getConfiger().enableDebugPrint) {
                    RADAR_STATIC_DEBUG_PRINT << "[GET-Response]Status: " << result->status;
                    RADAR_STATIC_DEBUG_PRINT << "[POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_DEBUG_PRINT << ss.str();
                    RADAR_STATIC_DEBUG_PRINT << "[POST-Response]Body: " << result->body;
                }
            } else {
                // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
                // 非200情况（如404/500等）
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_ERROR_PRINT << "[GET-Response]Unexpected Status: " << result->status;;
                    RADAR_STATIC_DEBUG_PRINT << "[POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_DEBUG_PRINT << ss.str();
                    RADAR_STATIC_DEBUG_PRINT << "[POST-Response]Body: " << result->body;
                }
            }
        }
        else { // 处理错误
            auto err = result.error();
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
            RADAR_STATIC_ERROR_PRINT << "[POST-Response]Error Code: " << static_cast<int>(err);
            RADAR_STATIC_ERROR_PRINT << "[POST-Response]Error Message: " << httplib::to_string(err) ;
        }
    }

}

bool CCINDEX_COMPONENT::timerUpdateConfigDataTsmtc()
{
    if(!m_HttpClientPtrUpdateTsmtc)
    {
        m_HttpCloudeHostIpPortUpdateTsmtc = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                                         std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtrUpdateTsmtc = std::unique_ptr<httplib::Client>(new httplib::Client(m_HttpCloudeHostIpPortUpdateTsmtc));
    }

    if(!m_HttpClientPtrUpdateTsmtc->is_valid())
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][error]radar_static http client error";
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, false);
        return false;
    }


    if (!m_ConfigUpdateDataJsonTsmtc.empty())
    {
//        if(getConfiger().enableDebugPrint)
//        {
//            RADAR_STATIC_POST_DEBUG_PRINT << "[post-tsmtc]";
//            for (const auto& pair : m_ConfigUpdateDataJsonTsmtc)
//            {
//                RADAR_STATIC_POST_DEBUG_PRINT << "[Key]" << pair.first << "[Value]" << pair.second.dump() << std::endl; // Pretty print JSON with indentation
//            }
//        }
    }
    else
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][error]m_ConfigUpdateDataJsonTsmtc is empty!";
        return false;
    }

    if (m_ConfigUpdateDataJsonTsmtc.begin()->second.find("replace") == m_ConfigUpdateDataJsonTsmtc.begin()->second.end())
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][error]The first JSON object does not contain 'replace'!";
        return false;
    }

    int replace = 1;
    try
    {
        replace = m_ConfigUpdateDataJsonTsmtc.begin()->second["replace"];
        if(!getConfiger().httpConfiger.replaceUseMsg)
        {
            replace = getConfiger().httpConfiger.replaceTsmtc;
        }
//        if (replace == 0)
//        {
//            replace = 0;
//        }
    } catch (json::exception &e)
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
        return false;
    }
    httplib::Headers headers = {
//            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientPostPathTsmtc = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                           + "&vendor=" + getConfiger().httpConfiger.vensorTsmtc
                           +  "&category=" + getConfiger().categoryUrlValue + "&replace=" + std::to_string(replace);

    // 遍历所有 ConfigData
    json mergedJson; // 存放合并结果的 JSON 对象
    if(mergeJsonConfigData(m_ConfigUpdateDataJsonTsmtc, mergedJson))
    {
        std::string body;
        try
        {
            body = mergedJson.dump();
        } catch (json::exception &e)
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
            return false;
        }

        // 检查路径是否有效
        if (m_HttpClientPostPathTsmtc.length() == 0)
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][error]Request path is empty!" << std::endl;
            return false;  // 路径无效，退出
        }
        // 检查请求体
        if (body.empty())
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][error]Request body is empty!" << std::endl;
            return false;  // 请求体为空，退出
        }

        // 发送 POST 请求
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_POST_DEBUG_PRINT << "[post-tsmtc]===>[url]http://" << m_HttpCloudeHostIpPortUpdateTsmtc <<  m_HttpClientPostPathTsmtc << std::endl;
            RADAR_STATIC_POST_DEBUG_PRINT << "[post-tsmtc][body]" << body << std::endl;
        }
        httplib::Result result = m_HttpClientPtrUpdateTsmtc->Post(m_HttpClientPostPathTsmtc.c_str(), headers, body, m_ContentType.c_str());
        if (result)
        { // 检查是否有响应
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = true;
            if (result->status == 200)
            {
                // 成功
                m_MecLinkStatucCcindexTmTsmtc = true;

                if(getConfiger().enableDebugPrint) {
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-tsmtc][POST-Response]Status: " << result->status;
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-tsmtc][POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_POST_SUCCESS_PRINT << ss.str();
                    std::string unescapedBody = result->body;
                    std::replace(unescapedBody.begin(), unescapedBody.end(), '\n', ' '); // 替换转义字符
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-tsmtc][POST-Response]Body: " << unescapedBody;
                }
            } else {
                m_MecLinkStatucCcindexTmTsmtc = false;
                // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
                // 非200情况（如404/500等）
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][POST-Response]Unexpected Status: " << result->status;;
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_POST_ERROR_PRINT << ss.str();
                    std::string unescapedBody = result->body;
                    std::replace(unescapedBody.begin(), unescapedBody.end(), '\n', ' '); // 替换转义字符
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][POST-Response]Body: " << unescapedBody;
                }
            }
        }
        else { // 处理错误
            auto err = result.error();
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
            RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][POST-Response]Error Code: " << static_cast<int>(err);
            RADAR_STATIC_POST_ERROR_PRINT << "[post-tsmtc][POST-Response]Error Message: " << httplib::to_string(err) ;
        }
    }
}
bool CCINDEX_COMPONENT::timerUpdateConfigDataDesaysv()
{
    if(!m_HttpClientPtrUpdateDesaysv)
    {
        m_HttpCloudeHostIpPortUpdateDesaysv = getConfiger().httpConfiger.httpCloudServerIp + ":" +
                                         std::to_string(getConfiger().httpConfiger.httpCloudServerPort);
        m_HttpClientPtrUpdateDesaysv = std::unique_ptr<httplib::Client>(new httplib::Client(m_HttpCloudeHostIpPortUpdateDesaysv));
    }

    if(!m_HttpClientPtrUpdateDesaysv->is_valid())
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][error]radar_static http client error";
        //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, false);
        return false;
    }


    if (!m_ConfigUpdateDataJsonDesaysv.empty())
    {
//        if(getConfiger().enableDebugPrint)
//        {
//            RADAR_STATIC_POST_DEBUG_PRINT << "[post-desaysv]";
//            for (const auto& pair : m_ConfigUpdateDataJsonDesaysv)
//            {
//                RADAR_STATIC_POST_DEBUG_PRINT << "[Key]" << pair.first << "[Value]" << pair.second.dump() << std::endl; // Pretty print JSON with indentation
//            }
//        }
    }
    else
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][error]m_ConfigUpdateDataJsonDesaysv is empty!";
        return false;
    }

    if (m_ConfigUpdateDataJsonDesaysv.begin()->second.find("replace") == m_ConfigUpdateDataJsonDesaysv.begin()->second.end())
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][errot]The first JSON object does not contain 'replace'!";
        return false;
    }

    int replace = 1;
    try
    {
        replace = m_ConfigUpdateDataJsonDesaysv.begin()->second["replace"];
        if(!getConfiger().httpConfiger.replaceUseMsg)
        {
            replace = getConfiger().httpConfiger.replaceDesaysv;
        }
//        if (replace == 0)
//        {
//            replace = 0;
//        }
    } catch (json::exception &e)
    {
        RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
        return false;
    }
    httplib::Headers headers = {
    //            { "Content-Type", m_ContentType }
    };
    std::string m_HttpClientPostPathDesaysv = "/static/device/config?cross_id=" + getConfiger().configerTopicRadarID.cross_id
                           + "&vendor=" + getConfiger().httpConfiger.vensorDesaysv
                           +  "&category=" + getConfiger().categoryUrlValue + "&replace=" + std::to_string(replace);
    // 遍历所有 ConfigData
    json mergedJson; // 存放合并结果的 JSON 对象
    if(mergeJsonConfigData(m_ConfigUpdateDataJsonDesaysv, mergedJson))
    {
        std::string body;
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_POST_DEBUG_PRINT << "[post-desaysv][mergedJson]" << mergedJson.dump() << std::endl;
        }
        try
        {
            body = mergedJson.dump();
        } catch (json::exception &e)
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
            return false;
        }

        // 检查路径是否有效
        if (m_HttpClientPostPathDesaysv.length() == 0)
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][error]Request path is empty!" << std::endl;
            return false;  // 路径无效，退出
        }

        // 检查请求体
        if (body.empty())
        {
            RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][error]Request body is empty!" << std::endl;
            return false;  // 请求体为空，退出
        }

        // 发送 POST 请求
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_POST_DEBUG_PRINT << "[post-desaysv]====>[url]http://" << m_HttpCloudeHostIpPortUpdateDesaysv <<  m_HttpClientPostPathDesaysv << std::endl;
            RADAR_STATIC_POST_DEBUG_PRINT << "[post-desaysv][body]" << body << std::endl;
        }
        httplib::Result result = m_HttpClientPtrUpdateDesaysv->Post(m_HttpClientPostPathDesaysv.c_str(), headers, body, m_ContentType.c_str());
        if (result)
        { // 检查是否有响应
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = true;
            if (result->status == 200) {
                // 成功
                m_MecLinkStatucCcindexTmDesaysv = true;

                if(getConfiger().enableDebugPrint) {
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-desaysv][POST-Response]Status: " << result->status;
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-desaysv][POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_POST_SUCCESS_PRINT << ss.str();
                    std::string unescapedBody = result->body;
                    std::replace(unescapedBody.begin(), unescapedBody.end(), '\n', ' '); // 替换转义字符
                    RADAR_STATIC_POST_SUCCESS_PRINT << "[post-desaysv][POST-Response]Body: " << unescapedBody;
                }
            }
            else
            {
                m_MecLinkStatucCcindexTmDesaysv = false;
                // m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
                // 非200情况（如404/500等）
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][POST-Response]Unexpected Status: " << result->status;;
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][POST-Response]Headers:";
                    std::stringstream ss;
                    for (const auto& header : result->headers)
                    {
                        ss << "  " << header.first << ": " << header.second << std::endl;
                    }
                    RADAR_STATIC_POST_ERROR_PRINT << ss.str();
                    std::string unescapedBody = result->body;
                    std::replace(unescapedBody.begin(), unescapedBody.end(), '\n', ' '); // 替换转义字符
                    RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][POST-Response]Body: " << unescapedBody;
                }
            }
        }
        else { // 处理错误
            auto err = result.error();
            m_CcindexStatusContainer.m_CcindexMonitor.ccindexStaticStatus.st_post_flag = false;
            RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][POST-Response]Error Code: " << static_cast<int>(err);
            RADAR_STATIC_POST_ERROR_PRINT << "[post-desaysv][POST-Response]Error Message: " << httplib::to_string(err) ;
        }
    }
}
//合并json数据
bool CCINDEX_COMPONENT::mergeJsonConfigData(const std::unordered_map<std::string, json>& m_ConfigUpdateDataJson, json& mergedJson)
{
    if (m_ConfigUpdateDataJson.empty())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]The map is empty!" << std::endl;
        return false;
    }

    // 获取第一个 pair
    auto firstPair = m_ConfigUpdateDataJson.begin();
    mergedJson = firstPair->second; // 用第一个 JSON 进行初始化

    // 检查第一个 JSON 对象中是否存在 "data" 字段
    if (mergedJson.find("data") == mergedJson.end())
    {
        RADAR_STATIC_ERROR_PRINT << "The first JSON object does not contain 'data'!" << std::endl;
        return false;
    }

    // 遍历 map 中的其他 JSON 对象
    for (auto it = std::next(firstPair); it != m_ConfigUpdateDataJson.end(); ++it)
    {
        const json& currentJson = it->second;

        // 检查当前 JSON 对象中是否存在 "data" 字段
        if (currentJson.find("data") != currentJson.end())
        {
            const json& currentData = currentJson["data"];

            // 将 currentData 合并到 mergedJson 的 "data" 中
            for (const auto& item : currentData)
            {
                mergedJson["data"].push_back(item);
            }
        }
    }
    return true;
}
bool CCINDEX_COMPONENT::timerQueryConfigDataOnce()
{
    if (!m_MqttConnected)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]mqtt has not connected!";
        return false;
    }
    else
    {
        timerQueryConfigData();
        if(m_TimerConfigQueryMonitorPushOnce > 0)
        {
            m_EventloopStatic->cancelTimer(m_TimerConfigQueryMonitorPushOnce);
            m_TimerConfigQueryMonitorPushOnce = -1;
        }
    }
}
//定时下发查询
bool CCINDEX_COMPONENT::timerQueryConfigData()
{
    if (!m_MqttConnected)
    {
        RADAR_STATIC_ERROR_PRINT << "[error]mqtt has not connected!";
        return false;
    }

    for(auto it = m_MqttClientConfig.topicUnMapCCIndex.begin(); it != m_MqttClientConfig.topicUnMapCCIndex.end(); ++it)
    {
        const MqttTopicConfigerCCIndex &mqttTopicConfigerCCIndex = it->second;
        ConfigQuery configQuery;

        if(getConfigQueryMsgFromTopic(mqttTopicConfigerCCIndex.Topic_Static_Query, configQuery))
        {
            std::string hint = "static";

            std::string topicPT = mqttTopicConfigerCCIndex.Topic_Static_Query;
            static int message_count_static_query_channel = 1; // 用于计数的静态变量
            static int MAX_MESSAGE_COUNT_STATIC_QUERY_CHANNEL = 1000; // 设置您的最大限制

            // 根据消息计数判断是偶数还是奇数
            if (message_count_static_query_channel % 2 == 0) // 偶数
            {
                m_EventloopStatic->runInLoop([this, topicPT, configQuery, hint]()
               {
                  if(!mqttPushMsg2Broker(topicPT, configQuery, hint))
                  {
                      RADAR_STATIC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                  }
               });
            }
            else // 奇数
            {
                m_EventloopStatic->runInLoop([this, topicPT, configQuery, hint]()
               {
                   if(!mqttPushMsg2Broker(topicPT, configQuery, hint))
                   {
                       RADAR_STATIC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
                   }
               });
            }
            if (message_count_static_query_channel > MAX_MESSAGE_COUNT_STATIC_QUERY_CHANNEL)
            {
                message_count_static_query_channel = 0; // 重置计数器
            }
            else
            {
                message_count_static_query_channel++;
            }

        }
    }

}

bool CCINDEX_COMPONENT::getConfigQueryMsgFromTopic(std::string topic, ConfigQuery& configQuery)
{
    if(topic.empty())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]topic is error format!";
        return false;
    }
    // 定义分隔符
    const std::string delimiter = "/";
    std::vector<std::string> tokens;

    // 使用字符串流分割字符串
    std::istringstream ss(topic);
    std::string token;

    while (std::getline(ss, token, '/'))
    {
        tokens.push_back(token);
    }

    // 检查长度并提取所需的值
    if (tokens.size() >= 8)
    {
        configQuery.vendor = tokens[5];      // {vendor}
        configQuery.category = tokens[6];    // {category}
        configQuery.cross_id = tokens[7];     // {cross_id}
        return true;
    } else
    {
        RADAR_STATIC_ERROR_PRINT << "[error]topic is error format!";
        return false;
    }
}
//云控下发查询get
bool CCINDEX_COMPONENT::processCloudQuery()
{
    // 使用 std::unique_ptr 封装 httplib::Server 实例
    m_HttpServerPtr = std::unique_ptr<httplib::Server>(new httplib::Server());
    if(!m_HttpServerPtr->is_valid())
    {
        RADAR_STATIC_ERROR_PRINT << "[error]radar_static http server error";
        return false;
    }
    else
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_DEBUG_PRINT << "[success]radar_static http server start success, [ip]" << getConfiger().httpConfiger.httpHostServerIp.c_str() << "[port]" << getConfiger().httpConfiger.httpHostServerPort;
        }
    }
//    if(getConfiger().enableDebugPrint)
//    {
//        RADAR_STATIC_DEBUG_PRINT << "[httpCloudClientGetPath]" << getConfiger().httpConfiger.httpCloudClientGetPath;
//    }
    // 处理 GET 请求
    m_HttpServerPtr->Get(getConfiger().httpConfiger.httpCloudClientGetPath, [&](const httplib::Request&req, httplib::Response &res)
    {
        if(getConfiger().enableDebugPrint)
        {
            RADAR_STATIC_DEBUG_PRINT << "[notice] get request!";
        }
        // 解析 GET 请求参数
        auto params = parseGetParams(req);
            // 输出解析结果
        for (const auto& param : params)
        {
            std::cout << "GET - Key: " << param.first << ", Value: " << param.second << std::endl;
        }
        // 检查所需参数是否存在
        if (params.find("cross_id") != params.end() &&
            params.find("vendor") != params.end() &&
            params.find("category") != params.end())
        {
            auto configQuery = buildConfigQuery(params);
            if(getConfiger().enableDebugPrint)
            {
                RADAR_STATIC_DEBUG_PRINT << "[configQuery]" << configQuery.to_string();
            }
            json mergedJson; // 存放合并结果的 JSON 对象
            for(auto pair: m_ConfigQueryDataJson)
            {
                RADAR_STATIC_DEBUG_PRINT << "[json]" << pair.second.dump();
            }
            if(mergeJsonConfigData(m_ConfigQueryDataJson, mergedJson))
            {

                std::string body;
                try {
                    body = mergedJson.dump();
                }
                catch (json::exception &e)
                {
                    RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
                    return false;
                }

                // 检查请求体
                if (body.empty())
                {
                    RADAR_STATIC_ERROR_PRINT << "Request body is empty!" << std::endl;
                    return false;  // 请求体为空，退出
                }
                // 设置响应内容
                res.set_content(body, m_ContentType.c_str());
            }
            else
            {
                RADAR_STATIC_ERROR_PRINT << "[error] merge json failure, will send query config!" << std::endl;
                timerQueryConfigData();
                m_TimerConfigQueryInstantly = m_EventloopStatic->addTimer(std::bind(&CCINDEX_COMPONENT::timerQueryConfigDataInstantly, this, res),
                                      4, true);
            }
        }

    });

    // 启动服务器，监听指定的主机和端口
    m_HttpServerPtr->listen(getConfiger().httpConfiger.httpHostServerIp.c_str(), getConfiger().httpConfiger.httpHostServerPort);
}

bool CCINDEX_COMPONENT::timerQueryConfigDataInstantly(httplib::Response &res)
{
    json mergedJson; // 存放合并结果的 JSON 对象
    if(mergeJsonConfigData(m_ConfigQueryDataJson, mergedJson))
    {
        std::string body;
        try {
            body = mergedJson.dump();
        }
        catch (json::exception &e)
        {
            RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
            return false;
        }

        // 检查请求体
        if (body.empty())
        {
            RADAR_STATIC_ERROR_PRINT << "Request body is empty!" << std::endl;
            return false;  // 请求体为空，退出
        }
        // 设置响应内容
        res.set_content(body, m_ContentType.c_str());
    }
    else
    {
        json responseJson; // 初始化响应 JSON 对象
        // 设置错误响应
        responseJson["code"] = 21; // 示例错误代码
        responseJson["message"] = "no query config data from radar!";
        responseJson["data"] = json::object(); // 或者根据需求初始化为空对象

        res.set_content(responseJson.dump(), m_ContentType.c_str());
        res.status = 400; // 设置 HTTP 状态码为 400

        RADAR_STATIC_ERROR_PRINT << "[error] process failure!" << std::endl;
    }
    if(m_TimerConfigQueryInstantly > 0)
    {
        m_EventloopStatic->cancelTimer(m_TimerConfigQueryInstantly);
        m_TimerConfigQueryInstantly = -1;
    }
    return true;
}
// 从参数中构建 ConfigQuery 或 ConfigUpdate
ConfigQuery CCINDEX_COMPONENT::buildConfigQuery(const std::unordered_map<std::string, std::string>& params)
{
    // 获取参数
    ConfigQuery query;
    query.cross_id = params.at("cross_id"); // 获取 cross_id
    query.vendor = params.at("vendor");      // 获取 vendor
    query.category = params.at("category");  // 获取 category
    return query;
}
ConfigUpdate CCINDEX_COMPONENT::buildConfigUpdate(const std::unordered_map<std::string, std::string>& params)
{
    ConfigUpdate update;
    update.cross_id = params.at("cross_id"); // 获取 cross_id
    update.vendor = params.at("vendor");      // 获取 vendor
    update.category = params.at("category");  // 获取 category

    // 获取 replace，进行字符串转整型转换
    if (params.find("replace") != params.end())
    {
        update.replace = std::stoi(params.at("replace")); // 转换为 int32_t
    } else {
        update.replace = 0; // 默认值，假设默认合并
    }

    return update;
}

std::unordered_map<std::string, std::string> CCINDEX_COMPONENT::parseGetParams(const httplib::Request &req)
{
    std::unordered_map<std::string, std::string> params;
    if(getConfiger().enableDebugPrint)
    {
        RADAR_STATIC_DEBUG_PRINT << "[path]" << req.path.c_str();
    }
    // 获取请求参数
    for (const auto& pair : req.params)
    {
        params[pair.first] = pair.second; // 直接从 req.params 提取键值对
    }
    // 找到 '?' 的位置
//    size_t questionMarkPos = req.path.find('?');
//    if (questionMarkPos != std::string::npos)
//    {
//        // 获取查询字符串
//        std::string queryString = req.path.substr(questionMarkPos + 1);
//
//        // 用 '&' 分割查询字符串
//        std::istringstream queryStream(queryString);
//        std::string pair;
//
//        while (std::getline(queryStream, pair, '&'))
//        {
//            // 用 '=' 分割键和值
//            size_t equalPos = pair.find('=');
//            if (equalPos != std::string::npos)
//            {
//                std::string key = pair.substr(0, equalPos);
//                std::string value = pair.substr(equalPos + 1);
//                params[key] = value; // 存储到字典中
//            }
//        }
//    }
    return params;
}
//////////////////////////////////////////////////////////////////////////////////////////
///
void CCINDEX_COMPONENT::mqttDispatchSubscribeMessageProto(std::string topic, void* data, int size)
{
    // if (!m_MqttConnectedCloud)
    // {
    //     return ;
    // }
    adu::st::TrafficInfos tis;
    std::vector<int> remove_tis_index;
    if(!tis.ParseFromArray(data, size))
    {
        RADAR_TC_ERROR_PRINT << "Recv radar Tc msg but proto parse fail";
        return;
    }

    m_recv_tc_count++;
    if(m_recv_tc_count == INT_MAX)
    {
        m_recv_tc_count == 0;
    }
    if(getConfiger().enableDebugPrintProto)
    {
        RADAR_TC_DEBUG_PRINT << "parse radar proto:" << tis.DebugString();
        RADAR_TC_DEBUG_PRINT << "recv radar tc count:" << m_recv_tc_count;
    }

    for(int i = 0; i < tis.traffic_info_size(); i++)
    {
        auto temp_info = tis.traffic_info().Get(i);

        if(!temp_info.has_header() || !temp_info.header().has_cross_id())
        {
            remove_tis_index.push_back(i);
            continue;
        }
        else
        {
            temp_info.set_cross_id(temp_info.header().cross_id());
        }

        if(!temp_info.has_time_type() || !temp_info.has_cross_id() || !temp_info.has_branch_id() || !temp_info.has_space_type())
        {
            remove_tis_index.push_back(i);
            continue;
        }

        if(getConfiger().enableDebugPrintProto)
        {
            std::string tempinfoStr = temp_info.DebugString();
            size_t pos = tempinfoStr.find("timestamp_sec");
            if(pos == std::string::npos)
            {
                RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                continue;
            }

            std::string tempStr = tempinfoStr.substr(pos);
            std::string tempPrintStr;
            for(char tempchar : tempStr)
            {
                if(tempchar == '}')
                {
                    continue;
                }

                if(tempchar == '\n')
                {
                    tempPrintStr = tempPrintStr + ';';
                }
                else
                {
                    tempPrintStr = tempPrintStr + tempchar;
                }
            }

            RADAR_TC_DEBUG_PRINT << "parse proto from radar:" << tempPrintStr;
        }

        if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_1_SECOND)//车道级/流向级，雷达过来一秒钟级数据，不透传
        {
            remove_tis_index.push_back(i);
            if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW)//流向级数据存储
            {
                if(temp_info.flow_to_type_size() <= 0)
                {
                    RADAR_TC_ERROR_PRINT << "flow type not found";
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + std::to_string((int)temp_info.flow_to_type(0));

                if(temp_info.has_queue_count() && temp_info.has_queue_trans_count() && temp_info.has_queue_length())
                {
                    if(m_RecvCreditControlInfo_spatcycle_levelMap.find(id) == m_RecvCreditControlInfo_spatcycle_levelMap.end() || m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime == 0)
                    {
                        radarCreditControlInfo_spatcycle_level rcci;
                        rcci.updatetime = afl::util::TimeStamp::now(true).millSeconds();
                        m_RecvCreditControlInfo_spatcycle_levelMap[id] = rcci;
                        for(int j = 0; j < temp_info.flow_to_type_size(); j++)
                        {
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].flowtotype.push_back(temp_info.flow_to_type(j));
                        }
                    }

                    m_RecvCreditControlInfo_spatcycle_levelMap[id].crossid = temp_info.cross_id();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].branchid = temp_info.branch_id();

                    m_RecvCreditControlInfo_spatcycle_levelMap[id].queuecount = temp_info.queue_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].queuetranscount = temp_info.queue_trans_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].queuelength = temp_info.queue_length();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_flow();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_number();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime = afl::util::TimeStamp::now(true).millSeconds();
                }

            }
            else if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE)//车道级数据存储
            {
                if(!temp_info.has_lane_id() || !temp_info.has_lane_turn_type())
                {
                    RADAR_TC_ERROR_PRINT << "lane id not found";
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + temp_info.lane_id();
                if(temp_info.has_queue_count() && temp_info.has_queue_trans_count() && temp_info.has_car_count() && temp_info.has_car_trans_count())
                {
                    
                    if(m_RecvCreditControlInfo_spatcycle_levelMap.find(id) == m_RecvCreditControlInfo_spatcycle_levelMap.end() || m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime == 0)
                    {
                        radarCreditControlInfo_spatcycle_level rcci;
                        rcci.islanelevel = true;
                        rcci.crossid = temp_info.cross_id();
                        rcci.branchid = temp_info.branch_id();
                        rcci.laneid = temp_info.lane_id();
                        rcci.updatetime = afl::util::TimeStamp::now(true).millSeconds();
                        rcci.laneturntype = temp_info.lane_turn_type();
                        m_RecvCreditControlInfo_spatcycle_levelMap[id] = rcci;
                    }

                    m_RecvCreditControlInfo_spatcycle_levelMap[id].queuecount = temp_info.queue_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].queuetranscount = temp_info.queue_trans_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].carcount = temp_info.car_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].cartranscount = temp_info.car_trans_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_flow();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_number();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime = afl::util::TimeStamp::now(true).millSeconds();

                    std::string temp_headtimediffcartype;
                    std::string temp_headtimediff;
                    if(temp_info.has_head_time_diff_detail())
                    {
                        int temp_pos = temp_info.head_time_diff_detail().find(';');
                        if(temp_pos != std::string::npos)
                        {
                            temp_headtimediffcartype = temp_info.head_time_diff_detail().substr(0, temp_pos);
                            temp_headtimediff = temp_info.head_time_diff_detail().substr(temp_pos + 1);
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediffcartype = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediffcartype + "," + temp_headtimediffcartype;
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediff = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediff + "," + temp_headtimediff;
                            if(temp_info.has_head_time_diff_time())
                            {
                                m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimedifftime = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimedifftime + "," + temp_info.head_time_diff_time();
                            }
                        }
                    }
                }
            }

            // if(temp_info.has_ped_num() && temp_info.has_bicycle_num())
            // {
            //     std::string id = temp_info.cross_id() + temp_info.branch_id();
            //     if(m_RecvCreditControlInfo_5m_levelMap.find(id) == m_RecvCreditControlInfo_5m_levelMap.end() || m_RecvCreditControlInfo_5m_levelMap[id].updatetime == 0)
            //     {
            //         radarCreditControlInfo_5m_level rcci_5m;
            //         rcci_5m.crossid = temp_info.cross_id();
            //         rcci_5m.branchid = temp_info.branch_id();
            //         m_RecvCreditControlInfo_5m_levelMap[id] = rcci_5m;
            //     }

            //     m_RecvCreditControlInfo_5m_levelMap[id].pednum = m_RecvCreditControlInfo_5m_levelMap[id].pednum + temp_info.ped_num();
            //     m_RecvCreditControlInfo_5m_levelMap[id].bicyclenum = m_RecvCreditControlInfo_5m_levelMap[id].bicyclenum + temp_info.bicycle_num();
            //     m_RecvCreditControlInfo_5m_levelMap[id].updatetime = afl::util::TimeStamp::now(true).millSeconds();
            // }

        }
        else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_3_SECOND)//流向级/车道级，雷达过来三秒钟级数据存储，不给平台透传
        {
            remove_tis_index.push_back(i);
            if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW)
            {
                if(temp_info.flow_to_type_size() <= 0)
                {
                    RADAR_TC_ERROR_PRINT << "flow type not found";
                    remove_tis_index.push_back(i);
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + std::to_string((int)temp_info.flow_to_type(0));

                if(temp_info.has_traffic_flow() && temp_info.has_traffic_number())
                {
                    if(m_RecvCreditControlInfo_spatcycle_levelMap.find(id) == m_RecvCreditControlInfo_spatcycle_levelMap.end() || m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime == 0)
                    {
                        radarCreditControlInfo_spatcycle_level rcci;
                        rcci.updatetime = afl::util::TimeStamp::now(true).millSeconds();
                        m_RecvCreditControlInfo_spatcycle_levelMap[id] = rcci;
                        for(int j = 0; j < temp_info.flow_to_type_size(); j++)
                        {
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].flowtotype.push_back(temp_info.flow_to_type(j));
                        }
                    }

                    m_RecvCreditControlInfo_spatcycle_levelMap[id].crossid = temp_info.cross_id();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].branchid = temp_info.branch_id();

                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].queuecount = temp_info.queue_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].queuetranscount = temp_info.queue_trans_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].queuelength = temp_info.queue_length();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_flow();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber + temp_info.traffic_number();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime = afl::util::TimeStamp::now(true).millSeconds();
                }
            }
            else if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE)
            {
                if(!temp_info.has_lane_id() || !temp_info.has_lane_turn_type())
                {
                    RADAR_TC_ERROR_PRINT << "lane id not found";
                    remove_tis_index.push_back(i);
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + temp_info.lane_id();

                if(temp_info.has_traffic_flow() && temp_info.has_traffic_number())
                {
                    if(m_RecvCreditControlInfo_spatcycle_levelMap.find(id) == m_RecvCreditControlInfo_spatcycle_levelMap.end() || m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime == 0)
                    {
                        radarCreditControlInfo_spatcycle_level rcci;
                        rcci.islanelevel = true;
                        rcci.crossid = temp_info.cross_id();
                        rcci.branchid = temp_info.branch_id();
                        rcci.laneid = temp_info.lane_id();
                        rcci.updatetime = afl::util::TimeStamp::now(true).millSeconds();
                        rcci.laneturntype = temp_info.lane_turn_type();
                        m_RecvCreditControlInfo_spatcycle_levelMap[id] = rcci;
                    }

                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].queuecount = temp_info.queue_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].queuetranscount = temp_info.queue_trans_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].carcount = temp_info.car_count();
                    // m_RecvCreditControlInfo_spatcycle_levelMap[id].cartranscount = temp_info.car_trans_count();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow + temp_info.traffic_flow();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber + temp_info.traffic_number();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow_5s = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficflow_5s + temp_info.traffic_flow();
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber_5s = m_RecvCreditControlInfo_spatcycle_levelMap[id].trafficnumber_5s + temp_info.traffic_number();                    
                    m_RecvCreditControlInfo_spatcycle_levelMap[id].updatetime = afl::util::TimeStamp::now(true).millSeconds();

                    std::string temp_headtimediffcartype;
                    std::string temp_headtimediff;
                    if(temp_info.has_head_time_diff_detail())
                    {
                        int temp_pos = temp_info.head_time_diff_detail().find(';');
                        if(temp_pos != std::string::npos)
                        {
                            temp_headtimediffcartype = temp_info.head_time_diff_detail().substr(0, temp_pos);
                            temp_headtimediff = temp_info.head_time_diff_detail().substr(temp_pos + 1);
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediffcartype = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediffcartype + "," + temp_headtimediffcartype;
                            m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediff = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimediff + "," + temp_headtimediff;
                            if(temp_info.has_head_time_diff_time())
                            {
                                m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimedifftime = m_RecvCreditControlInfo_spatcycle_levelMap[id].headtimedifftime + "," + temp_info.head_time_diff_time();
                            }
                        }
                    }
                }
            }
        }
        else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_SECOND)//车道级，雷达过来五秒钟级数据透传给平台
        {

        }
        else if(temp_info.time_type() == adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_MINUTE)//车道级/流向级，雷达过来的五分钟级数据
        {
            if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW)
            {
                if(temp_info.flow_to_type_size() <= 0)
                {
                    RADAR_TC_ERROR_PRINT << "flow type not found";
                    remove_tis_index.push_back(i);
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + std::to_string((int)temp_info.flow_to_type(0));
                if(m_RecvCreditControlInfo_5m_levelMap.find(id) == m_RecvCreditControlInfo_5m_levelMap.end())
                {
                    radarCreditControlInfo_5m_level rcci_5m;
                    rcci_5m.islanelevel = false;
                    rcci_5m.crossid = temp_info.cross_id();
                    rcci_5m.branchid = temp_info.branch_id();
                    m_RecvCreditControlInfo_5m_levelMap[id] = rcci_5m;
                    for(int j = 0; j < temp_info.flow_to_type_size(); j++)
                    {
                        m_RecvCreditControlInfo_5m_levelMap[id].flowtotype.push_back(temp_info.flow_to_type(j));
                    }
                }
                m_RecvCreditControlInfo_5m_levelMap[id].updatetime = temp_info.header().timestamp_sec()*1000;
                m_RecvCreditControlInfo_5m_levelMap[id].trafficflow = temp_info.traffic_flow();
                m_RecvCreditControlInfo_5m_levelMap[id].trafficnumber = temp_info.traffic_number();
                if(getConfiger().enableMec)
                {
                    remove_tis_index.push_back(i);//若存储5分钟数据则删掉不透传
                }
            }
            else if(temp_info.space_type() == adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE)
            {
                if(!temp_info.has_lane_id())
                {
                    RADAR_TC_ERROR_PRINT << "lane id not found";
                    remove_tis_index.push_back(i);
                    continue;
                }

                std::string id = temp_info.cross_id() + temp_info.branch_id() + temp_info.lane_id();
                if(temp_info.has_traffic_flow() && m_greenTimeCountMap.find(id) != m_greenTimeCountMap.end() && m_greenTimeCountMap[id] != -1)
                {
                    double traffic_flow = temp_info.traffic_flow();
                    double wast_time = std::round((m_greenTimeCountMap[id] - 2*traffic_flow)*10)/10;//五分钟内绿灯总时长-2*交通流量（当量数）
                    if(wast_time < 0)
                    {
                        wast_time = 0;
                    }
                    tis.mutable_traffic_info(i)->set_waste_time(wast_time);
                    tis.mutable_traffic_info(i)->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_WASTE_TIME);
                    m_greenTimeCountMap[id] = -1;//赋默认值，重新记录绿灯时长
                }
                //保存非出口道车道级数据
                if(std::find(m_outBranchList.begin(), m_outBranchList.end(), temp_info.branch_id()) == m_outBranchList.end() && temp_info.has_traffic_flow() && temp_info.has_traffic_number())
                {
                    if(m_RecvCreditControlInfo_5m_levelMap.find(id) == m_RecvCreditControlInfo_5m_levelMap.end())
                    {
                        radarCreditControlInfo_5m_level rcci_5m;
                        rcci_5m.islanelevel = true;
                        rcci_5m.crossid = temp_info.cross_id();
                        rcci_5m.branchid = temp_info.branch_id();
                        rcci_5m.laneturntype = temp_info.lane_turn_type();
                        rcci_5m.laneid = temp_info.lane_id();
                        m_RecvCreditControlInfo_5m_levelMap[id] = rcci_5m;
                    }
                    m_RecvCreditControlInfo_5m_levelMap[id].updatetime = temp_info.header().timestamp_sec()*1000;
                    m_RecvCreditControlInfo_5m_levelMap[id].trafficflow = temp_info.traffic_flow();
                    m_RecvCreditControlInfo_5m_levelMap[id].trafficnumber = temp_info.traffic_number();
                    m_RecvCreditControlInfo_5m_levelMap[id].headtimediffdetail = temp_info.head_time_diff_detail();
                    if(tis.mutable_traffic_info(i)->has_waste_time())
                    {
                        m_RecvCreditControlInfo_5m_levelMap[id].waste_time = temp_info.waste_time();
                    }
                    if(getConfiger().enableMec)
                    {
                        remove_tis_index.push_back(i);//若存储5分钟数据则删掉不透传
                    }
                }
            }
            else
            {
                //透传
            }

        }
    }

    //删掉部分数据
    for(int tempi = 0; tempi < remove_tis_index.size(); tempi++)
    {
        tis.mutable_traffic_info()->DeleteSubrange(remove_tis_index[tempi] - tempi, 1);
    }

    if(tis.traffic_info_size() < 1)
    {
        RADAR_TC_DEBUG_PRINT << "traffic info size is 0, do not send to cloud!";
        return;
    }
    else
    {
        m_needsend_tc_count++;
        if(m_needsend_tc_count == INT_MAX)
        {
            m_needsend_tc_count = 0;
        }
        if(this->getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "need send radar tc count:" << m_needsend_tc_count;
        }
    }
    std::string trafficinfos = tis.SerializeAsString();

    std::string hint = "tc";
    std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
    //新增
    static int message_count_tc_channel = 1; // 用于计数的静态变量

    // 根据消息计数判断是偶数还是奇数
    if (message_count_tc_channel % 2 == 0) // 偶数
    {
        if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
        {
            RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
        }
        else
        {
            m_send_tc_count++;
            if(m_send_tc_count == INT_MAX)  
            {
                m_send_tc_count = 0;
            }
            if(this->getConfiger().enableDebugPrintProto)
            {
                RADAR_TC_DEBUG_PRINT << "send radar tc count:" << m_send_tc_count;
                RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                if(this->getConfiger().enableDebugPrintProto)
                {
                    for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                    {
                        auto temp_info = tis.traffic_info().Get(tempi);
                        std::string tempinfoStr = temp_info.DebugString();
                        size_t pos = tempinfoStr.find("timestamp_sec");
                        if(pos == std::string::npos)
                        {
                            RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                            continue;
                        }

                        std::string tempStr = tempinfoStr.substr(pos);
                        std::string tempPrintStr;
                        for(char tempchar : tempStr)
                        {
                            if(tempchar == '}')
                            {
                                continue;
                            }

                            if(tempchar == '\n')
                            {
                                tempPrintStr = tempPrintStr + ';';
                            }
                            else
                            {
                                tempPrintStr = tempPrintStr + tempchar;
                            }
                        }

                        RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                    }

                }
            }
        }
        // m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
        //     {
        //         if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
        //         {
        //             RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
        //         }
        //         else
        //         {
        //             m_send_tc_count++;
        //             if(m_send_tc_count == INT_MAX)  
        //             {
        //                 m_send_tc_count = 0;
        //             }
        //             if(this->getConfiger().enableDebugPrintProto)
        //             {
        //                 RADAR_TC_DEBUG_PRINT << "send radar tc count:" << m_send_tc_count;
        //                 RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        //                 if(this->getConfiger().enableDebugPrintProto)
        //                 {
        //                     for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
        //                     {
        //                         auto temp_info = tis.traffic_info().Get(tempi);
        //                         std::string tempinfoStr = temp_info.DebugString();
        //                         size_t pos = tempinfoStr.find("timestamp_sec");
        //                         if(pos == std::string::npos)
        //                         {
        //                             RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
        //                             continue;
        //                         }

        //                         std::string tempStr = tempinfoStr.substr(pos);
        //                         std::string tempPrintStr;
        //                         for(char tempchar : tempStr)
        //                         {
        //                             if(tempchar == '}')
        //                             {
        //                                 continue;
        //                             }

        //                             if(tempchar == '\n')
        //                             {
        //                                 tempPrintStr = tempPrintStr + ';';
        //                             }
        //                             else
        //                             {
        //                                 tempPrintStr = tempPrintStr + tempchar;
        //                             }
        //                         }

        //                         RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
        //                     }

        //                 }
        //             }
        //         }
        //     });
    }
    else // 奇数
    {

        if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
        {
            RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
        }
        else
        {
            m_send_tc_count++;
            if(m_send_tc_count == INT_MAX)  
            {
                m_send_tc_count = 0;
            }
            if(this->getConfiger().enableDebugPrintProto)
            {
                RADAR_TC_DEBUG_PRINT << "send radar tc count:" << m_send_tc_count;
                RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                {
                    auto temp_info = tis.traffic_info().Get(tempi);
                    std::string tempinfoStr = temp_info.DebugString();
                    size_t pos = tempinfoStr.find("timestamp_sec");
                    if(pos == std::string::npos)
                    {
                        RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                        continue;
                    }

                    std::string tempStr = tempinfoStr.substr(pos);
                    std::string tempPrintStr;
                    for(char tempchar : tempStr)
                    {
                        if(tempchar == '}')
                        {
                            continue;
                        }

                        if(tempchar == '\n')
                        {
                            tempPrintStr = tempPrintStr + ';';
                        }
                        else
                        {
                            tempPrintStr = tempPrintStr + tempchar;
                        }
                    }

                    RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                }
            }
        }

        // m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
        // {
        //     if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
        //     {
        //         RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
        //     }
        //     else
        //     {
        //         m_send_tc_count++;
        //         if(m_send_tc_count == INT_MAX)  
        //         {
        //             m_send_tc_count = 0;
        //         }
        //         if(this->getConfiger().enableDebugPrintProto)
        //         {
        //             RADAR_TC_DEBUG_PRINT << "send radar tc count:" << m_send_tc_count;
        //             RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        //             for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
        //             {
        //                 auto temp_info = tis.traffic_info().Get(tempi);
        //                 std::string tempinfoStr = temp_info.DebugString();
        //                 size_t pos = tempinfoStr.find("timestamp_sec");
        //                 if(pos == std::string::npos)
        //                 {
        //                     RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
        //                     continue;
        //                 }

        //                 std::string tempStr = tempinfoStr.substr(pos);
        //                 std::string tempPrintStr;
        //                 for(char tempchar : tempStr)
        //                 {
        //                     if(tempchar == '}')
        //                     {
        //                         continue;
        //                     }

        //                     if(tempchar == '\n')
        //                     {
        //                         tempPrintStr = tempPrintStr + ';';
        //                     }
        //                     else
        //                     {
        //                         tempPrintStr = tempPrintStr + tempchar;
        //                     }
        //                 }

        //                 RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
        //             }
        //         }
        //     }
        // });
    }
    if (message_count_tc_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
    {
        message_count_tc_channel = 0; // 重置计数器
    }
    else
    {
        message_count_tc_channel++;
    }

}

bool CCINDEX_COMPONENT::udpInit()
{
	m_Udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_Udpfd < 0)
	{
		RADAR_TC_ERROR_PRINT << "Create socket error! reason:" << strerror(errno);
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	addr.sin_port = htons(50500);

	if(bind(m_Udpfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		RADAR_TC_ERROR_PRINT << "Socket bind error! reason: " << strerror(errno);
		return false;
	}

	m_UdpChnl.reset(new afl::net::Channel(m_EventloopTc.get(), m_Udpfd));
	m_UdpChnl->setReadCallback(std::bind(&CCINDEX_COMPONENT::readUdpData, this) );
	m_UdpChnl->enableReading();

	return true;
}

void CCINDEX_COMPONENT::readUdpData()
{
    ssize_t recv_len = 0;
    struct sockaddr_in recv_addr;
    socklen_t sock_len;
    sock_len = sizeof(recv_addr);
    recv_len = recvfrom(m_Udpfd, m_UdpBuffer->beginWrite(), m_UdpBuffer->writableBytes(), 0, (struct sockaddr *) &recv_addr, &sock_len);

    if (recv_len < 48)
    {
		RADAR_TC_ERROR_PRINT << "recv udp data len error(must > 48):" << recv_len;
        return;
    }
    m_UdpBuffer->hasWritten(recv_len);

    uint16_t type = uint16_t(*(m_UdpBuffer->peek() + 26)) | uint16_t(*(m_UdpBuffer->peek() + 27) << 8);
    if(type != 0x0103)
    {
		RADAR_TC_ERROR_PRINT << "recv udp data type error(must be 0x0103):" << type;
		m_UdpBuffer->retrieveAll();
    	return;
    }
    // printf(std::endl);
    // for(int i = 0; i < m_UdpBuffer->readableBytes(); i++)
    // {
    //     printf("%02x ", *(m_UdpBuffer->peek() + i));
    // }
    // printf(std::endl);

    uint8_t entrance_cnt = *(m_UdpBuffer->peek() + 44);

    int offset = 45;
    uint8_t light_cnt = 0;
    uint8_t light_type = 0;
    uint8_t light_color = 0;
    uint8_t light_countdown = 0;
    uint8_t light_id = 0;
    uint16_t tmp_entry_direction = 0; // 车道进口方向
    uint8_t phase_id_high = 0;  // 根据方向计算出高位ID

    adu::st::TrafficInfos tis;
	tis.mutable_traffic_info();

    for(const auto& kiter : m_RecvCreditControlInfo_spatcycle_levelMap)
    {
        RADAR_TC_DEBUG_PRINT << "CreditControlInfo: " << kiter.first;
    }

    uint64_t nowtime = afl::util::TimeStamp::now(true).millSeconds();
    for(uint8_t i = 0; i < entrance_cnt; i++)
    {
		if((offset + 3) >= recv_len)
		{
			RADAR_TC_ERROR_PRINT << "curr period error, frame data len illegal.";
			m_UdpBuffer->retrieveAll();
			return;
		}

        tmp_entry_direction = uint16_t(*(m_UdpBuffer->peek() + offset) | (*(m_UdpBuffer->peek() + offset + 1) << 8));
        phase_id_high = static_cast<uint8_t>(std::floor(tmp_entry_direction / 22.5));
		light_cnt = *(m_UdpBuffer->peek() + offset + 2);
		offset += 3;


		for(uint8_t j = 0; j < light_cnt; j++)
		{
			if ((offset + 4) >= recv_len)
			{
				RADAR_TC_ERROR_PRINT << "curr period error, frame data len illegal.";
				m_UdpBuffer->retrieveAll();
				return;
			}
			light_type  = *(m_UdpBuffer->peek() + offset + 1);
			light_color = *(m_UdpBuffer->peek() + offset + 2);
			light_color = CovertGatLightColor(light_color);

            // 根据<<T_ITS 0117-2022合作式智能运输系统 RSU与中心子系统间数据接口规范>> 附录E
            // 对phaseId进行赋值, 高4bit为进口道方向, 低4位为灯组类型
			light_id = (phase_id_high << 4) | (light_type & 0x0F);

            if(getConfiger().enableDebugPrintProto)
            {
                printf("phase id: %02x, color: %d\n", light_id, light_color);
            }

			if(light_color == 0)
			{
                if(getConfiger().enableDebugPrintProto)
                {
                    RADAR_TC_ERROR_PRINT << "light state invalid, light_id " << light_id << ", color is zero.";
                }
			}
			light_countdown = *(m_UdpBuffer->peek() + offset + 3);

            if(m_LaneOrFlowPhaseIdMap.find(light_id) == m_LaneOrFlowPhaseIdMap.end())
            {
                // RADAR_TC_DEBUG_PRINT << "phase id not needed";
                offset += 4;
                continue;
            }
            if(getConfiger().enableDebugPrintProto)
            {
                RADAR_TC_DEBUG_PRINT << "get phase id:" << light_id;
            }
			//统计五分钟内各个车道、流向的绿灯时长,红绿灯0.1s推送一次，故每次收到后统计时长加0.1s
			if(light_color == 1)
			{
                for(int index = 0; index < (int)m_LaneOrFlowPhaseIdMap[light_id].size(); index++)
                {
                    if(m_greenTimeCountMap.find(m_LaneOrFlowPhaseIdMap[light_id][index]) != m_greenTimeCountMap.end())
                    {
                        if(m_greenTimeCountMap[m_LaneOrFlowPhaseIdMap[light_id][index]] == -1)
                        {
                            m_greenTimeCountMap[m_LaneOrFlowPhaseIdMap[light_id][index]] = 0.1;
                        }
                        else
                        {
                            m_greenTimeCountMap[m_LaneOrFlowPhaseIdMap[light_id][index]] = m_greenTimeCountMap[m_LaneOrFlowPhaseIdMap[light_id][index]] + 0.1;
                        }
                    }
                    else
                    {
                        m_greenTimeCountMap[m_LaneOrFlowPhaseIdMap[light_id][index]] = 0.1;
                    }
                }
			}

			bool startgreen = ((m_phaseInfoMap[light_id].first == 3) && light_color == 1);//绿灯启亮
			bool startred = ((m_phaseInfoMap[light_id].first == 1 || m_phaseInfoMap[light_id].first == 2) && light_color == 3);//红灯启亮
            bool startyellow = (m_phaseInfoMap[light_id].first == 1 && light_color == 2);//黄灯启亮
            m_phaseInfoMap[light_id].first = light_color;
            if(startgreen)
            {
                m_phaseInfoMap[light_id].second.greentime = light_countdown;
            }

            if(startred)
            {
                m_phaseInfoMap[light_id].second.redtime = light_countdown;
            }

            if(startyellow)
            {
                m_phaseInfoMap[light_id].second.yellowtime = light_countdown;
            }


            if(!startgreen && !startred)
            {
                // RADAR_TC_DEBUG_PRINT  << "light is not turn to red or green" ;
                offset += 4;
                continue;
            }

            //遍历所有绑定该phase id的lane or flow
            for(int index = 0; index < (int)m_LaneOrFlowPhaseIdMap[light_id].size(); index++)
            {
                if(m_RecvCreditControlInfo_spatcycle_levelMap.find(m_LaneOrFlowPhaseIdMap[light_id][index]) != m_RecvCreditControlInfo_spatcycle_levelMap.end())
                {
                    auto tempInfo = m_RecvCreditControlInfo_spatcycle_levelMap[m_LaneOrFlowPhaseIdMap[light_id][index]];
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "find phase id success! lane or flow level id:" << m_LaneOrFlowPhaseIdMap[light_id][index] << ", light id:" << light_id;
                    }
                    if(tempInfo.updatetime != 0)//雷达上报信控数据有效
                    {
                        if(getConfiger().enableDebugPrintProto)
                        {
                            RADAR_TC_DEBUG_PRINT << "crossid:" << tempInfo.crossid;
                            RADAR_TC_DEBUG_PRINT << "branchid:" << tempInfo.branchid;
                            RADAR_TC_DEBUG_PRINT << "islanelevel:" << tempInfo.islanelevel;
                            RADAR_TC_DEBUG_PRINT << "queuecount:" << tempInfo.queuecount;
                            RADAR_TC_DEBUG_PRINT << "queuetranscount:" << tempInfo.queuetranscount;
                        }
                        
                        if(tempInfo.islanelevel)//车道级：绿灯启亮or红灯启亮
                        {
                            if(startgreen)//周期性数据均在绿灯启亮时推送
                            {
                                auto ti = tis.add_traffic_info();
                                auto header = ti->mutable_header();
                                header->set_timestamp_sec((double)nowtime/1000);
                                header->set_cross_id(tempInfo.crossid);
                                header->set_module_name("V2X");
                                ti->set_normal(true);
                                ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE);
                                ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
                                ti->set_cross_id(tempInfo.crossid);
                                ti->set_branch_id(tempInfo.branchid);
                                ti->set_lane_id(tempInfo.laneid);
                                ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_CYCLE);

                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_GREEN_TIME);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_FLOW);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_NUMBER);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_COUNT_WHEN_GREEN);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_TRANS_COUNT_WHEN_GREEN);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_COUNT_WHEN_RED);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_TRANS_COUNT_WHEN_RED);
                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_HEAD_TIME_DIFF);
                                // ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_HEAD_TIME_DIFF_TIME);

                                ti->mutable_flow_to_type();
                                if(tempInfo.laneturntype == 1 || tempInfo.laneturntype == 5 || tempInfo.laneturntype == 7 || tempInfo.laneturntype == 8 ||
                                    tempInfo.laneturntype == 9 ||tempInfo.laneturntype == 12 ||tempInfo.laneturntype == 15 ||tempInfo.laneturntype == 16)//直行
                                {
                                    ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(1));
                                }
                                if(tempInfo.laneturntype == 2 || tempInfo.laneturntype == 5 || tempInfo.laneturntype == 6 || tempInfo.laneturntype == 9 ||
                                    tempInfo.laneturntype == 11 ||tempInfo.laneturntype == 12 ||tempInfo.laneturntype == 14 ||tempInfo.laneturntype == 16)//左转
                                {
                                    ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(2));
                                }
                                if(tempInfo.laneturntype == 3 || tempInfo.laneturntype == 8 || tempInfo.laneturntype == 11 || tempInfo.laneturntype == 12 ||
                                    tempInfo.laneturntype == 13 ||tempInfo.laneturntype == 14 ||tempInfo.laneturntype == 15 ||tempInfo.laneturntype == 16)//右转
                                {
                                    ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(3));
                                }
                                if(tempInfo.laneturntype == 4 || tempInfo.laneturntype == 6 || tempInfo.laneturntype == 7 || tempInfo.laneturntype == 9 ||
                                    tempInfo.laneturntype == 13 ||tempInfo.laneturntype == 14 ||tempInfo.laneturntype == 15 ||tempInfo.laneturntype == 16)//调头
                                {
                                    ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(4));
                                }                                                    


                                double cycletime = m_phaseInfoMap[light_id].second.greentime + m_phaseInfoMap[light_id].second.yellowtime + m_phaseInfoMap[light_id].second.redtime;
                                ti->set_green_time(m_phaseInfoMap[light_id].second.greentime);
                                ti->set_start_time((double)nowtime/1000 - cycletime);
                                ti->set_end_time((double)nowtime/1000);
                                ti->set_time_span((double)cycletime);
                                ti->set_lane_turn_type((adu::stb_map::LaneTurnType)tempInfo.laneturntype);

                                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_WASTE_TIME);
                                ti->set_car_count_when_green(tempInfo.carcount);
                                ti->set_car_trans_count_when_green(tempInfo.cartranscount);
                                ti->set_car_count_when_red(tempInfo.carcount_when_red);
                                ti->set_car_trans_count_when_red(tempInfo.cartranscount_when_red);
                                ti->set_traffic_flow(tempInfo.trafficflow);
                                ti->set_traffic_number(tempInfo.trafficnumber);

                                // if(tempInfo.headtimedifftime != "")
                                // {
                                //     ti->set_head_time_diff_time(tempInfo.headtimedifftime);
                                // }
                                // else
                                // {
                                //     ti->set_head_time_diff_time("0");
                                // }

                                if(tempInfo.headtimediff != "")
                                {
                                    ti->set_head_time_diff_detail(fixHeadtimediffdetail((tempInfo.headtimediffcartype + ";" + tempInfo.headtimediff), tempInfo.trafficnumber));
                                }
                                else
                                {
                                    ti->set_head_time_diff_detail("0");
                                }

                                double wast_time = std::round((m_phaseInfoMap[light_id].second.greentime - 2*tempInfo.trafficflow)*10)/10;//周期内绿灯时长-2*交通流量（当量数）
                                if(wast_time < 0)
                                {
                                    wast_time = 0;
                                }
                                ti->set_waste_time(wast_time);

                                radarCreditControlInfo_spatcycle_level new_rcci;
                                m_RecvCreditControlInfo_spatcycle_levelMap[m_LaneOrFlowPhaseIdMap[light_id][index]] = new_rcci;//周期清空数据
                            }
                            else
                            {
                                // ti->set_car_count_when_green(0);
                                // ti->set_car_trans_count_when_green(0);
                                // ti->set_car_count_when_red(tempInfo.carcount);
                                // ti->set_car_trans_count_when_red(tempInfo.cartranscount);
                                m_RecvCreditControlInfo_spatcycle_levelMap[m_LaneOrFlowPhaseIdMap[light_id][index]].carcount_when_red = tempInfo.carcount;
                                m_RecvCreditControlInfo_spatcycle_levelMap[m_LaneOrFlowPhaseIdMap[light_id][index]].cartranscount_when_red = tempInfo.cartranscount;
                            }

                        }
                        else if(startgreen)//流向级：绿灯启亮
                        {
                            auto ti = tis.add_traffic_info();
                            auto header = ti->mutable_header();
                            header->set_timestamp_sec((double)nowtime/1000);
                            header->set_cross_id(tempInfo.crossid);
                            header->set_module_name("V2X");

                            ti->set_normal(true);
                            ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW);
                            ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
                            ti->set_cross_id(tempInfo.crossid);
                            ti->set_branch_id(tempInfo.branchid);
                            ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_CYCLE);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_FLOW);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_NUMBER);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_COUNT);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_TRANS_COUNT);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_WASTE_TIME);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_LENGTH);
                            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_GREEN_TIME);

                            ti->mutable_flow_to_type();
                            for(int j = 0; j < tempInfo.flowtotype.size(); j++)
                            {
                                ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(tempInfo.flowtotype[j]));
                            }

                            double cycletime = m_phaseInfoMap[light_id].second.greentime + m_phaseInfoMap[light_id].second.yellowtime + m_phaseInfoMap[light_id].second.redtime;
                            ti->set_green_time(m_phaseInfoMap[light_id].second.greentime);
                            ti->set_start_time((double)nowtime/1000 - cycletime);
                            ti->set_end_time((double)nowtime/1000);
                            ti->set_time_span((double)cycletime);
                            ti->set_queue_count(tempInfo.queuecount);
                            ti->set_queue_trans_count(tempInfo.queuetranscount);
                            ti->set_queue_length(tempInfo.queuelength);
                            ti->set_traffic_flow(tempInfo.trafficflow);
                            ti->set_traffic_number(tempInfo.trafficnumber);
                            double wast_time = std::round((m_phaseInfoMap[light_id].second.greentime - 2*tempInfo.trafficflow)*10)/10;//周期内绿灯时长-2*交通流量（当量数）
                            if(wast_time < 0)
                            {
                                wast_time = 0;
                            }
                            ti->set_waste_time(wast_time);

                            radarCreditControlInfo_spatcycle_level new_rcci;
                            m_RecvCreditControlInfo_spatcycle_levelMap[m_LaneOrFlowPhaseIdMap[light_id][index]] = new_rcci;//周期清空数据
                        }
                    }
                }

            }
			offset += 4;
		}
    }

    if(tis.traffic_info_size() > 0)
    {
        for(int i = 0; i < tis.traffic_info_size(); i++)
        {
            tis.mutable_traffic_info(i)->mutable_header()->set_sequence_num(m_spat_cycle_seqnum);
        }
        m_spat_cycle_seqnum++;
        if(m_spat_cycle_seqnum == UINT64_MAX)
        {
            m_spat_cycle_seqnum = 0;
        }

    	std::string trafficinfos = tis.SerializeAsString();

        if(getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "ready to send traffic light proto:" << tis.DebugString();
        }
    	std::string hint = "tc";

//    	mqttPushStringMsg2Broker(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc, trafficinfos, hint);

        static int message_count_tc_udp_channel = 1; // 用于计数的静态变量
        std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        // 根据消息计数判断是偶数还是奇数
        if (message_count_tc_udp_channel % 2 == 0) // 偶数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {

               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(this->getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar with trafficlight proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        else // 奇数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar with trafficlight proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        if (message_count_tc_udp_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
        {
            message_count_tc_udp_channel = 0; // 重置计数器
        }
        else
        {
            message_count_tc_udp_channel++;
        }

    }

    m_UdpBuffer->retrieveAll();
}

uint8_t CCINDEX_COMPONENT::CovertGatLightColor(uint8_t gat_light_color)
{
  uint8_t std_light_color = 0;
  if (gat_light_color & 0x2)
  {  // red
    std_light_color = 3;
  }
  else if ((gat_light_color >> 2) & 0x2)
  {  // yellow
    std_light_color = 2;
  }
  else if ((gat_light_color >> 4) & 0x2)
  {  // green
    std_light_color = 1;
  }

  return std_light_color;
}

std::string CCINDEX_COMPONENT::fixHeadtimediffdetail(std::string head_time_diff_detail, int count)
{
    std::string fix_head_time_diff_detail = "";
    size_t semicolon_pos = head_time_diff_detail.find(';');
    if(semicolon_pos == std::string::npos)
    {
        return fix_head_time_diff_detail;
        RADAR_TC_DEBUG_PRINT << "head_time_diff_detail string error";

    }
    std::string vehicle_part = head_time_diff_detail.substr(0, semicolon_pos);
    std::string time_part = head_time_diff_detail.substr(semicolon_pos);

    // 分割车辆列表
    std::vector<std::string> vehicles;
    std::vector<std::string> times;
    std::stringstream ss_vp(vehicle_part);
    std::stringstream ss_tp(time_part);

    std::string item;
    while (std::getline(ss_vp, item, ',')) 
    {
        vehicles.push_back(item);
    }
    while (std::getline(ss_tp, item, ',')) 
    {
        times.push_back(item);
    }

    std::string fix_vehicle_part = "";
    std::string fix_time_part = "";
    
    if(vehicles.size() <= count)
    {
        fix_head_time_diff_detail = head_time_diff_detail;
        RADAR_TC_DEBUG_PRINT << "head_time_diff_detail fix count error";
    }
    else
    {
        for(int i = 0; i < count; i++)
        {
            if(i == 0)
            {
                fix_vehicle_part = vehicles[0];
                fix_time_part = times[0];
            }
            else
            {
                fix_vehicle_part = fix_vehicle_part + "," + vehicles[i];
                fix_time_part = fix_time_part + "," + times[i];
            }
        }
        fix_head_time_diff_detail = fix_vehicle_part + fix_time_part;
    }

    RADAR_TC_DEBUG_PRINT << "head_time_diff_detail : " << head_time_diff_detail;
    RADAR_TC_DEBUG_PRINT << "count : " << count;    
    RADAR_TC_DEBUG_PRINT << "fixed head_time_diff_detail : " << fix_head_time_diff_detail;

    return fix_head_time_diff_detail;
}

void CCINDEX_COMPONENT::send_5m_level_data()
{
    uint64_t nowtime = afl::util::TimeStamp::now(true).millSeconds();
    adu::st::TrafficInfos tis;
    tis.mutable_traffic_info();
    // 添加5m级数据
    for(const auto& temp_iter : m_RecvCreditControlInfo_5m_levelMap)
    {
        if(temp_iter.second.updatetime != 0 && (nowtime - temp_iter.second.updatetime > 60*5*1000))//雷达5分钟数据，更新时间不为0且超过5m不上报平台
        {
            continue;
        }

        double temp_traffic_number = temp_iter.second.trafficnumber;
        double temp_traffic_flow = temp_iter.second.trafficflow;
        uint64_t updatetime = temp_iter.second.updatetime;
        if(m_RecvMecInfo_5m_levelMap.find(temp_iter.first) != m_RecvMecInfo_5m_levelMap.end() && (nowtime - m_RecvMecInfo_5m_levelMap[temp_iter.first].updatetime.load() <= 60*5*1000))
        {
            temp_traffic_number = m_RecvMecInfo_5m_levelMap[temp_iter.first].mec_traffic_number.load();
            temp_traffic_flow = m_RecvMecInfo_5m_levelMap[temp_iter.first].mec_traffic_flow.load();
            updatetime = m_RecvMecInfo_5m_levelMap[temp_iter.first].updatetime.load();
        }
        else
        {
            if(temp_iter.second.updatetime == 0)//雷达和感知算法都没有有效数据，不往云控上报
            {
                continue;
            }
        }

        if(temp_iter.second.islanelevel)//车道级5m级数据
        {

            auto ti = tis.add_traffic_info();
            auto header = ti->mutable_header();
            header->set_timestamp_sec((double)updatetime/1000);
            header->set_cross_id(temp_iter.second.crossid);
            header->set_module_name("V2X");
            ti->set_normal(true);
            ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE);
            ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
            ti->set_cross_id(temp_iter.second.crossid);
            ti->set_branch_id(temp_iter.second.branchid);
            ti->set_lane_id(temp_iter.second.laneid);
            ti->set_lane_turn_type((adu::stb_map::LaneTurnType)temp_iter.second.laneturntype);
            ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_MINUTE);
            ti->set_start_time((double)updatetime/1000 - 5*60);
            ti->set_end_time((double)updatetime/1000);
            ti->set_time_span(300);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_NUMBER);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_FLOW);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_HEAD_TIME_DIFF);


            ti->set_traffic_number(temp_traffic_number);
            ti->set_traffic_flow(temp_traffic_flow);
            ti->set_head_time_diff_detail(fixHeadtimediffdetail(temp_iter.second.headtimediffdetail, temp_traffic_number));
            if(temp_iter.second.waste_time != -1)
            {
                ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_WASTE_TIME);
                ti->set_waste_time(temp_iter.second.waste_time);
            }
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TC_DEBUG_PRINT << "5m V2X data, crossid: " << temp_iter.second.crossid << ",branchid: "<< temp_iter.second.branchid <<", laneid: " << temp_iter.second.laneid 
                << ", radar traffic number: " << temp_iter.second.trafficnumber << ", radar traffic flow: " << temp_iter.second.trafficflow
                << ", send traffic number: " << temp_traffic_number << ", send traffic flow: " << temp_traffic_flow;
            }
        }
        else//流向级5m级数据
        {
            auto ti = tis.add_traffic_info();
            auto header = ti->mutable_header();
            header->set_timestamp_sec((double)updatetime/1000);
            header->set_cross_id(temp_iter.second.crossid);
            header->set_module_name("V2X");
            ti->set_normal(true);
            ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW);
            ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
            ti->set_cross_id(temp_iter.second.crossid);
            ti->set_branch_id(temp_iter.second.branchid);
            ti->mutable_flow_to_type();
            for(int j = 0; j < temp_iter.second.flowtotype.size(); j++)
            {
                ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(temp_iter.second.flowtotype[j]));
            }
            ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_MINUTE);
            ti->set_start_time((double)updatetime/1000 - 5*60);
            ti->set_end_time((double)updatetime/1000);
            ti->set_time_span(300);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_NUMBER);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_FLOW);

            ti->set_traffic_number(temp_traffic_number);
            ti->set_traffic_flow(temp_traffic_flow);
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TC_DEBUG_PRINT << "5m V2X data, crossid: " << temp_iter.second.crossid << ",branchid: "<< temp_iter.second.branchid <<", flow type: " << temp_iter.second.flowtotype[0] 
                << ", radar traffic number: " << temp_iter.second.trafficnumber << ", radar traffic flow: " << temp_iter.second.trafficflow
                << ", send traffic number: " << temp_traffic_number << ", send traffic flow: " << temp_traffic_flow;
            }
        }
    }
    m_RecvMecInfo_5m_levelMap.clear();
    
    if(tis.traffic_info_size() > 0)
    {
        std::string trafficinfos = tis.SerializeAsString();

        std::string hint = "V2X信控指标";
        if(getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "ready to send 5m V2X proto:" << tis.DebugString();
        }
//    	mqttPushStringMsg2Broker(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc, trafficinfos, hint);

        static int message_count_tc_5m_channel = 1; // 用于计数的静态变量
        std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        // 根据消息计数判断是偶数还是奇数
        if (message_count_tc_5m_channel % 2 == 0) // 偶数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {

               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {    
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                        {
                           auto temp_info = tis.traffic_info().Get(tempi);
                           std::string tempinfoStr = temp_info.DebugString();
                           size_t pos = tempinfoStr.find("timestamp_sec");
                           if(pos == std::string::npos)
                           {
                               RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                               continue;
                           }

                           std::string tempStr = tempinfoStr.substr(pos);
                           std::string tempPrintStr;
                           for(char tempchar : tempStr)
                           {
                               if(tempchar == '}')
                               {
                                   continue;
                               }

                               if(tempchar == '\n')
                               {
                                   tempPrintStr = tempPrintStr + ';';
                               }
                               else
                               {
                                   tempPrintStr = tempPrintStr + tempchar;
                               }
                           }

                           RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                       }
                   }

               }
           });
        }
        else // 奇数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;

                       for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                       {
                           auto temp_info = tis.traffic_info().Get(tempi);
                           std::string tempinfoStr = temp_info.DebugString();
                           size_t pos = tempinfoStr.find("timestamp_sec");
                           if(pos == std::string::npos)
                           {
                               RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                               continue;
                           }

                           std::string tempStr = tempinfoStr.substr(pos);
                           std::string tempPrintStr;
                           for(char tempchar : tempStr)
                           {
                               if(tempchar == '}')
                               {
                                   continue;
                               }

                               if(tempchar == '\n')
                               {
                                   tempPrintStr = tempPrintStr + ';';
                               }
                               else
                               {
                                   tempPrintStr = tempPrintStr + tempchar;
                               }
                           }

                           RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                       }
                   }
               }
           });
        }
        if (message_count_tc_5m_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
        {
            message_count_tc_5m_channel = 0; // 重置计数器
        }
        else
        {
            message_count_tc_5m_channel++;
        }
    }
}

void CCINDEX_COMPONENT::send_5m_level_radardata()
{
    m_EventloopTc->addTimer(std::bind(&CCINDEX_COMPONENT::send_5m_level_data, this), 10, false);
}


void CCINDEX_COMPONENT::send_3s_level_radardata()
{
    uint64_t nowtime = afl::util::TimeStamp::now(true).millSeconds();
    adu::st::TrafficInfos tis;
    tis.mutable_traffic_info();
    //添加3s级数据
    for(const auto& temp_iter : m_RecvCreditControlInfo_spatcycle_levelMap)
    {
        if(nowtime - temp_iter.second.updatetime > 3000)//雷达1hz上报排队数据，超过3s的数据不上报平台
        {
            continue;
        }

        if(temp_iter.second.islanelevel)//车道级3s级数据
        {
            auto ti = tis.add_traffic_info();
            auto header = ti->mutable_header();
            header->set_timestamp_sec((double)nowtime/1000);
            header->set_cross_id(temp_iter.second.crossid);
            header->set_module_name("V2X");
            ti->set_normal(true);
            ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE);
            ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
            ti->set_cross_id(temp_iter.second.crossid);
            ti->set_branch_id(temp_iter.second.branchid);
            ti->set_lane_id(temp_iter.second.laneid);
            ti->set_lane_turn_type((adu::stb_map::LaneTurnType)temp_iter.second.laneturntype);
            ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_3_SECOND);
            ti->set_start_time((double)nowtime/1000 - 3);
            ti->set_end_time((double)nowtime/1000);
            ti->set_time_span(3);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_COUNT);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_TRANS_COUNT);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_LENGTH);
            ti->set_queue_count(temp_iter.second.queuecount);
            ti->set_queue_trans_count(temp_iter.second.queuetranscount);
            ti->set_queue_length(temp_iter.second.queuelength);
        }
        else//流向级3s级数据
        {
            auto ti = tis.add_traffic_info();
            auto header = ti->mutable_header();
            header->set_timestamp_sec((double)nowtime/1000);
            header->set_cross_id(temp_iter.second.crossid);
            header->set_module_name("V2X");
            ti->set_normal(true);
            ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_FLOW);
            ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
            ti->set_cross_id(temp_iter.second.crossid);
            ti->set_branch_id(temp_iter.second.branchid);
            ti->mutable_flow_to_type();
            for(int j = 0; j < temp_iter.second.flowtotype.size(); j++)
            {
                ti->add_flow_to_type(adu::st::TrafficInfo::FlowToType(temp_iter.second.flowtotype[j]));
            }
            ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_3_SECOND);
            ti->set_start_time((double)nowtime/1000 - 3);
            ti->set_end_time((double)nowtime/1000);
            ti->set_time_span(3);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_COUNT);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_TRANS_COUNT);
            ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_QUEUE_LENGTH);
            ti->set_queue_count(temp_iter.second.queuecount);
            ti->set_queue_trans_count(temp_iter.second.queuetranscount);
            ti->set_queue_length(temp_iter.second.queuelength);
        }
    }

    if(tis.traffic_info_size() > 0)
    {
        std::string trafficinfos = tis.SerializeAsString();

        std::string hint = "V2X信控指标";
        if(getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "ready to send 3s V2X proto:" << tis.DebugString();
        }
    //    	mqttPushStringMsg2Broker(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc, trafficinfos, hint);

        static int message_count_tc_3s_channel = 1; // 用于计数的静态变量
        std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        // 根据消息计数判断是偶数还是奇数
        if (message_count_tc_3s_channel % 2 == 0) // 偶数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(this->getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                           for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                           {
                               auto temp_info = tis.traffic_info().Get(tempi);
                               std::string tempinfoStr = temp_info.DebugString();
                               size_t pos = tempinfoStr.find("timestamp_sec");
                               if(pos == std::string::npos)
                               {
                                   RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                   continue;
                               }

                               std::string tempStr = tempinfoStr.substr(pos);
                               std::string tempPrintStr;
                               for(char tempchar : tempStr)
                               {
                                   if(tempchar == '}')
                                   {
                                       continue;
                                   }

                                   if(tempchar == '\n')
                                   {
                                       tempPrintStr = tempPrintStr + ';';
                                   }
                                   else
                                   {
                                       tempPrintStr = tempPrintStr + tempchar;
                                   }
                               }

                               RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                           }
                       }
                   }
               }
           });
        }
        else // 奇数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                           for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                           {
                               auto temp_info = tis.traffic_info().Get(tempi);
                               std::string tempinfoStr = temp_info.DebugString();
                               size_t pos = tempinfoStr.find("timestamp_sec");
                               if(pos == std::string::npos)
                               {
                                   RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                   continue;
                               }

                               std::string tempStr = tempinfoStr.substr(pos);
                               std::string tempPrintStr;
                               for(char tempchar : tempStr)
                               {
                                   if(tempchar == '}')
                                   {
                                       continue;
                                   }

                                   if(tempchar == '\n')
                                   {
                                       tempPrintStr = tempPrintStr + ';';
                                   }
                                   else
                                   {
                                       tempPrintStr = tempPrintStr + tempchar;
                                   }
                               }

                               RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                           }
                       }
                   }
               }
           });
        }
        if (message_count_tc_3s_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
        {
            message_count_tc_3s_channel = 0; // 重置计数器
        }
        else
        {
            message_count_tc_3s_channel++;
        }
    }
}
void CCINDEX_COMPONENT::send_5s_level_radardata()
{
    uint64_t nowtime = afl::util::TimeStamp::now(true).millSeconds();
    uint64_t nowtimeSec = afl::util::TimeStamp::now(true).seconds();
    if(!getConfiger().enableMec)
    {
        if(nowtimeSec%5 != 0)
        {
            return;
        }
    }

    adu::st::TrafficInfos tis;
	tis.mutable_traffic_info();
    //添加5s级数据
    for(auto& temp_iter : m_RecvCreditControlInfo_spatcycle_levelMap)
    {
        if(!temp_iter.second.islanelevel)
        {
            continue;
        }
        if(nowtime - temp_iter.second.updatetime > 5000)//雷达1hz上报“类排队”，超过5s的数据不上报平台
        {
            continue;
        }
        //去掉出口道数据
        if(std::find(m_outBranchList.begin(), m_outBranchList.end(), temp_iter.second.branchid) != m_outBranchList.end())
        {
            continue;
        }
        int mec_non_ptccount = 0;
        uint64_t send_time = nowtimeSec/5;
        send_time = send_time*5000 + nowtime%1000;
        int diff = temp_iter.second.updatetime - m_RecvMecInfo_5s_levelMap[temp_iter.first].updatetime.load();
        if(m_RecvMecInfo_5s_levelMap.find(temp_iter.first) != m_RecvMecInfo_5s_levelMap.end() && abs(diff) <= 1500)//感知和雷达数据更新时间小于1.5s时才考虑优化
        {
            mec_non_ptccount = m_RecvMecInfo_5s_levelMap[temp_iter.first].ptc_non_motor_queue_count.load() + m_RecvMecInfo_5s_levelMap[temp_iter.first].ptc_pedestrian_queue_count.load();
            if(getConfiger().enableDebugPrint)
            {
                RADAR_TC_DEBUG_PRINT << "get mec non_motor and pedestrian count:" << mec_non_ptccount;
            }
        }

        auto ti = tis.add_traffic_info();
        auto header = ti->mutable_header();
        header->set_timestamp_sec((double)send_time/1000);
        header->set_cross_id(temp_iter.second.crossid);
        ti->set_normal(true);
        ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE);
        ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
        ti->set_cross_id(temp_iter.second.crossid);
        ti->set_branch_id(temp_iter.second.branchid);
        ti->set_lane_id(temp_iter.second.laneid);
        ti->set_lane_turn_type((adu::stb_map::LaneTurnType)temp_iter.second.laneturntype);
        ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_SECOND);
        ti->set_start_time((double)send_time/1000 - 5);
        ti->set_end_time((double)send_time/1000);
        ti->set_time_span(5);
        ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_COUNT);
        ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_TRANS_COUNT);
        // ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_NUMBER);
        // ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_TRAFFIC_FLOW);
        double fix_car_count =  temp_iter.second.carcount - mec_non_ptccount;
        double fix_car_trans_count = temp_iter.second.cartranscount - mec_non_ptccount;
        if(fix_car_count < 0 )
        {
            fix_car_count = 0;
        }
        if(fix_car_trans_count < 0 )
        {
            fix_car_trans_count = 0;
        }
        ti->set_car_count(fix_car_count);
        ti->set_car_trans_count(fix_car_trans_count);
        // ti->set_traffic_flow(temp_iter.second.trafficflow_5s);
        // ti->set_traffic_number(temp_iter.second.trafficnumber_5s);
        temp_iter.second.trafficflow_5s = 0;
        temp_iter.second.trafficnumber_5s = 0;
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TC_DEBUG_PRINT << "5s V2X data, crossid: " << temp_iter.second.crossid << ", branchid: " << temp_iter.second.branchid << ", laneid: " << temp_iter.second.laneid
            << ", radar car count: " << temp_iter.second.carcount << ", radar car trans count: " << temp_iter.second.cartranscount
            << ", non_motor and pedestrian queue count: " << mec_non_ptccount
            << ", send car count: " << fix_car_count << ", send car trans count: " << fix_car_trans_count;
        }

    }
    

    if(tis.traffic_info_size() > 0)
    {
    	std::string trafficinfos = tis.SerializeAsString();

    	std::string hint = "tc";
        if(getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "ready to send 5s V2X proto:" << tis.DebugString();
        }
//    	mqttPushStringMsg2Broker(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc, trafficinfos, hint);

        static int message_count_tc_5s_channel = 1; // 用于计数的静态变量
        std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        // 根据消息计数判断是偶数还是奇数
        if (message_count_tc_5s_channel % 2 == 0) // 偶数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {

               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(this->getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        else // 奇数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        if (message_count_tc_5s_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
        {
            message_count_tc_5s_channel = 0; // 重置计数器
        }
        else
        {
            message_count_tc_5s_channel++;
        }
    }


}

void CCINDEX_COMPONENT::send_5s_level_radardatabymec()
{
    uint64_t nowtime = afl::util::TimeStamp::now(true).millSeconds();
    uint64_t nowtimeSec = afl::util::TimeStamp::now(true).seconds();
    if(!getConfiger().enableMec)
    {
        if(nowtimeSec%5 != 0)
        {
            return;
        }
    }

    adu::st::TrafficInfos tis;
	tis.mutable_traffic_info();
    //添加5s级数据
    for(auto& temp_iter : m_RecvCreditControlInfo_spatcycle_levelMap)
    {
        if(!temp_iter.second.islanelevel)
        {
            continue;
        }
        if(temp_iter.second.updatetime != 0 && (nowtime - temp_iter.second.updatetime > 5000))//雷达1hz上报“类排队”，超过5s的数据不上报平台
        {
            continue;
        }
        //去掉出口道数据
        if(std::find(m_outBranchList.begin(), m_outBranchList.end(), temp_iter.second.branchid) != m_outBranchList.end())
        {
            continue;
        }
        uint64_t send_time = nowtimeSec/5;
        send_time = send_time*5000 + nowtime%1000;
        double temp_car_count = temp_iter.second.carcount;
        double temp_car_trans_count = temp_iter.second.cartranscount;
        if(m_RecvMecInfo_5s_levelMap.find(temp_iter.first) != m_RecvMecInfo_5s_levelMap.end())//优先用感知数据
        {
            temp_car_count = m_RecvMecInfo_5s_levelMap[temp_iter.first].mec_car_count.load();
            temp_car_trans_count = m_RecvMecInfo_5s_levelMap[temp_iter.first].mec_car_trans_count.load();
        }
        else 
        {
            if(temp_iter.second.updatetime == 0)//雷达和感知算法都没有有效数据，不往云控上报
            {
                continue;
            }
        }
        {

        }

        auto ti = tis.add_traffic_info();
        auto header = ti->mutable_header();
        header->set_timestamp_sec((double)send_time/1000);
        header->set_cross_id(temp_iter.second.crossid);
        ti->set_normal(true);
        ti->set_space_type(adu::st::TrafficInfo::SpaceType::TrafficInfo_SpaceType_SPACE_LANE);
        ti->set_source_type(adu::st::TrafficInfo::SourceType::TrafficInfo_SourceType_V2X);
        ti->set_cross_id(temp_iter.second.crossid);
        ti->set_branch_id(temp_iter.second.branchid);
        ti->set_lane_id(temp_iter.second.laneid);
        ti->set_lane_turn_type((adu::stb_map::LaneTurnType)temp_iter.second.laneturntype);
        ti->set_time_type(adu::st::TrafficInfo::TimeType::TrafficInfo_TimeType_TIME_5_SECOND);
        ti->set_start_time((double)send_time/1000 - 5);
        ti->set_end_time((double)send_time/1000);
        ti->set_time_span(5);
        ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_COUNT);
        ti->add_traffic_status_type(adu::st::TrafficInfo::TrafficStatausType::TrafficInfo_TrafficStatausType_CAR_TRANS_COUNT);

        ti->set_car_count(temp_car_count);
        ti->set_car_trans_count(temp_car_trans_count);

        temp_iter.second.trafficflow_5s = 0;
        temp_iter.second.trafficnumber_5s = 0;
        if(getConfiger().enableDebugPrint)
        {
            RADAR_TC_DEBUG_PRINT << "5s V2X data, crossid: " << temp_iter.second.crossid << ", branchid: " << temp_iter.second.branchid << ", laneid: " << temp_iter.second.laneid
            << ", radar car count: " << temp_iter.second.carcount << ", radar car trans count: " << temp_iter.second.cartranscount
            << ", send car count: " << temp_car_count << ", send car trans count: " << temp_car_trans_count;
        }

    }
    

    if(tis.traffic_info_size() > 0)
    {
    	std::string trafficinfos = tis.SerializeAsString();

    	std::string hint = "tc";
        if(getConfiger().enableDebugPrintProto)
        {
            RADAR_TC_DEBUG_PRINT << "ready to send 5s V2X proto:" << tis.DebugString();
        }
//    	mqttPushStringMsg2Broker(m_MqttClientConfig.configerTopicCCIndex.Topic_Tc, trafficinfos, hint);

        static int message_count_tc_5s_bymec_channel = 1; // 用于计数的静态变量
        std::string topicPT = m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
        // 根据消息计数判断是偶数还是奇数
        if (message_count_tc_5s_bymec_channel % 2 == 0) // 偶数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {

               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(this->getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        else // 奇数
        {
            m_EventloopTc->runInLoop([this, topicPT, trafficinfos, hint, tis]()
           {
               if (!this->mqttPushStringMsg2BrokerCloud(topicPT, trafficinfos, hint))
               {
                   RADAR_TC_ERROR_PRINT << "[error] Publish data failure![msg]" << hint.c_str();
               }
               else
               {
                    m_send_v2x_tc_count++;
                    if(m_send_v2x_tc_count == INT_MAX)
                    {
                        m_send_v2x_tc_count = 0;
                    }
                    if(getConfiger().enableDebugPrintProto)
                    {
                        RADAR_TC_DEBUG_PRINT << "send v2x tc count:" << m_send_v2x_tc_count;
                        RADAR_TC_DEBUG_PRINT << "up topic:" << m_MqttClientConfig.configerTopicCCIndex.Topic_Tc;
                        if(getConfiger().enableDebugPrintProto)
                        {
                            for(int tempi = 0; tempi < tis.traffic_info_size(); tempi++)
                            {
                                auto temp_info = tis.traffic_info().Get(tempi);
                                std::string tempinfoStr = temp_info.DebugString();
                                size_t pos = tempinfoStr.find("timestamp_sec");
                                if(pos == std::string::npos)
                                {
                                    RADAR_TC_DEBUG_PRINT << "timestamp_sec not found";
                                    continue;
                                }

                                std::string tempStr = tempinfoStr.substr(pos);
                                std::string tempPrintStr;
                                for(char tempchar : tempStr)
                                {
                                    if(tempchar == '}')
                                    {
                                        continue;
                                    }

                                    if(tempchar == '\n')
                                    {
                                        tempPrintStr = tempPrintStr + ';';
                                    }
                                    else
                                    {
                                        tempPrintStr = tempPrintStr + tempchar;
                                    }
                                }

                                RADAR_TC_DEBUG_PRINT << "send radar proto to cloud:" << tempPrintStr;
                            }
                        }
                   }
               }
           });
        }
        if (message_count_tc_5s_bymec_channel > MAX_MESSAGE_COUNT_TC_CHANNEL)
        {
            message_count_tc_5s_bymec_channel = 0; // 重置计数器
        }
        else
        {
            message_count_tc_5s_bymec_channel++;
        }
    }
}

// CloudData 发布通用函数实现
bool CCINDEX_COMPONENT::publishCloudDataMqtt(const std::string& topic, const afl::base::json& data)
{
    try {
        // 创建 CloudData 消息
        auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
        
        // 设置 MQTT 数据
        auto mqtt_data = cloud_data->mutable_mqtt_data();
        mqtt_data->set_topic(topic);  // 设置 topic
        
        // 将 JSON 数据转换为 bytes
        std::string json_str = data.dump();
        mqtt_data->set_data(json_str.data(), json_str.size());
        
        // 使用父类的 Send 方法发布到 CyberRT
        if (this->Send("/airos/cloud/report/mqtt", cloud_data)) {
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "[success] Published CloudData to /airos/cloud/report/mqtt";
                CCINDEX_INTER_DEBUG_PRINT << "[topic] " << topic;
                CCINDEX_INTER_DEBUG_PRINT << "[data_size] " << json_str.size() << " bytes";
            }
            return true;
        } else {
            CCINDEX_INTER_ERROR_PRINT << "[error] Failed to publish CloudData to /airos/cloud/report/mqtt";
            return false;
        }
    } catch (const std::exception& e) {
        CCINDEX_INTER_ERROR_PRINT << "[error] Exception while publishing CloudData: " << e.what();
        return false;
    }
}

void CCINDEX_COMPONENT::publishRadarPulse(const std::string& topic, const afl::base::json& data)
{
    auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
    auto mqtt_data = cloud_data->mutable_mqtt_data();
    mqtt_data->set_topic(topic);
    std::string json_str = data.dump();
    mqtt_data->set_data(json_str.data(), json_str.size());
    if (!this->Send("/airos/radar/pulse", cloud_data))
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[pulse][error] failed to publish to /airos/radar/pulse";
    }
}

void CCINDEX_COMPONENT::publishRadarStatistic(const std::string& topic, const afl::base::json& data)
{
    auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
    auto mqtt_data = cloud_data->mutable_mqtt_data();
    mqtt_data->set_topic(topic);
    std::string json_str = data.dump();
    mqtt_data->set_data(json_str.data(), json_str.size());
    if (!this->Send("/airos/radar/statistics", cloud_data))
    {
        RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[statistic][error] failed to publish to /airos/radar/statistics";
    }
}

bool CCINDEX_COMPONENT::publishCloudDataHttp(const std::string& url, const std::string& content_type, const afl::base::json& data)
{
    try {
        // 创建 CloudData 消息
        auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
        
        // 设置 HTTP 数据（静态数据使用 http_data 字段）
        auto http_data = cloud_data->mutable_http_data();
        http_data->set_url_path(url);  // 设置 URL
        http_data->set_content_type(content_type);  // 设置内容类型
        
        // 将 JSON 数据转换为 string 并填充到 data 字段
        std::string json_str = data.dump();
        http_data->set_data(json_str);
        
        // 使用父类的 Send 方法发布到 CyberRT
        if (this->Send("/airos/cloud/report/mqtt", cloud_data)) {
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "[success] Published CloudData to /airos/cloud/report/http";
                CCINDEX_INTER_DEBUG_PRINT << "[url] " << url;
                CCINDEX_INTER_DEBUG_PRINT << "[data_size] " << json_str.size() << " bytes";
            }
            //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, true);
            return true;
        } else {
            CCINDEX_INTER_ERROR_PRINT << "[error] Failed to publish CloudData to /airos/cloud/report/http";
            //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_ST, false);
            return false;
        }
    } catch (const std::exception& e) {
        CCINDEX_INTER_ERROR_PRINT << "[error] Exception while publishing CloudData: " << e.what();
        return false;
    }
}

void CCINDEX_COMPONENT::pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag tag,bool con_flag)
{
    //
    switch (tag)
    {
    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_ST:
        {
            auto* ml_ccindex_st =  mec_monitor_->mutable_ml_ccindex_st();
            ml_ccindex_st->set_tag(tag);
            ml_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            ml_ccindex_st->set_con_flag(con_flag);
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/link_ ,[data]" << mec_monitor_->ShortDebugString();
            }
            Send("/v2x/mec/om/check/ccindex/st/link_", mec_monitor_);
        }
        break;
    case airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_TM:
        {
            auto* ml_ccindex_tm =  mec_monitor_->mutable_ml_ccindex_tm();
            ml_ccindex_tm->set_tag(tag);
            ml_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
            ml_ccindex_tm->set_con_flag(con_flag);
            if(getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/link_,[data]" << mec_monitor_->ShortDebugString();
            }
            Send("/v2x/mec/om/check/ccindex/tm/link_", mec_monitor_);
        }
        break;
    default:
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out]no support this tag!";
        break;
    }

}
void CCINDEX_COMPONENT::pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag tag)
{
    //airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC
    auto* md_ccindex_tc =  mec_monitor_->mutable_md_ccindex_tc();
    md_ccindex_tc->set_tag(tag);
    md_ccindex_tc->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tc/data_,[data]" << mec_monitor_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/ccindex/tc/data_", mec_monitor_);
}

void CCINDEX_COMPONENT::pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id)
{
    //airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC
    auto* md_ccindex_tm =  mec_monitor_->mutable_md_ccindex_tm();
    md_ccindex_tm->set_tag(tag);
    md_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    if (!topic.empty())
    {
        md_ccindex_tm->set_topic(topic);
    }
    md_ccindex_tm->set_device_id(device_id);
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/data_,[data]" << mec_monitor_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/ccindex/tm/data_", mec_monitor_);
}
void CCINDEX_COMPONENT::pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id)
{
    //airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_IN_CCINDEX_TC
    auto* md_ccindex_st =  mec_monitor_->mutable_md_ccindex_st();
    md_ccindex_st->set_tag(tag);
    md_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    if (!topic.empty())
    {
        md_ccindex_st->set_topic(topic);
    }
    md_ccindex_st->set_device_id(device_id);
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/data_,[data]" << mec_monitor_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/ccindex/st/data_", mec_monitor_);
}

void CCINDEX_COMPONENT::monitorMecLinkStatusCcindexSt()
{
    //
    auto* ml_ccindex_st =  mec_monitor_link_status_ccindex_st_->mutable_ml_ccindex_st();
    ml_ccindex_st->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_ST);
    ml_ccindex_st->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());

    size_t uniqueCategoryCount = m_VendorStats.size();

    if (uniqueCategoryCount > 0)
    {
        // =======================================================
        // 1. 逻辑判断部分：直接查找特定 Key，而不是依赖循环顺序
        // =======================================================
        if (getConfiger().enableDebugPrint)
        {
            CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] uniqueCategoryCount： " << uniqueCategoryCount ;
        }

        bool final_con_flag = false;       // 最终要设置的状态
        bool is_radar_found = false;       // 是否找到了关心的雷达类型

        // --- 检查 tsmtc ---
        auto it_tsmtc = m_VendorStats.find("tsmtc");
        if (it_tsmtc != m_VendorStats.end() && it_tsmtc->second > 0)
        {
            is_radar_found = true;
            if (getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] tsmtc radar found" ;
            }
            // 如果存在 tsmtc，状态取决于 m_MecLinkStatucCcindexTmTsmtc
            final_con_flag = m_MecLinkStatucCcindexTmTsmtc;
        }

        // --- 检查 desaysv ---
        auto it_desaysv = m_VendorStats.find("desaysv");
        if (it_desaysv != m_VendorStats.end() && it_desaysv->second > 0)
        {
            // 如果之前已经找到了 tsmtc (即两种雷达共存)
            if (is_radar_found)
            {
                if (getConfiger().enableDebugPrint)
                {
                    CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] desaysv radar found" ;
                }
                // 逻辑策略：假设所有存在的雷达都必须连接正常，Flag 才为 true (逻辑与)
                // 如果您的业务是“只要有一个正常就行”，请将 && 改为 ||
                final_con_flag = final_con_flag && m_MecLinkStatucCcindexTmDesaysv;
            }
            else
            {
                // 只有 desaysv
                is_radar_found = true;
                final_con_flag = m_MecLinkStatucCcindexTmDesaysv;
            }
        }

        // =======================================================
        // 2. 执行赋值：只在最后执行一次
        // =======================================================
        if (is_radar_found)
        {
            ml_ccindex_st->set_con_flag(final_con_flag);
        }
        else
        {
            // 如果 map 里有数据，但不是 tsmtc 也不是 desaysv，或者数量为0
            // 根据需求设置默认值，通常设为 false
            ml_ccindex_st->set_con_flag(false);
        }

        // =======================================================
        // 3. 日志打印部分：保留遍历用于输出信息
        // =======================================================
        for (const auto& item : m_VendorStats)
        {
            // 注意：RSAP_DEBUG_PRINT 流式输出通常不需要 c_str()，除非是 printf 风格
            // 且 item.second 就是 count
            if (getConfiger().enableDebugPrint)
            {
                CCINDEX_INTER_DEBUG_PRINT << "Vendor: " << item.first << ", Count: " << item.second;
            }
        }
    }

    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/st/link_,[data]" << mec_monitor_link_status_ccindex_st_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/ccindex/st/link_", mec_monitor_link_status_ccindex_st_);
}

void CCINDEX_COMPONENT::monitorMecLinkStatusCcindexTm()
{
    auto* ml_ccindex_tm =  mec_monitor_link_status_ccindex_tm_->mutable_ml_ccindex_tm();
    ml_ccindex_tm->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_LINK_CCINDEX_TM);
    ml_ccindex_tm->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
    if (m_MqttConnectedCloud )
    {
        ml_ccindex_tm->set_con_flag(true);
    }
    else
    {
        ml_ccindex_tm->set_con_flag(false);
    }
    if(getConfiger().enableDebugPrint)
    {
        CCINDEX_INTER_DEBUG_PRINT << "[monitor-mec-out] Send topic: /v2x/mec/om/check/ccindex/tm/link_,[data]" << mec_monitor_link_status_ccindex_tm_->ShortDebugString();
    }
    Send("/v2x/mec/om/check/ccindex/tm/link_", mec_monitor_link_status_ccindex_tm_);
}
NAMESPACE_ENDED_RADAR_CCINDEX
