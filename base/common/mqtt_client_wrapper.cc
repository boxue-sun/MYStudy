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

#include "base/common/mqtt_client_wrapper.h"

#include <uuid/uuid.h>

#include "base/common/log.h"

namespace airos {
namespace base {

class MQTTClientImplement final : public MQTTClientInterface {
 private:
  using guard = std::lock_guard<std::recursive_mutex>;
  using ulock = std::unique_lock<std::mutex>;

  std::atomic_bool _m_inited             = {false};
  std::atomic_int _m_count_connect_retry = {0};
  const std::string _m_server_uri;
  const std::string _m_client_id;
  const std::string _m_client_username;
  const std::string _m_client_password;

  const std::string _m_client_ssl_filepath_ca;
  const std::string _m_client_ssl_filepath_client_cert;
  const std::string _m_client_ssl_filepath_client_key;
  const std::string _m_client_ssl_filepath_client_key_password;

  const int _m_connect_timeout_s;

  const MQTTClient_SSLOptions _m_connect_ssl_opts;
  const MQTTClient_connectOptions _m_connect_opts;

  MQTTClient _m_client;
  mutable std::recursive_mutex _m_lock;
  std::map<std::string, int> _m_subscribed_topics;

  mutable std::mutex _m_lock_messages;
  std::condition_variable _m_cond_messages;
  msglist_t _m_list_received_messages;

 private:
  // C Callbacks
  static void _s_callback_connection_lost(void *context, char *cause);
  static int _s_callback_message_arrived(
      void *context, char *topicName, int topicLen,
      MQTTClient_message *message);
  static void _s_callback_delivery_complete(
      void *context, MQTTClient_deliveryToken dt);
  static int _s_callback_openssl_cb(const char *str, size_t len, void *u);

  void on_connection_lost();
  void on_message_arrived(
      std::string topic, std::shared_ptr<MQTTClient_message> message_ptr);

 public:
  explicit MQTTClientImplement(
      std::string server_uri,
      std::string client_id,                       //
      std::string username, std::string password,  //
      std::string ca_file, std::string cert_file,  //
      std::string key_file,
      std::string key_file_password,  //
      int connect_timeout_s           //
  );
  virtual ~MQTTClientImplement();

  bool inited() const override;

  int count_connect_retry() const override;
  bool subscribe(const std::string &topic, int qos) override;
  bool unsubscribe(const std::string &topic) override;
  bool publish(
      const std::string &topic, const std::string &data, int qos) override;
  msglist_t get_received_messages(int block_ms) override;
};

void MQTTClientImplement::_s_callback_connection_lost(
    void *context, char *cause) {
  auto client = (MQTTClientImplement *)context;
 APP_FRAMEWORK_LOG_WARN << "Connection lost: " << cause;
  client->on_connection_lost();
}
int MQTTClientImplement::_s_callback_message_arrived(
    void *context, char *topicName, int topicLen, MQTTClient_message *message) {
  if (!context || !topicName || !message) {
   APP_FRAMEWORK_LOG_WARN << "Invalid Params!";
    return 0;
  }
  // Generic Topic
  std::string topic;
  if (topicLen == 0) {
    topic.append(topicName);
  } else {
    topic.append(topicName, topicLen);
  }
  MQTTClient_free(topicName);
  std::shared_ptr<MQTTClient_message> message_ptr(
      message, [](MQTTClient_message *p) {
        MQTTClient_freeMessage(&p);
      });
  auto client = (MQTTClientImplement *)context;
  client->on_message_arrived(std::move(topic), std::move(message_ptr));
  return 1;
}
void MQTTClientImplement::_s_callback_delivery_complete(
    void *context, MQTTClient_deliveryToken dt) {
  //    auto client = (MQTTClientImplement *) context;
  // TODO
 APP_FRAMEWORK_LOG_WARN << "Message with token value " << dt << " delivery confirmed";
}

int MQTTClientImplement::_s_callback_openssl_cb(
    const char *str, size_t len, void *u) {
 APP_FRAMEWORK_LOG_WARN << "OPENSSL: " << str;
  return 1;
}

void MQTTClientImplement::on_connection_lost() {
 APP_FRAMEWORK_LOG_WARN << "Connection lost. try to reconnect ...";
  // Connect
 APP_FRAMEWORK_LOG_WARN << "MQTTClient connecting ...";
  int rc = MQTTCLIENT_SUCCESS;
  do {
    auto conn_opts = _m_connect_opts;
    {
      guard g(_m_lock);
      if (!_m_inited || !_m_client) {
        return;
      }
      rc = MQTTClient_connect(_m_client, &conn_opts);
      _m_count_connect_retry++;
    }
    if (MQTTCLIENT_SUCCESS != rc) {
     APP_FRAMEWORK_LOG_WARN << "Failed to start connect, return code: " << rc;
      usleep(100 * 1000);
    }
  } while (MQTTCLIENT_SUCCESS != rc);
 APP_FRAMEWORK_LOG_WARN << "MQTTClient connected ...";
  _m_count_connect_retry = 0;

  {
    guard g(_m_lock);
    if (!_m_inited || !_m_client) {
      return;
    }
    for (const auto &s : _m_subscribed_topics) {
     APP_FRAMEWORK_LOG_WARN << "MQTTClient subscribing \"" << s.first << "\"...";
      // TODO retry subscribe
      rc = MQTTClient_subscribe(_m_client, s.first.c_str(), s.second);
      if (MQTTCLIENT_SUCCESS != rc) {
       APP_FRAMEWORK_LOG_WARN << "Failed to subscribe topic: \"" << s.first
                 << "\", return code: " << rc;
      }
    }
  }
}
void MQTTClientImplement::on_message_arrived(
    std::string topic, std::shared_ptr<MQTTClient_message> message_ptr) {
 APP_FRAMEWORK_LOG_WARN << "Message arrived. Topic: " << topic;
  {
    ulock ul(_m_lock_messages);
    _m_list_received_messages.emplace_back(
        std::make_pair(std::move(topic), std::move(message_ptr)));
    // TODO move magic num to Params
    if (_m_list_received_messages.size() > 1024) {
      _m_list_received_messages.pop_front();
    }
  }
  _m_cond_messages.notify_all();
}

MQTTClientImplement::MQTTClientImplement(
    std::string server_uri, std::string client_id,        //
    std::string username, std::string password,           //
    std::string ca_file, std::string cert_file,           //
    std::string key_file, std::string key_file_password,  //
    int connect_timeout_s                                 //
    )
    : _m_server_uri(std::move(server_uri))
    , _m_client_id([](std::string client_id) {
      if (client_id.empty()) {
        uuid_t uuid;
        uuid_generate(uuid);
        char buffer[256] = {};
        uuid_unparse(uuid, buffer);
        return std::string("unknown_client_id-").append(buffer);
      }
      return client_id;
    }(std::move(client_id)))
    , _m_client_username(std::move(username))
    , _m_client_password(std::move(password))
    , _m_client_ssl_filepath_ca(std::move(ca_file))
    , _m_client_ssl_filepath_client_cert(std::move(cert_file))
    , _m_client_ssl_filepath_client_key(std::move(key_file))
    , _m_client_ssl_filepath_client_key_password(std::move(key_file_password))
    , _m_connect_timeout_s(connect_timeout_s)
    , _m_connect_ssl_opts([this]() {
      MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
      // TODO USE FLAG TO CONTROL IT
      //        ssl_opts.verify = 1;
      ssl_opts.enableServerCertAuth = 1;
      ssl_opts.ssl_error_cb      = MQTTClientImplement::_s_callback_openssl_cb;
      ssl_opts.ssl_error_context = this;
      ssl_opts.trustStore        = this->_m_client_ssl_filepath_ca.c_str();
      ssl_opts.keyStore   = this->_m_client_ssl_filepath_client_cert.c_str();
      ssl_opts.privateKey = this->_m_client_ssl_filepath_client_key.c_str();
      ssl_opts.privateKeyPassword =
          this->_m_client_ssl_filepath_client_key_password.c_str();
      return ssl_opts;
    }())
    , _m_connect_opts([this]() {
      MQTTClient_connectOptions conn_opts =
          MQTTClient_connectOptions_initializer;
      conn_opts.keepAliveInterval = 20;
      conn_opts.cleansession      = 1;
      conn_opts.username          = _m_client_username.c_str();
      conn_opts.password          = _m_client_password.c_str();
      conn_opts.ssl =
          const_cast<MQTTClient_SSLOptions *>(&this->_m_connect_ssl_opts);
      conn_opts.connectTimeout = _m_connect_timeout_s;
      return conn_opts;
    }())
    , _m_client(nullptr) {
  int rc = MQTTCLIENT_SUCCESS;
  // Create client
 APP_FRAMEWORK_LOG_WARN << "MQTTClient creating ...";
  if (MQTTCLIENT_SUCCESS !=
      (rc = MQTTClient_create(
           &_m_client, _m_server_uri.c_str(), _m_client_id.c_str(),
           MQTTCLIENT_PERSISTENCE_NONE, nullptr))) {
   APP_FRAMEWORK_LOG_WARN << "Failed to create client, return code: " << rc << ": "
              << MQTTClient_strerror(rc);
    return;
  }
  if (MQTTCLIENT_SUCCESS !=  //
      (rc = MQTTClient_setCallbacks(
           _m_client,
           this,                                              //
           MQTTClientImplement::_s_callback_connection_lost,  //
           MQTTClientImplement::_s_callback_message_arrived,  //
           MQTTClientImplement::_s_callback_delivery_complete))) {
   APP_FRAMEWORK_LOG_WARN << "Failed to set callbacks, return code: " << rc;
    return;
  }
  // Connect
 APP_FRAMEWORK_LOG_WARN << "MQTTClient connecting ...";
  auto conn_opts = _m_connect_opts;
  if (MQTTCLIENT_SUCCESS != (rc = MQTTClient_connect(_m_client, &conn_opts))) {
   APP_FRAMEWORK_LOG_WARN << "Failed to start connect, return code: " << rc << ": "
              << MQTTClient_strerror(rc);
    return;
  }

 APP_FRAMEWORK_LOG_WARN << "MQTTClient connected ...";
  // Finally
  _m_inited = true;
}
MQTTClientImplement::~MQTTClientImplement() {
  _m_inited = false;
  _m_cond_messages.notify_all();
  {
    guard g(_m_lock);
    int rc = MQTTCLIENT_SUCCESS;
   APP_FRAMEWORK_LOG_WARN << "MQTTClient disconnecting ...";
    if (MQTTCLIENT_SUCCESS != (rc = MQTTClient_disconnect(_m_client, 10000))) {
     APP_FRAMEWORK_LOG_WARN << "Failed to start disconnect, return code: " << rc;
    }
    MQTTClient_destroy(&_m_client);
  }
  ulock ul(_m_lock_messages);
}

bool MQTTClientImplement::inited() const {
  return _m_inited;
}
int MQTTClientImplement::count_connect_retry() const {
  return _m_count_connect_retry;
}
bool MQTTClientImplement::subscribe(const std::string &topic, int qos) {
  qos = qos < 0 ? 0 : (qos > 2 ? 2 : qos);
 APP_FRAMEWORK_LOG_WARN << "MQTTClient subscribing \"" << topic << "\"...";
  int rc = MQTTCLIENT_SUCCESS;
  {
    guard g(_m_lock);
    _m_subscribed_topics.emplace(topic, qos);
   APP_FRAMEWORK_LOG_WARN << "Topic is already subscribed, topic : " << topic;
    rc = MQTTClient_subscribe(_m_client, topic.c_str(), qos);
  }
  if (MQTTCLIENT_SUCCESS != rc) {
   APP_FRAMEWORK_LOG_WARN << "Failed to subscribe topic: \"" << topic
             << "\", return code: " << rc << ": " << MQTTClient_strerror(rc);
    return false;
  }
 APP_FRAMEWORK_LOG_WARN << "MQTTClient subscribe \"" << topic << "\" OK";
  return true;
}
bool MQTTClientImplement::unsubscribe(const std::string &topic) {
 APP_FRAMEWORK_LOG_WARN << "MQTTClient unsubscribing \"" << topic << "\"...";
  int rc = MQTTCLIENT_SUCCESS;
  {
    guard g(_m_lock);
    _m_subscribed_topics.erase(topic);

   APP_FRAMEWORK_LOG_WARN << "Topic is not subscribed, topic : " << topic;
    rc = MQTTClient_unsubscribe(_m_client, topic.c_str());
  }
  if (MQTTCLIENT_SUCCESS != rc) {
   APP_FRAMEWORK_LOG_WARN << "Failed to unsubscribe, return code: " << rc << ": "
             << MQTTClient_strerror(rc);
    return false;
  }
 APP_FRAMEWORK_LOG_WARN << "MQTTClient unsubscribe \"" << topic << "\" OK";
  return true;
}
bool MQTTClientImplement::publish(
    const std::string &topic, const std::string &data, int qos) {
  qos                            = qos < 0 ? 0 : (qos > 2 ? 2 : qos);
  MQTTClient_message message     = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token = 0;
  message.payload                = const_cast<char *>(data.c_str());
  message.payloadlen             = int(data.length());
  message.qos                    = qos;
  message.retained               = 0;
  int rc                         = MQTTCLIENT_SUCCESS;
  {
    guard g(_m_lock);
    rc = MQTTClient_publishMessage(_m_client, topic.c_str(), &message, &token);
  }
  if (MQTTCLIENT_SUCCESS != rc) {
   APP_FRAMEWORK_LOG_WARN << "Failed to publish, return code: " << rc;
    return false;
  }
  APP_FRAMEWORK_LOG_INFO << "Publishing messages. Token: " << token;
  // TODO
  //    MQTTClient_waitForCompletion(_m_client, token, 1000);
  return true;
}
MQTTClientImplement::msglist_t MQTTClientImplement::get_received_messages(
    int block_ms) {
  ulock ul(_m_lock_messages);
  if (block_ms > 0) {
    _m_cond_messages.wait_for(
        ul, std::chrono::milliseconds(block_ms), [this]() {
          return _m_inited && !_m_list_received_messages.empty();
        });
  }
  msglist_t result;
  _m_list_received_messages.swap(result);
  return result;
}

std::shared_ptr<MQTTClientInterface> MQTTClientBuilder::build() const {
  auto result = std::make_shared<MQTTClientImplement>(
      _m_server_uri,
      _m_client_id,  //
      _m_client_username,
      _m_client_password,                          //
      _m_client_ssl_filepath_ca,                   //
      _m_client_ssl_filepath_client_cert,          //
      _m_client_ssl_filepath_client_key,           //
      _m_client_ssl_filepath_client_key_password,  //
      _m_connect_timeout_s);
  if (!result->inited()) {
   APP_FRAMEWORK_LOG_WARN << "Failed to create mqtt client.";
    return nullptr;
  }
  return result;
}

}  // namespace base
}  // namespace airos
