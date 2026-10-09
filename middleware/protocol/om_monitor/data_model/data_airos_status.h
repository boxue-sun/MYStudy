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
#ifndef MQTT_CLIENT_MEC_OM_MONITOR_DATA_AIROS_STATUS_H
#define MQTT_CLIENT_MEC_OM_MONITOR_DATA_AIROS_STATUS_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"

NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;
struct ProcessInfo
{
    std::string pid;          // 进程ID
    std::string comm;         // 用户
    std::string lstart;         // 启动时间
    std::string etime;         // 经过的时间
    double cpu;                // CPU 使用率
    double mem;                // mem 使用率
    std::string cmd;        // 开始时间
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "pid: "           << pid << std::endl;
        ss << std::left << std::setw(40) << "comm: "           << comm << std::endl;
        ss << std::left << std::setw(40) << "lstart: "            << lstart << std::endl;
        ss << std::left << std::setw(40) << "etime: "          << etime << std::endl;
        ss << std::left << std::setw(40) << "cpu: "            << cpu << std::endl;
        ss << std::left << std::setw(40) << "mem: "          << mem << std::endl;
        ss << std::left << std::setw(40) << "cmd: "           << cmd << std::endl;
        return ss.str();
    }
};

struct ProcessMoudlesNeedInfo
{
    std::string user;         // 用户
    std::string pid;          // 进程ID
    std::string ppid;         // 父进程ID
    float cpu;                // CPU 使用率
    std::string start;        // 开始时间
    std::string tty;          // TTY
    std::string cpuTime;      // CPU 时间
    std::string command_name;
    std::string command_type;
    std::string moudle_name;
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "user: "          << user << std::endl;
        ss << std::left << std::setw(40) << "pid: "           << pid << std::endl;
        ss << std::left << std::setw(40) << "ppid: "           << ppid << std::endl;
        ss << std::left << std::setw(40) << "cpu: "            << cpu << std::endl;
        ss << std::left << std::setw(40) << "start: "          << start << std::endl;
        ss << std::left << std::setw(40) << "tty: "            << tty << std::endl;
        ss << std::left << std::setw(40) << "cpuTime: "          << cpuTime << std::endl;
        ss << std::left << std::setw(40) << "command_name: "           << command_name << std::endl;
        ss << std::left << std::setw(40) << "command_type: "           << command_type << std::endl;
        ss << std::left << std::setw(40) << "moudle_name: "           << moudle_name << std::endl;
        return ss.str();
    }
};



struct AirosMoudlesData  : public afl::base::SerializableData
{
    std::string name = "";
    std::string pid = "";
    double cpu = 999.999;
    double mem = 999.999;
    std::string start_time = "";

    void serialize(json& j) override
    {
        JsonSerialize(name,                 "name",             j, false);
        JsonSerialize(pid,              "pid",           j, false);
        JsonSerialize(cpu,                "cpu",            j, false);
        JsonSerialize(mem,                 "mem",             j, false);
        JsonSerialize(start_time,                 "start_time",             j, false);

    }

    void deserialize(const json& j)  override
    {
        JsonDeserialize(name,                 "name",             j, noUse_isEmptyFlag);
        JsonDeserialize(pid,              "pid",           j, noUse_isEmptyFlag);
        JsonDeserialize(cpu,                "cpu",            j, noUse_isEmptyFlag);
        JsonDeserialize(mem,                 "mem",             j, noUse_isEmptyFlag);
        JsonDeserialize(start_time,                 "start_time",             j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(10) <<  std::setfill(' ') << " [name]"         << std::setw(30) <<  std::setfill(' ')   << name ;
        ss << std::left << std::setw(10) <<  std::setfill(' ')<< " [pid]"        << std::setw(10) <<  std::setfill(' ')  << pid ;
        ss << std::left << std::setw(10) <<  std::setfill(' ')<< " [cpu]"         << std::setw(10) <<  std::setfill(' ')  << cpu ;
        ss << std::left << std::setw(10) <<  std::setfill(' ')<< " [mem]"           << std::setw(10) <<  std::setfill(' ') << mem ;
        ss << std::left << std::setw(10) <<  std::setfill(' ') << " [start_time]"    << std::setw(20) <<  std::setfill(' ')        << start_time <<std::endl;

        return ss.str();
    }
};


struct AirosStatusData  : public afl::base::SerializableData
{
    std::string container_name = "";
    std::string container_start_time = "";
    std::vector<AirosMoudlesData> modules_status;
    void serialize(json& j) override
    {
        JsonSerialize(container_name,              "container_name",             j, false);
        JsonSerialize(container_start_time,              "container_start_time",             j, false);
        JsonSerialize(modules_status,                 "modules_status",             j, false);
    }

    void deserialize(const json& j)  override
    {
        JsonDeserialize(container_name,              "container_name",             j, noUse_isEmptyFlag);
        JsonDeserialize(container_start_time,              "container_start_time",             j, noUse_isEmptyFlag);
        JsonDeserialize(modules_status,                 "modules_status",             j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const {
        std::stringstream ss;
        ss << std::left << std::setw(40) <<  std::setfill(' ') << "container_name: "               << std::setw(30) <<  std::setfill(' ')    << container_name << std::endl;
        ss << std::left << std::setw(40) <<  std::setfill(' ') << "container_start_time: "            << std::setw(20) <<  std::setfill(' ')      << container_start_time << std::endl;
        ss << std::left << std::setw(40) <<  std::setfill(' ') << "modules_status: "      << std::setw(20) <<  std::setfill(' ')  << std::endl;
        for(auto module : modules_status)
        {
            ss << std::left << std::setw(40) << "module_status: " << module.to_string();
        }
        return ss.str();
    }
};

struct AirosStatus : public afl::base::SerializableData
{
    QUERY_MSG_TAG tag;
    std::string seqnum;
    uint64_t timestamp;
    std::string device_esn;
    AirosStatusData data;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(tag,                                 "tag",                         j, false);
        JsonSerialize(seqnum,                              "seqnum",                      j, false);
        JsonSerialize(timestamp,                           "timestamp",                   j, false);
        JsonSerialize(device_esn,                          "device_esn",                 j, false);
        JsonSerialize(data,                          "data",                 j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(tag,                                "tag",                         j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum,                             "seqnum",                      j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp,                          "timestamp",                   j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn,                         "device_esn",                 j, noUse_isEmptyFlag);
        JsonDeserialize(data,                          "data",                 j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "tag: "                           << std::setw(20) <<  std::setfill(' ')  << tag << std::endl;
        ss << std::left << std::setw(40) << "seqnum: "                        << std::setw(20) <<  std::setfill(' ')  << seqnum << std::endl;
        ss << std::left << std::setw(40) << "timestamp: "                     << std::setw(20) <<  std::setfill(' ')  << timestamp << std::endl;
        ss << std::left << std::setw(40) << "device_esn: "                    << std::setw(20) <<  std::setfill(' ')  << device_esn << std::endl;
        ss << std::left << std::setw(40) << "data: "                << std::endl  << data.to_string() << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
