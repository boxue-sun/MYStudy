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

#include "app/modules/v2x_application/usecase/base/config_manager.h"

namespace airos {
namespace perception {
namespace usecase {

class ConfigManagerImpl {
 public:
  bool Init(const std::string &config_file) {
    try {
      yaml_node_ = YAML::LoadFile(config_file);
    } catch (YAML::BadFile &e) {
      // APP_LOG_FATAL << "Read " << config_file << " failed," << strerror(errno);
      return false;
    }

    if (yaml_node_.IsNull()) {
      // APP_LOG_WARN << "Read " << config_file << " empty";
    }

    return true;
  }

  bool LoadFor(std::shared_ptr<BaseParams> params) {
    if (yaml_node_[params->Name()].IsDefined()) {
      if (!params->ParseNode(yaml_node_[params->Name()])) {
        return false;
      }
      params->PrintConfig();
      return true;
    }
    return false;
  }

  void GetYamlNode(YAML::Node *yaml_node) {
    *yaml_node = yaml_node_;
  }

 private:
  YAML::Node yaml_node_;
};

bool ConfigManager::Init(const std::string &config_file) {
  impl_.reset(new ConfigManagerImpl());
  return impl_->Init(config_file);
}

bool ConfigManager::LoadFor(std::shared_ptr<BaseParams> parmas) {
  return impl_->LoadFor(parmas);
}

void ConfigManager::GetYamlNode(YAML::Node *yaml_node) {
  impl_->GetYamlNode(yaml_node);
}

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
