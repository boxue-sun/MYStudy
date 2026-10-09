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

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "base/device_connect/proto/cloud_data.pb.h"
#include "base/device_connect/cloud/device_base.h"
#include "base/common/mqtt_client_wrapper.h"

namespace os {
namespace v2x {
namespace device {

class StandardMqtt : public CloudDevice {
 public:
  StandardMqtt(const CloudCallBack& cb)
      : CloudDevice(cb) {}
  ~StandardMqtt() {
    Stop();
  }

  bool Init(const YAML::Node& root_node) override;
  void Start() override;
  void WriteToDevice(const std::shared_ptr<const os::v2x::device::CloudData>&
                         re_proto) override;
  CloudDeviceState GetState() override;

  void ReConnect();
  int Connect();
  void RcvData();

 private:
  void Stop();

 private:
  std::string mqtt_username_;
  std::string mqtt_password_;
  std::string cloud_url_;
  std::string client_id_;
  int timeout_s_;
  std::vector<std::string> subscribe_topic_;

  std::shared_ptr<std::thread> mq_rcv_ptr_{nullptr};
  std::atomic<bool> mq_rcv_{true};

  std::shared_ptr<std::thread> mq_reconn_ptr_{nullptr};
  std::atomic<bool> mq_reconn_{true};
  std::shared_ptr<airos::base::MQTTClientInterface> m_mqtt_client_{nullptr};
};

}  // namespace device
}  // namespace v2x
}  // namespace os
