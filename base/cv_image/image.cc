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

#include "image.h"

#include <utility>

namespace airos {
namespace base {

const std::map<Color, int> kChannelsMap{
    {Color::GRAY, 1}, {Color::RGB, 3}, {Color::BGR, 3}};

Image8U::Image8U(
    int rows, int cols, Color type, std::shared_ptr<Blob<uint8_t>> blob,
    int offset)
    : rows_(rows)
    , cols_(cols)
    , type_(type)
    , blob_(std::move(blob))
    , offset_(offset) {
  channels_ = kChannelsMap.at(type);
  CHECK_EQ(blob_->num_axes(), 3);
  CHECK_EQ(blob_->shape(2), channels_);
  CHECK_LE(
      offset_ + blob_->offset({rows - 1, cols - 1, channels_ - 1}),
      blob_->count());
  width_step_ = blob_->offset({1, 0, 0}) * sizeof(uint8_t);
}

// Image8U(int rows, int cols, Color type, bool use_tensor = false)
Image8U::Image8U(int rows, int cols, Color type)
    : rows_(rows)
    , cols_(cols)
    , type_(type)
    , offset_(0) {
  channels_ = kChannelsMap.at(type);

  blob_.reset(new Blob<uint8_t>({rows_, cols_, channels_}));
  width_step_ = blob_->offset({1, 0, 0}) * sizeof(uint8_t);
}

}  // namespace base
}  // namespace airos
