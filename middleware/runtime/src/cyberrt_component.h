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

#define AIROS_COMPONENT_CLASS_NAME(class_name) class_name##Adapter

#define REGISTER_AIROS_COMPONENT_NULLTYPE_CLASS(class_name) \
  class class_name : public apollo::cyber::Component<> {    \
   public:                                                  \
    class_name() = default;                                 \
    virtual ~class_name() override {}                       \
    virtual bool Init() override {                          \
      impl_.RegisterNode(node_);                            \
      impl_.RegisterComponent(this);                        \
      return impl_.Init();                                  \
    }                                                       \
                                                            \
   protected:                                               \
    AIROS_COMPONENT_CLASS_NAME(class_name)                  \
    impl_;                                                  \
  };                                                        \
  CYBER_REGISTER_COMPONENT(class_name);

#define REGISTER_AIROS_COMPONENT_CLASS(class_name, datatype)           \
  class class_name : public apollo::cyber::Component<datatype> {       \
   public:                                                             \
    class_name() = default;                                            \
    virtual ~class_name() override {}                                  \
    virtual bool Init() override {                                     \
      impl_.RegisterNode(node_);                                       \
      impl_.RegisterComponent(this);                                   \
      return impl_.Init();                                             \
    }                                                                  \
                                                                       \
   protected:                                                          \
    AIROS_COMPONENT_CLASS_NAME(class_name)                             \
    impl_;                                                             \
                                                                       \
   private:                                                            \
    virtual bool Proc(const std::shared_ptr<datatype> &msg) override { \
      return impl_.Proc(msg);                                          \
    }                                                                  \
  };                                                                   \
  CYBER_REGISTER_COMPONENT(class_name);
