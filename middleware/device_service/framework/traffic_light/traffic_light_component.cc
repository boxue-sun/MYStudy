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

#include "traffic_light_component.h"

#include <algorithm>
#include <experimental/filesystem>

#include "base/common/log.h"
#include "base/device_connect/traffic_light/device_factory.h"
#include "base/plugin/modules_loader/dynamic_loader.h"
#include "base/common/auth/Authenticator.h"
#include "base/work_param/configer_om_work_param.h"

namespace os {
namespace v2x {
namespace device {

bool AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::Init() {
  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/traffic_light/");

#if ENABLE_ENCRYPTION
  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "License is: " << license << std::endl;
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif
  WriteBackFromCommParam();

  if (!LoadConfig(&conf_)) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "load component proto config error";
    return false;
  }

  device_ = TrafficLightDeviceFactory::Instance().GetUnique(
      conf_.device(),
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::CallBack,
          this,
          std::placeholders::_1));

  // 调用厂商的init方法，连接设备
  if (device_ == nullptr || !device_->Init(conf_.config_file())) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "device_ init error";
    return false;
  }

  // 启动设备处理pipline，向node_发送处理后的proto
  task_.reset(new std::thread([&]() {
    device_->Start();
  }));

  // 订阅雷达脉冲数据，转发给信号机
  pulse_reader_ = node_->CreateReader<os::v2x::device::CloudData>(
      "/airos/radar/pulse",
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::OnPulseData,
          this,
          std::placeholders::_1));

  // 订阅雷达统计数据，转发给信号机（A13统计数据）
  statistic_reader_ = node_->CreateReader<os::v2x::device::CloudData>(
      "/airos/radar/statistics",
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::OnRadarStatisticData,
          this,
          std::placeholders::_1));

  return true;
}

// Proc方法：用于解析接收到的proto，并写入设备
bool AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::Proc(
    const std::shared_ptr<const os::v2x::device::TrafficLightReceiveData>&
        recv_data) {
  device_->WriteToDevice(recv_data);
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::OnPulseData(
    const std::shared_ptr<const os::v2x::device::CloudData>& data) {
  if (!data->has_mqtt_data()) {
    return;
  }
  const std::string& json_str = data->mqtt_data().data();

  PulseData pulse;
  try {
    afl::base::json j = afl::base::json::parse(json_str);
    pulse.deserialize(j);
  } catch (const afl::base::json::exception& e) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[pulse] deserialize failed: " << e.what();
    return;
  }

  if (pulse.lanes.empty()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[pulse] lanes field missing or empty in: " << json_str;
    return;
  }

  std::vector<std::pair<uint8_t, uint8_t>> lanes;
  for (const auto& lane : pulse.lanes) {
    uint8_t lane_no = static_cast<uint8_t>(lane.lane_no);
    // 只转发 lane_no 100~199 的检测器数据给信号机
    if (lane_no < 100 || lane_no > 199) {
      continue;
    }
    lanes.emplace_back(lane_no, static_cast<uint8_t>(lane.direction));
  }

  device_->WritePulseData(lanes);
}

void AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::OnRadarStatisticData(
    const std::shared_ptr<const os::v2x::device::CloudData>& data) {
  if (!data->has_mqtt_data()) {
    return;
  }
  const std::string& json_str = data->mqtt_data().data();

  RadarStatisticData statistic;
  try {
    afl::base::json j = afl::base::json::parse(json_str);
    statistic.deserialize(j);
  } catch (const afl::base::json::exception& e) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[statistic] deserialize failed: " << e.what();
    return;
  }

  if (statistic.statistics.empty()) {
    TRAFFIC_LIFGT_SERVICE_LOG_ERROR << "[statistic] statistics field missing or empty in: " << json_str;
    return;
  }

  // 雷达4.14统计字段与GA/T 1743 A.52逐字段对齐，直接透传，仅做范围保护
  std::vector<os::v2x::device::CoilFlowData> coils;
  for (const auto& st : statistic.statistics) {
    uint8_t lane_no = static_cast<uint8_t>(st.lane_no);
    // 只转发 lane_no 100~199（1xx进口检测器）的统计数据给信号机
    if (lane_no < 100 || lane_no > 199) {
      continue;
    }
    os::v2x::device::CoilFlowData c{};
    c.coil_no    = lane_no;
    c.volume     = static_cast<uint8_t>(std::max(0, std::min(st.volume, 255)));
    c.occupancy  = static_cast<uint8_t>(std::max(0, std::min(st.occupancy, 200)));
    c.avg_speed  = static_cast<uint8_t>(std::max(1, std::min(st.speed, 255)));
    c.avg_length = static_cast<uint8_t>(std::max(1, std::min(st.length, 255)));
    c.headway    = static_cast<uint8_t>(std::max(1, std::min(st.distance, 255)));
    coils.push_back(c);
  }
  if (!coils.empty()) {
    TRAFFIC_LIFGT_SERVICE_LOG_WARN << "[radar-statistic] send to signal controller, coil count=" << coils.size();
    device_->WriteTrafficFlowData(coils);
  }
}

void AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::CallBack(
    const TrafficLightDataType& data) {
    Send("/airos/device/traffic_light/data", data);
    //信号机原始数据
    {
      auto* md_spat_src_data =  mec_monitor_->mutable_md_spat_src_data();
      md_spat_src_data->set_tag(airos::monitor_mec::MonitorMecTag::MONITOR_TAG_DATA_SPAT_SRC);
      md_spat_src_data->set_timestamp(afl::util::TimeStamp::now(true).millSeconds());
      TRAFFIC_LIFGT_SERVICE_LOG_INFO << "[monitor-out] Send topic: /v2x/mec/om/check/spat_src/data_,[data]" << mec_monitor_->ShortDebugString();
      Send("/v2x/mec/om/check/spat_src/data_", mec_monitor_);
    }
}

void AIROS_COMPONENT_CLASS_NAME(TrafficLightComponent)::WriteBackFromCommParam() {
  auto omWorkParamConfiger =
      airos::base::workparam::WorkParam::getWorkParamFromFile();

  std::string filePath = "/home/airos/param/device/traffic_light/gat_device/device.yaml";

  YAML::Node config = YAML::LoadFile(filePath);
  config["ip"] = omWorkParamConfiger.trafficLightParamConfiger.serverIp;
  config["port"] = omWorkParamConfiger.trafficLightParamConfiger.serverPort;
  config["local_port"] = omWorkParamConfiger.trafficLightParamConfiger.localPort;
  if (config["peerAddresslist"] && config["peerAddresslist"].size() > 0) {
    config["peerAddresslist"][0]["peer_ip"] = omWorkParamConfiger.trafficLightParamConfiger.peerIp;
    config["peerAddresslist"][0]["peer_port"] = omWorkParamConfiger.trafficLightParamConfiger.peerPort;
  }

  std::ofstream fout(filePath);
  fout << config;
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "TrafficLight server ip: " << config["ip"]
        << ", port: " << config["port"]
        << ", local_port: " << config["local_port"]
        << ", peer ip: " << config["peerAddresslist"][0]["peer_ip"]
        << ", peer port: " << config["peerAddresslist"][0]["peer_port"];
  TRAFFIC_LIFGT_SERVICE_LOG_INFO << "TrafficLight configuration updated and saved to " << filePath;
}



}  // namespace device
}  // namespace v2x
}  // namespace os
