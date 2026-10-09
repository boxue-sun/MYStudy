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

#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/common.h"

namespace airos {
namespace perception {
namespace msf {
namespace base {

// @brief internal object type used by visual perception
enum class VisualObjectType {
  CAR,
  VAN,
  BUS,
  TRUCK,
  BICYCLE,
  TRICYCLE,
  PEDESTRIAN,
  TRAFFICCONE,
  SAFETY_TRIANGLE,
  UNKNOWN_MOVABLE,
  UNKNOWN_UNMOVABLE,
  MAX_OBJECT_TYPE
};

// @brief motion state
enum class MotionState { UNKNOWN = 0, MOVING = 1, STATIONARY = 2 };

/**
 * ObjectType mapping
 */
const std::map<ObjectType, std::string> kObjectType2NameMap = {
    {ObjectType::UNKNOWN, "UNKNOWN"},
    {ObjectType::UNKNOWN_MOVABLE, "UNKNOWN_MOVABLE"},
    {ObjectType::UNKNOWN_UNMOVABLE, "UNKNOWN_UNMOVABLE"},
    {ObjectType::PEDESTRIAN, "PEDESTRIAN"},
    {ObjectType::BICYCLE, "BICYCLE"},
    {ObjectType::VEHICLE, "VEHICLE"},
    {ObjectType::MAX_OBJECT_TYPE, "MAX_OBJECT_TYPE"}};

const std::map<std::string, ObjectType> kObjectName2TypeMap = {
    {"UNKNOWN", ObjectType::UNKNOWN},
    {"UNKNOWN_MOVABLE", ObjectType::UNKNOWN_MOVABLE},
    {"UNKNOWN_UNMOVABLE", ObjectType::UNKNOWN_UNMOVABLE},
    {"PEDESTRIAN", ObjectType::PEDESTRIAN},
    {"BICYCLE", ObjectType::BICYCLE},
    {"VEHICLE", ObjectType::VEHICLE},
    {"MAX_OBJECT_TYPE", ObjectType::MAX_OBJECT_TYPE}};

/**
 * VisualObjectType mapping
 */
const std::map<VisualObjectType, ObjectType> kVisualTypeMap = {
    {VisualObjectType::CAR, ObjectType::VEHICLE},
    {VisualObjectType::VAN, ObjectType::VEHICLE},
    {VisualObjectType::BUS, ObjectType::VEHICLE},
    {VisualObjectType::TRUCK, ObjectType::VEHICLE},
    {VisualObjectType::BICYCLE, ObjectType::BICYCLE},
    {VisualObjectType::TRICYCLE, ObjectType::BICYCLE},
    {VisualObjectType::PEDESTRIAN, ObjectType::PEDESTRIAN},
    {VisualObjectType::TRAFFICCONE, ObjectType::UNKNOWN_UNMOVABLE},
    {VisualObjectType::SAFETY_TRIANGLE, ObjectType::UNKNOWN_UNMOVABLE},
    {VisualObjectType::UNKNOWN_MOVABLE, ObjectType::UNKNOWN_MOVABLE},
    {VisualObjectType::UNKNOWN_UNMOVABLE, ObjectType::UNKNOWN_UNMOVABLE},
    {VisualObjectType::MAX_OBJECT_TYPE, ObjectType::MAX_OBJECT_TYPE},
};

const std::map<VisualObjectType, std::string> kVisualType2NameMap = {
    {VisualObjectType::CAR, "CAR"},
    {VisualObjectType::VAN, "VAN"},
    {VisualObjectType::BUS, "BUS"},
    {VisualObjectType::TRUCK, "TRUCK"},
    {VisualObjectType::BICYCLE, "BICYCLE"},
    {VisualObjectType::TRICYCLE, "TRICYCLE"},
    {VisualObjectType::PEDESTRIAN, "PEDESTRIAN"},
    {VisualObjectType::TRAFFICCONE, "TRAFFICCONE"},
    {VisualObjectType::SAFETY_TRIANGLE, "SAFETY_TRIANGLE"},
    {VisualObjectType::UNKNOWN_MOVABLE, "UNKNOWN_MOVABLE"},
    {VisualObjectType::UNKNOWN_UNMOVABLE, "UNKNOWN_UNMOVABLE"},
    {VisualObjectType::MAX_OBJECT_TYPE, "MAX_OBJECT_TYPE"},
};

const std::map<std::string, base::VisualObjectType> kVisualName2TypeMap = {
    {"CAR", VisualObjectType::CAR},
    {"VAN", VisualObjectType::VAN},
    {"BUS", VisualObjectType::BUS},
    {"TRUCK", VisualObjectType::TRUCK},
    {"BICYCLE", VisualObjectType::BICYCLE},
    {"TRICYCLE", VisualObjectType::TRICYCLE},
    {"PEDESTRIAN", VisualObjectType::PEDESTRIAN},
    {"TRAFFICCONE", VisualObjectType::TRAFFICCONE},
    {"SAFETY_TRIANGLE", VisualObjectType::SAFETY_TRIANGLE},
    {"UNKNOWN_MOVABLE", VisualObjectType::UNKNOWN_MOVABLE},
    {"UNKNOWN_UNMOVABLE", VisualObjectType::UNKNOWN_UNMOVABLE},
    {"MAX_OBJECT_TYPE", VisualObjectType::MAX_OBJECT_TYPE},
};

/**
 * ObjectSubType mapping
 */
const std::map<ObjectSubType, ObjectType> kSubType2TypeMap = {
    {ObjectSubType::UNKNOWN, ObjectType::UNKNOWN},
    {ObjectSubType::UNKNOWN_MOVABLE, ObjectType::UNKNOWN_MOVABLE},
    {ObjectSubType::UNKNOWN_UNMOVABLE, ObjectType::UNKNOWN_UNMOVABLE},
    {ObjectSubType::CAR, ObjectType::VEHICLE},
    {ObjectSubType::VAN, ObjectType::VEHICLE},
    {ObjectSubType::TRUCK, ObjectType::VEHICLE},
    {ObjectSubType::BUS, ObjectType::VEHICLE},
    {ObjectSubType::CYCLIST, ObjectType::BICYCLE},
    {ObjectSubType::MOTORCYCLIST, ObjectType::BICYCLE},
    {ObjectSubType::TRICYCLIST, ObjectType::BICYCLE},
    {ObjectSubType::PEDESTRIAN, ObjectType::PEDESTRIAN},
    {ObjectSubType::TRAFFICCONE, ObjectType::UNKNOWN_UNMOVABLE},
    {ObjectSubType::SAFETY_TRIANGLE, ObjectType::UNKNOWN_UNMOVABLE},
    {ObjectSubType::MAX_OBJECT_TYPE, ObjectType::MAX_OBJECT_TYPE},
};

const std::map<ObjectSubType, std::string> kSubType2NameMap = {
    {ObjectSubType::UNKNOWN, "UNKNOWN"},
    {ObjectSubType::UNKNOWN_MOVABLE, "UNKNOWN_MOVABLE"},
    {ObjectSubType::UNKNOWN_UNMOVABLE, "UNKNOWN_UNMOVABLE"},
    {ObjectSubType::CAR, "CAR"},
    {ObjectSubType::VAN, "VAN"},
    {ObjectSubType::TRUCK, "TRUCK"},
    {ObjectSubType::BUS, "BUS"},
    {ObjectSubType::CYCLIST, "CYCLIST"},
    {ObjectSubType::MOTORCYCLIST, "MOTORCYCLIST"},
    {ObjectSubType::TRICYCLIST, "TRICYCLIST"},
    {ObjectSubType::PEDESTRIAN, "PEDESTRIAN"},
    {ObjectSubType::TRAFFICCONE, "TRAFFICCONE"},
    {ObjectSubType::SAFETY_TRIANGLE, "SAFETY_TRIANGLE"},
    {ObjectSubType::MAX_OBJECT_TYPE, "MAX_OBJECT_TYPE"},
};

const std::map<std::string, ObjectSubType> kName2SubTypeMap = {
    {"UNKNOWN", ObjectSubType::UNKNOWN},
    {"UNKNOWN_MOVABLE", ObjectSubType::UNKNOWN_MOVABLE},
    {"UNKNOWN_UNMOVABLE", ObjectSubType::UNKNOWN_UNMOVABLE},
    {"CAR", ObjectSubType::CAR},
    {"VAN", ObjectSubType::VAN},
    {"TRUCK", ObjectSubType::TRUCK},
    {"BUS", ObjectSubType::BUS},
    {"CYCLIST", ObjectSubType::CYCLIST},
    {"MOTORCYCLIST", ObjectSubType::MOTORCYCLIST},
    {"TRICYCLIST", ObjectSubType::TRICYCLIST},
    {"PEDESTRIAN", ObjectSubType::PEDESTRIAN},
    {"TRAFFICCONE", ObjectSubType::TRAFFICCONE},
    {"SAFETY_TRIANGLE", ObjectSubType::SAFETY_TRIANGLE},
    {"MAX_OBJECT_TYPE", ObjectSubType::MAX_OBJECT_TYPE},
};

}  // namespace base
}  // namespace msf
}  // namespace perception
}  // namespace airos
