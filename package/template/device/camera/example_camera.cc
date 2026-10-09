#include "example_camera.h"

#include "base/common/log.h"
#include "base/device_connect/camera/camera_factory.h"

namespace airos {
namespace base {
namespace device {

bool ExampleCamera::Init(const CameraInitConfig& config) {
  LOG_INFO << "camera ip: " << config.ip;

  auto data = std::make_shared<CameraImageData>();
  /*
    fill data
  */
  image_sender_(data);
  return true;
}

AIROS_CAMERA_REG_FACTORY(ExampleCamera, "example_camera");

}  // namespace device
}  // namespace base
}  // namespace airos
