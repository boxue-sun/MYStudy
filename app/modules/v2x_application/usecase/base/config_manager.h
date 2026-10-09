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

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "app/modules/v2x_application/usecase/base/meta_data.h"
#include "app/modules/v2x_application/usecase/common/common.h"
#include "yaml-cpp/yaml.h"

namespace airos {
namespace perception {
namespace usecase {

class ConfigManagerImpl;
class BaseParams;

class ConfigManager {
 public:
  ConfigManager() = default;
  bool Init(const std::string& config_file);
  bool LoadFor(std::shared_ptr<BaseParams> params);
  void GetYamlNode(YAML::Node* yaml_node);

 private:
  std::shared_ptr<ConfigManagerImpl> impl_;
};

class BaseParams : public Noncopyable {
 public:
  virtual ~BaseParams()                          = default;
  virtual void PrintConfig()                     = 0;
  virtual bool ParseNode(const YAML::Node& node) = 0;
  virtual std::string Name() const               = 0;

  Meta GetVal(const std::string& key) {
    if (!mass_.count(key)) {
      APP_LOG_ERROR << "confg has no key: " << key;
      std::cout << "debug-> confg has no key: " << key << std::endl;
      throw 1;
    }
    return mass_[key];
  }

 protected:
  std::map<std::string, Meta> mass_;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
