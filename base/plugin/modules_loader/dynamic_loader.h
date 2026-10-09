#pragma once
#include <memory>
#include <string>
#include <vector>

#include "library-holder.h"

namespace airos {
namespace base {

class DynamicLoader {
 public:
  bool LoadDevicePackage(const std::string &device_lib_path);
  bool LoadAppPackage(const std::string &app_lib_path);
  static DynamicLoader &GetInstance();

 private:
  DynamicLoader() = default;
  ~DynamicLoader();
  std::vector<std::shared_ptr<LibraryHolder>> holder_list_;
};

}  // namespace base
}  // namespace airos
