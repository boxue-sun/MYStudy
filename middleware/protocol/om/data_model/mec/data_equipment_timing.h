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
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_EQUIPMENT_TIMING_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_EQUIPMENT_TIMING_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;
struct Peripheral : public afl::base::SerializableData
{
    std::string deviceEsn;
    uint64_t devTime;
    int64_t lastTime;
    int64_t timeDiff;
    virtual void serialize(json& j) override
    {
        JsonSerialize(deviceEsn, 	"deviceEsn", 	j, false);
        JsonSerialize(devTime, 	"devTime", 	j, false);
        JsonSerialize(lastTime, 	"lastTime", 	j, false);
        JsonSerialize(timeDiff, 	"timeDiff", 	j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(deviceEsn, 	"deviceEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(devTime, 	"devTime", 	j, noUse_isEmptyFlag);
        JsonDeserialize(lastTime, 	"lastTime", 	j, noUse_isEmptyFlag);
        JsonDeserialize(timeDiff, 	"timeDiff", 	j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "DeviceEsn: " << deviceEsn << std::endl;
        ss << std::left << std::setw(40) << "DevTime: " << devTime << std::endl;
        ss << std::left << std::setw(40) << "LastTime: " << lastTime << std::endl;
        ss << std::left << std::setw(40) << "TimeDiff: " << timeDiff << std::endl;

        return ss.str();
    }
};
//设备授时
struct EquipmentTimingData : public afl::base::SerializableData
{
    uint64_t     timeStamp = 0;
    std::string seqNum = "";
    std::string rscuEsn = "";
    int64_t lastTime = 0;
    int timeNode = 0;
    int64_t ptpDifference = 0;
    int64_t gpsDifference = 0;

//    std::vector<Peripheral> peripherals;
//    bool peripheralsEmpty = true;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, 	"timestamp", 	j, false);
        JsonSerialize(seqNum, 	"seqNum", 	j, false);
        JsonSerialize(rscuEsn, 	"rscuEsn", 	j, false);
        JsonSerialize(lastTime, 	"lastTime", 	j, false);
        JsonSerialize(timeNode, 	"timeNode", 	j, false);
        JsonSerialize(ptpDifference, 	"ptpDifference", 	j, false);
        JsonSerialize(gpsDifference, 	"gpsDifference", 	j, false);
//        JsonSerialize(peripherals, 	"peripherals", 	j, peripheralsEmpty);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, 	"timestamp", 	j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, 	"seqNum", 	j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, 	"rscuEsn", 	j, noUse_isEmptyFlag);
        JsonDeserialize(lastTime, 	"lastTime", 	j, noUse_isEmptyFlag);
        JsonDeserialize(timeNode, 	"timeNode", 	j, noUse_isEmptyFlag);
        JsonDeserialize(ptpDifference, 	"ptpDifference", 	j, noUse_isEmptyFlag);
        JsonDeserialize(gpsDifference, 	"gpsDifference", 	j, noUse_isEmptyFlag);
//        JsonDeserialize(peripherals, 	"peripherals", 	j, peripheralsEmpty);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "RSCU ES/N: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Last Time: " << lastTime << std::endl;
        ss << std::left << std::setw(40) << "Time Node: " << timeNode << std::endl;
        ss << std::left << std::setw(40) << "Gps Time Difference: " << gpsDifference << std::endl;
        ss << std::left << std::setw(40) << "Ptp Time Difference: " << ptpDifference << std::endl;
//        ss << std::left << std::setw(40) << "Peripherals:\n";
//        for (const auto& peripheral : peripherals) {
//            ss << peripheral.to_string() << std::endl;
//        }
        return ss.str();
    }
};
NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
