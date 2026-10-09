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
#include <memory>
#include <string>

#include "cyber/cyber.h"
#define IMPL_NAMESPACE apollo::cyber
#define WRITER_IMPL(MessageT) apollo::cyber::Writer<MessageT>
#define READER_IMPL(MessageT) apollo::cyber::Reader<MessageT>
#define NODE_IMPL apollo::cyber::Node
#define RATE_IMPL apollo::cyber::Rate

namespace airos {
namespace middleware {
inline std::shared_ptr<NODE_IMPL> CreateNodeImpl(const std::string &node_name) {
  return apollo::cyber::CreateNode(node_name);
}

inline bool AirRuntimeInit(const char *binary_name) {
  return apollo::cyber::Init(binary_name);
}

inline void WaitForShutdown() {
  apollo::cyber::WaitForShutdown();
}

}  // namespace middleware
}  // namespace airos
