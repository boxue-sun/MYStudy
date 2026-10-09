#pragma once

#include "base/device_connect/rsu/device_base.h"

namespace os {
namespace v2x {
namespace device {

class ExampleRSU : public RSUDevice {
 public:
  ExampleRSU(const RSUCallBack& cb)
      : RSUDevice(cb) {}
  virtual ~ExampleRSU() = default;
  bool Init(const std::string& config_file) override;
  void Start() override;
  void WriteToDevice(const std::shared_ptr<const RSUData>& re_proto) override;
  RSUDeviceState GetState() override {
    return RSUDeviceState::NORMAL;
  }
};

}  // namespace device
}  // namespace v2x
}  // namespace os
