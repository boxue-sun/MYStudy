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
#include <thread>

#include "app/modules/v2x_application/common/producer_consumer_queue.h"
#include "base/common/mqtt_client_wrapper.h"
#include "base/device_connect/proto/cloud_data.pb.h"

namespace airos {
namespace app {

class CloudAdapter {
 public:
  CloudAdapter() {}
  ~CloudAdapter();
  bool Init(ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
                cloud_sequence);

 private:
  void mqtt_reconnect();
  int mqtt_connect();
  void mqtt_send_data();

  std::shared_ptr<std::thread> mqtt_send_ptr_{nullptr};
  std::atomic<bool> mqtt_send_flag_{true};

  std::shared_ptr<std::thread> mqtt_reconnect_ptr_{nullptr};
  std::atomic<bool> mqtt_reconnect_flag_{true};

  std::string rscu_sn_;
  std::shared_ptr<base::MQTTClientInterface> m_mqtt_client_{nullptr};
  ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
      cloud_sequence_;
};

}  // namespace app
}  // namespace airos
