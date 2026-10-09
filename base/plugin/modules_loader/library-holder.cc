#include "base/plugin/modules_loader/library-holder.h"

namespace airos {
namespace base {

bool LibraryHolder::Load(const std::string& file) {
  auto h = dlmopen(_m_lmid, file.c_str(), RTLD_NOW | RTLD_NOLOAD);
  // Check already Loaded
  if (h) {
    VLOG(5) << "Ignore " << file << ": Loaded";
    // Attention: `dlmopen` with flag RTLD_NOLOAD cannot free!
    return true;
  }
  // Not loaded yet. Load it now
  h = dlmopen(_m_lmid, file.c_str(), RTLD_NOW);
  if (!h) {
   APP_FRAMEWORK_LOG_WARN << "Failed to load library: " << dlerror();
    return false;
  }
  // Get LMID
  if (__glibc_unlikely(_m_lmid == LM_ID_NEWLM)) {
    auto lmid = _m_lmid;
    if (0 > dlinfo(h, RTLD_DI_LMID, &lmid)) {
     APP_FRAMEWORK_LOG_FATAL << "Failed to get dl info: " << dlerror();
    }
    LOG_ASSERT(lmid != LM_ID_NEWLM);
    _m_lmid = lmid;
  }
  // Loaded
  VLOG(5) << "Loaded: " << file;
  _m_holders.emplace_back(h, dlclose);
  return true;
}

LibraryHolder::LibraryHolder(
    std::string file, std::vector<std::string> deps, bool new_library_namespace)
    : _m_main_file(std::move(file))
    , _m_lmid(new_library_namespace ? LM_ID_NEWLM : LM_ID_BASE) {
  _m_holders.reserve(deps.size() + 1);
  while (true) {
    // 主文件加载成功
    if (Load(_m_main_file)) {
      return;
    }
    // 主文件加载失败, 开始加载依赖库
    size_t failed_counter = 0;
    for (size_t i = 0; i < deps.size(); i++) {
      // 如果加载成功
      if (Load(deps[i])) {
        continue;
      }
      // 如果加载失败
      LOG_ASSERT(failed_counter <= i);
      if (__glibc_likely(failed_counter != i)) {
        deps[failed_counter] = std::move(deps[i]);
      }
      failed_counter++;
    }
    if (failed_counter == deps.size()) {
      break;
    }
    deps.resize(failed_counter);
  }
  // 主文件未加载成功,且依赖库没有能加载的了
  _m_lmid = LM_ID_NEWLM;  // 借用此值表示不可用
  std::stringstream ss;
  ss << _m_main_file;
  for (const auto& item : deps) {
    ss << ";" << item;
  }
 APP_FRAMEWORK_LOG_WARN << "Failed to load libraries: " << ss.str();
}

LibraryHolder::~LibraryHolder() {
  // Must free reverse!
  while (!_m_holders.empty()) {
    _m_holders.pop_back();
  }
}

LibraryHolder::operator bool() const {
  return _m_lmid != LM_ID_NEWLM;
}

}  // namespace base
}  // namespace airos
