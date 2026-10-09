/*
 * @Author: zhangenwei
 * @Date: 2026-03-04 14:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_PROTOCOL_DATA_MODEL_TRAFFIC_LIGHT_DATA_STATUS
#define MIDDLEWART_PROTOCOL_DATA_MODEL_TRAFFIC_LIGHT_DATA_STATUS
#include "middleware/protocol/om_common/namespace.h"


NAMESPACE_START_OM_COMPONENT_MEC
using namespace afl::base;
struct TrafficLightDataStatus : public afl::base::SerializableData
{

    int64_t     timestamp;          // 时间戳，UTC时间，单位为毫秒，精确到毫秒

    std::string seqNum;             // 会话唯一标识

    std::string rscuEsn;            // 设备唯一标识码

    std::string lightEsn;           // 信号机序列号，作为信号机设备唯一标识

    int         status;             // 信号机数据状态：0：正常；1：异常；

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                     "timestamp",                     j, g_flag_empty_novalid);
        JsonSerialize(seqNum,                        "seqNum",                        j, g_flag_empty_novalid);
        JsonSerialize(rscuEsn,                       "rscuEsn",                       j, g_flag_empty_novalid);
        JsonSerialize(lightEsn,                      "lightEsn",                      j, g_flag_empty_novalid);
        JsonSerialize(status,                        "status",                       j, g_flag_empty_novalid);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timestamp,                   "timestamp",                     j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                      "seqNum",                        j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,                     "rscuEsn",                       j, noUse_isEmptyFlag);
        JsonDeserialize(lightEsn,                    "lightEsn",                      j, noUse_isEmptyFlag);
        JsonDeserialize(status,                      "status",                       j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "                     << timestamp                     << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                        << seqNum                        << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: "                       << rscuEsn                       << std::endl;
        ss << std::left << std::setw(40) << "lightEsn: "                      << lightEsn                      << std::endl;
        ss << std::left << std::setw(40) << "status: "                        << status                        << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC

#endif