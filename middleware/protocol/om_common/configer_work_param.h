/*********************************************************************************
 * @file		configer_work_param.h
 * @brief		configer_work_param belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-6-19
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-6-19 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS2_0_CONFIGER_WORK_PARAM_H
#define AIROS2_0_CONFIGER_WORK_PARAM_H
#include "namespace.h"
#include "configer_common.h"

NAMESPACE_START_OM_COMPONENT_COMMON
using namespace airos::base::workparam;
using namespace os::v2x::protocol::om::common;

struct WorkParamConfiger : public afl::base::SerializableData
{
public:
    std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag";
    std::string configerVersionFilePath = "/home/airos/os/.airos_version";
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(workParamFilePath, "A_workParamFilePath", j, false);
        JsonSerialize(configerVersionFilePath, "B_configerVersionFilePath", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(workParamFilePath, "A_workParamFilePath", j, noUse_isEmptyFlag);
        JsonDeserialize(configerVersionFilePath, "B_configerVersionFilePath", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "WorkParamFilePath: " << workParamFilePath <<  std::endl;
        ss << std::left << std::setw(40) << "ConfigerVersionFilePath: " << workParamFilePath <<  std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
