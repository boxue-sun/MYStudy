#pragma once

#include "app/framework/interface/app_base.h"

namespace airos {
namespace app {

class ExampleApp : public airos::app::AirOSApplication {
 public:
  ExampleApp(const ApplicationCallBack& cb)
      : AirOSApplication(cb) {}

  ~ExampleApp() = default;

  bool Init(const AppliactionConfig& conf) override;

  void Start() override;
};
}  // namespace app
}  // namespace airos
