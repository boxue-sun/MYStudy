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

namespace airos {
namespace perception {
namespace algorithm {

enum class SensorType {
  UNKNOWN_SENSOR_TYPE = -1,
  VELODYNE_64         = 0,
  VELODYNE_32         = 1,
  VELODYNE_16         = 2,
  LDLIDAR_4           = 3,
  LDLIDAR_1           = 4,
  SHORT_RANGE_RADAR   = 5,
  LONG_RANGE_RADAR    = 6,
  MONOCULAR_CAMERA    = 7,
  STEREO_CAMERA       = 8,
  ULTRASONIC          = 9,
  FISHEYE_CAMERA      = 10,
  VELODYNE_128        = 11,
  TRACKER             = 12,
  LOCALIZATION        = 13,
  MAX_SENSOR_TYPE     = 14
};

enum class ObjectMajorType {
  UNKNOWN           = 0,
  UNKNOWN_MOVABLE   = 1,
  UNKNOWN_UNMOVABLE = 2,
  PEDESTRIAN        = 3,
  BICYCLE           = 4,
  VEHICLE           = 5,
  MAX_OBJECT_TYPE   = 6,
};

enum class ObjectType {
  UNKNOWN            = 0,
  UNKNOWN_MOVABLE    = 1,
  UNKNOWN_UNMOVABLE  = 2,
  CAR                = 3,
  VAN                = 4,
  TRUCK              = 5,
  BUS                = 6,
  CYCLIST            = 7,
  MOTORCYCLIST       = 8,
  TRICYCLIST         = 9,
  PEDESTRIAN         = 10,
  TRAFFICCONE        = 11,
  SAFETY_TRIANGLE    = 12,
  BARRIER_DELINEATOR = 13,
  BARRIER_WATER      = 14,
  MAX_OBJECT_TYPE    = 15,
};

const std::map<ObjectType, ObjectMajorType> ObjectTypeMapMajorType = {
    {ObjectType::CAR, ObjectMajorType::VEHICLE},
    {ObjectType::VAN, ObjectMajorType::VEHICLE},
    {ObjectType::BUS, ObjectMajorType::VEHICLE},
    {ObjectType::TRUCK, ObjectMajorType::VEHICLE},

    {ObjectType::CYCLIST, ObjectMajorType::BICYCLE},
    {ObjectType::MOTORCYCLIST, ObjectMajorType::BICYCLE},
    {ObjectType::TRICYCLIST, ObjectMajorType::BICYCLE},

    {ObjectType::PEDESTRIAN, ObjectMajorType::PEDESTRIAN},

    {ObjectType::TRAFFICCONE, ObjectMajorType::UNKNOWN_UNMOVABLE},
    {ObjectType::UNKNOWN_MOVABLE, ObjectMajorType::UNKNOWN_MOVABLE},
    {ObjectType::UNKNOWN_UNMOVABLE, ObjectMajorType::UNKNOWN_UNMOVABLE},
    {ObjectType::SAFETY_TRIANGLE, ObjectMajorType::UNKNOWN_UNMOVABLE},
    {ObjectType::BARRIER_DELINEATOR, ObjectMajorType::UNKNOWN_UNMOVABLE},
    {ObjectType::BARRIER_WATER, ObjectMajorType::UNKNOWN_UNMOVABLE},

    {ObjectType::MAX_OBJECT_TYPE, ObjectMajorType::MAX_OBJECT_TYPE},
};

struct Point2f {
  float x = 0;
  float y = 0;
};

struct Point3f {
  float x = 0;
  float y = 0;
  float z = 0;
};

struct Box2f {
  float Width() const {
    return right_bottom.x - left_top.x;
  }
  float Height() const {
    return right_bottom.y - left_top.y;
  }

  Point2f left_top;
  Point2f right_bottom;
};

struct SizeShape {
  float length = 0;
  float width  = 0;
  float height = 0;
};

struct CubeBox2f {
  Point2f pts8[8];
};

enum class TriStatus {
  UNKNOWN = 0,
  TRUE    = 1,
  FALSE   = 2,
};

struct SegGroundPlaneParams {
  double range_max_x = 0.0;
  double range_max_z = 0.0;
  double range_min_x = 0.0;
  double range_min_z = 0.0;
  double seg_size    = 0.1;
  double seg_radius  = 5.0;
  int min_cal_points = 100;
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
