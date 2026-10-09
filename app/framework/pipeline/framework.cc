#include "app/framework/proto/airos_app_config.pb.h"

#include "app/framework/pipeline/app_loader.h"
#include "app/framework/pipeline/send_controller.h"
#include "base/io/protobuf_util.h"
#include "middleware/runtime/src/air_middleware_node.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"

int main(int argc, char* argv[]) {
  std::string app_path = "app/lib";
  if (argc < 2) {
    // ./bin/framework conf/app/airos_v2x_app.pb
    std::cout << "usage: ./bin/framework <conf path>" << std::endl;
    return -1;
  }

#if ENABLE_ENCRYPTION
  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  APP_LOG_INFO << "License is: " << license << std::endl;
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    APP_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif

  airos::app::AppliactionConfig cfg;
  if (airos::base::ParseProtobufFromFile<airos::app::AppliactionConfig>(
          std::string(argv[1]), &cfg) == false) {
    APP_LOG_WARN << argv[1] << " parse failed!";
    return false;
  }

  airos::middleware::AirRuntimeInit(argv[0]);

  auto send_ctrl = std::make_shared<::airos::app::SendController>();
  auto cb        = std::bind(
      &airos::app::SendController::AppSendCallback, send_ctrl,
      std::placeholders::_1);
  auto loader = std::make_shared<::airos::app::AppLoader>();
  if (!loader->LoadAppliaction(app_path, cfg, cb)) {
    std::cout << "app loading failed" << std::endl;
    return -1;
  }
  pause();
  return 0;
}
