/*********************************************************************************
 * @file		maintenance_management_reboot_data.h
 * @brief		maintenance_management_reboot_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-22
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-22 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_REBOOT_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_REBOOT_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;

struct RebootData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string protocolVersion;
    uint64_t time;
    PowerOperationTypeEnum power;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(time, "time", j, false);
        JsonSerialize(power, "power", j, false); // Assuming power is an enum that can be cast to int
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(time, "time", j, noUse_isEmptyFlag);
        JsonDeserialize(power, "power", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "Rscu Esn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Protocol Version: " << protocolVersion << std::endl;
        ss << std::left << std::setw(40) << "Time: " << time << std::endl;
        ss << std::left << std::setw(40) << "Power: " << power << std::endl;
        return ss.str();
    }
};


struct RebootAckData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    StatusEnum status;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(status, "status", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(status, "status", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Sequence Number: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "RSCU Esn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Status: " << status << std::endl;

        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
