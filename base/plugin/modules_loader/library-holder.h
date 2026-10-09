#pragma once
#include <dlfcn.h>

#include <memory>
#include <vector>

#include "base/common/log.h"

namespace airos {
namespace base {

class LibraryHolder {
 private:
  const std::string _m_main_file;
  Lmid_t _m_lmid;
  /**
   * 所有库与依赖库的存放处.
   * 特别注意,主库存放在最后一项
   */
  std::vector<std::shared_ptr<void>> _m_holders;

  bool Load(const std::string& file);

 public:
  /**
   * @brief 实例化 Library Holder
   *
   *
   * @param file 主库文件
   * @param deps 主库的依赖文件
   * @param new_library_namespace 是否需要使用单独的 Library Namespace
   *
   * @note 当主库文件加载成功时,剩余未加载的依赖库将不会再尝试加载
   * @note 找库逻辑遵从 dlopen(), 即,不会自动从当前路径寻找库
   *
   * @see dlopen(3)
   */
  explicit LibraryHolder(
      std::string file, std::vector<std::string> deps = {},
      bool new_library_namespace = false);
  virtual ~LibraryHolder();
  operator bool() const;  // NOLINT(google-explicit-constructor)

 public:  // DELETED
  LibraryHolder(LibraryHolder&)             = delete;
  LibraryHolder& operator=(LibraryHolder&)  = delete;
  LibraryHolder(LibraryHolder&&)            = default;
  LibraryHolder& operator=(LibraryHolder&&) = default;
};

}  // namespace base
}  // namespace airos
