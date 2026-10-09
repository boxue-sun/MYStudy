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

#include "gtest/gtest.h"

#include "base/common/log.h"
#include "base/device_connect/cloud/device_factory.h"
#include "middleware/device_service/modules/cloud/standard_mqtt/standard_mqtt.h"

namespace os {
namespace v2x {
namespace device {

TEST(CloudTest, test_mqtt) {
  std::shared_ptr<CloudDevice> device =
      CloudDeviceFactory::Instance().GetUnique(
          "standard_mqtt", [](const CloudPBDataType& data) {
            LOG_INFO << "test mqtt";
          });
  ASSERT_NE(device, nullptr);
  YAML::Node root_node =
      YAML::LoadFile("/airos/base/device_connect/cloud/ut/mqtt.yaml");
  ASSERT_TRUE(device->Init(root_node));
}

}  // namespace device
}  // namespace v2x
}  // namespace os
