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
#include <fcntl.h>

#include <string>

#include "base/common/log.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

namespace airos {
namespace base {

// @brief load protobuf(TXT) data from file.
template <typename T>
bool ParseProtobufFromFile(const std::string &file_name, T *pb) {
  int fd = open(file_name.c_str(), O_RDONLY);
  if (fd < 0) {
   APP_FRAMEWORK_LOG_WARN << "ProtobufParser load file failed. file: " << file_name;
    return false;
  }

  google::protobuf::io::FileInputStream fs(fd);
  if (!google::protobuf::TextFormat::Parse(&fs, pb)) {
   APP_FRAMEWORK_LOG_WARN << "ProtobufParser parse data failed. file:" << file_name;
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

}  // namespace base
}  // namespace airos
