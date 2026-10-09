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

#ifndef AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_QUERY_H
#define AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_QUERY_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "inter_exter_param.h"
NAMESPACE_START_OM_COMPONENT_CAMERA
#include <string>
#include <vector>
using namespace os::v2x::protocol::om::camera;

struct CameraCalibrationQueryData : public afl::base::SerializableData
{
    uint64_t        timestamp;
    std::string     seqNum;               // e.g., "18.22.57.11"
    std::string deviceID;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                     "timestamp",                     j, false);
        JsonSerialize(seqNum,                        "seqNum",                        j, false);
        JsonSerialize(deviceID,                      "deviceID",                      j, false);
    }

    virtual void deserialize(const json &j) override
    {

        JsonDeserialize(timestamp,                  "timestamp",                     j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                     "seqNum",                        j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID,                   "deviceID",                      j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "                     << timestamp                    << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                        << seqNum                       << std::endl;
        ss << std::left << std::setw(40) << "deviceID: "                      << deviceID                     << std::endl;

        return ss.str();
    }
};


NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
