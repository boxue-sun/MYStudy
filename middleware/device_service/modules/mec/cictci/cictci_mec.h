/*
 * @Author: zhangenwei
 * @Date: 2024-01-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-19 16:12:15
 * @Description:
 * @FilePath: /airos/base/device_connect/mec/cictci/cictci_mec.h
 */

#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/common/network/byte_buffer.h"
#include "base/common/network/serializable_data.h"
#include "base/common/time_util.h"
#include "base/device_connect/mec/device_base.h"
#include "base/device_connect/mec/device_factory.h"
#include "cictci_communication.h"
#include "cictci_mec_json.h"
#include "cicti_id_allocator.h"
#include "glog/logging.h"
#include "yaml-cpp/yaml.h"
#include "base/common/network/exception.h"
#include "base/common/network/event_loop.h"
#include "middleware/protocol/proto/monitor.pb.h"
#include "air_service/framework/proto/airos_usecase.pb.h"

#define LOG_KEY_MEC_IN "[mec_in]"
#define MEC_IN_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_MEC_IN
#define MEC_IN_WARN_PRINT LOG_WARN_IF << LOG_KEY_MEC_IN
#define MEC_IN_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_MEC_IN
#define MEC_IN_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_MEC_IN
#define MEC_IN_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_MEC_IN
using namespace airos::monitor;
namespace os {
namespace v2x {
namespace device {
using namespace afl::base;

enum CICTCI_EventType : int {
    CICTCI_NONE = 0,                                    // 非 use_case，防止不使用 has 判断
    CICTCI_TRAFFIC_ANTIDROMIC = 1,                      // 交通逆行
    CICTCI_TRAFFIC_CONGESTION = 2,                      // 交通拥堵
    CICTCI_ROAD_DEBRIS = 3,                             // 道路遗撒
    CICTCI_PARKING = 4,                                 // 停车
    CICTCI_ROAD_CONSTRUCTION = 10,                      // 道路施工
    CICTCI_SLOW_VEHICLE = 11,                           // 车辆慢行
    CICTCI_ABNORMAL_LANE_CHANGE = 13,                   // 异常变道
    CICTCI_OVERSPEED = 14,                              // 超速
    CICTCI_EMERGENCY_LANE_OCCUPY = 15,                  // 占用应急车道
    CICTCI_INTRUSION = 16,                              // 闯入事件
    CICTCI_QUEUE_OVERFLOW = 17,                         // 排队超限
    CICTCI_PEDESTRIAN_CROSSING = 18,                    // 行人横穿（弱势交通参与者）
    CICTCI_SPILL_EVENT = 19,                            // 溢出事件
    CICTCI_CONTINUOUS_MULTI_LANE_CHANGE = 20,           // 连续变多道
    CICTCI_LANE_DEPARTURE = 21,                         // 压线
    CICTCI_PEDESTRIAN_ON_CROSSWALK = 22,                // 行人在斑马线
    CICTCI_VEHICLE_EXITING = 23,                        // 机动车驶离
    CICTCI_REVERSING = 24,                              // 倒车
    CICTCI_VRU_PEDESTRIAN_CROSSING_ALERT = 25,          // VRU 行人横穿预警
    CICTCI_EMERGENCY_BRAKING = 26,                      // 紧急制动
    CICTCI_VEHICLE_MALFUNCTION = 27,                    // 车辆故障
    CICTCI_PEDESTRIAN_INTRUSION_MOTORWAY = 28,          // 行人闯入机动车道
    CICTCI_NON_MOTOR_INTRUSION_MOTORWAY = 29,           // 非机动车闯入机动车道
    CICTCI_TRAFFIC_LIGHT_FAULT = 30,                    // 信号灯故障
    CICTCI_VEHICLE_RED_LIGHT_VIOLATION = 31,            // 机动车闯红灯
    CICTCI_NON_MOTOR_RED_LIGHT_VIOLATION = 32,          // 非机动车闯红灯
    CICTCI_PEDESTRIAN_RED_LIGHT_VIOLATION = 33,         // 行人闯红灯
    CICTCI_EMERGENCY_VEHICLE_ALERT = 34,                // 紧急车辆提醒
};

/**
 * @brief cictci-MEC设备
 *
 */
class CICTCIMec : public MecDevice
{
public:
    CICTCIMec(const MecCallBack &cb) : MecDevice(cb)
    {
        output_data_obj_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_data_event_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_data_hearbeat_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_data_angle_offset_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_data_trafficlight_list_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_data_trafficflow_ = std::make_shared<airos::usecase::EventOutputResult>();
        output_monitor_ = std::make_shared<airos::usecase::EventOutputResult>();
    }

    virtual ~CICTCIMec();

    bool Init(const std::string &config_file) override;
    void Start() override;

    void WriteToDevice(
        const std::shared_ptr<const airos::usecase::EventOutputResult>&re_proto) override;

    MecDeviceState GetState() override
    {
        return MecDeviceState::RUNNING;
    }
private:
    void Stop();
    bool ParseJsonData2PbDataObj(json& j);
    bool ParseJsonData2PbDataEvent(json& j);
    bool InitProtocol(const std::string &remote_ip, const uint16_t remote_port,
              const std::string &host_ip, const uint16_t host_port,
              const std::string &protocol);
    ssize_t SendFrame(uint8_t *packet_addr, size_t packet_len);
    void TaskProcessRecvFrame();
    uint64_t TimeStampTransfer(std::string& timeInString);
    bool ParseJsonData2PbDataHeartBeat(json &j);
    bool ParseJsonData2PbDataAngleOffset(json &j);
    bool ParseJsonData2PbDataTrafficlight(json &j);
    bool ParseJsonData2PbDataTrafficFlow(json &j);
    bool InitEventLoop();
    void MonitorSensorDataIn();

private:
    const std::unordered_map<os::v2x::device::CICTCI_EventType, airos::usecase::EventInformation_EventType> kCictciToAirosMap_ = {
        {os::v2x::device::CICTCI_EventType::CICTCI_NONE,                    airos::usecase::EventInformation_EventType_NONE},
        {os::v2x::device::CICTCI_EventType::CICTCI_TRAFFIC_CONGESTION,      airos::usecase::EventInformation_EventType_LANE_CONGESTION},
        {os::v2x::device::CICTCI_EventType::CICTCI_TRAFFIC_ANTIDROMIC,      airos::usecase::EventInformation_EventType_ANTIDROMIC},
        {os::v2x::device::CICTCI_EventType::CICTCI_OVERSPEED,               airos::usecase::EventInformation_EventType_OVERSPEED},
        {os::v2x::device::CICTCI_EventType::CICTCI_SLOW_VEHICLE,            airos::usecase::EventInformation_EventType_LOWSPEED},
        {os::v2x::device::CICTCI_EventType::CICTCI_ROAD_CONSTRUCTION,       airos::usecase::EventInformation_EventType_REGIONAL_CONSTRUCTION},
        {os::v2x::device::CICTCI_EventType::CICTCI_PARKING,                 airos::usecase::EventInformation_EventType_ZOMBIES_CAR},
        {os::v2x::device::CICTCI_EventType::CICTCI_EMERGENCY_LANE_OCCUPY,   airos::usecase::EventInformation_EventType_EMERGENCY_LANE_OCCUPY},
        {os::v2x::device::CICTCI_EventType::CICTCI_ABNORMAL_LANE_CHANGE,    airos::usecase::EventInformation_EventType_CONTINUOUS_CHANGE_LANE},
        {os::v2x::device::CICTCI_EventType::CICTCI_PEDESTRIAN_CROSSING,     airos::usecase::EventInformation_EventType_PEOPLE_ENTER},
        {os::v2x::device::CICTCI_EventType::CICTCI_NON_MOTOR_INTRUSION_MOTORWAY, airos::usecase::EventInformation_EventType_NONVEHICLE_ENTER},
        {os::v2x::device::CICTCI_EventType::CICTCI_VEHICLE_RED_LIGHT_VIOLATION,  airos::usecase::EventInformation_EventType_VEHICLE_RUN_RED_LIGHT},
        {os::v2x::device::CICTCI_EventType::CICTCI_PEDESTRIAN_RED_LIGHT_VIOLATION, airos::usecase::EventInformation_EventType_PEOPLE_RUN_RED_LIGHT},
        {os::v2x::device::CICTCI_EventType::CICTCI_NON_MOTOR_RED_LIGHT_VIOLATION, airos::usecase::EventInformation_EventType_NONVEHICLE_RUN_RED_LIGHT},
        {os::v2x::device::CICTCI_EventType::CICTCI_INTRUSION,                  airos::usecase::EventInformation_EventType_PEOPLE_ENTER},
        {os::v2x::device::CICTCI_EventType::CICTCI_PEDESTRIAN_INTRUSION_MOTORWAY, airos::usecase::EventInformation_EventType_PEOPLE_ENTER},
        {os::v2x::device::CICTCI_EventType::CICTCI_LANE_DEPARTURE,             airos::usecase::EventInformation_EventType_CROSS_SOLID_LINE},
        {os::v2x::device::CICTCI_EventType::CICTCI_VEHICLE_MALFUNCTION,        airos::usecase::EventInformation_EventType_VEHICLE_MALFUNCTION},
        {os::v2x::device::CICTCI_EventType::CICTCI_TRAFFIC_LIGHT_FAULT,        airos::usecase::EventInformation_EventType_TRAFFICLIGHT_MALFUNCTION},
        {os::v2x::device::CICTCI_EventType::CICTCI_EMERGENCY_VEHICLE_ALERT,    airos::usecase::EventInformation_EventType_EMERGENCY_VEHICLE}
    };

    std::unique_ptr <std::thread> thread_process_recv_ = nullptr;
    std::shared_ptr <CictciCommunication> communication_ = nullptr;
    volatile bool stop_ = false;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_obj_ = nullptr;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_event_ = nullptr;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_hearbeat_ = nullptr;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_angle_offset_ = nullptr;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_trafficlight_list_ = nullptr;
    std::shared_ptr <airos::usecase::EventOutputResult> output_data_trafficflow_ = nullptr;


    uint32_t mec_seqnum_ = 0;
    uint32_t mec_recv_packagenum_ = 0;  // 统计收包的数量
    uint64_t mec_recv_objnum_ = 0;      // 统计收到目标的总总数
    bool has_send_once = false;
    int  sensor_data_in_port = 0;
    uint64_t receive_obj_package_old = 0;
    uint64_t receive_obj_package_new = 0;
    int timer_monitor_sensor_data_in = -1;
    std::shared_ptr<afl::net::EventLoop>    m_Eventloop;
    std::unique_ptr<std::thread>    m_Task;

    std::shared_ptr <airos::usecase::EventOutputResult> output_monitor_ = nullptr;
};

}  // namespace device
}  // namespace v2x
}  // namespace os