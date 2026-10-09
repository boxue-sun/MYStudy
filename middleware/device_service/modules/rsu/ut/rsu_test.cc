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

#include "base/common/log.h"
#include "base/device_connect/rsu/device_factory.h"
#include "middleware/device_service/modules/rsu/standard_rsu/protocol/internal_protocol.h"
#include "middleware/device_service/modules/rsu/standard_rsu/standard_rsu.h"

namespace os {
namespace v2x {
namespace device {

static std::stringstream g_rsu_cout_buf;
void proc_rsu_data(const RSUPBDataType &rsu_data) {
  std::cout << rsu_data->sequence_num();
}

class RsuTest : public ::testing::Test {
 public:
  RsuTest()
      : device_(nullptr)
      , sbuf_(nullptr) {}
  virtual ~RsuTest() {}
  void SetUp() override {
    sbuf_ = std::cout.rdbuf();
    std::cout.rdbuf(g_rsu_cout_buf.rdbuf());
    RSUData rsu_data;
    rsu_data.set_type(RSU_SPAT);
    rsu_data.set_data("abcdefg");
    rsu_data.set_time_stamp(1668402441551);
    rsu_input_data_ = std::make_shared<const RSUData>(rsu_data);

    transfer_ = std::make_shared<InternalProtocol>();
  }
  void TearDown() override {
    std::cout.rdbuf(sbuf_);
    sbuf_ = nullptr;
  }

 protected:
  std::shared_ptr<RSUDevice> device_;
  std::streambuf *sbuf_;
  std::shared_ptr<InternalProtocol> transfer_;
  std::shared_ptr<const RSUData> rsu_input_data_;
};

TEST_F(RsuTest, test_all_interface) {
  device_ = RSUDeviceFactory::Instance().GetUnique("dummy_rsu", proc_rsu_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(device_->Init(
      "/airos/middleware/device_service/modules/rsu/ut/testdata/rsu.cfg"));

  device_->Start();
  std::string expect{"01234"};
  std::string res = g_rsu_cout_buf.str();
  EXPECT_EQ(expect, res);

  EXPECT_EQ(RSUDeviceState::NORMAL, device_->GetState());
}

TEST_F(RsuTest, test_init_failed) {
  device_ = RSUDeviceFactory::Instance().GetUnique("dummy_rsu", proc_rsu_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_FALSE(
      device_->Init("/airos/middleware/device_service/modules/rsu/ut/testdata/"
                    "rsu.cfg-no-exists"));
}

TEST_F(RsuTest, test_internal_protocol) {
  std::string res = transfer_->Encode(rsu_input_data_);
  ASSERT_NE(res, "");

  auto output_data = std::make_shared<RSUData>();
  ASSERT_TRUE(transfer_->Decode(res.data(), res.size(), output_data));
  RSU_SERVICE_LOG_INFO << output_data->DebugString();

  std::string error_data = "abcdefghijklmn";
  EXPECT_FALSE(
      transfer_->Decode(error_data.data(), error_data.size(), output_data));
}

TEST_F(RsuTest, test_rsu_impl) {
  device_ =
      RSUDeviceFactory::Instance().GetUnique("standard_rsu", proc_rsu_data);
  ASSERT_NE(device_, nullptr);
  ASSERT_TRUE(
      device_->Init("/airos/middleware/device_service/modules/rsu/ut/testdata/"
                    "standard_rsu.yaml"));

  std::unique_ptr<std::thread> send_thread = nullptr;
  std::unique_ptr<std::thread> recv_thread = nullptr;
  send_thread.reset(new std::thread([this]() {
    int cnt = 0;
    while (cnt++ < 5) {
      device_->WriteToDevice(rsu_input_data_);
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }));
  // recv_thread.reset(new std::thread([this]() {
  //   device_->Start();
  // }));

  if (send_thread != nullptr && send_thread->joinable()) {
    send_thread->join();
  }
}

}  // namespace device
}  // namespace v2x
}  // namespace os
