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

#include "cloud_adapter.h"

#include "app/modules/v2x_application/common/app_flag.h"
#include "base/common/log.h"
#include "base/env/env.h"
#include "gflags/gflags.h"

namespace airos {
namespace app {

bool CloudAdapter::Init(
    ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
        cloud_sequence) {
  rscu_sn_        = airos::base::Environment::GetDeviceSn();
  cloud_sequence_ = cloud_sequence;
  mqtt_reconnect();
  mqtt_send_data();
  return true;
}

int CloudAdapter::mqtt_connect() {
  std::shared_ptr<base::MQTTClientInterface> cli = nullptr;
  if (m_mqtt_client_) {
    APP_LOG_INFO << "MQTT Client has already connected.";
    return 0;
  }

  // Need Rebuild MQTT
  APP_LOG_WARN << "Building/Rebuilding MQTT Client ...";
  auto builder = base::MQTTClientBuilder();
  builder.set_client_id(rscu_sn_);
  builder.set_server_uri(FLAGS_cloud_mqtt_addr);
  builder.set_client_username(FLAGS_cloud_mqtt_user);
  builder.set_client_password(FLAGS_cloud_mqtt_passwd);
  builder.set_connect_timeout_s(FLAGS_cloud_mqtt_connect_timeout);
  cli = builder.build();
  if (!cli) {
    APP_LOG_ERROR << "Failed to create mqtt client.";
    return -1;
  }
  APP_LOG_WARN << "MQTT Client Built/Rebuilt success.";
  m_mqtt_client_.swap(cli);
  return 0;
}

void CloudAdapter::mqtt_reconnect() {
  mqtt_reconnect_flag_ = true;
  mqtt_reconnect_ptr_.reset(new std::thread([&] {
    while (mqtt_reconnect_flag_) {
      mqtt_connect();
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }));
}

void CloudAdapter::mqtt_send_data() {
  mqtt_send_flag_ = true;
  mqtt_send_ptr_.reset(new std::thread([&] {
    while (mqtt_send_flag_) {
      std::shared_ptr<os::v2x::device::CloudData> cloud_data;
      cloud_sequence_->wait_and_pop(cloud_data);
      if (!m_mqtt_client_ || !cloud_data) {
        continue;
      }
      APP_LOG_INFO << cloud_data->DebugString();
      if (cloud_data->payload_case() !=
          os::v2x::device::CloudData::PayloadCase::kMqttData) {
        continue;
      }
      auto mq_data = cloud_data->mqtt_data();
      if (!m_mqtt_client_->publish(mq_data.topic(), mq_data.data())) {
        APP_LOG_ERROR << "Failed to publish topic: " << mq_data.topic();
      }
    }
  }));
}

CloudAdapter::~CloudAdapter() {
  if (mqtt_reconnect_ptr_ && mqtt_reconnect_ptr_->joinable()) {
    mqtt_reconnect_flag_ = false;
    mqtt_reconnect_ptr_->join();
    mqtt_reconnect_ptr_.reset();
    APP_LOG_INFO << __FUNCTION__ << " wait thread reconnect";
  }

  if (mqtt_send_ptr_ && mqtt_send_ptr_->joinable()) {
    mqtt_send_flag_ = false;
    mqtt_send_ptr_->join();
    mqtt_send_ptr_.reset();
    APP_LOG_INFO << __FUNCTION__ << " wait thread send";
  }
}

}  // namespace app
}  // namespace airos
