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

#include <deque>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/modules/v2x_application/proto/roi_lane.pb.h"

#include "app/modules/v2x_application/usecase/base/event_collector.h"
#include "app/modules/v2x_application/usecase/base/tracked_obj.h"
#include "app/modules/v2x_application/usecase/common/common.h"
#include "app/modules/v2x_application/usecase/common/timer.h"
#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace usecase {

class FrameData {
 public:
  using Event = EventCollector::EventInfo;

  virtual ~FrameData() = default;
  virtual void Init()  = 0;
  virtual void Update(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles)                                          = 0;
  virtual const std::deque<TrackObj>& Data() const            = 0;
  virtual uint64_t TimeStamp() const                          = 0;
  virtual Tensor GetCrossCenter() const                       = 0;
  virtual const std::unordered_set<int64_t>& TrackIds() const = 0;
  virtual void Collect(Event&& evt)                           = 0;
  virtual std::vector<Event> Events() const                   = 0;
  virtual std::shared_ptr<airos::usecase::EventOutputResult> Dump(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles) = 0;

 protected:
  Timer timer_;
};

class FrameTrackData : public FrameData {
 public:
  FrameTrackData()
      : FrameData()
      , collector_(base::Singleton<EventCollector>::get_instance()) {}
  void Init() override;
  void Update(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles) override;  // 障碍物匹配

  const std::deque<TrackObj>& Data() const override {
    return trackeds_;
  }

  uint64_t TimeStamp() const override {
    return timestamp_sec_;
  }

  const std::unordered_set<int64_t>& TrackIds() const override {
    return track_ids_;
  }

  Tensor GetCrossCenter() const override {
    return cross_center_;
  }

  std::vector<Event> Events() const override {
    return collector_->GetEvent();
  }

  void Collect(Event&& evt) {
    collector_->AddEvent(std::forward<Event>(evt));
  }

  // dump 透传感知障碍物和事件信息
  std::shared_ptr<airos::usecase::EventOutputResult> Dump(
      const std::shared_ptr<const airos::perception::PerceptionObstacles>&
          obstacles) override {
    timer_.Reset();
    output_proto_.reset(new airos::usecase::EventOutputResult());
    collector_->UpdateProto(
        obstacles,
        output_proto_);  // 得到感知障碍物和事件信息
    APP_LOG_INFO << "debug-> fill proto cost: " << timer_.DurationMicrosec();
    return output_proto_;
  }

 private:
  float MatchScore(const PerceptionObstacle& obstacle, const TrackObj& obj) {
    if (obstacle.type() != airos::perception::PerceptionObstacle_Type_VEHICLE ||
        obj.Type() != airos::perception::PerceptionObstacle_Type_VEHICLE) {
      return -2;
    }
    if (!obj.IsStop()) {
      return -2;
    }
    return DIoU(obstacle, obj);
  }

  float DIoU(
      const airos::perception::PerceptionObstacle& obstacle,
      const TrackObj& obj);

 private:
  std::shared_ptr<EventCollector> collector_;  // 事件的收集以及管理
  std::deque<TrackObj> trackeds_;
  std::shared_ptr<airos::usecase::EventOutputResult> output_proto_;
  std::unordered_set<int64_t> track_ids_;

  // for match
  float ignore_thresh_ = -0.7;
  float diou_thresh_   = 0.2;
  int64_t global_id_   = 0;

  // for stop
  float stop_speed_thre_;
  float stop_speed_cnt_thre_;
  float stop_shift_cnt_thre_;
  int tracked_life_;

  // //定义在proto/v2x_obstacles_cpmpress/proto
  airos::usecase::roi::LaneParam
      mini_map_params_;  // 定义在proto/v2x_obstacles_cpmpress.proto

  Tensor cross_center_;

  // for timestamp
  uint64_t timestamp_sec_;

  struct Score {
    int target;
    int object;
    float score;

    Score() {
      this->target = -1;
      this->object = -1;
      this->score  = -3;
    }

    Score(int tar, int obj, float score) {
      this->target = tar;
      this->object = obj;
      this->score  = score;
    }

    bool operator<(const Score& rhs) const {
      return score < rhs.score;
    }

    bool operator>(const Score& rhs) const {
      return score > rhs.score;
    }
  };
};

}  // end of namespace usecase
}  // end of namespace perception
}  // end of namespace airos
