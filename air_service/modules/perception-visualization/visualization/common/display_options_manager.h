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

#include <string>
#include <unordered_map>
#include <vector>

namespace airos {
namespace perception {
namespace visualization {

class DisplayOptionsManager {
 public:
  static DisplayOptionsManager *Instance();
  ~DisplayOptionsManager();

  void set_option(const std::string &option, bool status) {
    options_[option] = status;
  }

  bool get_option(const std::string &option) const {
    if (options_.find(option) == options_.end()) {
      return false;
    }

    return options_.at(option);
  }

  void invert_option(const std::string &option) {
    if (options_.find(option) == options_.end()) {
      return;
    }

    options_[option] = !options_[option];
  }

  void ReadOptionsFromINIFile(const std::string &filepath);

 private:
  DisplayOptionsManager();

 private:
  std::unordered_map<std::string, bool> options_;
};

}  // namespace visualization
}  // namespace perception
}  // namespace airos
