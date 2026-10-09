/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#pragma once

#include <gflags/gflags.h>

namespace airos {
namespace app {

DECLARE_string(city_string);
DECLARE_string(asn_message_version);
DECLARE_int32(xml_map_send_rate);
DECLARE_bool(enable_send_ssm);
DECLARE_bool(enable_send_map);
DECLARE_bool(enable_send_spat);
DECLARE_bool(enable_send_rsi);
DECLARE_bool(enable_send_rsm);
DECLARE_bool(enable_print);


DECLARE_string(cloud_mqtt_addr);
DECLARE_string(cloud_mqtt_user);
DECLARE_string(cloud_mqtt_passwd);
DECLARE_int32(cloud_mqtt_connect_timeout);

}  // namespace app
}  // namespace airos
