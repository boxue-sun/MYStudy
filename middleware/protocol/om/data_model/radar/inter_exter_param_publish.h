/*********************************************************************************
* @file		inter_exter_param
* @brief	inter_exter_param belongs to CICTCI
* @details
* @author		alfred
* @email       zhangenwei64@gmail.com
* @date		24-6-9
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  25-3-4 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#ifndef AIROS2_0_CONFIGER_OM_RADAR_INTER_EXTER_PARAM_PUBLISH_H
#define AIROS2_0_CONFIGER_OM_RADAR_INTER_EXTER_PARAM_PUBLISH_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
NAMESPACE_START_OM_COMPONENT_RADAR
#include <string>
#include <vector>
using namespace os::v2x::protocol::om::radar;
struct RadarCalibrationPublishData : public afl::base::SerializableData
{
    uint64_t timeStamp;          // 时间戳，UTC时间，单位为毫秒，精确到毫秒
    std::string seqNum;      // 会话唯一标识
    std::string deviceID;    // 设备唯一标识码
    double longitude;        // 02坐标系的经度
    double latitude;         // 02坐标系的纬度
    double northAngle;      // 雷达偏北角
    bool    ack;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timeStamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(deviceID, "deviceID", j, false);
        JsonSerialize(longitude, "longitude", j, false);
        JsonSerialize(latitude, "latitude", j, false);
        JsonSerialize(northAngle, "northAngle", j, false);
        JsonSerialize(ack, "ack", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timeStamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID, "deviceID", j, noUse_isEmptyFlag);
        JsonDeserialize(longitude, "longitude", j, noUse_isEmptyFlag);
        JsonDeserialize(latitude, "latitude", j, noUse_isEmptyFlag);
        JsonDeserialize(northAngle, "northAngle", j, noUse_isEmptyFlag);
        JsonDeserialize(ack, "ack", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp<<  std::endl;
        ss << std::left << std::setw(40) << "seqNum:" << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "deviceID:" << deviceID <<  std::endl;
        ss << std::left << std::setw(40) << "longitude:" << deviceID <<  std::endl;
        ss << std::left << std::setw(40) << "latitude:" << deviceID <<  std::endl;
        ss << std::left << std::setw(40) << "northAngle:" << deviceID <<  std::endl;
        ss << std::left << std::setw(40) << "ack:" << deviceID <<  std::endl;
        return ss.str();
    }
};
enum RadarParamPubStatus
{
    RADAR_PARAM_PUB_SUCESS = 0,
    RADAR_PARAM_PUB_FAILURE = 1,

};
struct RadarCalibrationPublishAckData : public afl::base::SerializableData
{
    uint64_t timeStamp;          // 时间戳，UTC时间，单位为毫秒，精确到毫秒
    std::string seqNum;      // 会话唯一标识
    std::string deviceID;    // 设备唯一标识码
    RadarParamPubStatus    status;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timeStamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(deviceID, "deviceID", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timeStamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID, "deviceID", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "timeStamp: " << timeStamp<<  std::endl;
        ss << std::left << std::setw(40) << "seqNum:" << seqNum <<  std::endl;
        ss << std::left << std::setw(40) << "deviceID:" << deviceID <<  std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_RADAR
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
