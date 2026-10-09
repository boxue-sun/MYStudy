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

#include "base/device_connect/ins/device_factory.h"

namespace os {
namespace v2x {
namespace device {

static std::stringstream g_ins_cout_buf;
void proc_gnss_data(const GnssPBDataTypePtr& data) {
  std::cout << data->sequence_num();
}

void proc_imu_data(const IMUPBDataTypePtr& data) {
  std::cout << data->sequence_num();
}

class InsTest : public ::testing::Test {
 public:
  InsTest()
      : device_(nullptr)
      , sbuf_(nullptr) {}
  virtual ~InsTest() {}

  void SetUp() override {
    sbuf_ = std::cout.rdbuf();
    std::cout.rdbuf(g_ins_cout_buf.rdbuf());
  }

  void TearDown() override {
    std::cout.rdbuf(sbuf_);
    sbuf_ = nullptr;
  }

 protected:
  std::shared_ptr<InsDevice> device_;
  std::streambuf* sbuf_;
};

TEST_F(InsTest, test_all_interface) {
  device_ = InsDeviceFactory::Instance().GetUnique(
      "dummy_ins", proc_gnss_data, proc_imu_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(
      device_->Init("/airos/middleware/device_service/modules/ins/ut/ins.cfg"));

  device_->Start();
  std::string expect{"0112233445"};
  std::string res = g_ins_cout_buf.str();
  EXPECT_EQ(expect, res);

  EXPECT_EQ(InsDeviceState::NORMAL, device_->GetState());
}

TEST_F(InsTest, test_init_failed) {
  device_ = InsDeviceFactory::Instance().GetUnique(
      "dummy_ins", proc_gnss_data, proc_imu_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_FALSE(
      device_->Init("/airos/base/device_connect/ins/ut/ins.cfg-no-exists"));
}

}  // namespace device
}  // namespace v2x
}  // namespace os
