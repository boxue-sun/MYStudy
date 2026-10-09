/*********************************************************************************
 * @file		participant_model.cc
 * @brief		participant_model
 * @details		participant_model
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

#include "participant_model.h"

#include "base/common/time_util.h"
#include "base/common/log.h"

namespace airos {
namespace app {

ParticipantModel::ParticipantModel() {
  path_history_ = std::make_shared<std::list<PositionModel>>();
}

void ParticipantModel::UpdatePosition(const PositionModel& pos) {
  position_ = pos;
  // 超过50个点, 删除最早添加的
  while (path_history_->size() > 50) {
    path_history_->pop_front();
  }
  path_history_->push_back(position_);
  update_time_ = airos::base::TimeUtil::GetCurrentTime();
}
}  // namespace app
}  // namespace airos
