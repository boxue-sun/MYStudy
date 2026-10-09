/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef MIDDLEWART_PROTOCOL_DATA_MODEL_COMMUNICATION_DEVICE_SCENARIO_RECEIVING_DATA
#define MIDDLEWART_PROTOCOL_DATA_MODEL_COMMUNICATION_DEVICE_SCENARIO_RECEIVING_DATA
#include "middleware/protocol/om_common/namespace.h"
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;
struct CommunicationDeviceScenarioReceivingData  :  public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string rscuEsn;
    std::string deviceID;
public:
    virtual void serialize(afl::base::json& j)
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(deviceID, "deviceID", j, false);
    }
    virtual void deserialize(const afl::base::json& j)
    {
        JsonDeserialize(timeStamp, "timestamp", j, g_flag_empty_novalid);
        JsonDeserialize(rscuEsn, "rscuEsn", j, g_flag_empty_novalid);
        JsonDeserialize(deviceID, "deviceID", j, g_flag_empty_novalid);
    }
};


NAMESPACE_ENDED_OM_COMPONENT_COMMON

#endif