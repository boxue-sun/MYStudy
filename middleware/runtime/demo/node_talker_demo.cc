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

#include <chrono>
#include <iostream>

#include "middleware/runtime/demo/test.pb.h"

#include "middleware/runtime/src/air_middleware_node.h"

int main(int argc, char **argv) {
  airos::middleware::AirRuntimeInit(argv[0]);
  airos::middleware::AirMiddlewareNode node("test_node_talker");
  auto writer = node.CreateWriter<Chatter>(std::string("test_node/chatter"));

  std::shared_ptr<Chatter> msg(new Chatter);
  int n = 0;
  msg->set_content("node test");

  while (1) {
    msg->set_seq(++n);
    std::cout << "send msg: " << msg->seq() << " " << msg->content()
              << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    writer->Write(msg);
  }

  return 0;
}
