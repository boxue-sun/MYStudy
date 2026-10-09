/*********************************************************************************
 * @file		alarm_management_data.h
 * @brief		alarm_management_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-19
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  	24-4-19  alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef MQTT_CLIENT_MEC_OM_MONITOR_DATA_DOCKER_STATUS_H
#define MQTT_CLIENT_MEC_OM_MONITOR_DATA_DOCKER_STATUS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"

NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;
struct ContainerInfo
{
    std::string name;
    double cpuUsage;
    double memUsage;
    double memPercent;
};
struct DockerStatusData  : public afl::base::SerializableData
{
    double cpu = 999.999;
    double mem_percent = 999.999;
    double mem = 999.999;
    bool run_flag = false;

    void serialize(json& j) override
    {
        JsonSerialize(cpu, "cpu", j, false);
        JsonSerialize(mem_percent, "mem_percent", j, false);
        JsonSerialize(mem, "mem", j, false);
        JsonSerialize(run_flag, "run_flag", j, false);
    }

    void deserialize(const json& j)  override
    {
        JsonDeserialize(cpu, "cpu", j, noUse_isEmptyFlag);
        JsonDeserialize(mem_percent, "mem_percent", j, noUse_isEmptyFlag);
        JsonDeserialize(mem, "mem", j, noUse_isEmptyFlag);
        JsonDeserialize(run_flag, "run_flag", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "cpu: "              << cpu << std::endl;
        ss << std::left << std::setw(40) << "mem_percent: "              << mem_percent << std::endl;
        ss << std::left << std::setw(40) << "mem: "              << mem << std::endl;
        ss << std::left << std::setw(40) << "run_flag: "         << (run_flag ? "true" : "false") << "\n";
        return ss.str();
    }
};

struct DockerStatus : public afl::base::SerializableData
{
    QUERY_MSG_TAG tag; // 接口号
    std::string seqnum; // 32位不重复的随机数
    uint64_t timestamp; // utc时间戳，毫秒
    std::string device_esn; // 设备esn号
    DockerStatusData data; // 消息结构

    virtual void serialize(json& j) override {
        JsonSerialize(tag, "tag", j, false);
        JsonSerialize(seqnum, "seqnum", j, false);
        JsonSerialize(timestamp, "timestamp", j, false);
        JsonSerialize(device_esn, "device_esn", j, false);
        JsonSerialize(data, "data", j, false);
    }

    virtual void deserialize(const json& j) override {
        JsonDeserialize(tag, "tag", j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum, "seqnum", j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn, "device_esn", j, noUse_isEmptyFlag);
        JsonDeserialize(data, "data", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "tag: "                     << tag << std::endl;
        ss << std::left << std::setw(40) << "seqnum: "                 << seqnum << std::endl;
        ss << std::left << std::setw(40) << "timestamp: "             << timestamp << std::endl;
        ss << std::left << std::setw(40) << "device_esn: "            << device_esn << std::endl;
        ss << std::left << std::setw(40) << "data: "              << data.to_string() << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
