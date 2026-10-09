#include "airos_lidar_perception.h"

#include "base/common/time_util.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace perception {
namespace lidar {

bool AirosLidarPerception::Init(const LidarPerceptionParam &param) {
  LOG_INFO << "AirosLidarPerception::Init, path: " << param.alg_path;
  return true;
}

bool AirosLidarPerception::Perception(
    const LidarPonitCloud &lidar_data, PerceptionObstacles *result) {
  if (!result) return false;
  LOG_INFO << "Run Lidar Perception";
  return true;
}

}  // namespace lidar
}  // namespace perception
}  // namespace airos
