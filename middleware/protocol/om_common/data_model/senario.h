/*
 * @Author: zhangenwei
 * @Date: 2024-08-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_PROTOCOL_DATA_MODEL_SENARION_DATA
#define MIDDLEWART_PROTOCOL_DATA_MODEL_SENARION_DATA
#include "middleware/protocol/om_common/namespace.h"

NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;
struct SenarioData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string asn1Data;

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(asn1Data, "asn1Data", j, false);
    }
    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(timeStamp, "timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(seqNum, "seqNum", j, g_flag_empty_novalid);
        JsonDeserialize(rscuEsn, "rscuEsn", j, g_flag_empty_novalid);
        JsonDeserialize(asn1Data, "asn1Data", j, g_flag_empty_novalid);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeStamp: "   << timeStamp   << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "      << seqNum      << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: "     << rscuEsn     << std::endl;
        ss << std::left << std::setw(40) << "asn1Data: "    << asn1Data    << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON

#endif