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

#include <memory>
#include <string>

#include "app/modules/v2x_application/common/producer_consumer_queue.h"
#include "app/modules/v2x_application/monitor/hardware/device_monitor.h"
#include "base/common/log.h"

namespace airos {
namespace monitor {

class Monitor {
 public:
  Monitor()          = default;
  virtual ~Monitor() = default;

  bool Init(
      app::ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
          cloud_sequence);
  void PostImpl(
      std::string channel,
      const std::shared_ptr<os::v2x::device::CloudData>& data) {
    cloud_sequence_->push(data);
  }

 private:
  std::shared_ptr<DeviceMonitor> device_monitor_ = nullptr;
  app::ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
      cloud_sequence_;
};

}  // namespace monitor
}  // namespace airos
