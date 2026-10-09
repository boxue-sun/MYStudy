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

#include "middleware/runtime/demo/test.pb.h"

#include "gflags/gflags.h"
#include "middleware/runtime/src/air_middleware_component.h"

DEFINE_string(name, "caros", "host name");

class AIROS_COMPONENT_CLASS_NAME(DemoComponent)
    : public airos::middleware::ComponentAdapter<Chatter> {
 public:
  AIROS_COMPONENT_CLASS_NAME(DemoComponent)
  () = default;

  virtual ~AIROS_COMPONENT_CLASS_NAME(DemoComponent)() override{};
  virtual bool Init() override {
    std::cout << "DemoComponent::Init" << std::endl;
    LoadConfig<AppConfig>(&config_);
    return true;
  };
  virtual bool Proc(const std::shared_ptr<const Chatter> &msg) override {
    std::cout << "get msg: " << msg->seq() << " " << msg->content()
              << std::endl;
    std::cout << FLAGS_name << std::endl;
    Send("test_node/chatter_trans", msg);
    std::cout << "output_name: " << config_.output_name() << std::endl;
    return true;
  };

 private:
  AppConfig config_;
};

REGISTER_AIROS_COMPONENT_CLASS(DemoComponent, Chatter);
