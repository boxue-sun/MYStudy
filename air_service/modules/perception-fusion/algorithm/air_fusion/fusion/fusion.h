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

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <Eigen/Core>

#include "air_service/modules/perception-fusion/proto/fusion_params.pb.h"

#include "air_service/modules/perception-fusion/algorithm/air_fusion/base/object.h"
#include "air_service/modules/perception-fusion/algorithm/air_fusion/fusion/km.h"
#include "base/common/log.h"

namespace airos {
namespace perception {
namespace msf {
class Fusion {
 public:
  Fusion();
  ~Fusion() {}
  bool Proc(
      const std::vector<std::vector<base::Object>> &input_objectlists,
      double timestamp);
  bool Init(airos::perception::msf::FusionParams &param);
  std::vector<base::Object> &get_fused_objects() {
    return updated_objects_;
  }
  std::vector<std::vector<base::Object>> &get_fusion_result() {
    return fusion_result_;
  }
  bool CombineNewResource(
      const std::vector<base::Object> &new_objects,
      std::vector<base::Object> *fused_objects,
      std::vector<std::vector<base::Object>> *fusion_result);
  int DeleteRedundant(std::vector<base::Object> *objects);

 private:
  bool CombineNewResource(const std::vector<base::Object> &new_objects);
  bool ComputeAssociateMatrix(
      const std::vector<base::Object> &in1_objects,
      const std::vector<base::Object> &in2_objects,
      Eigen::MatrixXf *association_mat);
  double CheckMdistance(
      const base::Object &in1_ptr, const base::Object &in2_ptr);
  double CheckOdistance(
      const base::Object &in1_ptr, const base::Object &in2_ptr);
  bool CheckDisScore(
      const base::Object &in1_ptr, const base::Object &in2_ptr, double &score);
  bool CheckTypeScore(
      const base::Object &in1_ptr, const base::Object &in2_ptr, double &score);
  // 计算两个分布融合后的位置
  void ComputeFusionPos(
      const base::Object &in1_ptr, const base::Object &in2_ptr,
      base::Info3d *pos);
  base::Info3d ComputeFusionPos(
      const base::Object &in1_ptr, const base::Object &in2_ptr);
  bool host_vehicle_ = false;
  bool zom_vehicle_  = false;
  double last_timestamp_;
  const double MAX_SCORE = 250000;
  float m_matched_dis_limit_;
  KMkernal km_matcher_;
  airos::perception::msf::ScoreParams score_params_;
  std::vector<std::vector<base::Object>> fusion_result_;
  std::vector<base::Object> updated_objects_;
  airos::perception::msf::FusionParams fusion_params_;
};

}  // namespace msf
}  // namespace perception
}  // namespace airos
