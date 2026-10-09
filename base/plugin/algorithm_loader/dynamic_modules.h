#pragma once
#include <vector>

#include "base/plugin/registerer.h"
/*
模块包 单独文件夹,包含程序so,程序内部使用的模型参数,airos模块动态加载参数

airos模块动态加载参数包含:
动态库名
模块类型名(基类名字)
模块名(派生类名,唯一标识,不能和其他模块冲突)
模块创建函数名(唯一,不能和其他模块冲突),返回void *
airos模块动态加载参数文件固定名字 dynamic_module.cfg
*/
namespace airos {
namespace base {
class DynamicModulesManager {
 public:
  bool Init(const std::string &modules_path);
  static DynamicModulesManager &GetInstance();

 private:
  DynamicModulesManager() = default;
  ~DynamicModulesManager();
  std::vector<void *> handles_;
};
}  // namespace base
}  // namespace airos
