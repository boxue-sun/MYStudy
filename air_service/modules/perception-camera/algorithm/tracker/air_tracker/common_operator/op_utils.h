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

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

#include <Eigen/Core>
#include <Eigen/StdVector>

namespace airos {
namespace perception {
namespace algorithm {

const float INFTY_COST = 1e5;

typedef Eigen::Matrix<float, 1, 4, Eigen::RowMajor> DETECTBOX;
typedef Eigen::Matrix<float, -1, 4, Eigen::RowMajor> DETECTBOXSS;

// Kalmanfilter
typedef Eigen::Matrix<float, 1, 8, Eigen::RowMajor> KAL_MEAN;
typedef Eigen::Matrix<float, 8, 8, Eigen::RowMajor> KAL_COVA;
typedef Eigen::Matrix<float, 1, 4, Eigen::RowMajor> KAL_HMEAN;
typedef Eigen::Matrix<float, 4, 4, Eigen::RowMajor> KAL_HCOVA;

using KAL_DATA  = std::pair<KAL_MEAN, KAL_COVA>;
using KAL_HDATA = std::pair<KAL_HMEAN, KAL_HCOVA>;

// main
using RESULT_DATA = std::pair<int, DETECTBOX>;

// tracker:
using MATCH_DATA = std::pair<int, int>;
struct TRACKER_MATCHED {
  std::vector<MATCH_DATA> matches;
  std::vector<float> confs;
  std::vector<int> unmatched_tracks;
  std::vector<int> unmatched_detections;
};

// linear_assignment:
typedef Eigen::Matrix<float, -1, -1, Eigen::RowMajor> DYNAMICM;

// detection_row
enum DETECTBOX_IDX { IDX_X = 0, IDX_Y, IDX_W, IDX_H };

class DETECTION_ROW {
 public:
  DETECTBOX tlwh;
  float confidence;
  DETECTBOX to_xyah() const {
    // (centerx, centery, ration, h)
    DETECTBOX ret = tlwh;
    ret(0, IDX_X) += (ret(0, IDX_W) * 0.5);
    ret(0, IDX_Y) += (ret(0, IDX_H) * 0.5);
    ret(0, IDX_W) /= ret(0, IDX_H);
    return ret;
  }

  DETECTBOX to_tlbr() const {
    // not yet used
    // (xmin, ymin, xmax, ymax)
    DETECTBOX ret = tlwh;
    ret(0, IDX_W) += ret(0, IDX_X);
    ret(0, IDX_H) += ret(0, IDX_Y);
    return ret;
  }
};

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
