/*********************************************************************************
 * @file		define.h
 * @brief		define belongs to CICTCI
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
#ifndef DEFINE_H
#define DEFINE_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <stdint.h>
#include <signal.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <thread>

#include <sstream>
#include <assert.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>           // printf("%"PRId64"\n", (int64_t)value);  
#undef __STDC_FORMAT_MACROS

//#include "smart_assert.h"
#define OS_LINUX
using std::string;
using std::vector;
using std::list;
using std::queue;
using std::map;
using std::set;
using std::multimap;
using std::multiset;

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus > 199711L || __cplusplus == 201103L
#define AFL_CXX11_ENABLED 1
#endif

#define NAMESPACE_AFL_START        namespace afl {
#define NAMESPACE_AFL_END          }  /* namespace afl */

#define NAMESPACE_AFL_BASE_START   NAMESPACE_AFL_START namespace base {
#define NAMESPACE_AFL_BASE_END     } NAMESPACE_AFL_END  /* namespace afl::base */

#define NAMESPACE_AFL_THREAD_START NAMESPACE_AFL_START namespace thread {
#define NAMESPACE_AFL_THREAD_END   } NAMESPACE_AFL_END  /* namespace afl::thread */

#define NAMESPACE_AFL_NET_START    NAMESPACE_AFL_START namespace net {
#define NAMESPACE_AFL_NET_END      } NAMESPACE_AFL_END  /* namespace afl::network */

#define NAMESPACE_AFL_UTIL_START   NAMESPACE_AFL_START namespace util {
#define NAMESPACE_AFL_UTIL_END     } NAMESPACE_AFL_END  /* namespace afl::util */

#define AFL_SNPRINTF  snprintf


#define AFL_UNUSED(statement)    ((void)(statement))     /** just disable some warnings of compliers */

#define SAFE_DELETE(p)        do { delete p; p = NULL; } while (0)
#define SAFE_DELETE_ARRAY(p)  do { delete[] p; p = NULL; } while (0)

#ifdef AFL_CXX11_ENABLED
#define DISALLOW_COPY_AND_ASSIGN(TypeName)            \
        TypeName(const TypeName&) = delete;           \
        TypeName& operator=(const TypeName&) = delete
#else
#define DISALLOW_COPY_AND_ASSIGN(TypeName)            \
        private:                                      \
            TypeName(const TypeName&);                \
            TypeName& operator=(const TypeName&)
#endif

#define USE_TRY_CATCH
#ifdef  USE_TRY_CATCH
#define AFL_TRY_BEGIN  try {
#define AFL_CATCH(x)   } catch (x) {
#define AFL_CATCH_ALL  } catch (...) {
#define AFL_CATCH_END  }

#define AFL_RAISE(x)   throw (x)
#define AFL_RERAISE    throw
#define AFL_THROWS(x)  throw (x)

#else    // USE_TRY_CATCH
#define AFL_TRY_BEGIN  {{
#define AFL_CATCH(x)   } if (0) {
#define AFL_CATCH_ALL  } if (0) {
#define AFL_CATCH_END  }}

#define AFL_RAISE(x)
#define AFL_RERAISE
#define AFL_THROWS(x)
#endif    // USE_TRY_CATCH

#endif /* DEFINE_H */
