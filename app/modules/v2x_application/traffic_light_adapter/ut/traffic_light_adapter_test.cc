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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"

#include "base/common/log.h"

#define private public
#include "app/modules/v2x_application/traffic_light_adapter/traffic_light_adapter.h"

namespace airos {
namespace app {

class TrafficLightAdapterTest : public ::testing::Test {
 public:
  TrafficLightAdapterTest() {}
  virtual ~TrafficLightAdapterTest() {}
  void SetUp() override {
    const std::string file =
        "/airos/app/modules/v2x_application/traffic_light_adapter/ut/testdata/"
        "traffic_light_service.pb.txt";
  }

 protected:
  TrafficLightAdapter traffic_light_adapter_component_;
  airos::trafficlight::TrafficLightServiceData service_data_;
};

TEST_F(TrafficLightAdapterTest, test_adapter) {
  std::shared_ptr<airos::trafficlight::TrafficLightServiceData> frame = nullptr;
  std::shared_ptr<v2xpb::asn::MessageFrame> asn_pb                    = nullptr;
  EXPECT_FALSE(traffic_light_adapter_component_.TrafficLightServicePb2AsnPb(
      frame, asn_pb));

  frame  = std::make_shared<airos::trafficlight::TrafficLightServiceData>();
  asn_pb = std::make_shared<v2xpb::asn::MessageFrame>();
  EXPECT_FALSE(traffic_light_adapter_component_.TrafficLightServicePb2AsnPb(
      frame, asn_pb));
  APP_LOG_INFO << asn_pb->DebugString();
  asn_pb->Clear();
  frame = std::make_shared<airos::trafficlight::TrafficLightServiceData>(
      service_data_);
  EXPECT_TRUE(traffic_light_adapter_component_.TrafficLightServicePb2AsnPb(
      frame, asn_pb));
  APP_LOG_INFO << asn_pb->DebugString();

  auto cloud_pb = std::make_shared<os::v2x::device::CloudData>();
  EXPECT_TRUE(traffic_light_adapter_component_.TrafficLightServicePb2Cloud(
      frame, cloud_pb));
  APP_LOG_INFO << cloud_pb->DebugString();

  int64_t dsecond = traffic_light_adapter_component_.get_mill_second_minute();
  int64_t moy     = traffic_light_adapter_component_.get_minute_year();
  APP_LOG_INFO << "moy:" << moy << "dsecond: " << dsecond;
}

}  // namespace app
}  // namespace airos
