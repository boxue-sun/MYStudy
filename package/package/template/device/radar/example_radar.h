#pragma once

#include "base/device_connect/radar/device_base.h"

namespace os {
namespace v2x {
namespace device {

class ExampleRadar : public RadarDevice {
 public:
  ExampleRadar(const RadarCallBack& cb)
      : RadarDevice(cb) {}
  virtual ~ExampleRadar() = default;
  bool Init(const std::string& config_file) override;
  void Start() override;
  RadarDeviceState GetState() override {
    return RadarDeviceState::UNKNOWN;
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
