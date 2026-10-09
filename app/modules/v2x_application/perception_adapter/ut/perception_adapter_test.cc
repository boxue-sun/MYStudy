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

#define private public
#include "app/modules/v2x_application/perception_adapter/perception_adapter.h"

namespace airos {
namespace app {

TEST(PerceptionAdapterTest, test_perception_adapter) {
  PerceptionAdapter perception_adapter_;
  auto usecase  = std::make_shared<airos::usecase::EventOutputResult>();
  auto cloud_pb = std::make_shared<os::v2x::device::CloudData>();
  auto event    = usecase->add_events();
  event->set_id(1);
  event->set_event_type(
      airos::usecase::EventInformation_EventType_LANE_CONGESTION);
  EXPECT_TRUE(perception_adapter_.UsecasePb2Cloud(usecase, cloud_pb));
}

}  // namespace app
}  // namespace airos
