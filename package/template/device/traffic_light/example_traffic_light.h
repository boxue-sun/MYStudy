#pragma once

#include "base/device_connect/traffic_light/device_base.h"

namespace os {
namespace v2x {
namespace device {

class ExampleTrafficLight : public TrafficLightDevice {
 public:
  ExampleTrafficLight(const TrafficLightCallBack& cb)
      : TrafficLightDevice(cb) {}
  virtual ~ExampleTrafficLight() = default;
  bool Init(const std::string& config_file) override;
  void Start() override;
  void WriteToDevice(
      const std::shared_ptr<const TrafficLightReceiveData>& re_proto) override;
  TrafficLightDeviceState GetState() override {
    return TrafficLightDeviceState::RUNNING;
  }
};
}  // namespace device
}  // namespace v2x
}  // namespace os
