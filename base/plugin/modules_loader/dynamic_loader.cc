#include "dynamic_loader.h"

#include "base/plugin/modules_loader/app_lib_cfg.pb.h"
#include "base/plugin/modules_loader/device_lib_cfg.pb.h"

#include "base/common/log.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace base {

const char *DEVICE_DESC_NAME = "device_lib_cfg.pb";
const char *APP_DESC_NAME    = "app_lib_cfg.pb";

bool DynamicLoader::LoadDevicePackage(const std::string &device_lib_path) {
  std::vector<std::string> dirs;
  FileUtil::GetDirList(device_lib_path, &dirs);
  for (auto &dir : dirs) {
    std::string module_cfg = dir + "/" + DEVICE_DESC_NAME;
    if (!FileUtil::Exists(module_cfg)) {
      APP_FRAMEWORK_LOG_WARN << module_cfg << " don't exist!";
      continue;
    }
    DeviceLibCfg cfg;
    if (airos::base::ParseProtobufFromFile<DeviceLibCfg>(module_cfg, &cfg) ==
        false) {
      APP_FRAMEWORK_LOG_WARN << module_cfg << " parse failed!";
      return false;
    }

    std::string module_lib = dir + "/" + cfg.so_name();
    if (!FileUtil::Exists(module_lib)) {
      APP_FRAMEWORK_LOG_WARN << module_lib << " don't exist!";
      continue;
    }
    std::string load_path(module_lib);
    std::shared_ptr<LibraryHolder> holder =
        std::make_shared<LibraryHolder>(load_path);
    if (!holder) {
      APP_FRAMEWORK_LOG_WARN << module_lib << " load failed!";
      continue;
    }
    holder_list_.push_back(holder);
    APP_FRAMEWORK_LOG_WARN << "load success:" << module_lib;
  }
  return true;
}

bool DynamicLoader::LoadAppPackage(const std::string &app_lib_path) {
  std::vector<std::string> dirs;
  FileUtil::GetDirList(app_lib_path, &dirs);
  for (auto &dir : dirs) {
    std::string module_cfg = dir + "/" + APP_DESC_NAME;
    if (!FileUtil::Exists(module_cfg)) {
      APP_FRAMEWORK_LOG_WARN << module_cfg << " don't exist!";
      continue;
    }
    AppLibCfg cfg;
    if (airos::base::ParseProtobufFromFile<AppLibCfg>(module_cfg, &cfg) ==
        false) {
      APP_FRAMEWORK_LOG_WARN << module_cfg << " parse failed!";
      return false;
    }

    std::string module_lib = dir + "/" + cfg.so_name();
    if (!FileUtil::Exists(module_lib)) {
      APP_FRAMEWORK_LOG_WARN << module_lib << " don't exist!";
      continue;
    }
    std::string load_path(module_lib);
    std::shared_ptr<LibraryHolder> holder =
        std::make_shared<LibraryHolder>(load_path);
    if (!holder) {
      APP_FRAMEWORK_LOG_WARN << module_lib << " load failed!";
      continue;
    }
    holder_list_.push_back(holder);
    APP_FRAMEWORK_LOG_WARN << "load success:" << module_lib;
  }
  return true;
}

DynamicLoader &DynamicLoader::GetInstance() {
  static DynamicLoader instance;
  return instance;
}

DynamicLoader::~DynamicLoader() {
  holder_list_.clear();
}

}  // namespace base
}  // namespace airos
