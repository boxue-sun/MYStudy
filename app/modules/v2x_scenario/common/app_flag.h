/*********************************************************************************
 * @file		app_flag.h
 * @brief		app_flag
 * @details		app_flag
 * @author		ChangXuhui
 * @date		2024/4/8
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/8  ChangXuhui      1.0          ————               ———— 
 *                                                             
 *
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <gflags/gflags.h>

namespace airos {
namespace app {

DECLARE_string(v2x_scenario_asn_message_version);
DECLARE_bool(enable_send);
}  // namespace app
}  // namespace airos
