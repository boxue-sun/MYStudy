#pragma once

#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "air_service/framework/perception-lidar/interface/base_lidar_perception.h"
#include "base/plugin/algorithm_plugin.h"

namespace airos {
namespace perception {
namespace lidar {

class AirosLidarPerception : public BaseLidarPerception {
 public:
  AirosLidarPerception()                                        = default;
  AirosLidarPerception(const AirosLidarPerception &)            = delete;
  AirosLidarPerception &operator=(const AirosLidarPerception &) = delete;
  ~AirosLidarPerception()                                       = default;

  bool Init(const LidarPerceptionParam &param);
  bool Perception(
      const LidarPonitCloud &lidar_data, PerceptionObstacles *result);
  std::string Name() const override {
    return "AirosLidarPerception";
  }
};

REGISTER_LIDAR_PERCEPTION(AirosLidarPerception);

}  // namespace lidar
}  // namespace perception
}  // namespace airos
