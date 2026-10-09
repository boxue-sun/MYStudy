/*********************************************************************************
 * @file		smart_assert.h
 * @brief		smart_assert belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-09-18
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-09-18 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _SMARTASSERT_H
#define _SMARTASSERT_H
#include "define.h"
#include <string>
#include <sstream>
#include <stdlib.h>
NAMESPACE_AFL_UTIL_START

#define ENABLE_SMART_ASSERT_MODE  //enable AFL_ASSERT macro, use in Debug/Release env 

#define ABORT_IF_ASSERT_FAILED    // if assert failed, abort(), except AFL_ASSERT_LOG

class SmartAssert
{
public:
    SmartAssert(const char* expr, const char* function, int line, const char* file,
                bool abortOnExit = false)
        : SMART_ASSERT_A(*this)
        , SMART_ASSERT_B(*this)
        , abortIfExit_(abortOnExit)
    {
        std::ostringstream oss;
        if (expr && *expr)
            oss << "Expression Failed: " << expr << "\n";
        if (function && *function)
            oss << "Failed in [func: " << function << "], [line: " << line << "], [file: " << file << "]\n";
        errMsg_ += oss.str();
    }

    ~SmartAssert()
    {
        std::cerr << errMsg_ << "\n";

        if (abortIfExit_)
        {
#if defined(ABORT_IF_ASSERT_FAILED)
            abort();
#endif
        }
    }

    template< typename T>
    SmartAssert& printValiable(const char* expr, const T& value)
    {
        std::ostringstream oss;
        oss << "ContextValiable: [" << expr << " = " << value << "]\n";
        errMsg_ += oss.str();
        return *this;
    }

public:
    SmartAssert& SMART_ASSERT_A;
    SmartAssert& SMART_ASSERT_B;

private:
    bool  abortIfExit_;
    std::string errMsg_;
};

static SmartAssert MakeAssert(const char* expr, const char* function, int line, const char* file,
                              bool abortOnExit)
{
    return afl::util::SmartAssert(expr, function, line, file, abortOnExit);
}

static SmartAssert __dont_use_this__ = MakeAssert(NULL, NULL, 0, 0,
                                                  false); //gcc: MakeAssert 定义未使用[-Wunused-function]

// run time assert
#ifndef ENABLE_SMART_ASSERT_MODE
#define AFL_ASSERT(expr)      ((void) 0)
#define AFL_ASSERTEX(expr, func, lineno , file)   ((void) 0)
#define AFL_ASSERT_LOG(expr)  ((void) 0)
#else
#define SMART_ASSERT_A(x)        SMART_ASSERT_OP(x, B)
#define SMART_ASSERT_B(x)        SMART_ASSERT_OP(x, A)
#define SMART_ASSERT_OP(x, next) SMART_ASSERT_A.printValiable(#x, (x)).SMART_ASSERT_##next

#define AFL_ASSERT(expr)          \
            if( (expr) ) ;       \
            else afl::util::MakeAssert(#expr, __FUNCTION__, __LINE__, __FILE__, true).SMART_ASSERT_A
#define AFL_ASSERTEX(expr, func, lineno , file) \
            if( (expr) ) ;                     \
            else afl::util::MakeAssert( #expr, func, lineno, file, true).SMART_ASSERT_A
#define AFL_ASSERT_LOG(expr)       \
            if( (expr) ) ;        \
            else afl::util::MakeAssert(#expr, __FUNCTION__, __LINE__, __FILE__, false).SMART_ASSERT_A
#endif

// compile time assert
#ifdef AFL_CXX11_ENABLED
#define AFL_STATIC_ASSERT(e, ...) static_assert(e, "" __VA_ARGS__)
#else
#define AFL_STATIC_ASSERT(e, ...) AFL_STATIC_ASSERT_IMPL(e, __FILE__, __LINE__)
#define AFL_STATIC_ASSERT_IMPL(e, file, line)  \
                  typedef char static_assert_fail_on_##file_##line[2 * ((e) != 0) - 1]
#endif

NAMESPACE_AFL_UTIL_END
#endif  /* _SMARTASSERT_H */
