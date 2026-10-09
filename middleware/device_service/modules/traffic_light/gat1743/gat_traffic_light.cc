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

#include "gat_traffic_light.h"

#include <iostream>

#include "base/common/log.h"
#include "base/device_connect/traffic_light/device_factory.h"
#include "yaml-cpp/yaml.h"
#include <regex>
#include <arpa/inet.h>
#include <netdb.h>
namespace os {
namespace v2x {
namespace device {

bool GatTrafficLight::Init(const std::string& config_file) {
    if(!has_send_once)
    {
        sender_(output_data_);
        has_send_once = true;
    }

  try {
    YAML::Node root_node = YAML::LoadFile(config_file);
    if (!root_node.IsMap()) {
      TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "config file format invalid, file=" << config_file;
      return false;
    }

    std::string ip      = root_node["ip"].as<std::string>();
    uint16_t port       = root_node["port"].as<int>();
    uint16_t local_port = root_node["local_port"].as<int>();
    double color_state_query_cycle = root_node["color_state_query_cycle"].as<double>();
    std::string local_ip;
    if (root_node["local_ip"].IsScalar()) {
      local_ip = root_node["local_ip"].as<std::string>();
    }
    std::string protocol = "udp";
    if (root_node["protocol"].IsScalar()) {
      protocol = root_node["protocol"].as<std::string>();
    }

    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "remote ip: " << ip << "\nremote port: " << port
             << "\nhost ip: " << local_ip << "\nhost port: " << local_port
             << "\nprotocol: " << protocol;

    std::vector<outAddress> peer_list;
    if(root_node["peerAddresslist"])
    {
    	const YAML::Node& list = root_node["peerAddresslist"];
        for(const auto& address : list)
        {
        	outAddress oa;
        	oa.peerip = address["peer_ip"].as<std::string>();
          if(!isIPv4(oa.peerip))
          {
            oa.peerip = getHostByName(oa.peerip);
          }
        	oa.peerport = address["peer_port"].as<int>();
        	peer_list.push_back(oa);
        }
    }
    else
    {
        TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "get peer address list failed";
        return false;
    }



    if (!gat_worker_->Init(ip, port, local_ip, local_port, protocol, peer_list, color_state_query_cycle)) {
      return false;
    }
  } catch (...) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "Config file format wrong! Please check the format(e.g. "
                 "indentation), file="
              << config_file;
    return false;
  }
  return true;
}

void GatTrafficLight::Start() {
  InitSenderTimer();
}

void GatTrafficLight::WriteToDevice(
    const std::shared_ptr<const TrafficLightReceiveData>& re_proto) {
  return;
}

void GatTrafficLight::WritePulseData(
    const std::vector<std::pair<uint8_t, uint8_t>>& lanes) {
  if (lanes.empty()) {
    return;
  }
  gat_worker_->SendPulseData(lanes);
}

void GatTrafficLight::WriteTrafficFlowData(
    const std::vector<CoilFlowData>& coils) {
  if (coils.empty()) {
    return;
  }
  gat_worker_->SendTrafficFlowData(coils);
}

void GatTrafficLight::SendData() {
  if (gat_worker_->GetTrafficLightData(*output_data_)) {
    sender_(output_data_);
  }
}

void GatTrafficLight::TimeoutSender(__sigval_t arg) {
  GatTrafficLight* gat_device =
      reinterpret_cast<GatTrafficLight*>(arg.sival_ptr);
  gat_device->SendData();
}

bool GatTrafficLight::InitSenderTimer() {
  struct sigevent evp;
  evp.sigev_notify            = SIGEV_THREAD;
  evp.sigev_notify_function   = GatTrafficLight::TimeoutSender;
  evp.sigev_value.sival_ptr   = this;
  evp.sigev_notify_attributes = NULL;

  if (0 != timer_create(CLOCK_REALTIME, &evp, &timer_fd_sender_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer create failed.";
    return false;
  }

  struct itimerspec ts;
  ts.it_interval.tv_sec  = 0;
  ts.it_interval.tv_nsec = 100 * 1000000L;  // ms, 10HZ
  ts.it_value.tv_sec     = 0;
  ts.it_value.tv_nsec    = 100 * 1000000L;  // ms, 10HZ

  if (0 != timer_settime(timer_fd_sender_, 0, &ts, NULL)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "timer set failed.";
    return false;
  }

  return true;
}

bool GatTrafficLight::isIPv4(const std::string& str)
{
	std::regex ipv4_regex("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

  return std::regex_match(str, ipv4_regex);
}

std::string GatTrafficLight::getHostByName(std::string name)
{
  std::string out_ip = "";
  // Linux 域名解析函数
  struct hostent* host = gethostbyname(name.c_str());
  if (host == nullptr || host->h_addr_list[0] == nullptr) 
  {
    std::cerr << "域名解析失败：" << strerror(errno) << std::endl;
    return out_ip;
  }

  // 转换为点分十进制字符串
  char ip_buf[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, host->h_addr_list[0], ip_buf, INET_ADDRSTRLEN);
  out_ip = ip_buf;

  return out_ip;
}

V2XOS_TRAFFIC_LIGHT_REG_FACTORY(GatTrafficLight, "gat_traffic_light");

}  // namespace device
}  // namespace v2x
}  // namespace os
