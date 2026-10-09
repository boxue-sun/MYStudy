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

namespace airos {
namespace perception {
namespace msf {
namespace base {

enum class OcclusionState {
  OCC_UNKNOWN           = 0,
  OCC_NONE_OCCLUDED     = 1,
  OCC_PARTIAL_OCCLUDED  = 2,
  OCC_COMPLETE_OCCLUDED = 3
};

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

// @brief general object type
enum class ObjectType {
  UNKNOWN           = 0,
  UNKNOWN_MOVABLE   = 1,
  UNKNOWN_UNMOVABLE = 2,
  PEDESTRIAN        = 3,
  BICYCLE           = 4,
  VEHICLE           = 5,
  MAX_OBJECT_TYPE   = 6
};

// @brief fine-grained object types
enum class ObjectSubType {
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
  MAX_OBJECT_TYPE    = 13,
  BARRIER_DELINEATOR = 14,
  BARRIER_WATER      = 15
};

enum class Error {
  NONE           = 0,
  INIT_TIMESTAMP = 1,
  MESSAGE        = 2,
  PROCESS        = 3,
  TIMESTAMP      = 4
};

}  // namespace base
}  // namespace msf
}  // namespace perception
}  // namespace airos
