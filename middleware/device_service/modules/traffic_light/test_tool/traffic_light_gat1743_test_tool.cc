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

#include <cassert>
#include <iostream>
#include <string>
#include <thread>

#include "base/common/log.h"
#include "base/device_connect/traffic_light/device_factory.h"

using os::v2x::device::TrafficLightDataType;
using os::v2x::device::TrafficLightDeviceFactory;

void callback(const TrafficLightDataType &traffic_data) {
  if (traffic_data) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << traffic_data->DebugString();
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "Args Error!\nUsage: traffic_light_gat1743_tool conf_file";
    return 1;
  }

  auto gat_traffic = TrafficLightDeviceFactory::Instance().GetUnique(
      "gat_traffic_light", callback);

  if (gat_traffic == nullptr) {
    return 0;
  }
  if (!gat_traffic->Init(std::string(argv[1]))) {
    return 0;
  }
  gat_traffic->Start();

  TRAFFIC_LIFGT_SERVICE_LOG_WARN << "gat traffic light demo device start";
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  return 0;
}
