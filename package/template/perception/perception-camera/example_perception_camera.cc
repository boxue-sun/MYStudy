#include "example_perception_camera.h"

namespace airos {
namespace perception {
namespace camera {

bool ExamplePerceptionCamera::Init(const CameraPerceptionParam &param) {
  std::cout << "sdk_path: " << param.sdk_path << std::endl;
  std::cout << "conf_path: " << param.conf_path << std::endl;
  /*
    init param
  */
  return true;
}

bool ExamplePerceptionCamera::Perception(
    const base::device::CameraImageData &camera_data,
    PerceptionObstacles *result, TrafficLightDetection *traffic_light) {
  if (result == nullptr) return false;
  /*
    perception algorithm
  */
  result->set_error_code(PerceptionErrorCode::ERROR_NONE);
  return true;
}

}  // namespace camera
}  // namespace perception
}  // namespace airos
