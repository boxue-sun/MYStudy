#include "example_perception_lidar.h"

namespace airos {
namespace perception {
namespace lidar {

bool ExamplePerceptionLidar::Init(const LidarPerceptionParam &param) {
  std::cout << "alg_path: " << param.alg_path << std::endl;
  std::cout << "conf_path: " << param.conf_path << std::endl;
  /*
    init param
  */
  return true;
}

bool ExamplePerceptionLidar::Perception(
    const LidarPonitCloud &lidar_data, PerceptionObstacles *result) {
  if (result == nullptr) return false;
  /*
    perception algorithm
  */
  result->set_error_code(PerceptionErrorCode::ERROR_NONE);
  return true;
}

}  // namespace lidar
}  // namespace perception
}  // namespace airos
