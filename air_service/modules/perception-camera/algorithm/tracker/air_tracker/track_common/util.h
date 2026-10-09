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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "base/common/box.h"
#include "base/common/log.h"
#include "object_types.h"

namespace airos {
namespace perception {
namespace algorithm {

template <typename T>
bool OutOfValidX(
    const airos::base::BBox2D<T> box, const T width, const T height,
    const T border_size = 0) {
  if (box.xmin < border_size || box.ymin < border_size ||
      box.xmax + border_size > width) {
    return true;
  }
  return false;
}

template <typename T>
bool OutOfValidX(
    const airos::base::Rect<T> rect, const T width, const T height,
    const T border_size = 0) {
  airos::base::BBox2D<T> box(rect);
  return OutOfValidX(box, width, height, border_size);
}

bool LoadAnchors(const std::string &path, std::vector<float> *anchors);
bool LoadTypes(
    const std::string &path, std::vector<track::ObjectSubType> *types);
bool LoadExpand(const std::string &path, std::vector<float> *expands);
bool LoadConfidenceMap(const std::string &path, std::vector<float> *confidence);
// bool ResizeCPU(const airos::base::Blob<uint8_t> &src_gpu,
//                std::shared_ptr<airos::base::Blob<float> > dst,
//                int stepwidth,
//                int start_axis);

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
