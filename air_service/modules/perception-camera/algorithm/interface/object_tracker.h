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
#include <memory>
#include <string>
#include <vector>

#include "air_service/modules/perception-camera/algorithm/interface/object_track_info.h"
#include "base/plugin/registerer.h"
/** 输入: 序列图, 前后之前有依赖关系;每路视频需要创建一个单独接口
 ** 输出: 跟踪后的目标，增加track_id等字段。
 ** 注意: 第一次调用没有结果输出, 两帧之间时间间隔不能大于200ms
 */
namespace airos {
namespace perception {
namespace algorithm {

class BaseObjectTracker {
 public:
  struct InitParam {
    std::string id;  // 透传字段，id标识接口
    std::string model_dir;
    int width  = 1920;
    int height = 1080;
  };

 public:
  virtual ~BaseObjectTracker()                   = default;
  virtual std::string ID() const                 = 0;
  virtual bool Init(const InitParam &init_param) = 0;
  // 0 success; otherwise fail
  virtual int Process(
      double timestamp, const std::vector<ObjectDetectInfoPtr> &detect_result,
      std::vector<ObjectTrackInfoPtr> &object_result) = 0;
};

PERCEPTION_REGISTER_REGISTERER(BaseObjectTracker);
#define REGISTER_OBSTACLE_TRACKER(name) \
  PERCEPTION_REGISTER_CLASS(BaseObjectTracker, name)

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
