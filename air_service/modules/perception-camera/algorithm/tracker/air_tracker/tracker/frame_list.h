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
#include <sstream>
#include <string>
#include <vector>

#include "base/common/log.h"
#include "track_frame.h"

namespace airos {
namespace perception {
namespace algorithm {

class FrameList {
 public:
  FrameList() {
    Init(1);
  }

  ~FrameList() {
    for (int i = 0; i < frames_.size(); i++) {
      delete frames_[i];
    }
  }

  bool Init(int cap) {
    if (cap <= 0) {
      return false;
    }
    capability_  = cap;
    frame_count_ = 0;
    frames_.resize(capability_);
    for (int i = 0; i < frames_.size(); i++) {
      frames_[i] = new TrackFrame;
    }
    return true;
  }

  inline int OldestFrameId() {
    if (frame_count_ < capability_) {
      return 0;
    } else {
      return get_frame(frame_count_)->frame_id;
    }
  }

  inline int Size() {
    if (frame_count_ < capability_) {
      return frame_count_;
    } else {
      return capability_;
    }
  }

  inline void Add(TrackFrame *frame) {
    frame->frame_id                      = frame_count_;
    *frames_[frame_count_ % capability_] = *frame;
    ++frame_count_;
  }

  inline TrackFrame *get_frame(int index) const {
    assert(index <= frame_count_);
    if (index < 0) {
      return frames_[(index + frame_count_) % capability_];
    } else {
      return frames_[index % capability_];
    }
  }

  inline TrackFrame *operator[](int index) const {
    return get_frame(index);
  }

 private:
  int frame_count_ = 0;
  int capability_  = 0;
  std::vector<TrackFrame *> frames_;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
