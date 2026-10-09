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

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "middleware/runtime/src/air_middleware_component.h"
#include "middleware/runtime/src/air_middleware_node.h"

namespace airos {
namespace middleware {

struct Chatter {
  int seq;
  std::string content;
  std::string type_sign = "struct Chatter";
};

TEST(AirRuntimeReaderaWriterTest, test_reader_writer) {
  //   std::mutex mtx;
  std::vector<Chatter> recv_msgs;
  std::shared_ptr<airos::middleware::AirMiddlewareNode> node(
      new airos::middleware::AirMiddlewareNode("test_node"));
  auto reader = node->CreateReader<Chatter>(
      std::string("test_node/chatter"),
      [&](const std::shared_ptr<const Chatter> &msg) {
        // std::lock_guard<std::mutex> lck(mtx);
        recv_msgs.emplace_back(*msg);
      });
  auto writer = node->CreateWriter<Chatter>(std::string("test_node/chatter"));
  std::shared_ptr<Chatter> msg(new Chatter);
  EXPECT_TRUE(writer->Write(msg));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(recv_msgs.size(), 1);
}

class AIROS_COMPONENT_CLASS_NAME(ComponentTest)
    : public airos::middleware::ComponentAdapter<Chatter> {
 public:
  AIROS_COMPONENT_CLASS_NAME(ComponentTest)() {}
  bool Init() {
    return true;
  }
  bool Proc(const std::shared_ptr<const Chatter> &) {
    return true;
  }

 private:
};
REGISTER_AIROS_COMPONENT_CLASS(ComponentTest, Chatter);

TEST(AirRuntimeComponent, test_component) {
  auto msg_str1 = std::make_shared<Chatter>();
  AIROS_COMPONENT_CLASS_NAME(ComponentTest) component_test;
  EXPECT_TRUE(component_test.Init());
  EXPECT_TRUE(component_test.Proc(msg_str1));
}

}  // namespace middleware
}  // namespace airos

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  airos::middleware::AirRuntimeInit(argv[0]);
  return RUN_ALL_TESTS();
}
