/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_PROTOCOL_DATA_MODEL_TRAFFIC_LIGHT_DATA
#define MIDDLEWART_PROTOCOL_DATA_MODEL_TRAFFIC_LIGHT_DATA
#include "middleware/protocol/om_common/namespace.h"


NAMESPACE_START_OM_COMPONENT_MEC
using namespace afl::base;
struct TrafficLightData : public afl::base::SerializableData
{
    uint64_t time;
    std::string rscuEsn;
    std::string sceneNo = "1000";
    std::string mesCon;

    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(time, "time", j, g_flag_empty_novalid);
        JsonSerialize(rscuEsn, "MECEsn", j, g_flag_empty_novalid);
        JsonSerialize(sceneNo, "sceneNo", j, g_flag_empty_novalid);
        JsonSerialize(mesCon, "mesCon", j, g_flag_empty_novalid);
    }
    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(time, "time", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "MECEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(sceneNo, "sceneNo", j, noUse_isEmptyFlag);
        JsonDeserialize(mesCon, "mesCon", j, noUse_isEmptyFlag);
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC

#endif