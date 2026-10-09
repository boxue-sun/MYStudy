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

#include "air_service/modules/perception-fusion/algorithm/air_fusion/fusion/fusion.h"

#include <cmath>
#include <utility>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace airos {
namespace perception {
namespace msf {

Fusion::Fusion() {}

bool Fusion::Init(airos::perception::msf::FusionParams &params) {
  fusion_params_       = params;
  score_params_        = fusion_params_.score_params();
  m_matched_dis_limit_ = sqrt(score_params_.chi_square_critical());
  return true;
}

bool Fusion::Proc(
    const std::vector<std::vector<base::Object>> &input_objectlists,
    double timestamp) {
  fusion_result_.clear();
  updated_objects_.clear();
  for (unsigned int i = 0; i < input_objectlists.size(); ++i) {
    CombineNewResource(input_objectlists[i]);
  }
  return true;
}

bool Fusion::CombineNewResource(const std::vector<base::Object> &new_objects) {
  return CombineNewResource(new_objects, &updated_objects_, &fusion_result_);
}

bool Fusion::CombineNewResource(
    const std::vector<base::Object> &new_objects,
    std::vector<base::Object> *fused_objects,
    std::vector<std::vector<base::Object>> *fusion_result) {
  if (new_objects.empty()) {
    return false;
  }
  if (fused_objects->size() < 1) {
    fused_objects->assign(new_objects.begin(), new_objects.end());
    for (unsigned int j = 0; j < new_objects.size(); ++j) {
      std::vector<base::Object> matched_objects;
      matched_objects.push_back(new_objects[j]);
      fusion_result->push_back(matched_objects);
    }
    return true;
  }
  int u_num = fused_objects->size();
  int v_num = new_objects.size();
  Eigen::MatrixXf association_mat(u_num, v_num);
  ComputeAssociateMatrix(*fused_objects, new_objects, &association_mat);
  std::vector<std::pair<int, int>> match_cps;
  if (u_num > v_num) {
    km_matcher_.GetKMResult(association_mat.transpose(), &match_cps, true);
  } else {
    km_matcher_.GetKMResult(association_mat, &match_cps, false);
  }
  for (auto it = match_cps.begin(); it != match_cps.end(); it++) {
    if (it->second != -1) {
      if (it->first == -1) {
        fused_objects->push_back(new_objects[it->second]);
        std::vector<base::Object> matched_objects;
        matched_objects.push_back(fused_objects->back());
        fusion_result->push_back(matched_objects);
      } else {
        (*fusion_result)[it->first].push_back(new_objects[it->second]);
      }
    }
  }
  return true;
}

base::Info3d Fusion::ComputeFusionPos(
    const base::Object &in1_ptr, const base::Object &in2_ptr) {
  base::Info3d pos;
  ComputeFusionPos(in1_ptr, in2_ptr, &pos);
  return pos;
}

void Fusion::ComputeFusionPos(
    const base::Object &in1_ptr, const base::Object &in2_ptr,
    base::Info3d *pos) {
  // 融合位置及位置协方差
  Eigen::Vector3d u;
  Eigen::Matrix3d sigma;
  u.setZero();
  sigma.setZero();
  const auto &covariance_params = fusion_params_.covariance_params();
  if (covariance_params.covariance_dim() == 3) {
    Eigen::Matrix3d sigma1 = in1_ptr.position.Variance();
    Eigen::Matrix3d sigma2 = in2_ptr.position.Variance();
    Eigen::Vector3d u1     = in1_ptr.position.Value();
    Eigen::Vector3d u2     = in2_ptr.position.Value();
    sigma                  = (sigma1.inverse() + sigma2.inverse()).inverse();
    u = sigma * (sigma1.inverse() * u1 + sigma2.inverse() * u2);
  } else if (covariance_params.covariance_dim() == 2) {
    Eigen::Matrix2d sigma1   = in1_ptr.position.Variance().block(0, 0, 2, 2);
    Eigen::Matrix2d sigma2   = in2_ptr.position.Variance().block(0, 0, 2, 2);
    Eigen::Vector2d u1       = in1_ptr.position.Value().head(2);
    Eigen::Vector2d u2       = in2_ptr.position.Value().head(2);
    Eigen::Matrix2d sigma_xy = (sigma1.inverse() + sigma2.inverse()).inverse();
    sigma.block(0, 0, 2, 2)  = sigma_xy;
    Eigen::Vector2d u_xy =
        sigma_xy * (sigma1.inverse() * u1 + sigma2.inverse() * u2);
    u << u_xy(0), u_xy(1), in1_ptr.position.Value()(2);
  } else {
    LOG_ERROR << in2_ptr.frame_id << " covariance dim param error.";
  }

  // sigma.setIdentity();  //实际使用时不需要赋单位阵
  if (in1_ptr.sub_type == base::ObjectSubType::BUS &&
      in2_ptr.sub_type == base::ObjectSubType::BUS) {
    sigma = sigma * 4;
  }

  pos->Set(u, sigma);
}

double Fusion::CheckMdistance(
    const base::Object &in1_ptr, const base::Object &in2_ptr) {
  base::Info3d f_pos            = ComputeFusionPos(in1_ptr, in2_ptr);
  Eigen::Vector3d delta1        = in1_ptr.position.Value() - f_pos.Value();
  Eigen::Vector3d delta2        = in2_ptr.position.Value() - f_pos.Value();
  double dis1                   = 0.0;
  double dis2                   = 0.0;
  const auto &covariance_params = fusion_params_.covariance_params();
  if (covariance_params.covariance_dim() == 3) {
    Eigen::Matrix3d sigma1 = in1_ptr.position.Variance();
    Eigen::Matrix3d sigma2 = in2_ptr.position.Variance();
    dis1 = sqrt(delta1.transpose() * sigma1.inverse() * delta1);
    dis2 = sqrt(delta2.transpose() * sigma2.inverse() * delta2);
  } else if (covariance_params.covariance_dim() == 2) {
    Eigen::Matrix2d sigma1 = in1_ptr.position.Variance().block(0, 0, 2, 2);
    Eigen::Matrix2d sigma2 = in2_ptr.position.Variance().block(0, 0, 2, 2);
    dis1                   = std::sqrt(
        delta1.transpose().head(2) * sigma1.inverse() * delta1.head(2));
    dis2 = std::sqrt(
        delta2.transpose().head(2) * sigma2.inverse() * delta2.head(2));
  } else {
    LOG_ERROR << in2_ptr.frame_id << " covariance dim param error.";
  }
  if (dis1 <= m_matched_dis_limit_ && dis2 <= m_matched_dis_limit_) {
    return (dis1 + dis2) / 2;
  }
  return score_params_.max_match_distance() + 1;
}

double Fusion::CheckOdistance(
    const base::Object &in1_ptr, const base::Object &in2_ptr) {
  double xi       = in1_ptr.position.x();
  double yi       = in1_ptr.position.y();
  double xj       = in2_ptr.position.x();
  double yj       = in2_ptr.position.y();
  double distance = std::hypot(xi - xj, yi - yj);
  return distance;
}

bool Fusion::CheckDisScore(
    const base::Object &in1_ptr, const base::Object &in2_ptr, double &score) {
  double dis = (score_params_.use_mahalanobis_distance())
                   ? CheckMdistance(in1_ptr, in2_ptr)
                   : CheckOdistance(in1_ptr, in2_ptr);

  double max_match_distance = 10.0;
  if ((in1_ptr.type == in1_ptr.type && in1_ptr.sub_type != in2_ptr.sub_type)) {
    max_match_distance = score_params_.max_match_distance_diff_subtype();
  } else if (
      in1_ptr.sub_type == base::ObjectSubType::CAR &&
      in2_ptr.sub_type == base::ObjectSubType::CAR &&
      CheckOdistance(in1_ptr, in2_ptr) > (5.0 + 1e-6)) {
    max_match_distance = score_params_.max_match_distance_diff_subtype();
  } else {
    max_match_distance = score_params_.max_match_distance();
  }
  score = 2.5 * std::max(0.0, max_match_distance - dis);
  return true;
}

bool Fusion::CheckTypeScore(
    const base::Object &in1_ptr, const base::Object &in2_ptr, double &score) {
  if (in1_ptr.sensor_type == base::SensorType::LONG_RANGE_RADAR ||
      in2_ptr.sensor_type == base::SensorType::LONG_RANGE_RADAR) {
    return true;
  }
  double same_prob = 0;
  double tp1       = in1_ptr.type_probs[static_cast<int>(in1_ptr.type)];
  double tp2       = in2_ptr.type_probs[static_cast<int>(in2_ptr.type)];
  if (score_params_.check_subtype() ||
      (in1_ptr.type == base::ObjectType::BICYCLE ||
       in2_ptr.type == base::ObjectType::BICYCLE) ||
      (in1_ptr.type == base::ObjectType::PEDESTRIAN ||
       in2_ptr.type == base::ObjectType::PEDESTRIAN)) {
    double stp1 = in1_ptr.sub_type_probs[0];
    double stp2 = in2_ptr.sub_type_probs[0];
    if (in1_ptr.sub_type == in2_ptr.sub_type) {
      same_prob = stp1 * stp2 +
                  (1 - stp1) * (1 - stp2) / (score_params_.subtype_count() - 1);
    } else if (in1_ptr.type == in2_ptr.type) {
      same_prob = (tp1 * tp2 +
                   (1 - tp1) * (1 - tp2) / (score_params_.type_count() - 1)) *
                  (stp1 * (1 - stp2) / (score_params_.subtype_count() - 1) +
                   stp2 * (1 - stp1) / (score_params_.subtype_count() - 1));
    } else {
      same_prob = (tp1 * (1 - tp2) / (score_params_.type_count() - 1) +
                   (1 - tp1) * tp2 / (score_params_.type_count() - 1)) *
                  (stp1 * (1 - stp2) / (score_params_.subtype_count() - 1) +
                   stp2 * (1 - stp1) / (score_params_.subtype_count() - 1)) /
                  2;
    }
  } else {
    if (in1_ptr.type == in2_ptr.type) {
      same_prob =
          tp1 * tp2 + (1 - tp1) * (1 - tp2) / (score_params_.type_count() - 1);
    } else {
      same_prob = (tp1 * (1 - tp2) / (score_params_.type_count() - 1) +
                   (1 - tp1) * tp2 / (score_params_.type_count() - 1)) /
                  2;
    }
  }
  score *= same_prob;
  return true;
}

bool Fusion::ComputeAssociateMatrix(
    const std::vector<base::Object> &in1_objects,  // fused
    const std::vector<base::Object> &in2_objects,  // new
    Eigen::MatrixXf *association_mat) {
  for (unsigned int i = 0; i < in1_objects.size(); ++i) {
    for (unsigned int j = 0; j < in2_objects.size(); ++j) {
      const base::Object &obj1_ptr = in1_objects[i];
      const base::Object &obj2_ptr = in2_objects[j];
      double score                 = 0;
      if (!CheckDisScore(obj1_ptr, obj2_ptr, score)) {
        LOG_ERROR << "V2X Fusion: check dis score failed";
      }
      if (score_params_.check_type() &&
          !CheckTypeScore(obj1_ptr, obj2_ptr, score)) {
        LOG_ERROR << "V2X Fusion: check type failed";
      }
      (*association_mat)(i, j) =
          (score >= score_params_.min_score()) ? score : 0;
    }
  }
  return true;
}

int Fusion::DeleteRedundant(std::vector<base::Object> *objects) {
  std::vector<unsigned int> to_be_deleted;
  for (unsigned int i = 0; i < objects->size(); ++i) {
    for (unsigned int j = i + 1; j < objects->size(); ++j) {
      double distance = CheckOdistance(objects->at(i), objects->at(j));
      if (distance < 1) {
        to_be_deleted.push_back(j);
      }
    }
  }
  for (auto iter = to_be_deleted.rbegin(); iter != to_be_deleted.rend();
       ++iter) {
    objects->erase(objects->begin() + *iter);
  }
  return to_be_deleted.size();
}

}  // namespace msf
}  // namespace perception
}  // namespace airos
