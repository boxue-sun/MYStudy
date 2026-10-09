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

#include <netinet/in.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "base/device_connect/rsu/device_base.h"
#include "middleware/device_service/modules/rsu/standard_rsu/protocol/trans_protocol.h"

namespace os {
namespace v2x {
namespace device {

class StandardRsu : public RSUDevice {
 public:
  StandardRsu(const RSUCallBack& cb)
      : RSUDevice(cb)
      , socket_fd_(-1) {}
  ~StandardRsu() {
    Stop();
  }

  bool Init(const std::string& conf) override;
  void Start() override;
  void WriteToDevice(
      const std::shared_ptr<const os::v2x::device::RSUData>& re_proto) override;
  RSUDeviceState GetState() override;

 private:
  void Stop();
  void TaskRecvData();

 private:
  int socket_fd_;
  std::atomic<bool> stop_ = ATOMIC_VAR_INIT(false);
  struct sockaddr_in peer_addr_;
  std::unique_ptr<std::thread> recv_thread_                = nullptr;
  std::shared_ptr<TransProtocol> transfer_                 = nullptr;
  std::shared_ptr<os::v2x::device::RSUData> rsu_data_recv_ = nullptr;
  std::mutex recv_mutex_;
  std::condition_variable recv_condition_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
