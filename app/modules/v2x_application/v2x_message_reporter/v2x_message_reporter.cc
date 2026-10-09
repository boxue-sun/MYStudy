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

#include "v2x_message_reporter.h"

#include <sys/time.h>

#include "base/common/log.h"
#include "base/env/env.h"
namespace airos {
namespace app {

::os::v2x::device::RSUData V2xMessageReporter::rsu_asn_data_;
std::mutex V2xMessageReporter::rsu_asn_mtx_;
std::condition_variable V2xMessageReporter::rsu_asn_condition_;

bool V2xMessageReporter::Init(
    ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
        cloud_sequence) {
  rscu_sn_        = airos::base::Environment::GetDeviceSn();
  version_        = os::v2x::cloud::CSAE_53_2020;
  cloud_sequence_ = cloud_sequence;
  UploadBsmMqttMsg();
  UploadAsnMqttMsg();

  return true;
}

bool V2xMessageReporter::Proc(
    const std::shared_ptr<const os::v2x::device::RSUData>& rsu_out) {
  if (rsu_out->type() != os::v2x::device::RSU_BSM) {
    return false;
  }
  std::lock_guard<std::mutex> lg(bsm_mutex_);
  bsm_cache_.push_back(rsu_out->data());
  return true;
}

void V2xMessageReporter::UploadBsmMqttMsg() {
  b_send_ = true;
  bsm_send_.reset(new std::thread([&] {
    while (b_send_) {
      {
        std::lock_guard<std::mutex> lg(bsm_mutex_);
        if (bsm_cache_.empty()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          continue;
        }
      }

      os::v2x::cloud::CloudV2xMessage msg;
      std::string report_json;
      msg.set_type(os::v2x::cloud::MSG_TYPE_BSM);
      {
        std::lock_guard<std::mutex> lg(bsm_mutex_);
        SerializeMessage(msg, bsm_cache_, report_json);
        bsm_cache_.clear();
      }
      auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
      auto mqtt_data  = cloud_data->mutable_mqtt_data();
      mqtt_data->set_topic(MQTT_BSM_TOPIC_PREFIX + rscu_sn_);
      mqtt_data->set_data(report_json);
      cloud_sequence_->push(cloud_data);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }));
}

bool V2xMessageReporter::SerializeMessage(
    os::v2x::cloud::CloudV2xMessage& msg,
    const std::vector<std::string>& asn_vec, std::string& output) {
  msg.set_rscu_sn(rscu_sn_);
  msg.set_timestamp(GetCurrentTimestamp());
  msg.set_data_format(os::v2x::cloud::DATA_FORMAT_ASN);

  os::v2x::cloud::MsgData msg_data;
  msg_data.set_version(version_);
  for (auto& asn : asn_vec) {
    msg_data.add_payload(asn);
  }
  std::string str_data;
  msg_data.SerializePartialToString(&str_data);

  msg.mutable_msg_data()->operator=(msg_data);
  msg.SerializePartialToString(&output);
  APP_LOG_INFO << msg.DebugString();
  return true;
}

int64_t V2xMessageReporter::GetCurrentTimestamp() {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void V2xMessageReporter::ProcRsuInData(
    const std::shared_ptr<const ::os::v2x::device::RSUData>& rsptr) {
  SetRsuInData(*rsptr);
}

void V2xMessageReporter::SetRsuInData(
    const ::os::v2x::device::RSUData& rsu_data) {
  std::lock_guard<std::mutex> guard(rsu_asn_mtx_);
  rsu_asn_data_.operator=(rsu_data);
  rsu_asn_condition_.notify_all();
}

void V2xMessageReporter::GetRsuInData(::os::v2x::device::RSUData& rsu_in) {
  std::unique_lock<std::mutex> guard(rsu_asn_mtx_);
  rsu_asn_condition_.wait(guard, [] {
    return rsu_asn_data_.ByteSizeLong() != 0;
  });
  rsu_in.operator=(rsu_asn_data_);
  rsu_asn_data_.Clear();
}

void V2xMessageReporter::UploadAsnMqttMsg() {
  a_send_ = true;
  asn_send_.reset(new std::thread([&] {
    while (a_send_) {
      os::v2x::device::RSUData rsu_data;
      GetRsuInData(rsu_data);
      auto rsu_in = std::make_shared<os::v2x::device::RSUData>(rsu_data);
      APP_LOG_INFO << rsu_in->DebugString();
      std::string report_json;
      std::string topic;
      if (TransAsnToMqttMsg(rsu_in, report_json, topic)) {
        auto cloud_data = std::make_shared<os::v2x::device::CloudData>();
        auto mqtt_data  = cloud_data->mutable_mqtt_data();
        mqtt_data->set_topic(topic);
        mqtt_data->set_data(report_json);
        cloud_sequence_->push(cloud_data);
      }
    }
  }));
}

bool V2xMessageReporter::TransAsnToMqttMsg(
    const std::shared_ptr<::os::v2x::device::RSUData>& asn_data,
    std::string& report_json, std::string& topic) {
  os::v2x::cloud::CloudV2xMessage msg;
  switch (asn_data->type()) {
    case os::v2x::device::RSU_SPAT:
      msg.set_type(os::v2x::cloud::MSG_TYPE_SPAT);
      topic = MQTT_SPAT_TOPIC_PREFIX + rscu_sn_;
      break;
    case os::v2x::device::RSU_MAP:
      msg.set_type(os::v2x::cloud::MSG_TYPE_MAP);
      topic = MQTT_MAP_TOPIC_PREFIX + rscu_sn_;
      break;
    case os::v2x::device::RSU_RSI:
      msg.set_type(os::v2x::cloud::MSG_TYPE_RSI);
      topic = MQTT_RSI_TOPIC_PREFIX + rscu_sn_;
      break;
    case os::v2x::device::RSU_RSM:
      msg.set_type(os::v2x::cloud::MSG_TYPE_RSI);
      topic = MQTT_RSM_TOPIC_PREFIX + rscu_sn_;
      break;
    default:
      return false;
  }
  os::v2x::cloud::MessageVersion rev_version;
  if (asn_data->has_version() &&
      asn_data->version() == os::v2x::device::YDT_3709_2020) {
    rev_version = os::v2x::cloud::YDT_3709_2020;
  } else {
    rev_version = os::v2x::cloud::CSAE_53_2020;
  }
  if (version_ != rev_version) {
    version_ = rev_version;
  }
  std::vector<std::string> vec_asn{asn_data->data()};
  SerializeMessage(msg, vec_asn, report_json);
  return true;
}

V2xMessageReporter::~V2xMessageReporter() {
  if (b_send_ && bsm_send_->joinable()) {
    b_send_ = false;
    bsm_send_->join();
  }
  if (a_send_ && asn_send_->joinable()) {
    a_send_ = false;
    asn_send_->join();
  }
}

}  // namespace app
}  // namespace airos
