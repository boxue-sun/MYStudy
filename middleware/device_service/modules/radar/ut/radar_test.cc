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

#include <cassert>
#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "base/device_connect/radar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

static std::stringstream g_radar_cout_buf;
void proc_radar_data(const RadarDataType &radar_data) {
  std::cout << radar_data->header().sequence_num();
}

class RadaeTest : public ::testing::Test {
 public:
  RadaeTest()
      : device_(nullptr)
      , sbuf_(nullptr) {}
  virtual ~RadaeTest() {}
  void SetUp() override {
    sbuf_ = std::cout.rdbuf();
    std::cout.rdbuf(g_radar_cout_buf.rdbuf());
  }
  void TearDown() override {
    std::cout.rdbuf(sbuf_);
    sbuf_ = nullptr;
  }

 protected:
  std::shared_ptr<RadarDevice> device_;
  std::streambuf *sbuf_;
};

TEST_F(RadaeTest, test_all_interface) {
  device_ =
      RadarDeviceFactory::Instance().GetUnique("dummy_radar", proc_radar_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(device_->Init(
      "/airos/middleware/device_service/modules/radar/ut/radar.cfg"));

  device_->Start();
  std::string expect{"01234"};
  std::string res = g_radar_cout_buf.str();
  EXPECT_EQ(expect, res);

  EXPECT_EQ(RadarDeviceState::RUNNING, device_->GetState());
}

TEST_F(RadaeTest, test_init_failed) {
  device_ =
      RadarDeviceFactory::Instance().GetUnique("dummy_radar", proc_radar_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_FALSE(
      device_->Init("/airos/base/device_connect/radar/ut/radar.cfg-no-exists"));
}

}  // namespace device
}  // namespace v2x
}  // namespace os
