/*********************************************************************************
 * @file		v2x_codec_component.h
 * @brief		v2x_codec_component belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-3-17
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-3-17 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <sqlite3.h>
#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "base/device_connect/proto/rsu_data.pb.h"
#include "middleware/runtime/src/air_middleware_component.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"
#include <memory>
#include <string>
#include "yaml-cpp/yaml.h"
#include <jsoncpp/json/json.h>
#include "middleware/runtime/src/air_middleware_component.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/common/network/tcp_client.h"
#include "base/common/network/call_backs.h"
#include "base/common/network/event_loop.h"
#include "base/common/network/inet_address.h"
#include "base/common/network/print.h"
#include "base/common/math_util.h"
#include "data_model/namespace.h"
#include "data_model/protocol_data.h"
#include "data_model/objs_data.h"
#include "data_model/event_data.h"
#include "data_model/status_data.h"
#include "data_model/tcp_server_config_data.h"
#include "base/common/network/date_time.h"
#include "base/common/log.h"
#include "base/common/time_util.h"
#include "parse_info.h"
#include <arpa/inet.h>
#include <ctime>
#include <chrono>
#include <boost/endian/conversion.hpp>
#include <mutex>
#include "middleware/protocol/om_common/sqlite_device_status.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"
#include "middleware/protocol/proto/monitor.pb.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
NAMESPACE_PROTOCOL_THREAD_START

#define RSAP_COMPONENT AIROS_COMPONENT_CLASS_NAME(RsapComponent)

using namespace os::v2x::protocol;
using namespace airos::perception;
using namespace airos::usecase;
using namespace afl::net;
using namespace os::v2x::protocol::om::db;
using namespace airos::base::workparam;
struct MsgRespFlag
{
    bool heartBeatRPushedFlag =false;
    bool heartBeatRespFlag =false;
    int heartBeatPushCount = 0;

    bool eventPushedFlag =false;
    bool eventRespFlag =false;
    int eventPushCount = 0;

    bool statusPushedFlag =false;
    bool statusRespFlag =false;
    int statusPushCount = 0;
};

struct FinishedGetNoFlag
{
    bool                                m_IsFinshGetObstacleMecNo = false;
    bool                                m_IsFinshGetEventMecNo = false;
    bool                                m_IsFinshGetHeartBeatMecNo = false;
    bool                                m_IsFinshGetHeartBeatCameraNo = false;
    bool                                m_IsFinshGetHeartBeatRadarNo = false;
    bool                                m_IsFinshGetHeartBeatLidarNo = false;
};

struct DeviceConfigInfo
{
    std::string rscuId;
    std::string objDeviceId;
    std::string statusCamId;
    std::string statusRadarId;
    std::string statusLidarId;
};


struct SensorDeviceEsnHash : public afl::base::SerializableData
{
public:

    std::string         deviceESn;
    std::string         hashValue;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(deviceESn, "A_deviceESn", j, false);
        JsonSerialize(hashValue, "B_hashValue", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(deviceESn, "A_deviceESn", j, noUse_isEmptyFlag);
        JsonDeserialize(hashValue, "B_hashValue", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "deviceESn: " << deviceESn <<  std::endl;
        ss << std::left << std::setw(40) << "hashValue: " << hashValue <<  std::endl;
        return ss.str();
    }
};

class RSAP_COMPONENT
    : public airos::middleware::ComponentAdapter<airos::usecase::EventOutputResult>
{
 public:
    RSAP_COMPONENT(){
        output_monitor_ = std::make_shared<airos::monitor::MonitorResponse>();
        mec_monitor_ = std::make_shared<airos::monitor_mec::MonitorMec>();
    };

   ~RSAP_COMPONENT()  {
       if (timer_monitor_sensor_data_up > 0)
       {
           m_Eventloop->cancelTimer(timer_monitor_sensor_data_up);
       }
   }
  bool Init() override;
  bool Proc(
      const std::shared_ptr<const airos::usecase::EventOutputResult>& frame) override;
private:
    bool monitorHeartBeatResp();
    bool tcpInit(CloudServiceConfig  cloudServiceConfig );
    void onMessage(afl::net::ByteBuffer *buffer);
    void onConnected(const afl::net::TcpConnectionPtr &conn);
    ////////////////////////////////////////////////////////////////////////
    bool processPbData2RsapData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    void printBufferData(afl::net::ByteBuffer& buf, std::string hint);
    std::string computeHash(const std::string deviceId);
    void getSensorDeviceNoHash();
    bool makeObjsUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    bool makeEventUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    bool makeStatusUpData(const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    bool makeStatusUpDataFromDb();

    bool getServerConfigInfo(CloudServiceConfig &cloudServiceConfig);
    void pushHearBeatInfo();
    int64_t getCurrentYearStartSeconds();
    std::string srandStr(uint32_t num);
    void getServerDisconnectedInfo(bool serverDis);
    void getEnableFlag(bool& enbleFlag, int type);
    std::string deviceCodeZip(std::string codeStr);
    void assignVectorToUint8Array(const std::vector<unsigned char>& bytes, uint8_t* camId, int count);
    bool cancelTimer(int fd);
    void reconnectServerHeartBeat();

    bool pushDeviceStatusData();
    bool monitorDeviceStatusResp();
    void reconnectServerDeviceStatus();
    bool timerDelDeviceStatusData();


    bool monitorEventResp();
    void reconnectServerEvent();
    bool cacheEventData(uint32_t monitorFd, const std::shared_ptr<const airos::usecase::EventOutputResult> &mecDeviceData);
    std::string convertToBigEndian(std::string input);
    std::string getIDStr(std::string inputString);
    bool getDeviceConfigInfo();
    bool getDeviceNoMapInfoFromFile(std::map<std::string, std::string>& deviceNoMapInfoMap);
    bool getWorkParamFromFile();
    bool writeDeviceNoInfoToFile(std::string mecSn, std::string mecEsn);
    int getRandom(int startNum, int endNum);
    void monitorSensorDataUp();
private:
    ParseInfo                               m_ParseInfo;
    std::shared_ptr <afl::net::EventLoop>   m_Eventloop;
    std::unique_ptr <afl::net::TcpClient>   m_TcpClient;
    afl::net::TcpConnectionPtr              m_CurrConn;
    CloudServiceConfig                      m_CloudServiceConfig;
    std::unique_ptr <std::thread>           m_Task;
    int                                     m_ObjDeviceType;
    int                                     m_HeartBeatTimer = -1;
    bool                                    m_ConnetedFlag = false;
    bool                                    m_CertVerifyFlag = true;
    afl::net::TlsCtxInfo                    m_TlsCtxInfo;
    MsgRespFlag                             m_MsgRespFlag;

    int                                     timerHeartBeatFd = -1;
    int                                     timerHeartBeatMonitorFd = -1;
    int                                     timerHeartBeatIntervalFd = -1;
    int                                     timeIntervalHeartBeat = 0;
    bool                                    m_HeartBeatReconnectFlag = false;

    int                                     timerDeviceStatusFd = -1;
    int                                     timerDeviceStatusMonitorFd = -1;
    int                                     timerDeviceStatusIntervalFd = -1;
    int                                     timeIntervalDeviceStatus = 0;
    bool                                    m_DeviceStatusReconnectFlag = false;
    std::mutex                              m_DeviceStatusMutex;
    std::map<uint64_t, std::shared_ptr<const airos::usecase::EventOutputResult>> m_DeviceStatusCache;

    int                                     timerEventMonitorFd = -1;
    int                                     timerEventIntervalFd = -1;
    int                                     timeIntervalEvent = 0;
    bool                                    m_EventReconnectFlag = false;


    std::map<uint64_t, std::pair<uint32_t , std::shared_ptr<const airos::usecase::EventOutputResult>>> m_EventCache;
    FinishedGetNoFlag m_FinishedGetNoFlag;
    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtr;
    std::map<std::string, std::string> m_DeviceNoMapInfoMap;
    std::string m_MecAllocDevNo;
    bool        m_HasedGetAllocDevNo = false;

    std::shared_ptr <airos::monitor::MonitorResponse> output_monitor_ = nullptr;
    int timer_monitor_sensor_data_up = -1;
    uint64_t send_obj_package_old = 0;
    uint64_t send_obj_package_new = 0;

    std::map<std::string, SensorDeviceEsnHash> m_SensorDeviceNoMapInfoMap;  //sn， <esn, hash>
    OmWorkParamConfiger  omWorkParamConfiger;

    // 收到package的数量
    uint64_t recv_total_package_num = 0;
    // 收到目标的数量
    uint64_t recv_total_obj_num = 0;

    std::shared_ptr <airos::monitor_mec::MonitorMec> mec_monitor_ = nullptr;
    void pushMonitorMecLinkStatus(bool con_flag);

    int timer_monitor_mec_link_status = -1;
    void monitorMecLinkStatus();
};

REGISTER_AIROS_COMPONENT_CLASS(RsapComponent, airos::usecase::EventOutputResult);

NAMESPACE_PROTOCOL_THREAD_END
