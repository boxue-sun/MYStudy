/*********************************************************************************
 * @file		configer_radar_static
 * @brief		configer_radar_static belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-12-3
 * @copyright	Copyright (c) 2024 Mec-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-12-3 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef MIDDLEWART_PROTOCOL_CCINDEX_DATA_MODEL_CONFIG_QUERY_DATA
#define MIDDLEWART_PROTOCOL_CCINDEX_DATA_MODEL_CONFIG_QUERY_DATA
#include "middleware/protocol/om_common/namespace.h"

NAMESPACE_START_RADAR_RADAR_STATIC
using namespace afl::base;
struct ConfigQuery : public afl::base::SerializableData
{
    std::string cross_id;     // 百度信控路口编号
    std::string vendor;       // 供应商名称
    std::string category;     // 设备类型

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(cross_id,   "cross_id",   j, false);
        JsonSerialize(vendor,     "vendor",     j, false);
        JsonSerialize(category,   "category",   j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(cross_id,   "cross_id",   j, noUse_isEmptyFlag);
        JsonDeserialize(vendor,     "vendor",     j, noUse_isEmptyFlag);
        JsonDeserialize(category,   "category",   j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "cross_id: "    << cross_id    << std::endl;
        ss << std::left << std::setw(40) << "vendor: "      << vendor      << std::endl;
        ss << std::left << std::setw(40) << "category: "    << category    << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_RADAR_RADAR_STATIC

#endif