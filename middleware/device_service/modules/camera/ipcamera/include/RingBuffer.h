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

#ifndef __cplusplus
#error "This is a c++ header!"
#endif

#include <mutex>
#include <string>

class RingBuffer final {
 private:
  const size_t _M_const_buffer_size;  // MUST 2^n
  void *const _M_buffer;
  std::mutex _M_lock;
  size_t _M_loc_head = 0;
  size_t _M_loc_tail = 0;

 public:
  static constexpr unsigned int BUF_SIZE_EXP_MIN = 12;
  static constexpr unsigned int BUF_SIZE_EXP_MAX = 30;

  /**
   * 构造一个环形缓冲区。
   *
   * 由于长度<b>必须</b>是 2 的指数，即长度的二进制码中只能有一个'1'，
   * 作为限制，传入的参数即为 2 的指数
   * @param size_exp 缓冲区长度指数
   */
  explicit RingBuffer(unsigned int size_exp = 20);

  RingBuffer(const RingBuffer &) = delete;

  const RingBuffer &operator=(const RingBuffer &) = delete;

  ~RingBuffer();

  /**
   * 清除 Buffer 中所有的数据
   */
  void clear();

  /**
   * 获取该缓冲区的总长度
   * @return 缓冲区总长度
   */
  size_t size();

  /**
   * 获取缓冲区已被使用的长度
   * @return 已被使用的长度
   */
  size_t length();

  /**
   * 将数据写入缓冲区结尾。如果缓冲区剩余长度不足，则写满为止，数据被截断，不会溢出。
   * @param buf  需要写入的数据
   * @param len  数据长度
   * @return     实际写入数据的长度
   */
  size_t putdata(const void *buf, size_t len);

  /**
   * 从缓冲区提取数据。
   * @param buf  [OUT] 提取出的数据
   * @param len  允许提取的最大长度
   * @return     实际提取的数据长度
   */
  size_t getdata(void *buf, size_t len);

  /**
   * 调试用，获取缓冲区内部情况。包括总长度，已被使用的长度，缓冲区头的位置。
   * @return 用于显示的字符串
   */
  std::string debug_info();
};
