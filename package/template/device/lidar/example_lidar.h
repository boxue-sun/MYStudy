#pragma once

#include "base/device_connect/lidar/device_base.h"

namespace os {
namespace v2x {
namespace device {

class ExampleLidar : public LidarDevice {
 public:
  ExampleLidar(const LidarCallBack& cb)
      : LidarDevice(cb) {}
  virtual ~ExampleLidar() = default;
  bool Init(const LidarInitConfig& config) override;
  LidarDeviceState GetState() override {
    return LidarDeviceState::NORMAL;
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
