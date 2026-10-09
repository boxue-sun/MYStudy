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
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace airos {
namespace perception {
namespace usecase {

template <typename Cls>
class Factory {
 public:
  template <typename Base>
  using CONSTRUCT = std::function<Base*(void)>;

  template <typename Inherit>
  class Register_t {
   public:
    explicit Register_t(const std::string& key) {
      Factory<Cls>::Instance().map_.emplace(key, [] {
        return new Inherit();
      });
    }

    template <typename... Args>
    explicit Register_t(const std::string& key, Args&&... args) {
      Factory<Cls>::Instance().map_.emplace(key, [&] {
        return new Inherit(std::forward<Args>(args)...);
      });
    }
  };
  /**
   * @brief      用于获取指定类实例的unique指针
   * @param[in]  key 指定类名
   * @retval     指定类实例的unique指针
   */
  std::unique_ptr<Cls> GetUnique(const std::string& key) {
    return std::unique_ptr<Cls>(Produce(key));
  }
  /**
   * @brief      用于获取指定类实例的shared指针
   * @param[in]  key 指定类名
   * @retval     指定类实例的shared指针
   */
  std::shared_ptr<Cls> GetShared(const std::string& key) {
    return std::shared_ptr<Cls>(Produce(key));
  }
  /**
   * @brief      用于获取类工厂实例（单例）
   * @retval     类工厂实例
   */
  inline static Factory<Cls>& Instance() {
    static Factory<Cls> Instance_;
    return Instance_;
  }

 private:
  Cls* Produce(const std::string& key) {
    if (map_.find(key) == map_.end()) {
      // APP_LOG_FATAL << key << "has not register to ParamsFactory," <<
      // strerror(errno);
      return nullptr;
    }
    return map_[key]();
  }

  Factory() {}
  Factory(const Factory&) = delete;
  Factory(Factory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT<Cls> > map_;
};

#define USECASE_REGISTER_CLASS(BASE, T) usecase_factory_##BASE##_##T##_
/**
 * @brief      用于注册指定的类
 * @param[in]  BASE 具体注册类的所属基类
 * @param[in]  T 具体的注册类
 * @param[in]  key 注册的类名
 */
#define USECASE_REGISTER_FACTORY(BASE, T, key, ...)                    \
  static Factory<BASE>::Register_t<T> USECASE_REGISTER_CLASS(BASE, T)( \
      key, ##__VA_ARGS__);

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
