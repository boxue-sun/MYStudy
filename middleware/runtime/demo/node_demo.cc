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

#include "middleware/runtime/src/air_middleware_node.h"
// #include "test.pb.h"
#include <chrono>
#include <iostream>
#include <thread>

struct Chatter {
  int seq;
  std::string content;
  std::string type_sign = "struct Chatter";
};

void write_chatter(std::shared_ptr<airos::middleware::AirMiddlewareNode> node) {
  auto writer = node->CreateWriter<Chatter>(std::string("test_node/chatter"));
  std::shared_ptr<Chatter> msg(new Chatter);
  int n        = 0;
  msg->content = "node test";
  while (1) {
    msg->seq = ++n;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    writer->Write(msg);
  }
  return;
}

int main(int argc, char **argv) {
  airos::middleware::AirRuntimeInit(argv[0]);
  std::shared_ptr<airos::middleware::AirMiddlewareNode> node(
      new airos::middleware::AirMiddlewareNode("test_node"));

  std::thread writer_thread(write_chatter, node);
  writer_thread.detach();

  node->CreateReader<Chatter>(
      std::string("test_node/chatter"),
      [](const std::shared_ptr<const Chatter> &data) {
        std::cout << "received msg: " << data->seq << " " << data->content
                  << " " << data->type_sign << std::endl;
      });
  airos::middleware::WaitForShutdown();
}
