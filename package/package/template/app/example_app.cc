#include "example_app.h"

#include "app/framework/interface/app_factory.h"
#include "base/common/log.h"

namespace airos {
namespace app {

bool ExampleApp::Init(const AppliactionConfig& conf) {
  LOG_INFO << "Init app!";
  return true;
};

void ExampleApp::Start() {
  std::cout << "Hello World!" << std::endl;
  return;
}

AIROS_APPLICATION_REG_FACTORY(ExampleApp, "example_app")
}  // namespace app
}  // namespace airos
