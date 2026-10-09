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

#include "device_monitor.h"

#include <fstream>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "app/modules/v2x_application/monitor/common/system_cmd.h"
#include "base/common/log.h"
#include "base/env/env.h"
#include "gflags/gflags.h"

namespace airos {
namespace monitor {

DeviceMonitor::DeviceMonitor() {
  rscu_sn_     = airos::base::Environment::GetDeviceSn();
  rscu_status_ = std::make_shared<::airos::v2x::RscuStatus>();
  rscu_status_->set_rscu_sn(rscu_sn_);
  LoadDeviceConfig();
  ping_thread_.reset(
      new std::thread(std::bind(&DeviceMonitor::StartPingAllDevice, this)));
  upload_thread_.reset(
      new std::thread(std::bind(&DeviceMonitor::UploadRscuStatus, this)));
}

void DeviceMonitor::UploadRscuStatus() {
  while (!exit_) {
    std::this_thread::sleep_for(std::chrono::seconds(UPLOAD_INTERNAL));
    auto cloud_pb = std::make_shared<os::v2x::device::CloudData>();
    auto mqtt_pb  = cloud_pb->mutable_mqtt_data();
    mqtt_pb->set_topic(MQTT_RSCU_STATUS_TOPIC_PREFIX + rscu_sn_);
    std::string str_data;
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    {
      std::lock_guard<std::mutex> lock(status_mutex_);
      rscu_status_->set_timestamp(now);
      rscu_status_->SerializePartialToString(&str_data);
    }
    mqtt_pb->set_data(str_data);
    if (post_cb_) {
      post_cb_("/v2x/cloud/report/mqtt", cloud_pb);
    }
  }
  return;
}

bool DeviceMonitor::StartPingAllDevice() {
  while (!exit_) {
    std::this_thread::sleep_for(std::chrono::seconds(PING_INTERNAL));
    ::airos::v2x::RscuStatus tmp_rscu_status;
    {
      std::lock_guard<std::mutex> lock(status_mutex_);
      tmp_rscu_status.operator=(*rscu_status_);
    }

    for (int i = 0; i < tmp_rscu_status.device_status_size(); i++) {
      if (!PingDevice(tmp_rscu_status.device_status(i).ip())) {
        tmp_rscu_status.mutable_device_status(i)->set_is_online(false);
      } else {
        tmp_rscu_status.mutable_device_status(i)->set_is_online(true);
      }
    }

    std::lock_guard<std::mutex> lock(status_mutex_);
    rscu_status_ =
        std::make_shared<::airos::v2x::RscuStatus>(std::move(tmp_rscu_status));
  }
  return true;
}

bool DeviceMonitor::PingDevice(const std::string device_ip) {
  std::string cmd =
      "ping " + device_ip + " -n -c 1 -W 1 >>/dev/null 2>>/dev/null";
  return SystemCmd::System(cmd);
}

std::string DeviceMonitor::GetConfigPathFromDesc(const std::string& desc_file) {
  std::string config_path;
  try {
    YAML::Node config = YAML::LoadFile(desc_file);
    config_path       = config["config_file"].as<std::string>();
  } catch (const std::exception& err) {
    APP_LOG_ERROR << err.what();
  } catch (...) {
    APP_LOG_ERROR << "Parse yaml config failed.";
  }
  return config_path;
}

void DeviceMonitor::LoadDeviceConfig() {
  std::function<void(::airos::v2x::DeviceStatus)> set_device =
      [this](::airos::v2x::DeviceStatus device) {
        rscu_status_->add_device_status()->operator=(device);
      };
  DeviceVector vec_rsu_info;
  if (GetRsuInfo(&vec_rsu_info)) {
    std::for_each(vec_rsu_info.begin(), vec_rsu_info.end(), set_device);
  }

  DeviceVector vec_signal_machine_info;
  if (GetSignalMachineInfo(&vec_signal_machine_info)) {
    std::for_each(
        vec_signal_machine_info.begin(), vec_signal_machine_info.end(),
        set_device);
  }
  return;
}

bool DeviceMonitor::GetRsuInfo(DeviceVector* vec_device) {
  std::string rsu_ip;
  try {
    std::string rsu_conf = airos::base::Environment::GetPublicParamPath() +
                           "device/rsu/config.pb.txt";
    YAML::Node root_node = YAML::LoadFile(GetConfigPathFromDesc(rsu_conf));
    rsu_ip               = root_node["ip"].as<std::string>();
  } catch (const std::exception& err) {
    APP_LOG_ERROR << err.what();
    return false;
  } catch (...) {
    APP_LOG_ERROR << "Parse yaml config failed.";
    return false;
  }

  ::airos::v2x::DeviceStatus device;
  device.set_type(::airos::v2x::DeviceType::RSU);
  device.set_ip(rsu_ip);
  vec_device->push_back(device);
  return true;
}

bool DeviceMonitor::GetSignalMachineInfo(DeviceVector* vec_device) {
  std::string signal_machine_ip;
  try {
    std::string signal_machine_conf =
        airos::base::Environment::GetPublicParamPath() +
        "device/traffic_light/config.pb.txt";
    YAML::Node root_node =
        YAML::LoadFile(GetConfigPathFromDesc(signal_machine_conf));
    signal_machine_ip = root_node["ip"].as<std::string>();
  } catch (const std::exception& err) {
    APP_LOG_ERROR << err.what();
    return false;
  } catch (...) {
    APP_LOG_ERROR << "Parse yaml config failed.";
    return false;
  }

  ::airos::v2x::DeviceStatus device;
  device.set_type(::airos::v2x::DeviceType::SIGNAL_MACHINE);
  device.set_ip(signal_machine_ip);
  vec_device->push_back(device);
  return true;
}

void DeviceMonitor::Stop() {
  exit_ = true;
  if (ping_thread_ != nullptr && ping_thread_->joinable()) {
    ping_thread_->join();
    ping_thread_.reset();
  }
  if (upload_thread_ != nullptr && upload_thread_->joinable()) {
    upload_thread_->join();
    upload_thread_.reset();
  }
}

}  // namespace monitor
}  // namespace airos
