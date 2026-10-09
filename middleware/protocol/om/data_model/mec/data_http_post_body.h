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

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_HTTP_POST_BODY_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_HTTP_POST_BODY_H
#include "middleware/protocol/om_common/data_model/data_common.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;
struct httpPostBody: public afl::base::SerializableData
{
    string device_sn;
    string type;
    int64_t start_time;
    int64_t end_time;
private:
    virtual void serialize(json &j) {
        JSON_SERIALIZE_VARS(
                device_sn, type, start_time, end_time
        );
    }

    virtual void deserialize(const json &j) override {
        JSON_DESERIALIZE_VARS(
                device_sn, type, start_time, end_time
        );
    }
};

NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
