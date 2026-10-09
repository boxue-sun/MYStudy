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

#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace visualization {

DisplayOptionsManager* DisplayOptionsManager::Instance() {
  static DisplayOptionsManager manager;
  return &manager;
}

DisplayOptionsManager::DisplayOptionsManager() {}

DisplayOptionsManager::~DisplayOptionsManager() {}

void DisplayOptionsManager::ReadOptionsFromINIFile(
    const std::string& filepath) {
  boost::property_tree::ptree pt;
  try {
    boost::property_tree::ini_parser::read_ini(filepath, pt);
  } catch (std::exception& ex) {
    LOG_ERROR << ex.what() << std::endl;
  }

  auto pt_items = pt.get_child("DisplayOptions");
  for (auto itr = pt_items.begin(); itr != pt_items.end(); ++itr) {
    std::string key = (*itr).first.data();
    options_[key]   = pt_items.get<bool>(key);
    LOG_INFO << "DisplayOptions: " << key << " = " << options_[key];
  }
}

}  // namespace visualization
}  // namespace perception
}  // namespace airos
