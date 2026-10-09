/*
* @Author: zhangenwei
* @Date: 2024-05-15 10:16:15
* @LastEditors: zhangenwei
* @LastEditTime: 2024-05-15 10:16:15
* @Description: 云控管理平台南向接口交互-摄像机与云控
*/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_MONITOR_COMPONENT
#define AIROS_MIDDLEWARE_PROTOCOL_OM_MONITOR_COMPONENT
#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include  "configer_om_monitor.h"
#include "base/common/network/print.h"
#include "middleware/protocol/om_monitor/data_model/data_common.h"
#include "base/common/auth/Authenticator.h"
#include "middleware/protocol/om_monitor/data_model/data_sys_performance_status.h"
#include "middleware/protocol/om_monitor/data_model/data_rsap_status.h"
#include "middleware/protocol/om_monitor/data_model/data_ccindex_status.h"
#include "middleware/protocol/proto/monitor.pb.h"
#include "middleware/protocol/om_monitor/data_model/data_om_status.h"
NAMESPACE_START_OM_COMPONENT_MONITOR
#define OM_MONITOR_COMPONENT AIROS_COMPONENT_CLASS_NAME(OmMonitorComponent)
using namespace os::v2x::protocol;
using namespace os::v2x::protocol::om::db;
using namespace afl::base;
using namespace os::v2x::protocol::om::mec;
using namespace afl::thread;
using namespace airos::monitor;
#define MODULE_CONFIG_DIR "/home/airos/protocol/monitor"
#define MODULE_NAME "OmMonitorComponent"
#define MODULE_CFG_NAME "om_monitor.flag"
//#define USE_RUNLOG
class OM_MONITOR_COMPONENT : public airos::middleware::ComponentAdapter<airos::monitor::MonitorResponse>
   , public Configurable<UdpConfigerMonitor>
{
public:
    OM_MONITOR_COMPONENT() : Configurable<UdpConfigerMonitor>(MODULE_NAME, MODULE_CONFIG_DIR, MODULE_CFG_NAME){};

    virtual ~OM_MONITOR_COMPONENT() override
    {
    };

    bool Init() override;

    bool Proc(const std::shared_ptr<const airos::monitor::MonitorResponse>& recv_data);
    void doBaiscWork();
    bool initEventLoop();
    void monitorEventloopRunning();
    bool getWorkParamFromFile();
    bool udpInit();
    void readUdpData();

    bool process_query_sys_performance_status(struct sockaddr_in clientAddr,json& j);
    bool process_query_docker_status(struct sockaddr_in clientAddr,json& j);
    bool process_query_airos_status(struct sockaddr_in clientAddr,json& j);
    bool process_query_disk_status(struct sockaddr_in clientAddr,json& j);
    bool process_query_sensor_data_status(struct sockaddr_in clientAddr,json& j);

    std::vector<ContainerInfo> extractAirosContainers(const std::string &output);
    bool getProcessInfo(std::string& command,  std::vector<ProcessInfo> &processes);
    std::vector<ProcessMoudlesNeedInfo> getModulesNeedInfo(std::string& command);
    std::vector<DiskInfo> extractDiskInfos(const std::string &output);

    bool period_query_docker_status();
    void period_query_docker_status_once();

    bool period_query_disk_status();
    void period_query_disk_status_once();

    bool period_query_airos_modules_status();
    void period_query_airos_modules_status_once();
    bool period_query_container_time_once();

    bool process_query_ccindex_status(struct sockaddr_in clientAddr,json& j);
    bool process_query_om_status(struct sockaddr_in clientAddr,json& j);
private:
    std::unique_ptr<std::thread>    m_Task;
    /////////////////////////////////////////////////////////////
    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::unique_ptr<afl::net::Channel>      m_UdpChnl;
    int m_Udpfd = -1;
    int m_EventloopIsRunning = -1;
    std::unique_ptr<afl::net::ByteBuffer> m_UdpBuffer;
    ////////////////////////////////////////////////////////////////
    //感知上云监控
    SensorDataStatus                  m_SensorDataStatusDataUp;
    //感知接入监控
    SensorDataStatus                  m_SensorDataStatusDataIn;
    ////////////////////////////////////////////////////////////////
    socklen_t m_ClientSocklen;
    SensorIpPortData m_SensorIpPortData;
    ////////////////////////////////////////////////////////////////
    SensorDataStatus                  m_SensorDataStatusData;
    std::vector<string>             m_AirosNeedRunModules = {
            "mec_service", "traffic_light_service", "v2x_codec", "rsu_service",  "rsap",  "airos_v2x_app",
            "airos_v2x_scenario", "om_device_status", "om",  "ccindex", "om_monitor"
    };
    ////////////////////////////////////////////////////////////////
    //定时监控airos模块
    int m_MonitorDockerAirosModulesStatus = -1;
    //定时监控docker
    int m_MonitorDockerStatus = -1;
    int m_MonitorDockerStatusOnce = -1;
    int m_MonitorDockerStatusOnceCount = 0;
    ////////////////////////////////////////////////////////////////
    afl::base::json m_JsonDockerStatus;
    ////////////////////////////////////////////////////////////////
    int m_MonitorAirosModulesStatus = -1;
    int m_MonitorAirosModulesStatusOnce = -1;
    int m_MonitorAirosMOdulesStatusOnceCount = 0;
    afl::base::json m_JsonAirosModulesStatus;
    ////////////////////////////////////////////////////////////////
    int m_MonitorAirosContainerTimeOnce = -1;
    std::string m_ContainerTime = "";
    ////////////////////////////////////////////////////////////////
    int m_MonitorDiskStatus = -1;
    int m_MonitorDiskStatusOnce = -1;
    int m_MonitorDiskStatusOnceCount = 0;
    afl::base::json m_JsonDiskStatus;

    ////////////////////////////////////////////////////////////////
    std::mutex m_CcindexStatusMonitor_Mutex;
    CcindexStatusResponse m_CcindexStatusMonitor;

    ////////////////////////////////////////////////////////////////
    std::mutex m_OmStatusMonitor_Mutex;
    OmStatusResponse m_OmStatusMonitor;

	std::string m_AirosDockerName = "";
};

REGISTER_AIROS_COMPONENT_CLASS(OmMonitorComponent, airos::monitor::MonitorResponse);
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif
