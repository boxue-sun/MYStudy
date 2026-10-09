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

#include "middleware/device_service/modules/camera/ipcamera/include/RingBuffer.h"

#include <unistd.h>

#include <cstring>
#include <sstream>

#ifdef __MIN
#undef __MIN
#endif
#define __MIN(a, b) ((a) > (b) ? (b) : (a))

#ifdef __MAX
#undef __MAX
#endif
#define __MAX(a, b) ((a) < (b) ? (b) : (a))

#ifdef __REAL_HEAD
#undef __REAL_HEAD
#endif
#define __REAL_HEAD (_M_loc_head & (_M_const_buffer_size - 1))

#ifdef __REAL_TAIL
#undef __REAL_TAIL
#endif
#define __REAL_TAIL (_M_loc_tail & (_M_const_buffer_size - 1))

RingBuffer::RingBuffer(unsigned int size_exp)
    : _M_const_buffer_size([](size_t size_exp) {
      size_exp = __MAX(size_exp, BUF_SIZE_EXP_MIN);
      size_exp = __MIN(size_exp, BUF_SIZE_EXP_MAX);
      return 1ULL << size_exp;
    }(size_exp))
    , _M_buffer(aligned_alloc(getpagesize(), _M_const_buffer_size)) {
  if (!_M_buffer) {
    throw std::bad_alloc();
  }
}

RingBuffer::~RingBuffer() {
  free(_M_buffer);
}

void RingBuffer::clear() {
  std::lock_guard<std::mutex> g(_M_lock);
  _M_loc_tail = 0;
  _M_loc_head = 0;
}

size_t RingBuffer::size() {
  return _M_const_buffer_size;
}

size_t RingBuffer::length() {
  std::lock_guard<std::mutex> g(_M_lock);
  return _M_loc_tail - _M_loc_head;
}

size_t RingBuffer::putdata(const void *buf, size_t len) {
  std::lock_guard<std::mutex> g(_M_lock);
  len        = __MIN(len, _M_const_buffer_size - _M_loc_tail + _M_loc_head);
  size_t tmp = __MIN(len, _M_const_buffer_size - __REAL_TAIL);
  memcpy(static_cast<char *>(_M_buffer) + __REAL_TAIL, buf, tmp);
  memcpy(_M_buffer, (const char *)(buf) + tmp, len - tmp);
  _M_loc_tail += len;
  return len;
}

size_t RingBuffer::getdata(void *buf, size_t len) {
  std::lock_guard<std::mutex> g(_M_lock);
  size_t tmp = 0;
  len        = __MIN(len, _M_loc_tail - _M_loc_head);
  tmp        = __MIN(len, _M_const_buffer_size - __REAL_HEAD);
  memcpy(buf, (const char *)(_M_buffer) + __REAL_HEAD, tmp);
  memcpy(static_cast<char *>(buf) + tmp, _M_buffer, len - tmp);
  _M_loc_head += len;
  return len;
}

std::string RingBuffer::debug_info() {
  std::lock_guard<std::mutex> g(_M_lock);
  std::stringstream res;
  res << _M_const_buffer_size << " : " << (_M_loc_tail - _M_loc_head) << " : "
      << __REAL_HEAD;
  return res.str();
}
