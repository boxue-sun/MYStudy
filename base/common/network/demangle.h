/*********************************************************************************
 * @file		demangle.h
 * @brief		demangle belongs to CICTCI
 * @details
 * @author		cs
 * @date		2024-01-24
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024-01-24 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _DEMANGLE_H
#define _DEMANGLE_H
#include "define.h"
#include <cxxabi.h>
NAMESPACE_AFL_UTIL_START

// 根据重整后的名字解析出原函数原型名字
// 如果则返回true, 并将解析后的名字保存在unmangled
// Demangle "mangled".  On success, return true and write the
// demangled symbol name to "unmangled".  Otherwise, return false.
// "unmangled" is modified even if demangling is unsuccessful.
bool demangleName(const char* mangled, char* unmangled, size_t buf_size);
bool demangleName(const char* mangled, std::string& unmangled);

NAMESPACE_AFL_UTIL_END
#endif  /* _DEMANGLE_H */
