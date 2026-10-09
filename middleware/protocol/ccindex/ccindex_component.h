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
#ifndef AIROS_MIDDLEWARE_PROTOCOL_CCINDEX_CCINDEX_COMPONENT_H
#define AIROS_MIDDLEWARE_PROTOCOL_CCINDEX_CCINDEX_COMPONENT_H
#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "base/common/auth/Authenticator.h"
#include "configer_ccindex.h"
#include "base/common/network/print.h"
#include <cstdint>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "middleware/protocol/proto/monitor.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
NAMESPACE_START_RADAR_CCINDEX
#define CCINDEX_COMPONENT AIROS_COMPONENT_CLASS_NAME(CcindexComponent)
/////////////////////////////////
//内部
#define LOG_KEY_CCINDEX_INTER "[ccindex_inter]"
#define CCINDEX_INTER_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_INTER
#define CCINDEX_INTER_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_INTER
#define CCINDEX_INTER_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_INTER
#define CCINDEX_INTER_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_INTER
#define CCINDEX_INTER_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_INTER

#define LOG_KEY_CCINDEX_CLOUD "[ccindex_cloud]"
#define CCINDEX_CLOUD_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_CLOUD
#define CCINDEX_CLOUD_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_CLOUD
#define CCINDEX_CLOUD_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_CLOUD
#define CCINDEX_CLOUD_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_CLOUD
#define CCINDEX_CLOUD_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_CLOUD


#define LOG_KEY_CCINDEX_STATIC "[radar_static]"
#define RADAR_STATIC_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC
#define RADAR_STATIC_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_STATIC
#define RADAR_STATIC_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_STATIC
#define RADAR_STATIC_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC
#define RADAR_STATIC_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_STATIC

#define LOG_KEY_CCINDEX_STATIC_POST "[radar_static_post]"
#define RADAR_STATIC_POST_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC_POST
#define RADAR_STATIC_POST_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_STATIC_POST
#define RADAR_STATIC_POST_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_STATIC_POST
#define RADAR_STATIC_POST_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC_POST
#define RADAR_STATIC_POST_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_STATIC_POST

#define LOG_KEY_CCINDEX_STATIC_GET "[radar_static_get]"
#define RADAR_STATIC_GET_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC_GET
#define RADAR_STATIC_GET_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_STATIC_GET
#define RADAR_STATIC_GET_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_STATIC_GET
#define RADAR_STATIC_GET_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_STATIC_GET
#define RADAR_STATIC_GET_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_STATIC_GET



#define LOG_KEY_RADAR_TC "[radar_tc]"
#define RADAR_TC_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_RADAR_TC
#define RADAR_TC_WARN_PRINT LOG_WARN_IF << LOG_KEY_RADAR_TC
#define RADAR_TC_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_RADAR_TC
#define RADAR_TC_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_RADAR_TC
#define RADAR_TC_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_RADAR_TC

#define LOG_KEY_RADAR_TRAFFIC_METRICS "[radar_trafficmetrics]"
#define RADAR_TRAFFIC_METRICS_DEBUG_PRINT LOG_INFO << LOG_KEY_RADAR_TRAFFIC_METRICS
#define RADAR_TRAFFIC_METRICS_WARN_PRINT LOG_WARN << LOG_KEY_RADAR_TRAFFIC_METRICS
#define RADAR_TRAFFIC_METRICS_ERROR_PRINT LOG_ERROR << LOG_KEY_RADAR_TRAFFIC_METRICS
#define RADAR_TRAFFIC_METRICS_SUCCESS_PRINT LOG_INFO << LOG_KEY_RADAR_TRAFFIC_METRICS
#define RADAR_TRAFFIC_METRICS_FATAL_PRINT   LOG_FATAL << LOG_KEY_RADAR_TRAFFIC_METRICS


#define LOG_KEY_RADAR_TRAFFIC_METRICS_count "[count_radar_trafficmetrics]"
#define COUNT_RADAR_TRAFFIC_METRICS_DEBUG_PRINT LOG_INFO << LOG_KEY_RADAR_TRAFFIC_METRICS_count
#define COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT LOG_WARN << LOG_KEY_RADAR_TRAFFIC_METRICS_count
#define COUNT_RADAR_TRAFFIC_METRICS_ERROR_PRINT LOG_ERROR << LOG_KEY_RADAR_TRAFFIC_METRICS_count
#define COUNT_RADAR_TRAFFIC_METRICS_SUCCESS_PRINT LOG_INFO << LOG_KEY_RADAR_TRAFFIC_METRICS_count
#define COUNT_RADAR_TRAFFIC_METRICS_FATAL_PRINT   LOG_FATAL << LOG_KEY_RADAR_TRAFFIC_METRICS_count



#define LOG_KEY_CCINDEX_MQTT "[ccindex_mqtt]"
#define CCINDEX_MQTT_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_MQTT
#define CCINDEX_MQTT_WARN_PRINT LOG_WARN_IF << LOG_KEY_CCINDEX_MQTT
#define CCINDEX_MQTT_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_CCINDEX_MQTT
#define CCINDEX_MQTT_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_CCINDEX_MQTT
#define CCINDEX_MQTT_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_CCINDEX_MQTT


#define MODULE_CONFIG_DIR "/home/airos/protocol/ccindex"
#define MODULE_NAME "CcindexComponent"
#define MODULE_CFG_NAME "ccindex.flag"


using namespace os::v2x::protocol;
using namespace afl::base;
using namespace airos::base::workparam;
using namespace os::v2x::protocol::ccindex;
///////////////////////////////////////////////////////////
//信控
//存储单个红绿灯周期内数据
struct radarCreditControlInfo_spatcycle_level
{
    bool islanelevel = false;
    uint64_t updatetime = 0;//信控信息更新时间
    std::string crossid;//路口id
    std::string branchid;//干线id
    std::vector<int> flowtotype;//流向级车流方向集合,islanelevel为false时填充并使用,1:直行 2：左转 3：右转 4：掉头
    int laneturntype = 0;//车道级道路转向属性，islanelevel为true时填充并使用;取值：cross.proto中LaneTurnType
    std::string laneid;//车道id,islanelevel为true时填充并使用
    double queuecount = 0.000000;//排队数（自然数，每1s更新）
    double queuetranscount = 0.000000;//排队数（当量数，每1s更新）
    double queuelength = 0.000000;//排队长度，每1s更新
    double carcount = 0.000000;//类排队（自然数，车道级，每1s更新）
    double cartranscount = 0.000000;//类排队（当量数， 车道级，每1s更新）
    double trafficflow = 0.000000;//交通流量（当量数，每3s叠加）
    double trafficnumber = 0.0000000;//交通流量（自然数，每3s叠加）
    double trafficflow_5s = 0.000000;//交通流量（当量数，5s级车道级数据使用）
    double trafficnumber_5s = 0.000000;//交通流量（自然数，5s级车道级数据使用）
    std::string headtimedifftime = "";//车头时距对应每个车辆穿过停止线的时间戳集合（按先后顺序）
    std::string headtimediffcartype = "";//车头时距对应每个车辆穿过停止线的车辆类型集合（按先后顺序）
    std::string headtimediff = "";//车头时距对应每个车辆穿过停止线的车头时距集合（按先后顺序）

    double carcount_when_red = 0.000000;//红灯启亮时刻类排队数（当量数）
    double cartranscount_when_red = 0.000000;//红灯启亮时刻类排队数（自然数）
};

struct radarCreditControlInfo_5m_level
{
    bool islanelevel = false;
    uint64_t updatetime = 0;//信控信息更新时间   
    std::string crossid;//路口id
    std::string branchid;//干线id
    std::vector<int> flowtotype;//流向级车流方向集合,islanelevel为false时填充并使用,1:直行 2：左转 3：右转 4：掉头
    int laneturntype = 0;//车道级道路转向属性，islanelevel为true时填充并使用;取值：cross.proto中LaneTurnType
    std::string laneid;//车道id,islanelevel为true时填充并使用

    double trafficflow = 0.000000;//交通流量（当量数）
    double trafficnumber = 0.0000000;//交通流量（自然数）
    std::string headtimediffdetail = "";//车头时距
    double waste_time = -1;
};

//
struct FlowTypesId
{
    int typenum = 0;
    std::string leftid = "";
    std::string no_turnid = "";
    std::string rightid = "";
    std::string u_turnid = "";
};

//mec推送的5分钟数据
class mecCreditControlInfo_5m_level
{
public:
    std::atomic<uint64_t> updatetime{0};//mec更新时间
    std::atomic<uint64_t> mec_traffic_number{0};//交通流量（自然数）
    std::atomic<double> mec_traffic_flow{0};//交通流量（当量数）
    std::atomic<uint64_t> ptc_non_motor_count{0};//非机动车数量
    std::atomic<uint64_t> ptc_pedestrian_count{0};//行人数量
public:
    mecCreditControlInfo_5m_level& operator= (const mecCreditControlInfo_5m_level& mcci)
    {
        updatetime.store(mcci.updatetime.load());
        mec_traffic_number.store(mcci.mec_traffic_number.load());
        mec_traffic_flow.store(mcci.mec_traffic_flow.load());
        ptc_non_motor_count.store(mcci.ptc_non_motor_count.load());
        ptc_pedestrian_count.store(mcci.ptc_pedestrian_count.load());
        return *this;
    }
};

//mec推送的5秒数据
class mecCreditControlInfo_5s_level
{
public:
    std::atomic<uint64_t> updatetime{0};//mec更新时间
    std::atomic<double> mec_car_count{0};//类排队（自然数）
    std::atomic<double> mec_car_trans_count{0};//类排队（当量数）
    std::atomic<uint64_t> ptc_non_motor_queue_count{0};//非机动车排队数量
    std::atomic<uint64_t> ptc_pedestrian_queue_count{0};//行人排队数量  
public:
    mecCreditControlInfo_5s_level& operator= (const mecCreditControlInfo_5s_level& mcci)
    {
        updatetime.store(mcci.updatetime.load());
        mec_car_count.store(mcci.mec_car_count.load());
        mec_car_trans_count.store(mcci.mec_car_trans_count.load());
        ptc_non_motor_queue_count.store(mcci.ptc_non_motor_queue_count.load());
        ptc_pedestrian_queue_count.store(mcci.ptc_pedestrian_queue_count.load());
        return *this;
    }
};

struct siglephaseInfo
{
    int color;
    int greentime;
    int yellowtime;
    int redtime;
};


struct TrafficMetricsStatistics
{
    std::string  deviceEsn = "";
    std::atomic<uint64_t> trajectories_recv_count{0}; // 接收的轨迹数
    std::atomic<uint64_t> trajectories_send_count{0};  // 发送的轨迹数
    std::atomic<uint64_t> vehiclePass_recv_count{0};   // 车辆通行接收计数
    std::atomic<uint64_t> vehiclePass_send_count{0};    // 车辆通行发送计数
    std::atomic<uint64_t> queueUp_recv_count{0};        // 排队接收计数
    std::atomic<uint64_t> queueUp_send_count{0};        // 排队发送计数
    std::atomic<uint64_t> areaState_recv_count{0};      // 区域状态接收计数
    std::atomic<uint64_t> areaState_send_count{0};      // 区域状态发送计数
    std::atomic<uint64_t> overflow_recv_count{0};       // 溢出接收计数
    std::atomic<uint64_t> overflow_send_count{0};       // 溢出发送计数
    std::atomic<uint64_t> outlane_recv_count{0};        // 出行车道接收计数
    std::atomic<uint64_t> outlane_send_count{0};        // 出行车道发送计数
    std::atomic<uint64_t> statistics_recv_count{0};     // 统计数据接收计数
    std::atomic<uint64_t> statistics_send_count{0};     // 统计数据发送计数
    std::atomic<uint64_t> evaluations_recv_count{0};    // 评估数据接收计数
    std::atomic<uint64_t> evaluations_send_count{0};    // 评估数据发送计数
    std::atomic<uint64_t> nonmotor_recv_count{0};       // 非机动车接收计数
    std::atomic<uint64_t> nonmotor_send_count{0};       // 非机动车发送计数

    std::string to_string()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
           "[轨迹]recv:" << trajectories_recv_count.load()  << "send:" << trajectories_send_count.load() <<
           "[过车]recv:" << vehiclePass_recv_count.load() << "send:" << vehiclePass_send_count.load() <<
           "[排队]recv:" << queueUp_recv_count.load()  << "send:" << queueUp_send_count.load() <<
           "[区域]recv:" << areaState_recv_count.load()  << "send:" << areaState_send_count.load() <<
           "[溢出]recv:" << overflow_recv_count.load() << "send:" << overflow_send_count.load() <<
           "[出口]recv:" << outlane_recv_count.load() << "send:" << outlane_send_count.load() <<
           "[统计]recv:" << statistics_recv_count.load() <<  "send:" << statistics_send_count.load() <<
           "[评价]recv:" << evaluations_recv_count.load() <<  "send:" << evaluations_send_count.load() <<
           "[行人及非机动车]recv:" << nonmotor_recv_count.load() <<  "send:" << nonmotor_send_count.load() << std::endl;
        return ss.str();
    }
    std::string to_string_send()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
            "[轨迹]" << "send:" << trajectories_send_count.load() <<
            "[过车]" << "send:" << vehiclePass_send_count.load() <<
            "[排队]" << "send:" << queueUp_send_count.load() <<
            "[区域]"  << "send:" << areaState_send_count.load() <<
            "[溢出]" << "send:" << overflow_send_count.load() <<
            "[出口]" << "send:" << outlane_send_count.load() <<
            "[统计]"  <<  "send:" << statistics_send_count.load() <<
            "[评价]"  <<  "send:" << evaluations_send_count.load() <<
            "[行人及非机动车]"  <<  "send:" << nonmotor_send_count.load() << std::endl;
        return ss.str();
    }
    std::string to_string_recv()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
           "[轨迹]recv:" << trajectories_recv_count.load()   <<
           "[过车]recv:" << vehiclePass_recv_count.load()  <<
           "[排队]recv:" << queueUp_recv_count.load()  <<
           "[区域]recv:" << areaState_recv_count.load()  <<
           "[溢出]recv:" << overflow_recv_count.load()  <<
           "[出口]recv:" << outlane_recv_count.load()  <<
           "[统计]recv:" << statistics_recv_count.load()  <<
           "[评价]recv:" << evaluations_recv_count.load() <<
           "[行人及非机动车]recv:" << nonmotor_recv_count.load() << std::endl;
        return ss.str();
    }
};
//动态数据状态
struct CcindexTmStatus
{
    std::string deviceEsn = "";
    std::string deviceSn = "";
    //推送标识
    std::atomic<bool> tm_pub_flag{false};
    //订阅标识
    std::atomic<bool> tm_sub_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss << "[esn]" << deviceEsn <<
           "[sn]" << deviceSn  <<
           "[tm_pub_flag]" << (tm_pub_flag?"true":"false") <<
           "[tm_sub_flag]" << (tm_sub_flag?"true":"false") << std::endl;
        return ss.str();
    }
};
//信控状态
struct CcindexTcStatus
{
    //发布标识
    std::atomic<bool> tc_pub_flag{false};
    //订阅标识
    std::atomic<bool> tc_sub_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss <<
           "[tc_pub_flag]" << (tc_pub_flag?"true":"false") <<
           "[tc_sub_flag]" << (tc_sub_flag?"true":"false") << std::endl;
        return ss.str();
    }
};
//静态数据
struct CcindexStaticStatus
{
    std::atomic<bool> st_post_flag{false};
    std::atomic<bool> st_get_flag{false};

    std::string to_string()
    {
        std::stringstream ss;
        ss <<
           "[st_post_flag]" << (st_post_flag?"true":"false") <<
           "[st_get_flag]" << (st_get_flag?"true":"false") << std::endl;
        return ss.str();
    }
};

struct CcindexMonitor {
    std::atomic<bool> con_flag{false};
    CcindexStaticStatus ccindexStaticStatus;
    CcindexTcStatus ccindexTcStatus;
    std::string to_string()
    {
        std::stringstream ss;
        ss <<
           "[con_flag]" << (con_flag?"True":"False") <<
           "[ccindexStaticStatus]" << ccindexStaticStatus.to_string()  <<
           "[ccindexTcStatus]" << ccindexTcStatus.to_string()
            << std::endl;
        return ss.str();
    }
};
struct CcindexStatusContainer
{
    CcindexMonitor m_CcindexMonitor;
    CcindexTmStatus m_CcindexTmStatus1;
    CcindexTmStatus m_CcindexTmStatus2;
    CcindexTmStatus m_CcindexTmStatus3;
    CcindexTmStatus m_CcindexTmStatus4;
    CcindexTmStatus m_CcindexTmStatus5;
    CcindexTmStatus m_CcindexTmStatus6;
};
///////////////////////////////////////////////////////////
class CCINDEX_COMPONENT : public airos::middleware::ComponentAdapter<os::v2x::device::CloudData>
                          , public Configurable<ConfigerCcindex>
{
public:
    CCINDEX_COMPONENT() : Configurable<ConfigerCcindex>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME)
    {
        output_monitor_ = std::make_shared<airos::monitor::MonitorResponse>();
        mec_monitor_link_status_ccindex_st_ = std::make_shared<airos::monitor_mec::MonitorMec>();
        mec_monitor_link_status_ccindex_tm_ = std::make_shared<airos::monitor_mec::MonitorMec>();

        mec_monitor_data_status_ccindex_st_ = std::make_shared<airos::monitor_mec::MonitorMec>();
        mec_monitor_data_status_ccindex_tm_ = std::make_shared<airos::monitor_mec::MonitorMec>();
        mec_monitor_data_status_ccindex_tc_ = std::make_shared<airos::monitor_mec::MonitorMec>();
        mec_monitor_ = std::make_shared<airos::monitor_mec::MonitorMec>();
    };

    virtual ~CCINDEX_COMPONENT()
    {
        // 依次停EventLoop/上传队列线程、断开MQTT。原来这里先断MQTT再join，而EventLoop从不退出，
        // join一直卡住、定时器还在析构中的对象上运行，重复节点/Ctrl+C退出时卡死或段错误
        shutdownWorkers();
    };

    bool Init() override;

    bool Proc(const std::shared_ptr<const os::v2x::device::CloudData>& recv_data);
    void doBaiscWork();
    bool initEventLoop();
    bool initEventLoopTrafficMetrics();
    bool initEventLoopTrafficMetricsPubFirst();
    bool initEventLoopTrafficMetricsPubSecond();
    bool initEventLoopTc();
    bool initEventLoopTcPubFirst();
    bool initEventLoopTcPubSecond();
    bool initEventLoopStatic();
    bool initEventLoopCloud();
    bool initEventLoopTrajectoriesTmstc();
    bool initEventLoopTrajectoriesDesaysv();
    bool initEventLoopNoTrajectoriesOne();
    bool initEventLoopNoTrajectoriesTwo();


    bool getMqttConfigerMsg();
    bool getWorkParamFromFile();
    bool initUseCase();
    void notifyEventLoopReady();
    void waitEventLoopsReady(int count);
    void shutdownWorkers();
    void processProcessEventOutputResult(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);

private:
    //动态数据
    bool determineOperationType(const std::string& path);
    bool getDeviceIdFromSubscribeData(const std::string& input, std::string& device_id, RADAR_STATIC_MESSAGE_TYPE optType = RadarStaticUpdate);
    bool insertOrUpdateTrajectoriesDataJson(std::unordered_map<std::string, std::vector<json>>& configDataMap, std::string key, const json& data, int& optType);
    bool processTrajectoriesData(std::string topic, const json& subScribeJson);
    bool timerPublishTrajectoriesData();
    bool timerTrajectoriesMonitorStart();
    bool uploadQueue1();
    bool uploadQueue2();
    
    // 上云数据通过cybert发布
    bool publishCloudDataMqtt(const std::string& topic, const afl::base::json& data);
    bool publishCloudDataHttp(const std::string& url, const std::string& content_type, const afl::base::json& data);
    void publishRadarPulse(const std::string& topic, const afl::base::json& data);
    void publishRadarStatistic(const std::string& topic, const afl::base::json& data);
    
    ////////////////////////////////////////////////////////
    //静态
    bool timerQueryConfigData();
    bool timerQueryConfigDataInstantly(httplib::Response& res);
    bool processCloudQuery();
    bool timerQueryConfig2HttpServerTsmtc();
    bool timerQueryConfig2HttpServerDesaysv();
    bool getConfigQueryMsgFromTopic(std::string topic, ConfigQuery& configQuery);
    bool insertOrUpdateConfigQueryData(std::unordered_map<std::string, ConfigQueryData>& configDataMap, std::string key, const ConfigQueryData& data);
    bool insertOrUpdateConfigQueryDataJson(std::unordered_map<std::string, json>& configDataMap, std::string key, const json& data);


    bool timerUpdateConfigData();
    bool timerUpdateConfigDataDesaysv();
    bool timerUpdateConfigDataTsmtc();
    bool insertOrUpdateConfigUpdateData(std::unordered_map<std::string, ConfigUpdateData>& configDataMap, std::string key, const ConfigUpdateData& data);
    bool insertOrUpdateConfigUpdateDataJsonStatic(std::unordered_map<std::string, json>& configDataMap, std::string key, const json& data);
    bool mergeJsonConfigData(const std::unordered_map<std::string, json>& m_ConfigUpdateDataJson, json& mergedJson);

    bool processConfigQeuryDataAck(std::string topic, const json& subScribeJson);
    bool processConfigUpdateDataAck(std::string topic, const json& subScribeJson);
    std::string extractIPAddress(const std::string& url);
    bool timerQueryConfigDataOnce();
    int determineOperationTypeStatic(const std::string& path);
    bool udpInit();
    void readUdpData();
    uint8_t CovertGatLightColor(uint8_t gat_light_color);
    std::unordered_map<std::string, std::string> parseGetParams(const httplib::Request &req);
    ConfigQuery buildConfigQuery(const std::unordered_map<std::string, std::string>& params);
    ConfigUpdate buildConfigUpdate(const std::unordered_map<std::string, std::string>& params);

    bool timerQueryConfig2HttpServer();
    bool processNumberConfigData(json& mergedJson);
    //信控
    void send_5s_level_radardata();
    void send_5s_level_radardatabymec();
    void send_3s_level_radardata();
    void send_5m_level_radardata();
    void send_5m_level_data();
    void temp_timer();
    void periodMonitorRadarCloudStatus();
    std::string fixHeadtimediffdetail(std::string head_time_diff_detail, int count);
    //订阅
    void mqttDispatchSubscripeMessageProto(std::string topic, void* data, int size);
    std::string getRadarEsn(std::string topicMid);
private:
    static void mqttConnectedCallbackCloud(void* context, char* cause);
    static void mqttConnectedCallback(void* context, char* cause);
    ///////////////////////////////////////////////
     //inter
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
    void mqttDispatchSubscribeMessage(const std::string &topic, const afl::base::json & subScribeJson);
    void mqttDispatchSubscribeMessageProto(std::string topic, void* data, int size);
    void mqttDispatchSubscribeMessageTrajectories(const std::string &topic, const afl::base::json & subScribeJson);
    bool mqttPublishMsgRetries(std::string topic, json payload, std::string hint);
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
            RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id << std::endl;
            return false;
        }

        if (!pData.empty())
        {
            if (!mqttPublishMsg(topic, pData))
            {
                RADAR_STATIC_ERROR_PRINT << "[error]push " << hint.c_str() << " error!";
                if(m_MqttClient)
                {
                    CCINDEX_MQTT_DEBUG_PRINT  << "[cloud][error]Connection is lost, reconnect cloud!";
                    mqttReconnect();
                }
                else
                {
                    mqttInit();
                }
                return false;
            }
            else
            {
                if(getConfiger().enableDebugPrint)
                {
                    RADAR_STATIC_DEBUG_PRINT << "[sucess]ccindex >>> mqtt-inter" << "[topic]" << topic.c_str() << "[data]" << pData.c_str();
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
    void mqttDispatchSubscribeMessageProtoCloud(std::string topic, void* data, int size);
    static int mqttSubscribeMsgArrvdCloud(void *context, char *topicName, int topicLen, MQTTAsync_message *message);

    template<typename T>
    bool mqttPushMsg2BrokerCloud(string topic, T info, string hint)
    {
        std::string pData;

        try
        {
            afl::base::json pushInfoJson = info;
            pData = pushInfoJson.dump();
        } catch (json::exception &e)
        {
            if (hint == "static")
            {
                RADAR_STATIC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else if (hint == "tc")
            {
                RADAR_TC_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else if (hint == "trafficMetrics")
            {
                RADAR_TRAFFIC_METRICS_ERROR_PRINT << "[what]" << e.what() << " [json-exception-id]" << e.id;
            }
            else
            {

            }

            return false;
        }

        if (!pData.empty())
        {

            if (!mqttPublishMsgCloud(topic, pData))
            {

                std::string deviceEsn = getRadarEsn(topic);
                std::string topicSecond = getContentBetweenFirstAndSecondSlash(topic);
                if (hint == "static")
                {
                    RADAR_STATIC_ERROR_PRINT  << "[error]push " <<   hint.c_str() << " error!";
                }
                else if (hint == "tc")
                {
                    RADAR_TC_ERROR_PRINT  << "[error]push " <<   hint.c_str() << " error!";
                    m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_pub_flag = false;
                }
                else if (hint == "trafficMetrics")
                {
                    RADAR_TRAFFIC_METRICS_ERROR_PRINT  << "[error]push " <<   hint.c_str() << " error!";
                    if(getConfiger().configerEnableCCIndex.Enable_Trajectories_Count)
                    {
                        if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus1.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus1.tm_pub_flag = false;}
                        else  if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus2.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus2.tm_pub_flag = false;}
                        else  if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus3.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus3.tm_pub_flag = false;}
                        else  if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus4.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus4.tm_pub_flag = false;}
                        else  if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus5.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus5.tm_pub_flag = false;}
                        else  if(deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus6.deviceEsn){m_CcindexStatusContainer.m_CcindexTmStatus6.tm_pub_flag = false;}
                        else{}
                    }
                }
                else
                {

                }
                if (m_MqttClientCloud) {
                    CCINDEX_MQTT_DEBUG_PRINT << "[cloud][error]Connection is lost, reconnect cloud!";
                    //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
                    mqttReconnectCloud();
                }
                else
                {
                    //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, false);
                    mqttInitCloud();
                }
                return false;
            }
            else
            {
                //pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_MONITOR_OUT_CCINDEX_TM, true);
                CCINDEX_MQTT_DEBUG_PRINT << "[ccindex-->cloud]" << topic;
                std::string deviceEsn = getRadarEsn(topic);
                std::string topicSecond = getContentBetweenFirstAndSecondSlash(topic);
                if(getConfiger().enableDebugPrint)
                {
                    if (hint == "tc")
                    {
                        m_CcindexStatusContainer.m_CcindexMonitor.ccindexTcStatus.tc_pub_flag = true;
                        RADAR_TC_DEBUG_PRINT  << "[sucess]ccindex >>> cloud" << "[topic]" << topic.c_str()
                                         << " [data]" << pData.c_str();
                    }
                    else if (hint == "trafficMetrics")
                    {

                        RADAR_TRAFFIC_METRICS_DEBUG_PRINT  << "[sucess]ccindex >>> cloud" << "[topic]" << topic.c_str()
                                                  << " [data]" << pData.c_str();
                        if(getConfiger().configerEnableCCIndex.Enable_Trajectories_Count)
                        {
                            if (topicSecond == "trajectories")
                            {
                                //计数
                                if (deviceEsn == m_TS_Radar1.deviceEsn) { m_TS_Radar1.trajectories_recv_count++; }
                                else if (deviceEsn == m_TS_Radar2.deviceEsn) { m_TS_Radar2.trajectories_recv_count++; }
                                else if (deviceEsn == m_TS_Radar3.deviceEsn) { m_TS_Radar3.trajectories_recv_count++; }
                                else if (deviceEsn == m_TS_Radar4.deviceEsn) { m_TS_Radar4.trajectories_recv_count++; }
                                else if (deviceEsn == m_TS_Radar5.deviceEsn) { m_TS_Radar5.trajectories_recv_count++; }
                                else if (deviceEsn == m_TS_Radar6.deviceEsn) { m_TS_Radar6.trajectories_recv_count++; }
                                else {}

                                //状态
                                if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus1.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus1.tm_pub_flag = true; }
                                else if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus2.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus2.tm_pub_flag = true; }
                                else if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus3.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus3.tm_pub_flag = true; }
                                else if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus4.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus4.tm_pub_flag = true; }
                                else if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus5.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus5.tm_pub_flag = true; }
                                else if (deviceEsn == m_CcindexStatusContainer.m_CcindexTmStatus6.deviceEsn) { m_CcindexStatusContainer.m_CcindexTmStatus6.tm_pub_flag = true; }
                                else {}
                            } else if (topicSecond == "vehiclePass")
                            {
                                if (deviceEsn == m_TS_Radar1.deviceEsn)
                                {
                                    m_TS_Radar1.vehiclePass_send_count++;
                                }
                                else if (deviceEsn == m_TS_Radar2.deviceEsn)
                                {
                                    m_TS_Radar2.vehiclePass_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.vehiclePass_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.vehiclePass_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.vehiclePass_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.vehiclePass_send_count++;
                                } else {}
                            } else if (topicSecond == "queueUp") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.queueUp_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.queueUp_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.queueUp_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.queueUp_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.queueUp_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.queueUp_send_count++;
                                } else {}
                            } else if (topicSecond == "areaState") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.areaState_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.areaState_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.areaState_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.areaState_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.areaState_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.areaState_send_count++;
                                } else {}
                            } else if (topicSecond == "overflow") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.overflow_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.overflow_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.overflow_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.overflow_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.overflow_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.overflow_send_count++;
                                } else {}
                            } else if (topicSecond == "outlane") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.outlane_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.outlane_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.outlane_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.outlane_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.outlane_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.outlane_send_count++;
                                } else {}
                            } else if (topicSecond == "statistics") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.statistics_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.statistics_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.statistics_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.statistics_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.statistics_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.statistics_send_count++;
                                } else {}
                            } else if (topicSecond == "evaluations") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.evaluations_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.evaluations_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.evaluations_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.evaluations_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.evaluations_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.evaluations_send_count++;
                                } else {}
                            } else if (topicSecond == "nonmotor") {
                                if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                    m_TS_Radar1.nonmotor_send_count++;
                                } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                    m_TS_Radar2.nonmotor_send_count++;
                                } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                    m_TS_Radar3.nonmotor_send_count++;
                                } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                    m_TS_Radar4.nonmotor_send_count++;
                                } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                    m_TS_Radar5.nonmotor_send_count++;
                                } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                    m_TS_Radar6.nonmotor_send_count++;
                                } else {}
                            }


                            RADAR_TRAFFIC_METRICS_DEBUG_PRINT << "[sucess]ccindex >>> cloud" << "[topic]" << topic.c_str()
                                                              << " [data]" << pData.c_str();
                            if (deviceEsn == m_TS_Radar1.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar1.to_string_send();
                            } else if (deviceEsn == m_TS_Radar2.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar2.to_string_send();
                            } else if (deviceEsn == m_TS_Radar3.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar3.to_string_send();
                            } else if (deviceEsn == m_TS_Radar4.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar4.to_string_send();
                            } else if (deviceEsn == m_TS_Radar5.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar5.to_string_send();
                            } else if (deviceEsn == m_TS_Radar6.deviceEsn) {
                                COUNT_RADAR_TRAFFIC_METRICS_WARN_PRINT << m_TS_Radar6.to_string_send();
                            } else {}
                        }
                    }
                    else
                    {

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
    bool processTrafficMetricsData(void *context, std::string topic, afl::base::json j);
    bool processTcData(void *context, std::string topic, std::string msg);
    bool processStaticData(void *context, std::string topic, afl::base::json j);
    std::string getContentBetweenFirstAndSecondSlash(const std::string& input);
    bool monitorMqttConnectStatus();

private:
    std::unique_ptr<std::thread>    m_Task;
    std::unique_ptr<std::thread>    m_TaskTrafficMetrics;
    std::unique_ptr<std::thread>    m_TaskTrafficMetricsPubFirst;
    std::unique_ptr<std::thread>    m_TaskTrafficMetricsPubSecond;
    std::unique_ptr<std::thread>    m_TaskTc;
    std::unique_ptr<std::thread>    m_TaskTcPubFirst;
    std::unique_ptr<std::thread>    m_TaskTcPubSecond;
    std::unique_ptr<std::thread>    m_TaskStatic;
    std::unique_ptr<std::thread>    m_TaskStaticCloud;
    std::unique_ptr<std::thread>    m_TaskUseCase;

    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTrafficMetrics;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTrafficMetricsPubFirst;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTrafficMetricsPubSecond;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTc;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTcPubFirst;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTcPubSecond;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopStatic;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopCloud;
    // 各EventLoop在各自线程中创建，Init()通过以下变量等待全部创建完成
    std::mutex                              m_EventloopReadyMutex;
    std::condition_variable                 m_EventloopReadyCv;
    int                                     m_EventloopReadyCount = 0;
    std::unique_ptr<afl::net::Channel>      m_Chnl;
    /////////////////////////////////////////////////////////////
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTrajectoriesTmstc;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopTrajectoriesDesaysv;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopNoTrajectoriesOne;
    std::shared_ptr<afl::net::EventLoop>    m_EventloopNoTrajectoriesTwo;
    std::unique_ptr<std::thread>    m_TaskTrajectoriesTmstc;
    std::unique_ptr<std::thread>    m_TaskTrajectoriesDesaysv;
    std::unique_ptr<std::thread>    m_TaskNoTrajectoriesOne;
    std::unique_ptr<std::thread>    m_TaskNoTrajectoriesTwo;



    //inter
    MQTTAsync_connectOptions        m_MqttConnOpts;
    MQTTAsync                       m_MqttClient;
    std::atomic<bool>               m_MqttConnected{false};
    ConfigerCcindex 	            m_MqttClientConfig;
    OmWorkParamConfiger  omWorkParamConfiger;
    /////////////////////////////////////////////////////////////
    //动态数据
    std::map<uint64_t, std::unordered_map<std::string, std::vector<json>>> m_TrajectoriesDataJsonRecivByUtc;
    int m_TimerTrajectoriesMonitor = -1;

    std::mutex                              m_TrajectoriesMutex;

    FifoConcurrentQueue<std::unordered_map<std::string, json>> queue1; // 队列 1
    FifoConcurrentQueue<std::unordered_map<std::string, json>> queue2; // 队列 2

    std::condition_variable cv1;          // 条件变量 1
    std::condition_variable cv2;          // 条件变量 2
    std::mutex cv_m1;                      // 条件变量的互斥锁
    std::mutex cv_m2;                      // 条件变量的互斥锁
    bool queue1_ready = false;            // 标志队列 1 是否准备好
    bool queue2_ready = false;            // 标志队列 2 是否准备好
    std::atomic<bool> m_StopWorkers{false};  // 析构时通知EventLoop/上传队列线程退出
    std::unique_ptr<std::thread>    m_TaskQueue1;
    std::unique_ptr<std::thread>    m_TaskQueue2;
    int m_TimerTrajectoriesMonitorStart = -1;
    std::unordered_map<std::string, std::vector<json>> m_TrajectoriesDataJsonFromQueue;
    ///////////////////////////////////////////////////////////
    //云控
    uint64_t m_MqttReconnectCountCloud = 0;

    ////////////////////////////////////////////////////////
    //静态
    //查询
    std::unique_ptr<httplib::Server> m_HttpServerPtr;
    std::unique_ptr<std::thread>    m_TaskHttpServer;
    std::unordered_map<std::string, ConfigQueryData> m_ConfigQueryData;
    std::unordered_map<std::string, json> m_ConfigQueryDataJson;
    int m_TimerConfigQueryMonitor = -1;
    int m_TimerConfigQueryMonitorPushOnce = -1;
    int m_TimerConfigQueryInstantly = -1;
    int m_TimerConfigQuery2HttpServer = -1;
    int m_TimerConfigQuery2HttpServerTsmtc = -1;
    int m_TimerConfigQuery2HttpServerDesaysv = -1;
    //更新
    std::unique_ptr<httplib::Client> m_HttpClientPtr;
    std::unique_ptr<httplib::Client> m_HttpClientPtrUpdateTsmtc;
    std::unique_ptr<httplib::Client> m_HttpClientPtrUpdateDesaysv;
    std::unique_ptr<httplib::Client> m_HttpClientPtrQueryTsmtc;
    std::unique_ptr<httplib::Client> m_HttpClientPtrQueryDesaysv;
    std::unordered_map<std::string, ConfigUpdateData> m_ConfigUpdateData;
    std::unordered_map<std::string, json> m_ConfigUpdateDataJson;
    std::unordered_map<std::string, json> m_ConfigUpdateDataJsonTsmtc;
    std::unordered_map<std::string, json> m_ConfigUpdateDataJsonDesaysv;
    std::string m_HttpCloudeHostIpPortUpdateTsmtc;
    std::string m_HttpCloudeHostIpPortUpdateDesaysv;
    std::string m_HttpCloudeHostIpPortQueryTsmtc;
    std::string m_HttpCloudeHostIpPortQueryDesaysv;
    json m_ConfigUpdateDataJsonUpload;

    int m_TimerConfigUpdateMonitor = -1;

    int m_TimerConfigUpdateMonitorTsmtc = -1;

    int m_TimerConfigUpdateMonitorDesaysv = -1;

    std::string m_ContentType = "application/json;charset=UTF-8";

    //////////////////////////////////////////////////
    //信控
    struct sockaddr_in m_UdpAddr;
    int m_Udpfd = -1;
    std::unique_ptr<afl::net::Channel> m_UdpChnl;
    std::unique_ptr<afl::net::ByteBuffer> m_UdpBuffer;
    std::map<int, std::vector<std::string>> m_LaneOrFlowPhaseIdMap;//lane level: <phaseid, vector<corssid+branchid+laneid>>  flow level:<phaseid, vector<corssid+branchid+turntype>>
    std::map<std::string, radarCreditControlInfo_spatcycle_level> m_RecvCreditControlInfo_spatcycle_levelMap;//lane level: <corssid+branchid+laneid, radarCreditControlInfo_spatcycle_level>  flow level:<corssid+branchid+turntype, radarCreditControlInfo_spatcycle_level>
    std::map<std::string, double> m_greenTimeCountMap;//lane level: <corssid+branchid+laneid, green time count>  flow level:<corssid+branchid+turntype, green time count>
    std::map<int, std::pair<int, siglephaseInfo>> m_phaseInfoMap;//<phaseid, <color, siglephaseInfo>> color: 0unkonw 1green 2yellow 3red
    static int MAX_MESSAGE_COUNT_TC_CHANNEL;

    //内部
    MQTTAsync_connectOptions        m_MqttConnOptsCloud;
    MQTTAsync                       m_MqttClientCloud;
    std::atomic<bool>               m_MqttConnectedCloud{false};
    std::string m_TrustCAFilePath ;
    std::string m_KeyFilePath ;
    std::string m_PrivateKeyFilePath ;

    int                             m_TimerMqttReconnectInter = -1;
    int                             m_TimerMqttReconnectCloud = -1;
    int                             m_temptimer = -1; 
    int                             m_timer_mec_radar_cloud_fd = -1;
    //////////////////////////////////////////////////////////
    //信控压测-指标统计
    int m_recv_tc_count = 0;
    int m_needsend_tc_count = 0;
    int m_send_tc_count = 0;
    int m_send_v2x_tc_count = 0;
    int m_recv_cloud_tc_count = 0;
	
	std::vector<std::string> m_outBranchList;
    std::map<std::string, radarCreditControlInfo_5m_level> m_RecvCreditControlInfo_5m_levelMap;//lane level: <corssid+branchid+laneid, radarCreditControlInfo_5m_level> flow level:<corssid+branchid+turntype, radarCreditControlInfo_5m_level>
    uint64_t m_spat_cycle_seqnum = 0;
    //////////////////////////////////////////////////////////

    TrafficMetricsStatistics  m_TS_Radar1;
    TrafficMetricsStatistics  m_TS_Radar2;
    TrafficMetricsStatistics  m_TS_Radar3;
    TrafficMetricsStatistics  m_TS_Radar4;
    TrafficMetricsStatistics  m_TS_Radar5;
    TrafficMetricsStatistics  m_TS_Radar6;
    /////////////////////////////////////////////////
    std::shared_ptr <airos::monitor::MonitorResponse> output_monitor_ = nullptr;
    bool                            m_TcFlag = false;
    bool                            m_TrafficMetrics_Flag = false;
    CcindexStatusContainer          m_CcindexStatusContainer;

    //mec2101接口数据
    std::map<std::string, mecCreditControlInfo_5m_level> m_RecvMecInfo_5m_levelMap;//lane level: <corssid+branchid+laneid, mecCreditControlInfo_5m_level> flow level:<corssid+branchid+turntype, mecCreditControlInfo_5m_level>
    std::map<std::string, mecCreditControlInfo_5s_level> m_RecvMecInfo_5s_levelMap;//<corssid+branchid+laneid, mecCreditControlInfo_5s_level>

    std::map<std::string, std::string> m_meclaneId_Map;//mec-雷达车道级路网映射<meclaneId, crossid+branchid+laneid>
    std::map<std::string, FlowTypesId> m_mecflowId_map;//mec-雷达流向级路网映射<meclaneId, crossid+branchid>

    int m_TimerMonitorMqttConnected = -1;

    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_link_status_ccindex_st_ = nullptr;
    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_link_status_ccindex_tm_ = nullptr;
    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_data_status_ccindex_st_ = nullptr;
    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_data_status_ccindex_tm_ = nullptr;
    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_data_status_ccindex_tc_ = nullptr;
    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_ = nullptr;
    void pushMonitorMecLinkStatus(airos::monitor_mec::MonitorMecTag tag, bool con_flag);
    void pushMonitorMecDataStatus(airos::monitor_mec::MonitorMecTag tag);
    void pushMonitorMecDataStatusTm(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id);
    bool getMecCheckCcindexTmTag(std::string& topic);
    void pushMonitorMecDataStatusSt(airos::monitor_mec::MonitorMecTag tag, std::string topic, std::string device_id);

    void monitorMecLinkStatusCcindexTm();
    int m_TimerMecLinkStatusCcindexTm = -1;
    void monitorMecLinkStatusCcindexSt();
    int m_TimerMecLinkStatusCcindexSt = -1;

    bool m_MecLinkStatucCcindexTmTsmtc = false;
    bool m_MecLinkStatucCcindexTmDesaysv = false;
    std::map<std::string, int> m_VendorStats;
};

REGISTER_AIROS_COMPONENT_CLASS(CcindexComponent, os::v2x::device::CloudData);
NAMESPACE_ENDED_RADAR_CCINDEX
#endif
