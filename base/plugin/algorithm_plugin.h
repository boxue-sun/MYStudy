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

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/common/log.h"

namespace airos {
namespace base {

struct PluginParam {
  std::string name;
  std::string config_file;
  std::string extra_describe;
};

template <typename Dtype>
class PluginElement {
 public:
  PluginElement()                             = default;
  virtual ~PluginElement()                    = default;
  virtual bool Init(const PluginParam& param) = 0;
  virtual bool Run(Dtype& data)               = 0;
  // virtual std::string Name() const = 0;
};

template <typename Dtype>
class PluginElementRegistry {
 public:
  typedef std::shared_ptr<PluginElement<Dtype> > (*Creator)();
  typedef std::map<std::string, Creator> CreatorRegistry;

  static CreatorRegistry& Registry() {
    static CreatorRegistry* g_registry_ = new CreatorRegistry();
    return *g_registry_;
  }

  // Adds a creator.
  static void AddCreator(const std::string& type, Creator creator) {
    CreatorRegistry& registry = Registry();
    CHECK_EQ(registry.count(type), 0)
        << "PluginElement type " << type << " already registered.";
    registry[type] = creator;
  }

  static std::shared_ptr<PluginElement<Dtype> > CreatePluginElement(
      const std::string& type) {
    CreatorRegistry& registry = Registry();
    LOG_INFO << "registry size: " << registry.size();
    CHECK_EQ(registry.count(type), 1)
        << "Unknown PluginElement type: " << type
        << " (known types: " << PluginElementTypeListString() << ")";
    // return registry[type](param);
    return registry[type]();
  }

  static std::vector<std::string> PluginElementTypeList() {
    CreatorRegistry& registry = Registry();
    std::vector<std::string> plugin_element_names;
    for (typename CreatorRegistry::iterator iter = registry.begin();
         iter != registry.end();
         ++iter) {
      plugin_element_names.push_back(iter->first);
    }
    return plugin_element_names;
  }

 private:
  PluginElementRegistry() {}

  static std::string PluginElementTypeListString() {
    std::vector<std::string> element_types = PluginElementTypeList();
    std::string element_types_str;
    for (std::vector<std::string>::iterator iter = element_types.begin();
         iter != element_types.end();
         ++iter) {
      if (iter != element_types.begin()) {
        element_types_str += ", ";
      }
      element_types_str += *iter;
    }
    return element_types_str;
  }
};

template <typename Dtype>
class PluginRegisterer {
 public:
  PluginRegisterer(
      const std::string& type,
      std::shared_ptr<PluginElement<Dtype> > (*creator)()) {
    // LOG_INFO << "Registering PluginElement name: " << v;
    PluginElementRegistry<Dtype>::AddCreator(type, creator);
  }
};

#define REGISTER_ALGORITHM_PLUGIN_ELEMENT(name, data_type)                   \
  std::shared_ptr<airos::base::PluginElement<data_type> > Creator_##name() { \
    return std::shared_ptr<airos::base::PluginElement<data_type> >(          \
        new name());                                                         \
  }                                                                          \
  static airos::base::PluginRegisterer<data_type> g_creator_f_##name(        \
      #name, Creator_##name);

template <typename Dtype>
class PluginManager {
 public:
  PluginManager()  = default;
  ~PluginManager() = default;

  bool Init(const std::vector<PluginParam>& element_params) {
    for (auto& param : element_params) {
      std::shared_ptr<PluginElement<Dtype> > element =
          PluginElementRegistry<Dtype>::CreatePluginElement(param.name);
      if (element == nullptr) return false;
      if (element->Init(param) == false) return false;
      _elements.emplace_back(std::move(element));
    }
    return true;
  }

  bool Run(Dtype& data) {
    for (auto& element : _elements) {
      if (element->Run(data) == false) return false;
    }
    return true;
  }

 private:
  std::vector<std::shared_ptr<PluginElement<Dtype> > > _elements;
};

}  // namespace base
}  // namespace airos
