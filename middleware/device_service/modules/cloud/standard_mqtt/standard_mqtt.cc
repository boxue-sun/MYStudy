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

#include "middleware/device_service/modules/cloud/standard_mqtt/standard_mqtt.h"

#include "base/common/log.h"
#include "base/device_connect/cloud/device_factory.h"
#include "yaml-cpp/yaml.h"

namespace os {
namespace v2x {
namespace device {

bool StandardMqtt::Init(const YAML::Node& root_node) {
  try {
    if (!root_node.IsMap()) {
      LOG_ERROR << "config file format error";
      return false;
    }

    mqtt_username_ = root_node["mqtt"]["username"].as<std::string>();
    mqtt_password_ = root_node["mqtt"]["password"].as<std::string>();
    cloud_url_     = root_node["mqtt"]["cloud_url"].as<std::string>();
    client_id_     = root_node["mqtt"]["client_id"].as<std::string>();
    if (root_node["mqtt"]["timeout_s"].IsDefined()) {
      timeout_s_ = root_node["mqtt"]["timeout_s"].as<int>();
    } else {
      timeout_s_ = 2;
    }
    if (root_node["mqtt"]["subscribe_topic"].IsDefined()) {
      auto topic_list = root_node["mqtt"]["subscribe_topic"];
      for (size_t i = 0; i < topic_list.size(); ++i) {
        subscribe_topic_.emplace_back(topic_list[i].as<std::string>());
      }
    }
  } catch (...) {
    LOG_ERROR << "Config file format wrong! Please check the format(e.g. "
                 "indentation)";
    return false;
  }
  return true;
}

void StandardMqtt::Start() {
  ReConnect();
  RcvData();
}

void StandardMqtt::WriteToDevice(
    const std::shared_ptr<const os::v2x::device::CloudData>& re_proto) {
  auto cli = m_mqtt_client_;
  if (!cli) {
    LOG_ERROR << "Invalid Client";
    return;
  }
  if (re_proto->payload_case() !=
      os::v2x::device::CloudData::PayloadCase::kMqttData) {
    LOG_WARN << "NOT MQTT MSG.";
    return;
  }
  auto mq_data = re_proto->mqtt_data();
  if (!cli->publish(mq_data.topic(), mq_data.data())) {
    LOG_ERROR << "Failed to publish topic: " << mq_data.topic();
    return;
  }
  return;
}

void StandardMqtt::Stop() {
  if (mq_rcv_ptr_ != nullptr && mq_rcv_ptr_->joinable()) {
    mq_rcv_ = false;
    mq_rcv_ptr_->join();
  }
  if (mq_reconn_ptr_ != nullptr && mq_reconn_ptr_->joinable()) {
    mq_reconn_ = false;
    mq_reconn_ptr_->join();
  }
}

CloudDeviceState StandardMqtt::GetState() {
  return CloudDeviceState::NORMAL;
}

int StandardMqtt::Connect() {
  std::shared_ptr<airos::base::MQTTClientInterface> cli = nullptr;
  if (m_mqtt_client_) {
    LOG_INFO << "MQTT client has connect.";
    return 0;
  }
  LOG_WARN << "Building/Rebuilding MQTT Client ...";
  auto builder = airos::base::MQTTClientBuilder();
  builder.set_client_id(client_id_);
  builder.set_server_uri(cloud_url_);
  builder.set_client_username(mqtt_username_);
  builder.set_client_password(mqtt_password_);
  builder.set_connect_timeout_s(timeout_s_);
  cli = builder.build();
  if (!cli) {
    LOG_ERROR << "Failed to create mqtt client.";
    return -1;
  }
  for (size_t i = 0; i < subscribe_topic_.size(); ++i) {
    cli->subscribe(subscribe_topic_[i]);
    LOG_WARN << "Subscribe Topic : " << subscribe_topic_[i];
  }
  LOG_WARN << "MQTT Client Built/Rebuilt.";
  m_mqtt_client_.swap(cli);
  return 0;
}

void StandardMqtt::ReConnect() {
  mq_reconn_ = true;
  mq_reconn_ptr_.reset(new std::thread([&] {
    while (mq_reconn_) {
      int result = Connect();
      if (result) {
        LOG_ERROR << "MQTT Connect Failed.";
      }
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }));
}

void StandardMqtt::RcvData() {
  mq_rcv_ = true;
  mq_rcv_ptr_.reset(new std::thread([&] {
    while (mq_rcv_) {
      auto cli = m_mqtt_client_;
      if (!cli) {
        LOG_WARN << "Invalid Client";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }
      auto msgs = cli->get_received_messages(1000);
      if (msgs.empty()) {
        LOG_EVERY_N(WARNING, 20) << "No mqtt message received";
        continue;
      }
      for (const auto& item : msgs) {
        LOG_WARN << "Received Topic : " << item.first;
        auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
        auto mqtt_data  = cloud_data->mutable_mqtt_data();
        mqtt_data->set_topic(item.first);
        mqtt_data->set_data(item.second->payload, item.second->payloadlen);
        sender_(cloud_data);
      }
    }
  }));
}

V2XOS_CLOUD_REG_FACTORY(StandardMqtt, "standard_mqtt");

}  // namespace device
}  // namespace v2x
}  // namespace os
