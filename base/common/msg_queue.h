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

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace airos {
namespace base {
namespace common {

enum class PushPolicy {
  blocking,     // 阻塞, 直到队列腾出空间
  discard_old,  // 丢弃队列中最老数据，非阻塞
  discard,      // 丢弃当前push的值，并返回false。非阻塞
};

enum class PopPolicy {
  blocking,  // 阻塞, 直到来数据
  nonblock,  // 直接返回false，不阻塞
};

template <typename T>
class MsgQueue {
 public:
  explicit MsgQueue(size_t capacity = 3)
      : _capacity(capacity == 0 ? 1 : capacity)
      , _enable(false) {}
  MsgQueue(const MsgQueue &)            = delete;
  MsgQueue(MsgQueue &&)                 = delete;
  MsgQueue &operator=(const MsgQueue &) = delete;
  MsgQueue &operator=(MsgQueue &&)      = delete;
  ~MsgQueue() {}

  template <typename TR>
  int push(
      TR &&data, PushPolicy policy = PushPolicy::discard_old,
      unsigned int timeout_ms = 0) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_data.size() >= _capacity) {
      // std::cout << "msg queue is full!!!!!" << std::endl;
      switch (policy) {
        case PushPolicy::blocking: {
          if (timeout_ms == 0) {
            _cv_push.wait(lock, [this]() {
              return !(_data.size() >= _capacity && _enable);
            });
          } else {
            _cv_push.wait_for(
                lock, std::chrono::milliseconds(timeout_ms), [this]() {
                  return !(_data.size() >= _capacity && _enable);
                });
          }
        } break;
        case PushPolicy::discard_old: {
          _data.pop_front();
        } break;
        default:
          return -1;
          break;
      }
    }
    if (!_enable || _data.size() >= _capacity) return -1;
    _data.push_back(std::forward<TR>(data));
    _cv_pop.notify_one();
    return 0;
  }

  template <typename TR>
  int operator<<(TR &&data) {
    return push(std::forward<TR>(data));
  }

  int pop(
      T *output, PopPolicy policy = PopPolicy::blocking,
      unsigned int timeout_ms = 0) {
    if (output == nullptr) return -1;
    std::unique_lock<std::mutex> lock(_mutex);
    if (_data.empty()) {
      switch (policy) {
        case PopPolicy::blocking: {
          if (timeout_ms == 0) {
            _cv_pop.wait(lock, [this]() {
              return !(_data.empty() && _enable);
            });
          } else {
            _cv_pop.wait_for(
                lock, std::chrono::milliseconds(timeout_ms), [this]() {
                  return !(_data.empty() && _enable);
                });
          }
        } break;
        default:
          return -1;
          break;
      }
    }
    if (!_enable || _data.empty()) return -1;
    *output = _data.front();
    _data.pop_front();
    _cv_push.notify_one();
    return 0;
  }
  void enable() {
    std::unique_lock<std::mutex> lock(_mutex);
    _enable = true;
  }
  void disable() {
    std::unique_lock<std::mutex> lock(_mutex);
    _enable = false;
    _data.clear();
    lock.unlock();
    _cv_pop.notify_one();
    _cv_push.notify_one();
  }
  void clear() {
    std::unique_lock<std::mutex> lock(_mutex);
    _data.clear();
  }

 private:
  mutable std::mutex _mutex;
  std::condition_variable _cv_push;
  std::condition_variable _cv_pop;
  const size_t _capacity;  // _data容量
  std::list<T> _data;
  bool _enable;
};
}  // namespace common
}  // namespace base
}  // namespace airos
