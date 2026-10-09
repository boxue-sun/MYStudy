/*********************************************************************************
 * @file		maintenance_management_ota_data.h
 * @brief		maintenance_management_ota_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-21 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_SPAT_CHECK_STATUS_DATA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_SPAT_CHECK_STATUS_DATA_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;
enum TrafficlightDetectStatus
{
    TLS_NORMAL = 0,  //正常
    TLS_BLACK_FAILURE = 1,  //黑灯故障
    TLS_LIGHT_COLOR_CONFLICT = 2,  //灯色冲突
    TLS_COUNTDOWN_MISMATCH_WITH_ON_SITE_CONDITIONS = 3,  //倒计时与现场不一致
    TLS_SECOND_FREEZING = 4,  //卡秒
    TLS_SKIPPING_SECONDS = 5,  //跳秒
    TLS_COUNTDOWN_REVERTING = 6,  //回跳
    TLS_COUNTDOWN_NOT_EQUAL_TO_1_WHEN_CHANGING_LIGHTS = 7,  //切灯时倒计时不为1
    TLS_ALL_RED = 8, //全红
    TLS_ALL_GREEN = 9, //全绿
    TLS_ALL_YELLOW = 10, //全黄
};
struct TrafficlightDetectData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    TrafficlightDetectStatus trafficlightStatus;
    int faultStatus;
    long faultStartTime;
    long faultStopTime;
    std::string faultDescription;


    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(trafficlightStatus, "trafficlightStatus", j, false);
        JsonSerialize(faultStatus, "faultStatus", j, false);
        JsonSerialize(faultStartTime, "faultStartTime", j, false);
        JsonSerialize(faultStopTime, "faultStopTime", j, false);
        JsonSerialize(faultDescription, "faultDescription", j, false);

    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(trafficlightStatus, "trafficlightStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(faultStatus, "faultStatus", j, noUse_isEmptyFlag);
        JsonDeserialize(faultStartTime, "faultStartTime", j, noUse_isEmptyFlag);
        JsonDeserialize(faultStopTime, "faultStopTime", j, noUse_isEmptyFlag);
        JsonDeserialize(faultDescription, "faultDescription", j, noUse_isEmptyFlag);


    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "seqNum: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "trafficlightStatus: " << trafficlightStatus << std::endl;
        ss << std::left << std::setw(40) << "faultStatus: " << faultStatus << std::endl;
        ss << std::left << std::setw(40) << "faultStartTime: " << faultStartTime << std::endl;
        ss << std::left << std::setw(40) << "faultStopTime: " << faultStopTime << std::endl;
        ss << std::left << std::setw(40) << "faultDescription: " << faultDescription << std::endl;

        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
