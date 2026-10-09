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

#include <thread>

#include "middleware/device_service/framework/proto/cloud_config.pb.h"

#include "base/device_connect/cloud/device_base.h"
#include "middleware/runtime/src/air_middleware_component.h"

namespace os {
namespace v2x {
namespace device {

class AIROS_COMPONENT_CLASS_NAME(CloudComponent)
    : public airos::middleware::ComponentAdapter<os::v2x::device::CloudData> {
 public:
  AIROS_COMPONENT_CLASS_NAME(CloudComponent)()           = default;
  ~AIROS_COMPONENT_CLASS_NAME(CloudComponent)() override = default;

  bool Init() override;
  bool Proc(const std::shared_ptr<const os::v2x::device::CloudData>& recv_data)
      override;

 private:
  void CallBack(const CloudPBDataType& data);

 private:
  std::unique_ptr<CloudDevice> device_;
  std::unique_ptr<std::thread> task_;
  os::v2x::device::cloud::Config conf_;
};

REGISTER_AIROS_COMPONENT_CLASS(CloudComponent, os::v2x::device::CloudData);

}  // namespace device
}  // namespace v2x
}  // namespace os
