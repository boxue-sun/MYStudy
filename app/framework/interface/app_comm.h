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
namespace app {

enum class OSMessageType {
  TRAFFIC_LIGHT_SERVICE         = 0,
  PERCEPTION_OBSTACLES_SERVICE  = 1,
  VISUAL_TRAFFIC_LIGHT          = 2,
  RSU_UPSTREAM                  = 3,
  RSU_DOWNSTREAM                = 4,
  V2X_MESSAGE_RECEIVED          = 5,
  V2X_MESSAGE_SEND              = 6,
  MEC_DATA                      = 7,
  PERCEPTION_OBSTACLES_CAMERA1  = 11,
  PERCEPTION_OBSTACLES_CAMERA2  = 12,
  PERCEPTION_OBSTACLES_CAMERA3  = 13,
  PERCEPTION_OBSTACLES_CAMERA4  = 14,
  PERCEPTION_OBSTACLES_CAMERA5  = 15,
  PERCEPTION_OBSTACLES_CAMERA6  = 16,
  PERCEPTION_OBSTACLES_CAMERA7  = 17,
  PERCEPTION_OBSTACLES_CAMERA8  = 18,
  PERCEPTION_OBSTACLES_CAMERA9  = 19,
  PERCEPTION_OBSTACLES_CAMERA10 = 20,
  PERCEPTION_OBSTACLES_CAMERA11 = 21,
  PERCEPTION_OBSTACLES_CAMERA12 = 22,
  PERCEPTION_OBSTACLES_CAMERA13 = 23,
  PERCEPTION_OBSTACLES_CAMERA14 = 24,
  PERCEPTION_OBSTACLES_CAMERA15 = 25,
  PERCEPTION_OBSTACLES_CAMERA16 = 26,
  PERCEPTION_OBSTACLES_CAMERA17 = 27,
  PERCEPTION_OBSTACLES_CAMERA18 = 28,
  PERCEPTION_OBSTACLES_CAMERA19 = 29,
  PERCEPTION_OBSTACLES_CAMERA20 = 30,
  PERCEPTION_OBSTACLES_LIDAR1   = 51,
  PERCEPTION_OBSTACLES_LIDAR2   = 52,
  PERCEPTION_OBSTACLES_LIDAR3   = 53,
  PERCEPTION_OBSTACLES_LIDAR4   = 54,
};

}  // end of namespace app
}  // end of namespace airos
