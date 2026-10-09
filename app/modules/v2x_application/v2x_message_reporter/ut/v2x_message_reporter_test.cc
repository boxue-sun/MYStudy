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
#include <iostream>

#include "gtest/gtest.h"

#define private public
#include "app/modules/v2x_application/v2x_message_reporter/v2x_message_reporter.h"

namespace airos {
namespace app {

TEST(V2xMessageReporterComponentTest, test_timestamp) {
  auto report_adapter = std::make_shared<V2xMessageReporter>();
  auto rsu_out        = std::make_shared<os::v2x::device::RSUData>();
  rsu_out->set_type(os::v2x::device::RSU_BSM);
  rsu_out->set_data("rsubsm");
  rsu_out->set_time_stamp(report_adapter->GetCurrentTimestamp());
  std::cout << rsu_out->DebugString() << std::endl;
  EXPECT_TRUE(report_adapter->Proc(rsu_out));
  auto rsu_data = std::make_shared<os::v2x::device::RSUData>();
  rsu_data->set_type(os::v2x::device::RSU_MAP);
  rsu_data->set_data("rsumap");
  rsu_data->set_time_stamp(report_adapter->GetCurrentTimestamp());
  std::cout << rsu_data->DebugString() << std::endl;
  EXPECT_FALSE(report_adapter->Proc(rsu_data));
  report_adapter->b_send_ = false;
  report_adapter->a_send_ = false;
}

}  // namespace app
}  // namespace airos
