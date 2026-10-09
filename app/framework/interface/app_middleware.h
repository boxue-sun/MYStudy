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
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/any.hpp>
#include <google/protobuf/message.h>

#include "app/framework/interface/app_comm.h"

namespace airos {
namespace app {

extern std::unordered_map<OSMessageType, ::boost::any> message_callbacks_;

template <
    typename T, typename = typename std::enable_if<std::is_base_of<
                    ::google::protobuf::Message, T>::value>::type>
using ReaderCallBack = std::function<void(const std::shared_ptr<const T>&)>;

template <
    typename T, typename = typename std::enable_if<std::is_base_of<
                    ::google::protobuf::Message, T>::value>::type>
void RegisterOSMessage(OSMessageType message_type, ReaderCallBack<T> callback) {
  message_callbacks_[message_type] = callback;
}

template <typename T>
void TriggerMessage(
    OSMessageType message_type, const std::shared_ptr<const T>& message) {
  auto it = message_callbacks_.find(message_type);
  if (it != message_callbacks_.end()) {
    try {
      boost::any_cast<ReaderCallBack<T>&>(it->second)(message);
    } catch (const boost::bad_any_cast& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
}

std::vector<OSMessageType> GetSubscribedMessageTypes();

// RegisterOSUsrMessage(topic, callback)

}  // end of namespace app
}  // end of namespace airos
