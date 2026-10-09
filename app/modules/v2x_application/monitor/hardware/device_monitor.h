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
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "app/modules/v2x_application/proto/rscu_status.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"

namespace airos {
namespace monitor {

using DeviceVector = std::vector<::airos::v2x::DeviceStatus>;
using PostCallBack = std::function<void(
    std::string, std::shared_ptr<os::v2x::device::CloudData>&)>;

class DeviceMonitor {
 public:
  DeviceMonitor();
  virtual ~DeviceMonitor() {
    Stop();
  }
  void SetPostCallBack(const PostCallBack& cb) {
    post_cb_ = cb;
  }

 private:
  void LoadDeviceConfig();
  bool StartPingAllDevice();
  void UploadRscuStatus();
  bool PingDevice(const std::string device_ip);
  bool GetSignalMachineInfo(DeviceVector* vec_device);
  bool GetRsuInfo(DeviceVector* vec_device);
  std::string GetConfigPathFromDesc(const std::string& desc_file);
  void Stop();

  std::unique_ptr<std::thread> ping_thread_   = nullptr;
  std::unique_ptr<std::thread> upload_thread_ = nullptr;

  std::string rscu_sn_;
  std::mutex status_mutex_;
  bool exit_                                = false;
  const int PING_INTERNAL                   = 2;
  const int UPLOAD_INTERNAL                 = 5;
  std::string MQTT_RSCU_STATUS_TOPIC_PREFIX = "upload/status/";
  std::shared_ptr<::airos::v2x::RscuStatus> rscu_status_;
  PostCallBack post_cb_ = nullptr;
};

}  // namespace monitor
}  // namespace airos
