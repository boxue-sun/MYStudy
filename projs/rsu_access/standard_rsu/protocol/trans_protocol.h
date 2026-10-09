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

#include "base/device_connect/proto/rsu_data.pb.h"

namespace os {
namespace v2x {
namespace device {

using RSUDataPtr      = std::shared_ptr<os::v2x::device::RSUData>;
using RSUDataConstPtr = std::shared_ptr<const os::v2x::device::RSUData>;

class TransProtocol {
 public:
  TransProtocol()                                         = default;
  virtual ~TransProtocol()                                = default;
  virtual std::string Encode(const RSUDataConstPtr& data) = 0;
  virtual bool Decode(const char* pack, std::size_t len, RSUDataPtr& data) = 0;
};

}  // namespace device
}  // namespace v2x
}  // namespace os
