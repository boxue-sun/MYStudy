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

#include <memory>
#include <string>

#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/device_connect/proto/cloud_data.pb.h"

#include "app/modules/v2x_application/common/producer_consumer_queue.h"

namespace airos {
namespace app {

class PerceptionAdapter {
 public:
  PerceptionAdapter() {}

  ~PerceptionAdapter(){};
  bool Init(ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
                cloud_sequence);
  bool Proc(
      const std::shared_ptr<const airos::usecase::EventOutputResult>& usecase);

 private:
  bool UsecasePb2Cloud(
      const std::shared_ptr<const airos::usecase::EventOutputResult>& usecase,
      std::shared_ptr<os::v2x::device::CloudData> cloud_pb);

  std::string rscu_sn_;
  std::string MQTT_PERCEPTION_TOPIC_PREFIX = "upload/event/";
  ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
      cloud_sequence_;
};

}  // namespace app
}  // namespace airos
