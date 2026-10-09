#pragma once

#include "air_service/framework/perception-lidar/interface/base_lidar_perception.h"

namespace airos {
namespace perception {
namespace lidar {

class ExamplePerceptionLidar : public BaseLidarPerception {
 public:
  ExamplePerceptionLidar()  = default;
  ~ExamplePerceptionLidar() = default;
  bool Init(const LidarPerceptionParam &param) override;
  bool Perception(
      const LidarPonitCloud &lidar_data, PerceptionObstacles *result) override;
  std::string Name() const override {
    return "ExamplePerceptionLidar";
  }
};

REGISTER_LIDAR_PERCEPTION(ExamplePerceptionLidar);

}  // namespace lidar
}  // namespace perception
}  // namespace airos
