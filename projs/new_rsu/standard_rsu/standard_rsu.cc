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

#include "standard_rsu.h"

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>

#include "base/common/log.h"
#include "base/device_connect/rsu/device_factory.h"
#include "protocol/internal_protocol.h"
#include "yaml-cpp/yaml.h"

namespace os {
namespace v2x {
namespace device {

bool StandardRsu::Init(const std::string& conf_file) {
  std::string ip, local_ip;
  int port = 0, local_port = 0;
  try {
    YAML::Node root_node = YAML::LoadFile(conf_file);
    ip                   = root_node["ip"].as<std::string>();
    port                 = root_node["port"].as<int>();
    local_ip             = root_node["local_ip"].as<std::string>();
    local_port           = root_node["local_port"].as<int>();
  } catch (const std::exception& err) {
    RSU_SERVICE_LOG_ERROR << err.what();
    return false;
  } catch (...) {
    RSU_SERVICE_LOG_ERROR << "Parse yaml config failed.";
    return false;
  }

  RSU_SERVICE_LOG_WARN << "ip= " << ip << ", port=" << port << ", local ip=" << local_ip
           << ", local port=" << local_port;

  memset(&peer_addr_, 0, sizeof(peer_addr_));
  peer_addr_.sin_family      = AF_INET;
  peer_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  peer_addr_.sin_port        = htons(port);

  struct sockaddr_in server_addr;
  server_addr.sin_family      = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(local_ip.c_str());
  server_addr.sin_port        = htons(local_port);
  socket_fd_                  = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    RSU_SERVICE_LOG_ERROR << "create socket error!";
    return false;
  }

  int yes = 1;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    RSU_SERVICE_LOG_ERROR << "fail to setsockopt SO_REUSEADDR, " << strerror(errno);
    close(socket_fd_);
    return false;
  }

  int ret =
      bind(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (ret < 0) {
    RSU_SERVICE_LOG_ERROR << "bind address error, " << strerror(errno);
    close(socket_fd_);
    return false;
  }

  rsu_data_recv_ = std::make_shared<os::v2x::device::RSUData>();
  transfer_      = std::make_shared<os::v2x::device::InternalProtocol>();
  return true;
}

void StandardRsu::Start() {
  recv_thread_.reset(
      new std::thread(std::bind(&StandardRsu::TaskRecvData, this)));

  while (!stop_) {
    std::unique_lock<std::mutex> guard(recv_mutex_);
    recv_condition_.wait(guard, [this] {
      return rsu_data_recv_->has_data();
    });
    sender_(rsu_data_recv_);
    rsu_data_recv_->Clear();
  }
}

void StandardRsu::WriteToDevice(
    const std::shared_ptr<const os::v2x::device::RSUData>& re_proto) {
  if (transfer_ != nullptr) {
    std::string pack = transfer_->Encode(re_proto);
    if (pack.empty()) {
      return;
    }
    ssize_t nsent = sendto(
        socket_fd_,
        pack.data(),
        pack.size(),
        0,
        (struct sockaddr*)&peer_addr_,
        (socklen_t)sizeof(peer_addr_));
    if (nsent == -1) {
      RSU_SERVICE_LOG_ERROR << "send rsu failed, errno: " << std::strerror(errno);
    } else {
      RSU_SERVICE_LOG_INFO << "send data type "
               << os::v2x::device::RSUDataType_Name(re_proto->type())
               << " finished";
    }
    return;
  }
}

RSUDeviceState StandardRsu::GetState() {
  return RSUDeviceState::NORMAL;
}

void StandardRsu::Stop() {
  stop_ = true;
  if (recv_thread_ != nullptr && recv_thread_->joinable()) {
    recv_thread_->join();
  }
}

void StandardRsu::TaskRecvData() {
  char buffer[1900]       = {0};
  const size_t max_length = sizeof(buffer);

  while (!stop_) {
    RSUDataPtr pb_data = std::make_shared<os::v2x::device::RSUData>();
    if (transfer_ != nullptr) {
      memset(buffer, 0, max_length);
      ssize_t ret = recv(socket_fd_, buffer, max_length, 0);
      if (ret < 0) {
        RSU_SERVICE_LOG_ERROR << "recvfrom rsu error: " << strerror(errno);
        return;
      }
      transfer_->Decode(buffer, ret, pb_data);
    }
    std::lock_guard<std::mutex> guard(recv_mutex_);
    rsu_data_recv_ = std::move(pb_data);
    recv_condition_.notify_all();
  }
}

V2XOS_RSU_REG_FACTORY(StandardRsu, "new_rsu");

}  // namespace device
}  // namespace v2x
}  // namespace os
