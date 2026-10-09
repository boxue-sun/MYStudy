/*********************************************************************************
 * @file		equipment_timing_data.h
 * @brief		equipment_timing_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-18
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-18 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS2_0_DATA_SYSCHRONIZE_H
#define AIROS2_0_DATA_SYSCHRONIZE_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "base/common/network/serializable_data.h"
NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS
using namespace afl::base;
struct CommonSyschronize : public afl::base::SerializableData
{
    uint64_t     timeStamp;
    std::string seqNum;
    std::string deviceID;
    uint64_t devTime;
    uint64_t lastTime;
    int64_t timeDiff;
private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(deviceID, "deviceID", j, false);
        JsonSerialize(devTime, "devTime", j, false);
        JsonSerialize(lastTime, "lastTime", j, false);
        JsonSerialize(timeDiff, "timeDiff", j, false);
    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID, "deviceID", j, noUse_isEmptyFlag);
        JsonDeserialize(devTime, "devTime", j, noUse_isEmptyFlag);
        JsonDeserialize(lastTime, "lastTime", j, noUse_isEmptyFlag);
        JsonDeserialize(timeDiff, "timeDiff", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp <<  std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "deviceID: " << deviceID <<  std::endl;
        ss << std::left << std::setw(40) << "devTime: " << devTime <<  std::endl;
        ss << std::left << std::setw(40) << "lastTime: " << lastTime <<  std::endl;
        ss << std::left << std::setw(40) << "timeDiff: " << timeDiff <<  std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS
#endif //AIROS2_0_DATA_SYSCHRONIZE_H
