/*********************************************************************************
 * @file		serializable_data.cc
 * @brief		serializable_data belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-05-16
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-05-16 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#include "serializable_data.h"

namespace afl{
namespace base{
bool g_flag_empty_novalid= false;
bool noUse_isEmptyFlag = false;
void to_json(json& j, const SerializableData& csd)
{
    SerializableData& sd = const_cast<SerializableData&>(csd);
    sd.serialize(j);
}

void from_json(const json& j, SerializableData& sd)
{
    sd.deserialize(j);
}

void to_json(json& j, const SerializableData* csd)
{
    SerializableData* sd = const_cast<SerializableData*>(csd);
    sd->serialize(j);
}
}
}

