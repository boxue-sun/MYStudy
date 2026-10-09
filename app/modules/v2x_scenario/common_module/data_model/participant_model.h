/*********************************************************************************
 * @file		position_model.h
 * @brief		position_model
 * @details		position_model
 * @author		ChangXuhui
 * @date		2024/4/1
 * @copyright	Copyright (c) 2024 Cictci V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024/4/1  ChangXuhui      1.0            ————                 ———— 
 *                                                             
 * @endverbatim
 ********************************************************************************/
#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <string>

#include "app/modules/v2x_scenario/common_module/data_model/position_model.h"

namespace airos {
namespace app {

struct ParticipantSize {
  double length;  // in meters.
  double width;   // in meters.
  double height;  // in meters.
};

enum ParticipantType {
  unknown = 0,
  motor = 1,
  non_motor = 2,
  pedestrian = 3,
  rsu = 4
};

class ParticipantModel {
 public:
  ParticipantModel();
  /**
   * @brief 获取参与者的尺寸数据
   * @return ParticipantSize
   */
  ParticipantSize GetParticipantSize() const { return participant_size_; }
  /**
   * @brief 获取参与者的类型
   * @return ParticipantType
   */
  ParticipantType GetParticipantType() const { return participant_type_; }
  /**
   * @brief 获取当前位置数据
   * @return PositionModel
   */
  PositionModel GetPosition() const { return position_; }
  /**
   * @brief 获取参与者历史轨迹点
   * @return std::shared_ptr<std::list<PositionModel>>
   */
  std::shared_ptr<std::list<PositionModel>> GetPathHistory() const {
    return path_history_;
  }
  /**
   * @brief 获取参与者的ID
   * @return std::string
   */
  std::string GetParticipantId() const { return participant_id_; }
  /**
   * @brief 获取参与者的更新时间
   * @return double 单位s
   */
  double GeUpdateTime() const { return update_time_; }

  /**
   * @brief 更新位置数据
   * @param pos 
   */
  void UpdatePosition(const PositionModel& pos);

  /**
   * @brief 设置参与者的ID
   */
  void SetParticipantId(const std::string& id) { participant_id_ = id; }
  /**
   * @brief 设置参与者的类型
   */
  void SetParticipantType(const ParticipantType& type) { participant_type_ = type; }
  /**
   * @brief 设置参与者的尺寸
   * @param type 
   */
  void SetParticipantSize(const ParticipantSize& size) {
    participant_size_ = size;
  }

 private:
  std::shared_ptr<std::list<PositionModel>> path_history_;  // 历史的轨迹点
  PositionModel position_;                                  // 当前的位置
  ParticipantSize participant_size_;  // 参与者的尺寸
  ParticipantType participant_type_;  // 参与者类型
  std::string participant_id_;        // 参与者Id
  double update_time_;                // 参与者的更新时间，s
};
}  // namespace app
}  // namespace airos
