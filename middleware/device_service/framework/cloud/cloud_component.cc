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

#include "cloud_component.h"

#include "base/common/log.h"
#include "base/device_connect/cloud/device_factory.h"
#include "base/env/env.h"
#include "base/plugin/modules_loader/dynamic_loader.h"
#include "yaml-cpp/yaml.h"

namespace os {
namespace v2x {
namespace device {

bool AIROS_COMPONENT_CLASS_NAME(CloudComponent)::Init() {
  airos::base::DynamicLoader::GetInstance().LoadDevicePackage(
      "device/lib/cloud/");
  if (!LoadConfig(&conf_)) {
    LOG_ERROR << "load component proto config error" << std::endl;
    return false;
  }

  device_ = CloudDeviceFactory::Instance().GetUnique(
      conf_.mqtt_config().device(),
      std::bind(
          &AIROS_COMPONENT_CLASS_NAME(CloudComponent)::CallBack,
          this,
          std::placeholders::_1));
  // replace mqtt config
  auto rscu_sn = airos::base::Environment::GetDeviceSn();
  std::string sn_alias("$sn");
  YAML::Node cloud_config;
  try {
    cloud_config     = YAML::LoadFile(conf_.mqtt_config().config_file());
    auto mqtt_config = cloud_config["mqtt"];
    // replace client id
    auto client_id = mqtt_config["client_id"].as<std::string>();
    if (client_id.compare(sn_alias) == 0) {
      mqtt_config["client_id"] = rscu_sn;
    }
    // replace subscribe topic
    if (mqtt_config["subscribe_topic"].IsDefined()) {
      auto topic_list = mqtt_config["subscribe_topic"];
      for (size_t i = 0; i < topic_list.size(); ++i) {
        std::string tmp_topic = topic_list[i].as<std::string>();
        auto pos              = tmp_topic.find(sn_alias);
        if (pos != std::string::npos) {
          // replace alias
          std::string prefix = tmp_topic.substr(0, pos);
          std::string suffix = tmp_topic.substr(pos + 3);
          topic_list[i]      = prefix + rscu_sn + suffix;
        }
      }
    }
  } catch (const std::exception& err) {
    LOG_ERROR << err.what();
    return false;
  } catch (...) {
    LOG_ERROR << "Parse yaml config failed.";
    return false;
  }

  if (device_ == nullptr || !device_->Init(cloud_config)) {
    LOG_ERROR << "device_ init error";
    return false;
  }

  task_.reset(new std::thread([&]() {
    device_->Start();
  }));

  return true;
}

bool AIROS_COMPONENT_CLASS_NAME(CloudComponent)::Proc(
    const std::shared_ptr<const os::v2x::device::CloudData>& recv_data) {
  device_->WriteToDevice(recv_data);
  return true;
}

void AIROS_COMPONENT_CLASS_NAME(CloudComponent)::CallBack(
    const CloudPBDataType& data) {
  Send("/airos/cloud/message", data);
}

}  // namespace device
}  // namespace v2x
}  // namespace os
