/*********************************************************************************
 * @file		device_management_data.h
 * @brief		device_management_data belongs to CICTCI
 * @details		
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-17
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-17 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_DEVICE_INFO_QUERY_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_DEVICE_INFO_QUERY_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;

struct DeviceBaseInfoQueryData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    DeviceManagementInfoIdTypeEnum infoId;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, 	"timestamp", 	j, false);
        JsonSerialize(seqNum, 	"seqNum", 	j, false);
        JsonSerialize(rscuEsn, 	"rscuEsn", 	j, false);
        JsonSerialize(infoId, 	"infoId", 	j, false);

    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, 	"timestamp", 	j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, 	"seqNum", 	j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, 	"rscuEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(infoId, 	"infoId", 	j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "[timestamp]" << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "[seqNum]" << seqNum << std::endl;
        ss << std::left << std::setw(40) << "[rscuEsn]" << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "[infoId]" << infoId << std::endl;
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
