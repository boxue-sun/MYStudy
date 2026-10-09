#include "base/env/env.h"

#include <fstream>

#include "base/common/log.h"

namespace airos {
namespace base {

std::string Environment::GetDeviceSn() {
  std::string rscu_sn      = "default";
  std::string rscu_sn_file = "/etc/v2x/SN";
  std::ifstream ifs(rscu_sn_file);
  if (!ifs.is_open()) {
   APP_FRAMEWORK_LOG_WARN << "Get RSCU SN Fail." << rscu_sn_file;
    return rscu_sn;
  }
  std::string content(
      (std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>()));

 APP_FRAMEWORK_LOG_INFO << "Get RSCU SN: " << content;
  size_t n = content.find_last_not_of("\r\n\t");
  if (n != std::string::npos) {
    content.erase(n + 1, content.size() - n);
  }
  rscu_sn = std::move(content);
  return rscu_sn;
}

std::string Environment::GetPublicParamPath() {
  const char* param_dir = std::getenv("PARAM_DIR");
  if (param_dir == nullptr) {
   APP_FRAMEWORK_LOG_WARN << "env param dir is null!";
    return "";
  }
  return std::string(param_dir);
}

}  // namespace base
}  // namespace airos
