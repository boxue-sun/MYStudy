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
#include "gtest/gtest.h"

#define private public
#include "app/modules/v2x_application/monitor/monitor.h"

namespace airos {
namespace monitor {

void PostImpl(
    std::string channel,
    const std::shared_ptr<os::v2x::device::CloudData>& data) {
  APP_LOG_WARN << channel << data->DebugString();
}

TEST(MonitorTest, test_device_monitor) {
  DeviceMonitor* device_monitor_ = new DeviceMonitor();
  device_monitor_->SetPostCallBack(
      std::bind(&PostImpl, std::placeholders::_1, std::placeholders::_2));
  delete device_monitor_;
}

}  // namespace monitor
}  // namespace airos
