#include "base/plugin/algorithm_loader/dynamic_modules.h"

#include <dlfcn.h>

#include <iostream>

#include "base/plugin/algorithm_loader/dynamic_modules_cfg.pb.h"

#include "base/common/log.h"
#include "base/io/file_util.h"
#include "base/io/protobuf_util.h"

namespace airos {
namespace base {

const char *CFG_NAME = "dynamic_module.cfg";

typedef void *(*CreatModuleFunction)();

class DynamicObjectFactory : public ObjectFactory {
 public:
  DynamicObjectFactory(CreatModuleFunction func)
      : func_(func) {}
  virtual ~DynamicObjectFactory() {}
  virtual Any NewInstance() {
    if (!func_) return Any(nullptr);
    return Any(func_());
  }

 private:
  CreatModuleFunction func_;
};

bool DynamicModulesManager::Init(const std::string &modules_path) {
  std::vector<std::string> dirs;
  FileUtil::GetDirList(modules_path, &dirs);

  for (auto &module_dir : dirs) {
    std::string module_cfg = module_dir + "/" + CFG_NAME;
    if (!FileUtil::Exists(module_cfg)) {
      LOG_WARN << "File does not exist: " << module_cfg;
      continue;
    }

    DynamicModulesCfg cfg;
    if (airos::base::ParseProtobufFromFile<DynamicModulesCfg>(
            module_cfg, &cfg) == false) {
      LOG_WARN << "File parse error: " << module_cfg;
      return false;
    }

    FactoryMap &map = GlobalFactoryMap()[cfg.module_type()];
    if (map.find(cfg.module_type()) != map.end()) return false;

    std::string module_lib = module_dir + "/" + cfg.lib_name();

    if (!FileUtil::Exists(module_lib)) {
      LOG_WARN << "Lib does not exist: " << module_lib;
      continue;
    }

    void *handle = dlopen(module_lib.c_str(), RTLD_NOW);
    if (!handle) {
      fprintf(stderr, "%s\n", dlerror());
      LOG_WARN << "Load error: " << module_lib;
      continue;
    }
    LOG_WARN << "Load module success: " << cfg.module_name();
    if (!cfg.creat_function().empty()) {
      CreatModuleFunction func = reinterpret_cast<CreatModuleFunction>(
          dlsym(handle, cfg.creat_function().c_str()));
      if (!func) {
        fprintf(stderr, "%s\n", dlerror());
        continue;
      }
      map[cfg.module_name()] = new DynamicObjectFactory(func);
    }
    handles_.push_back(handle);
  }
  return true;
}

DynamicModulesManager &DynamicModulesManager::GetInstance() {
  static DynamicModulesManager instance;
  return instance;
}

DynamicModulesManager::~DynamicModulesManager() {
  for (auto &handle : handles_) {
    dlclose(handle);
  }
}

}  // namespace base
}  // namespace airos
