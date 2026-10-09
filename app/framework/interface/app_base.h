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
#pragma once

#include <functional>
#include <memory>

#include "app/framework/proto/airos_app_config.pb.h"
#include "app/framework/proto/airos_app_data.pb.h"

#include "app/framework/interface/app_comm.h"

namespace airos {
namespace app {

using ApplicationDataType = std::shared_ptr<const ApplicationData>;
using ApplicationCallBack = std::function<void(const ApplicationDataType&)>;

class AirOSApplication {
 public:
  AirOSApplication() = default;
  explicit AirOSApplication(const ApplicationCallBack& cb)
      : sender_(cb) {}
  virtual ~AirOSApplication()                      = default;
  virtual bool Init(const AppliactionConfig& conf) = 0;
  virtual void Start()                             = 0;

 protected:
  ApplicationCallBack sender_;
};

}  // end of namespace app
}  // end of namespace airos
