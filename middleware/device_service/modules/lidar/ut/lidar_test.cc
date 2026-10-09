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

#include "base/device_connect/lidar/device_factory.h"

namespace os {
namespace v2x {
namespace device {

static std::stringstream g_lidar_cout_buf;
void proc_lidar_data(const LidarPBDataTypePtr &lidar_data) {
  std::cout << lidar_data->header().sequence_num();
}

class LidarTest : public ::testing::Test {
 public:
  LidarTest()
      : device_(nullptr)
      , sbuf_(nullptr) {}
  virtual ~LidarTest() {}

  void SetUp() override {
    sbuf_ = std::cout.rdbuf();
    std::cout.rdbuf(g_lidar_cout_buf.rdbuf());
  }

  void TearDown() override {
    std::cout.rdbuf(sbuf_);
    sbuf_ = nullptr;
  }

 protected:
  std::shared_ptr<LidarDevice> device_;
  std::streambuf *sbuf_;
};

TEST_F(LidarTest, test_all_interface) {
  device_ =
      LidarDeviceFactory::Instance().GetUnique("dummy_lidar", proc_lidar_data);
  ASSERT_NE(device_, nullptr);
  LidarInitConfig config;
  ASSERT_TRUE(device_->Init(config));

  std::string expect{"01234"};
  std::string res = g_lidar_cout_buf.str();
  EXPECT_EQ(expect, res);

  EXPECT_EQ(LidarDeviceState::NORMAL, device_->GetState());
}

TEST_F(LidarTest, test_init_failed) {
  device_ = LidarDeviceFactory::Instance().GetUnique(
      "noexist_lidar", proc_lidar_data);
  ASSERT_EQ(device_, nullptr);
}

}  // namespace device
}  // namespace v2x
}  // namespace os
