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

#include "air_service/modules/perception-camera/algorithm/interface/algorithm_base.h"

namespace airos {
namespace perception {
namespace algorithm {

static const ObjectType typeID2Type[] = {
    ObjectType::CAR,
    ObjectType::TRUCK,
    ObjectType::VAN,
    ObjectType::BUS,
    ObjectType::PEDESTRIAN,
    ObjectType::CYCLIST,
    ObjectType::TRICYCLIST,
    ObjectType::MOTORCYCLIST,
    ObjectType::UNKNOWN,
    ObjectType::TRAFFICCONE,
    ObjectType::UNKNOWN_UNMOVABLE,
    ObjectType::UNKNOWN_MOVABLE,
    ObjectType::SAFETY_TRIANGLE,
    ObjectType::BARRIER_DELINEATOR,
    ObjectType::BARRIER_WATER,
};

inline ObjectType DetectObjectType(int type_id) {
  if (type_id < 0 || type_id >= 15) {
    return ObjectType::MAX_OBJECT_TYPE;
  }
  return typeID2Type[type_id];
}

struct ObjectDetectInfo {
  // common attr
  int type_id              = -1;  // 物体类别, 对应 category_list列表
  ObjectType type          = ObjectType::UNKNOWN;
  float type_id_confidence = 0;       // @typeID 置信度
  std::vector<float> sub_type_probs;  // @brief probability for each sub-type,
                                      // optional， 所有类型的置信度

  Box2f box;  // 2d box

  SizeShape size;  // 长宽高
  // CubeBox2f pts8;    // 8点
  Point2f bottom_uv;  // 1点

  float alpha           = 0;  // 朝向
  float direction_alpha = 0;  // 朝向360
  // int heading = 0;           //车头车尾， 1 车头 2车尾

  TriStatus is_truncated = TriStatus::UNKNOWN;  // 是否截断（具体见标注文档）
  TriStatus is_occluded = TriStatus::UNKNOWN;  // 是否遮挡（具体见标注文档）
  int caseflag = 0;  // 属于1个点不可见还是2个点不可见的case
  // std::vector<float> object_feature; //每个object对应的feature，用于跟踪匹配
};

using ObjectDetectInfoPtr = std::shared_ptr<ObjectDetectInfo>;

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
