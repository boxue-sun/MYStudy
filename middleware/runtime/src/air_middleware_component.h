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
#include <unordered_map>

#include "air_middleware_common.h"
#include "air_middleware_writer.h"
#include "middleware/runtime/src/air_middleware_node.h"

namespace airos {
namespace middleware {

class NullType {};

template <typename M0 = NullType>
class ComponentAdapter {
 public:
  ComponentAdapter() {}
  virtual ~ComponentAdapter() {}
  virtual bool Init()                                     = 0;
  virtual bool Proc(const std::shared_ptr<const M0> &msg) = 0;

  template <typename ProtoT>
  bool LoadConfig(ProtoT *config) {
    return component_->GetProtoConfig(config);
  }

  template <typename M>
  bool Send(const std::string &channel, const std::shared_ptr<M> &msg) {
    static std::
        unordered_map<std::string, std::shared_ptr<AirMiddlewareWriter<M>>>
            writers_map_;
    std::lock_guard<std::mutex> lg(writers_mutex_);
    if (writers_map_.find(channel) == writers_map_.end()) {
      auto writer = node_->CreateWriter<M>(channel);
      writers_map_.insert(std::make_pair(channel, writer));
    }
    return writers_map_[channel]->Write(msg);
  }

  void RegisterNode(std::shared_ptr<NODE_IMPL> &node_impl) {
    node_.reset(new AirMiddlewareNode(node_impl));
  }

  void RegisterComponent(IMPL_NAMESPACE::Component<M0> *component) {
    component_ = component;
  }

 protected:
  std::shared_ptr<AirMiddlewareNode> node_;

 private:
  std::mutex writers_mutex_;
  IMPL_NAMESPACE::Component<M0> *component_ = nullptr;
};

template <>
class ComponentAdapter<NullType> {
 public:
  ComponentAdapter() {}
  virtual ~ComponentAdapter() {}
  virtual bool Init() = 0;

  template <typename ProtoT>
  bool LoadConfig(ProtoT *config) {
    return component_->GetProtoConfig(config);
  }

  template <typename M>
  bool Send(const std::string &channel, const std::shared_ptr<M> &msg) {
    static std::
        unordered_map<std::string, std::shared_ptr<AirMiddlewareWriter<M>>>
            writers_map_;
    std::lock_guard<std::mutex> lg(writers_mutex_);
    if (writers_map_.find(channel) == writers_map_.end()) {
      auto writer = node_->CreateWriter<M>(channel);
      writers_map_.insert(std::make_pair(channel, writer));
    }
    return writers_map_[channel]->Write(msg);
  }

  void RegisterNode(std::shared_ptr<NODE_IMPL> &node_impl) {
    node_.reset(new AirMiddlewareNode(node_impl));
  }

  void RegisterComponent(IMPL_NAMESPACE::Component<> *component) {
    component_ = component;
  }

 protected:
  std::shared_ptr<AirMiddlewareNode> node_;

 private:
  std::mutex writers_mutex_;
  IMPL_NAMESPACE::Component<> *component_ = nullptr;
};

#include "middleware/runtime/src/cyberrt_component.h"

}  // namespace middleware
}  // namespace airos
