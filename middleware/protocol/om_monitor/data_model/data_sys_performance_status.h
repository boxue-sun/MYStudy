/*********************************************************************************
 * @file		data_sys_performance_status.h
 * @brief		data_sys_performance_status belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       xx.com
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
#ifndef MQTT_CLIENT_MEC_OM_MONITOR_SYS_PERFORMANCE_STATUS_H
#define MQTT_CLIENT_MEC_OM_MONITOR_SYS_PERFORMANCE_STATUS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/om_monitor/data_model/data_docker_status.h"
#include "middleware/protocol/om_monitor/data_model/data_airos_status.h"
#include "middleware/protocol/om_monitor/data_model/data_disk_status.h"

NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;

struct SysPerformanceData  : public afl::base::SerializableData
{
    double cpu = 999.999;
    double mem_percent = 999.999;
    double mem = 999.999;
    bool run_flag = false;

    std::string container_name = "";
    std::string container_start_time = "";
    std::vector<AirosMoudlesData> modules_status;

    double root_size = 999.999;
    double root_avail = 999.999;
    double root_avail_percent = 999.999;
    double work_size = 999.999;
    double work_avail = 999.999;
    double work_avail_percent = 999.999;


    void serialize(json& j) override
    {
        JsonSerialize(cpu, "cpu", j, false);
        JsonSerialize(mem_percent, "mem_percent", j, false);
        JsonSerialize(mem, "mem", j, false);
        JsonSerialize(run_flag, "run_flag", j, false);

        JsonSerialize(container_name,                 "container_name",             j, false);
        JsonSerialize(container_start_time,                 "container_start_time",             j, false);
        JsonSerialize(modules_status,                 "modules_status",             j, false);

        JsonSerialize(root_size, "root_size", j, false);
        JsonSerialize(root_avail, "root_avail", j, false);
        JsonSerialize(root_avail_percent, "root_avail_percent", j, false);
        JsonSerialize(work_size, "work_size", j, false);
        JsonSerialize(work_avail, "work_avail", j, false);
        JsonSerialize(work_avail_percent, "work_avail_percent", j, false);

    }

    void deserialize(const json& j)  override
    {
        JsonDeserialize(cpu, "cpu", j, noUse_isEmptyFlag);
        JsonDeserialize(mem_percent, "mem_percent", j, noUse_isEmptyFlag);
        JsonDeserialize(mem, "mem", j, noUse_isEmptyFlag);
        JsonDeserialize(run_flag, "run_flag", j, noUse_isEmptyFlag);

        JsonDeserialize(container_name,                "container_name",             j, noUse_isEmptyFlag);
        JsonDeserialize(container_start_time,                "container_start_time",             j, noUse_isEmptyFlag);
        JsonDeserialize(modules_status,                 "modules_status",             j, noUse_isEmptyFlag);

        JsonDeserialize(root_size, "root_size", j, noUse_isEmptyFlag);
        JsonDeserialize(root_avail, "root_avail", j, noUse_isEmptyFlag);
        JsonDeserialize(root_avail_percent, "root_avail_percent", j, noUse_isEmptyFlag);
        JsonDeserialize(work_size, "work_size", j, noUse_isEmptyFlag);
        JsonDeserialize(work_avail, "work_avail", j, noUse_isEmptyFlag);
        JsonDeserialize(work_avail_percent, "work_avail_percent", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "cpu: "              << cpu << std::endl;
        ss << std::left << std::setw(40) << "mem_percent: "              << mem_percent << std::endl;
        ss << std::left << std::setw(40) << "mem: "              << mem << std::endl;
        ss << std::left << std::setw(40) << "run_flag: "         << (run_flag ? "true" : "false") << "\n";
        ss << std::left << std::setw(40) << "container_name: "            << container_name << std::endl;
        ss << std::left << std::setw(40) << "container_start_time: "            << container_start_time << std::endl;
        ss << std::left << std::setw(40) << "modules_status: "  << std::endl;
        for(auto module_head : modules_status)
        {
            ss << std::left << std::setw(40) << "module_status: " << module_head.to_string() << std::endl;
        }

        ss << std::left << std::setw(40) << "root_size: "              << root_size << std::endl;
        ss << std::left << std::setw(40) << "root_avail: "              << root_avail << std::endl;
        ss << std::left << std::setw(40) << "root_avail_percent: "              << root_avail_percent << std::endl;
        ss << std::left << std::setw(40) << "work_size: "         << work_size << std::endl;
        ss << std::left << std::setw(40) << "work_avail: "              << work_avail << std::endl;
        ss << std::left << std::setw(40) << "work_avail_percent: "         << work_avail_percent << std::endl;

        return ss.str();
    }
};

struct SysPerformanceStatus : public afl::base::SerializableData
{
    QUERY_MSG_TAG tag; // 接口号
    std::string seqnum; // 32位不重复的随机数
    uint64_t timestamp; // utc时间戳，毫秒
    std::string device_esn; // 设备esn号
    SysPerformanceData data; // 消息结构

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
