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

#include "perception_adapter.h"

#include "base/env/env.h"

namespace airos {
namespace app {

bool PerceptionAdapter::Init(
    ProducerConsumerQueue<std::shared_ptr<os::v2x::device::CloudData>>*
        cloud_sequence) {
  rscu_sn_        = airos::base::Environment::GetDeviceSn();
  cloud_sequence_ = cloud_sequence;
  return true;
}

bool PerceptionAdapter::Proc(
    const std::shared_ptr<const airos::usecase::EventOutputResult>& usecase) {
  auto cloud_pb = std::make_shared<os::v2x::device::CloudData>();
  if (UsecasePb2Cloud(usecase, cloud_pb)) {
    cloud_sequence_->push(cloud_pb);
  }
  return true;
}
bool PerceptionAdapter::UsecasePb2Cloud(
    const std::shared_ptr<const airos::usecase::EventOutputResult>& usecase,
    std::shared_ptr<os::v2x::device::CloudData> cloud_pb) {
  if (!usecase || !cloud_pb) {
    return false;
  }
  auto mqtt_pb = cloud_pb->mutable_mqtt_data();
  mqtt_pb->set_topic(MQTT_PERCEPTION_TOPIC_PREFIX + rscu_sn_);
  std::string str_data;
  usecase->SerializePartialToString(&str_data);
  mqtt_pb->set_data(str_data);
  return true;
}

}  // namespace app
}  // namespace airos
