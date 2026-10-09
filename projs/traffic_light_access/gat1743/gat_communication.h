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

#include <arpa/inet.h>
#include <stdint.h>

#include <string>

namespace os {
namespace v2x {
namespace device {

/**
 * @brief 信号机通信类
 * @note 和信号机的通信
 */
class GatCommunication {
 public:
  enum class ProtocolType { UDP = 0, TCP = 1 };

  GatCommunication();
  ~GatCommunication();
  bool Init(
      const std::string &remote_ip, const uint16_t remote_port,
      const std::string &host_ip, const uint16_t host_port,
      const ProtocolType protocol);
  bool Connect();

  ssize_t SendData(uint8_t *send_data, size_t send_len);
  ssize_t RecvData(uint8_t *recv_buf, size_t recv_buf_len);
  ssize_t RecvDataWait(
      uint8_t *recv_buf, size_t recv_buf_len, int timeout_ms = 1000);

 private:
  bool UdpInit();
  bool TcpInit();

 private:
  int socket_fd_              = -1;
  ProtocolType protocol_type_ = ProtocolType::UDP;
  struct sockaddr_in remote_addr_;
  struct sockaddr_in host_addr_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
