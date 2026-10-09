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
#include "air_middleware_common.h"
#include "air_middleware_reader.h"
#include "air_middleware_writer.h"

namespace airos {
namespace middleware {

class AirMiddlewareNode {
 public:
  explicit AirMiddlewareNode(const std::string &node_name)
      : node_impl_(CreateNodeImpl(node_name)) {}
  explicit AirMiddlewareNode(const std::shared_ptr<NODE_IMPL> &node_impl) {
    node_impl_ = node_impl;
  }
  ~AirMiddlewareNode() = default;

  template <typename MessageT>
  std::shared_ptr<AirMiddlewareReader<MessageT>> CreateReader(
      const std::string &channel,
      std::function<void(const std::shared_ptr<const MessageT> &)> callback) {
    return std::make_shared<AirMiddlewareReader<MessageT>>(
        node_impl_->CreateReader<MessageT>(channel, callback));
  }

  template <typename MessageT>
  std::shared_ptr<AirMiddlewareWriter<MessageT>> CreateWriter(
      const std::string &channel) {
    return std::make_shared<AirMiddlewareWriter<MessageT>>(
        node_impl_->CreateWriter<MessageT>(channel));
  }

  template <typename MessageT>
  std::shared_ptr<AirMiddlewareReader<MessageT>> CreateReader(
      const apollo::cyber::ReaderConfig &config,
      std::function<void(const std::shared_ptr<const MessageT> &)> callback) {
    return std::make_shared<AirMiddlewareReader<MessageT>>(
        node_impl_->CreateReader<MessageT>(config, callback));
  }

 private:
  std::shared_ptr<NODE_IMPL> node_impl_;
};

}  // namespace middleware
}  // namespace airos
