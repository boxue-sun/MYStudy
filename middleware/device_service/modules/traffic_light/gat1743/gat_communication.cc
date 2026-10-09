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

#include "gat_communication.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "base/common/log.h"
#include "gat_util.h"

namespace os {
namespace v2x {
namespace device {

GatCommunication::GatCommunication() {}

GatCommunication::~GatCommunication() {
  if (socket_fd_ > 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
}

bool GatCommunication::Init(
    const std::string &remote_ip, const uint16_t remote_port,
    const std::string &host_ip, const uint16_t host_port,
    const ProtocolType protocol) {
  memset(&remote_addr_, 0, sizeof(remote_addr_));
  remote_addr_.sin_family      = AF_INET;
  remote_addr_.sin_addr.s_addr = inet_addr(remote_ip.c_str());
  remote_addr_.sin_port        = htons(remote_port);

  memset(&host_addr_, 0, sizeof(host_addr_));
  host_addr_.sin_family = AF_INET;
  if (host_ip.empty()) {
    host_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    host_addr_.sin_addr.s_addr = inet_addr(host_ip.c_str());
  }
  host_addr_.sin_port = htons(host_port);

  if ((protocol != ProtocolType::UDP) && (protocol != ProtocolType::TCP)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "not support protocol type: " << unsigned(protocol);
    return false;
  }

  return this->Connect();
}

bool GatCommunication::Connect() {
  switch (protocol_type_) {
    case ProtocolType::UDP:
      return this->UdpInit();
      break;
    case ProtocolType::TCP:
      return this->TcpInit();
      break;
  }

  return false;
}

ssize_t GatCommunication::SendData(uint8_t *send_data, size_t send_len) {
  if (socket_fd_ < 0) {
    return 0;
  }
  ssize_t ret_val = -1;
  switch (protocol_type_) {
    case ProtocolType::UDP:
      ret_val = sendto(
          socket_fd_,
          send_data,
          send_len,
          0,
          (struct sockaddr *)&remote_addr_,
          sizeof(remote_addr_));
      break;
    case ProtocolType::TCP:
      ret_val = send(socket_fd_, send_data, send_len, 0);
      break;
  }

  if (ret_val > 0) {
    GatUtil::DbgPrintBinary(send_data, send_len, "socket send : ");
  }
  return ret_val;
}

ssize_t GatCommunication::RecvData(uint8_t *recv_buf, size_t recv_buf_len) {
  if (socket_fd_ < 0) {
    return 0;
  }
  ssize_t recv_len = 0;
  struct sockaddr_in recv_addr;
  socklen_t sock_len;

  memset(recv_buf, 0, recv_buf_len);
  switch (protocol_type_) {
    case ProtocolType::UDP:
      sock_len = sizeof(recv_addr);
      recv_len = recvfrom(
          socket_fd_,
          recv_buf,
          recv_buf_len,
          0,
          (struct sockaddr *)&recv_addr,
          &sock_len);
      break;
    case ProtocolType::TCP:
      recv_len = recv(socket_fd_, recv_buf, recv_buf_len, 0);
      break;
  }

  if (recv_len < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "socket recv error, errno=" << unsigned(errno) << " "
              << strerror(errno);
  } else {
    GatUtil::DbgPrintBinary(
        recv_buf, recv_len, std::string("recv from remote:"));
  }

  return recv_len;
}

ssize_t GatCommunication::RecvDataWait(
    uint8_t *recv_buf, size_t recv_buf_len, int timeout_ms) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(socket_fd_, &readfds);

  struct timeval time_wait;
  time_wait.tv_sec  = timeout_ms / 1000;
  time_wait.tv_usec = (timeout_ms % 1000) * 1000;
  int ret           = -1;
  ssize_t recv_len  = 0;

  uint64_t begin_timestamp = GatUtil::GetCurTimestampMsec();
  while (true) {
    if ((GatUtil::GetCurTimestampMsec() - begin_timestamp) >=
        uint64_t(timeout_ms)) {
      break;
    }
    ret = select(socket_fd_ + 1, &readfds, NULL, NULL, &time_wait);
    if ((ret <= 0) || (!FD_ISSET(socket_fd_, &readfds))) {
      continue;
    }

    recv_len = this->RecvData(recv_buf, recv_buf_len);
    if (recv_len > 0) {
      break;
    }
  }

  return recv_len;
}

bool GatCommunication::UdpInit() {
  struct timeval timeout = {2, 0};

  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }

  int tmp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (tmp_fd < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "udp socket create error, errno=" << strerror(errno);
    return false;
  }

  int ret_val = 1;
  if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, &ret_val, sizeof(ret_val)) <
      0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "udp socket setsockopt SO_REUSEADDR error, "
              << strerror(errno);
    close(tmp_fd);
    return false;
  }

  ret_val = 1;
  if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEPORT, &ret_val, sizeof(ret_val)) <
      0) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEPORT error, "
              << strerror(errno);
  }

  ret_val =
      bind(tmp_fd, (const struct sockaddr *)&host_addr_, sizeof(host_addr_));
  if (ret_val < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "udp bind socket error, errno=" << strerror(errno);
    close(tmp_fd);
    return false;
  }

  ret_val = setsockopt(
      tmp_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
  if (ret_val < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "udp socket setsockopt SO_RCVTIMEO error, errno="
              << strerror(errno);
    close(tmp_fd);
    return false;
  }

  socket_fd_ = tmp_fd;
  return true;
}

bool GatCommunication::TcpInit() {
  struct timeval timeout = {2, 0};

  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }

  int tmp_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (tmp_fd < 0) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket create error, errno=" << strerror(errno);
    return false;
  }

  // disable Nagle
  int retval = 0;
  int enable = 1;
  retval     = setsockopt(
      tmp_fd,
      IPPROTO_TCP,
      TCP_NODELAY,
      reinterpret_cast<void *>(&enable),
      sizeof(enable));
  if (retval == -1) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket setsockopt TCP_NODELAY error, errno="
              << strerror(errno);
    return false;
  }

  retval = 1;
  if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, &retval, sizeof(retval)) <
      0) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEADDR error, "
              << strerror(errno);
    return false;
  }

  retval = 1;
  if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEPORT, &retval, sizeof(retval)) <
      0) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEPORT error, "
              << strerror(errno);
  }

  retval =
      connect(tmp_fd, (struct sockaddr *)&remote_addr_, sizeof(remote_addr_));
  if (retval < 0) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket connect error, errno=" << strerror(errno);
    return false;
  }

  retval = setsockopt(
      tmp_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
  if (retval < 0) {
    close(tmp_fd);
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_RCVTIMEO error, errno="
              << strerror(errno);
    return false;
  }

  socket_fd_ = tmp_fd;
  return true;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
