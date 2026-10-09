/*********************************************************************************
 * @file		app_flag.cpp
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

#include "app_flag.h"

namespace airos {
namespace app {
DEFINE_string(v2x_scenario_asn_message_version, "new_4layer",
              "v2x message Asn.1 file version");
DEFINE_bool(enable_send, true, "enable send");
}  // namespace app
}  // namespace airos
