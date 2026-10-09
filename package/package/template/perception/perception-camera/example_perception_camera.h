#pragma once

#include "air_service/framework/perception-camera/interface/base_camera_perception.h"

namespace airos {
namespace perception {
namespace camera {

class ExamplePerceptionCamera : public BaseCameraPerception {
 public:
  ExamplePerceptionCamera()  = default;
  ~ExamplePerceptionCamera() = default;
  bool Init(const CameraPerceptionParam &param) override;
  bool Perception(
      const base::device::CameraImageData &camera_data,
      PerceptionObstacles *result,
      TrafficLightDetection *traffic_light) override;
  std::string Name() const override {
    return "ExamplePerceptionCamera";
  }
};

REGISTER_CAMERA_PERCEPTION(ExamplePerceptionCamera);

}  // namespace camera
}  // namespace perception
}  // namespace airos
