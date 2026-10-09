/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_TRAFFIC_LIGHT_SRC_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_TRAFFIC_LIGHT_SRC_DATA_H
#include "middleware/protocol/om_common/namespace.h"

NAMESPACE_START_OM_COMPONENT_MEC
using namespace afl::base;
struct TrafficLightSrcData : public afl::base::SerializableData
{
    uint64_t     timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string lightEsn;
    std::string messType;
    std::string message;

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(lightEsn, "lightEsn", j, false);
        JsonSerialize(messType, "messType", j, false);
        JsonSerialize(message, "message", j, false);
    }
    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(timeStamp, "timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(seqNum, "seqNum", j, g_flag_empty_novalid);
        JsonDeserialize(rscuEsn, "rscuEsn", j, g_flag_empty_novalid);
        JsonDeserialize(lightEsn, "lightEsn", j, g_flag_empty_novalid);
        JsonDeserialize(messType, "messType", j, g_flag_empty_novalid);
        JsonDeserialize(message, "message", j, g_flag_empty_novalid);
    }
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(30) << "timeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(30) << "seqNum: " << seqNum << std::endl;
        ss << std::left << std::setw(30) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(30) << "lightEsn: " << lightEsn << std::endl;
        ss << std::left << std::setw(30) << "messType: " << messType << std::endl;
        ss << std::left << std::setw(30) << "message: " << message << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC

#endif