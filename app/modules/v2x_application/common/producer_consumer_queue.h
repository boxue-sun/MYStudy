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

#ifndef PRODUER_CONSUMER_QUEUE_H
#define PRODUER_CONSUMER_QUEUE_H
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>
#include <vector>

namespace airos {
namespace app {

template <typename T>
class ProducerConsumerQueue {
 private:
  std::mutex _queueLock;
  std::queue<T> _queue;
  std::condition_variable _condition;
  std::atomic<bool> _shutdown;

 public:
  ProducerConsumerQueue<T>()
      : _shutdown(false) {}

  void push(const T &value) {
    std::lock_guard<std::mutex> lock(_queueLock);
    _queue.push(std::move(value));

    _condition.notify_one();
  }

  void push(const std::vector<T> &vec_values) {
    std::lock_guard<std::mutex> lock(_queueLock);

    for (unsigned int i = 0; i < vec_values.size(); ++i) {
      _queue.push(std::move(vec_values[i]));
    }

    _condition.notify_one();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(_queueLock);

    return _queue.empty();
  }

  bool pop(T &value, int &remain_num) {
    std::lock_guard<std::mutex> lock(_queueLock);

    if (_queue.empty() || _shutdown) {
      return false;
    }

    value = _queue.front();

    _queue.pop();
    remain_num = _queue.size();

    return true;
  }

  bool pop(std::vector<T> &vec_values) {
    std::lock_guard<std::mutex> lock(_queueLock);

    for (; !_queue.empty();) {
      T &value = _queue.front();
      vec_values.push_back(value);

      _queue.pop();
    }

    return !vec_values.empty();
  }

  bool pop_multi(T &value, int num, int &remain_num) {
    std::lock_guard<std::mutex> lock(_queueLock);

    if (_queue.empty() || _shutdown) {
      return false;
    }

    int pop_num = _queue.size() - num;
    for (int i = 0; i < pop_num; ++i) {
      _queue.pop();
    }

    if (_queue.empty() || _shutdown) {
      return false;
    }

    value      = _queue.front();
    remain_num = _queue.size();

    return true;
  }

  void wait_and_pop(T &value, int &remain_num) {
    std::unique_lock<std::mutex> lock(_queueLock);

    while (_queue.empty() && !_shutdown) {
      _condition.wait(lock);
    }

    if (_queue.empty() || _shutdown) {
      return;
    }

    value = _queue.front();

    _queue.pop();
    remain_num = _queue.size();
  }

  bool pop(T &value) {
    int32_t remain_size = 0;
    return pop(value, remain_size);
  }

  void wait_and_pop(T &value) {
    int32_t remain_size = 0;
    wait_and_pop(value, remain_size);
  }

  void cancel() {
    std::unique_lock<std::mutex> lock(_queueLock);

    while (!_queue.empty()) {
      T &value = _queue.front();

      delete_queued_object(value);

      _queue.pop();
    }

    _shutdown = true;

    _condition.notify_all();
  }

 private:
  template <typename E = T>
  typename std::enable_if<std::is_pointer<E>::value>::type delete_queued_object(
      E &obj) {
    delete obj;
  }

  template <typename E = T>
  typename std::enable_if<!std::is_pointer<E>::value>::type
  delete_queued_object(E const & /*packet*/) {}
};

template <typename T>
class MPSCQueueNonIntrusive {
 public:
  MPSCQueueNonIntrusive()
      : _head(new Node())
      , _tail(_head.load(std::memory_order_release))
      , _shutdown(false) {
    Node *front = _head.load();
    front->Next.store(nullptr);
  }

  ~MPSCQueueNonIntrusive() {
    Node *front = _head.load(std::memory_order_release);
    delete front;
  }

  void push(const T &input) {
    Node *node     = new Node(input);
    Node *prevHead = _head.exchange(node, std::memory_order_acq_rel);
    prevHead->Next.store(node, std::memory_order_release);

    ++_queue_size;

    _condition.notify_one();
  }

  bool pop(T &value, uint32_t &remain_size) {
    Node *tail = _tail.load(std::memory_order_release);
    Node *next = tail->Next.load(std::memory_order_acquire);
    if (!next) {
      return false;
    }

    value = next->Data;
    _tail.store(next, std::memory_order_release);
    delete tail;

    remain_size = --_queue_size;
    return true;
  }

  void wait_and_pop(T &value, uint32_t &remain_size) {
    std::unique_lock<std::mutex> lock(_queueLock);

    while (!pop(value, remain_size) && !_shutdown) {
      _condition.wait(lock);
    }
  }

  bool pop(T &value) {
    uint32_t remain_size = 0;
    return pop(value, remain_size);
  }

  void wait_and_pop(T &value) {
    uint32_t remain_size = 0;
    wait_and_pop(value, remain_size);
  }

  void cancel() {
    for (;;) {
      Node *tail = _tail.load(std::memory_order_release);
      Node *next = tail->Next.load(std::memory_order_acquire);
      if (!next) break;

      _tail.store(next, std::memory_order_release);
      delete tail;

      --_queue_size;
    }

    _shutdown = true;
    _condition.notify_all();
  }

 private:
  struct Node {
    Node() = default;
    explicit Node(const T &data)
        : Data(data) {
      Next.store(nullptr);
    }

    T Data;
    std::atomic<Node *> Next;
  };

  std::atomic<Node *> _head;
  std::atomic<Node *> _tail;
  std::atomic<bool> _shutdown;

  std::atomic<uint32_t> _queue_size;

  std::mutex _queueLock;
  std::condition_variable _condition;

  MPSCQueueNonIntrusive(MPSCQueueNonIntrusive const &)            = delete;
  MPSCQueueNonIntrusive &operator=(MPSCQueueNonIntrusive const &) = delete;
};
}  // namespace app
}  // namespace airos
#endif
