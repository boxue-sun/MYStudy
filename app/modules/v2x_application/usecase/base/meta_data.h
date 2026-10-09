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
#include <string>
#include <typeinfo>
#include <utility>

#include "app/modules/v2x_application/usecase/common/tensor.h"
#include "base/common/log.h"

namespace airos {
namespace perception {

namespace usecase {

class Meta {
 public:
  Meta()
      : _content(nullptr) {}

  template <typename ValueType>
  explicit Meta(const ValueType& value)
      : _content(new holder<ValueType>(value)) {}

  Meta(const Meta& other)
      : _content(other._content ? other._content->Clone() : 0) {}

  Meta& Swap(Meta& rhs) {
    std::swap(_content, rhs._content);
    return *this;
  }

  ~Meta() {
    if (_content != nullptr) {
      delete _content;
      _content = nullptr;
    }
  }

  template <typename ValueType>
  Meta& operator=(const ValueType& rhs) {
    Meta tmp(rhs);
    tmp.Swap(*this);
    return *this;
  }

  Meta& operator=(const Meta& rhs) {
    if (_content == rhs._content) {
      return *this;
    }

    if (rhs._content == nullptr) {
      delete _content;
      _content = nullptr;
      return *this;
    }

    _content = rhs._content->Clone();
    return *this;
  }

  bool Empty() const {
    return !_content;
  }

  const std::type_info& Type() const {
    return _content ? _content->Type() : typeid(void);
  }

  template <typename ValueType>
  ValueType Cast() const {
    if (Type() != typeid(ValueType)) {
      APP_LOG_ERROR << "Meta caset error: " << Type().name()
                << " != " << typeid(ValueType).name();
      std::cout << "debug-> Meta caset error: " << Type().name()
                << " != " << typeid(ValueType).name() << std::endl;
      throw 7;
    }
    return ((holder<ValueType>*)(_content))->_held;
  }

 private:
  class placeholder {
   public:
    virtual ~placeholder()                     = default;
    virtual const std::type_info& Type() const = 0;
    virtual placeholder* Clone() const         = 0;
  };

  template <typename ValueType>
  class holder : public placeholder {
   public:
    explicit holder(const ValueType& value)
        : _held(value) {}
    virtual const std::type_info& Type() const {
      return typeid(ValueType);
    }
    virtual placeholder* Clone() const {
      return new holder(_held);
    }

   public:
    ValueType _held;
  };

 private:
  placeholder* _content;
};

struct Sensor {
  std::string name;
  int32_t id;
  Tensor position;
  double timestamp;
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
