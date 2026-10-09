#pragma once

#include "base/device_connect/camera/camera_base.h"

namespace airos {
namespace base {
namespace device {

class ExampleCamera : public CameraDevice {
 public:
  ExampleCamera(const CameraImageCallBack& cb)
      : CameraDevice(cb) {}
  virtual ~ExampleCamera() = default;
  bool Init(const CameraInitConfig& config) override;
};

}  // namespace device
}  // namespace base
}  // namespace airos
