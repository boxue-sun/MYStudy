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
#ifndef MQTT_CLIENT_MEC_OM_MONITOR_DATA_COMMON_H
#define MQTT_CLIENT_MEC_OM_MONITOR_DATA_COMMON_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MONITOR
using namespace afl::base;
using namespace os::v2x::protocol::om::common;
enum QUERY_MSG_TAG
{
    QUARY_TAG_DOCKER_STA = 3011,
    QUARY_TAG_AIROS_STA = 3012,
    QUARY_TAG_DISK_STA = 3013,
    QUARY_TAG_SYS_PERFORMANCE_STA = 3014,
    QUARY_TAG_RSAP_DATA_STA = 3021,
    QUARY_TAG_OM_DATA_STA = 3022,
    QUARY_TAG_CCINDEX_DATA_STA = 3023,
};

struct QueryData : public afl::base::SerializableData
{
    QUERY_MSG_TAG             tag;
    std::string     seqnum;
    uint64_t        timestamp;
    std::string     device_esn;
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(tag, "tag", j, false);
        JsonSerialize(seqnum, "seqnum", j, false);
        JsonSerialize(timestamp, "timestamp", j, false);
        JsonSerialize(device_esn, "device_esn", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(tag, "tag", j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum, "seqnum", j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn, "device_esn", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "tag: " << tag <<  std::endl;
        ss << std::left << std::setw(40) << "seqnum: " << seqnum <<  std::endl;
        ss << std::left << std::setw(40) << "timestamp: " << timestamp <<  std::endl;
        ss << std::left << std::setw(40) << "device_esn: " << device_esn <<  std::endl;
        return ss.str();
    }
};

struct QueryDataAck : public afl::base::SerializableData
{
    QUERY_MSG_TAG             tag;
    std::string     seqnum;
    uint64_t        timestamp;
    std::string     device_esn;
    bool            status;
private:
    virtual void serialize(json& j) override
    {
        JsonSerialize(tag, "tag", j, false);
        JsonSerialize(seqnum, "seqnum", j, false);
        JsonSerialize(timestamp, "timestamp", j, false);
        JsonSerialize(device_esn, "device_esn", j, false);
        JsonSerialize(status, "status", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(tag, "tag", j, noUse_isEmptyFlag);
        JsonDeserialize(seqnum, "seqnum", j, noUse_isEmptyFlag);
        JsonDeserialize(timestamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(device_esn, "device_esn", j, noUse_isEmptyFlag);
        JsonDeserialize(status, "status", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "tag: " << tag <<  std::endl;
        ss << std::left << std::setw(40) << "seqnum: " << seqnum <<  std::endl;
        ss << std::left << std::setw(40) << "timestamp: " << timestamp <<  std::endl;
        ss << std::left << std::setw(40) << "device_esn: " << device_esn <<  std::endl;
        ss << std::left << std::setw(40) << "status: " << status <<  std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MONITOR
#endif //NCS_MEC_ALARM_MANAGEMENT_DATA_H
