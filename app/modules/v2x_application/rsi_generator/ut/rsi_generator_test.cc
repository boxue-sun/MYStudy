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
#include "app/modules/v2x_application/rsi_generator/rsi_generator.h"

namespace airos {
namespace app {

TEST(RsiGeneratorComponentTest, test_rsi) {
  auto rsi_adapter = std::make_shared<RsiGenerator>();
  auto event_data  = std::make_shared<airos::usecase::EventOutputResult>();
  auto event_ptr   = event_data->add_events();
  event_ptr->set_event_type(
      airos::usecase::EventInformation_EventType_LANE_CONGESTION);
  event_ptr->set_id(57);
  event_ptr->set_timestamp(1672211558);
  auto position_llh = event_ptr->mutable_location_point();
  position_llh->set_x(750385);
  position_llh->set_y(2564273);
  position_llh->set_z(19.11);
  event_ptr->add_sensor_id("sncamera1");
  event_ptr->add_lane_id("5");
  EXPECT_TRUE(rsi_adapter->Proc(event_data));
}

}  // namespace app
}  // namespace airos
