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

#include "app/framework/interface/app_base.h"

namespace airos {
namespace app {

class ApplicationFactory {
 public:
  using CONSTRUCT =
      std::function<AirOSApplication*(const ApplicationCallBack& cb)>;

  template <typename Inherit>
  class Register_t {
   public:
    Register_t(const std::string& key) {
      ApplicationFactory::Instance().map_.emplace(
          key, [](const ApplicationCallBack& cb) {
            return new Inherit(cb);
          });
    }
  };

  std::unique_ptr<AirOSApplication> GetUnique(
      const std::string& key, const ApplicationCallBack& cb);

  std::shared_ptr<AirOSApplication> GetShared(
      const std::string& key, const ApplicationCallBack& cb);

  static ApplicationFactory& Instance();

 private:
  AirOSApplication* Produce(
      const std::string& key, const ApplicationCallBack& cb);

  ApplicationFactory(){};
  ApplicationFactory(const ApplicationFactory&) = delete;
  ApplicationFactory(ApplicationFactory&&)      = delete;

 private:
  std::map<std::string, CONSTRUCT> map_;
};

#define AIROS_APPLICATION_REG(T) airos_reg_func_str_##T##_

#define AIROS_APPLICATION_REG_FACTORY(T, key)                                 \
  static airos::app::ApplicationFactory::Register_t<T> AIROS_APPLICATION_REG( \
      T)(key);

}  // end of namespace app
}  // end of namespace airos
