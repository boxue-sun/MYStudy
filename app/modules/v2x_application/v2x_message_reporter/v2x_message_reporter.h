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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "app/framework/proto/cloud_v2x_message.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"
#include "base/device_connect/proto/rsu_data.pb.h"

#include "app/modules/v2x_application/common/producer_consumer_queue.h"

namespace airos {
namespace app {

class V2xMessageReporter {
 public:
  V2xMessageReporter(){};

  virtual ~V2xMessageReporter();

  bool Init(ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
                cloud_sequence);
  bool Proc(const std::shared_ptr<const os::v2x::device::RSUData>& rsu_out);
  void ProcRsuInData(
      const std::shared_ptr<const ::os::v2x::device::RSUData>& rsptr);

 private:
  int64_t GetCurrentTimestamp();
  bool TransAsnToMqttMsg(
      const std::shared_ptr<::os::v2x::device::RSUData>& asn_data,
      std::string& report_json, std::string& topic);
  void UploadAsnMqttMsg();
  void UploadBsmMqttMsg();
  bool SerializeMessage(
      os::v2x::cloud::CloudV2xMessage& msg,
      const std::vector<std::string>& asn_vec, std::string& output);
  static void GetRsuInData(::os::v2x::device::RSUData& rsu_in);
  static void SetRsuInData(const ::os::v2x::device::RSUData& rsu_data);
  static ::os::v2x::device::RSUData rsu_asn_data_;
  static std::mutex rsu_asn_mtx_;
  static std::condition_variable rsu_asn_condition_;

  std::atomic<os::v2x::cloud::MessageVersion> version_{
      os::v2x::cloud::CSAE_53_2020};
  std::string rscu_sn_;
  std::mutex bsm_mutex_;
  std::vector<std::string> bsm_cache_;

  std::shared_ptr<std::thread> bsm_send_{nullptr};
  std::atomic<bool> b_send_{true};

  std::shared_ptr<std::thread> asn_send_{nullptr};
  std::atomic<bool> a_send_{true};

  std::string MQTT_RSI_TOPIC_PREFIX  = "upload/v2i/rsi/";
  std::string MQTT_SPAT_TOPIC_PREFIX = "upload/v2i/spat/";
  std::string MQTT_BSM_TOPIC_PREFIX  = "upload/v2i/bsm/";
  std::string MQTT_MAP_TOPIC_PREFIX  = "upload/v2i/map/";
  std::string MQTT_RSM_TOPIC_PREFIX  = "upload/v2i/rsm";

  ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
      cloud_sequence_;
};

}  // namespace app
}  // namespace airos
