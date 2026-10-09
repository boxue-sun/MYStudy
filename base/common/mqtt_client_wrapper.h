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

#include <MQTTClient.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace airos {
namespace base {

class MQTTClientInterface {
 public:
  using msglist_t =
      std::list<std::pair<std::string, std::shared_ptr<MQTTClient_message>>>;

  /**
   * 是否初始化成功
   * @return
   */
  virtual bool inited() const = 0;
  /**
   * 当前客户端自动重连的次数。当连接成功时，自动归零
   *
   * @return
   */
  virtual int count_connect_retry() const = 0;
  /**
   * 订阅
   *
   * @param topic
   * @param qos
   * @return
   */
  virtual bool subscribe(const std::string &topic, int qos) = 0;
  /**
   * 取消订阅
   *
   * @param topic
   * @return
   */
  virtual bool unsubscribe(const std::string &topic) = 0;
  /**
   * 发布
   *
   * @param topic
   * @param data
   * @param qos
   * @return
   */
  virtual bool publish(
      const std::string &topic, const std::string &data, int qos) = 0;
  /**
   *
   * @param block_ms <=0 DO NOT BLOCK >0: Block time
   * @return received message
   */
  virtual msglist_t get_received_messages(int block_ms) = 0;

  virtual inline bool subscribe(const std::string &topic) final {
    return subscribe(topic, 0);  // DEFAULT: QoS 0
  }
  virtual inline bool publish(
      const std::string &topic, const std::string &data) final {
    return publish(topic, data, 0);  // DEFAULT: QoS 0
  }
  virtual inline msglist_t get_received_messages() final {
    return get_received_messages(0);  // Default: NONBLOCK
  }
};

class MQTTClientBuilder {
  std::string _m_server_uri;
  std::string _m_client_id;
  std::string _m_client_username;
  std::string _m_client_password;

  std::string _m_client_ssl_filepath_ca;
  std::string _m_client_ssl_filepath_client_cert;
  std::string _m_client_ssl_filepath_client_key;
  std::string _m_client_ssl_filepath_client_key_password;

  int _m_connect_timeout_s;

 public:
  MQTTClientBuilder &set_server_uri(const std::string &val) {
    _m_server_uri = val;
    return *this;
  }
  MQTTClientBuilder &set_client_id(const std::string &val) {
    _m_client_id = val;
    return *this;
  }
  MQTTClientBuilder &set_client_username(const std::string &val) {
    _m_client_username = val;
    return *this;
  }
  MQTTClientBuilder &set_client_password(const std::string &val) {
    _m_client_password = val;
    return *this;
  }
  MQTTClientBuilder &set_client_ssl_filepath_ca(const std::string &val) {
    _m_client_ssl_filepath_ca = val;
    return *this;
  }
  MQTTClientBuilder &set_client_ssl_filepath_client_cert(
      const std::string &val) {
    _m_client_ssl_filepath_client_cert = val;
    return *this;
  }
  MQTTClientBuilder &set_client_ssl_filepath_client_key(
      const std::string &val) {
    _m_client_ssl_filepath_client_key = val;
    return *this;
  }
  MQTTClientBuilder &set_client_ssl_filepath_client_key_password(
      const std::string &val) {
    _m_client_ssl_filepath_client_key_password = val;
    return *this;
  }
  MQTTClientBuilder &set_connect_timeout_s(int val) {
    _m_connect_timeout_s = val < 0 ? 0 : (val > 60 ? 60 : val);
    return *this;
  }

  std::shared_ptr<MQTTClientInterface> build() const;
};

}  // namespace base
}  // namespace airos
