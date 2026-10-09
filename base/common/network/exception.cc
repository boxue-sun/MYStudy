/*********************************************************************************
 * @file		exception.cpp
 * @brief		exception belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-10-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-10-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#include "exception.h"

# define DO_NAME_DEMANGLE

#ifdef DO_NAME_DEMANGLE
# include "demangle.h"
#endif

NAMESPACE_AFL_UTIL_START

Exception::Exception(const char* errinfo)
    : line_(0)
    , filename_("unknown filename")
    , errmsg_(errinfo)
{
    trace_stack();
}

Exception::Exception(const char* filename, int linenumber, const char* errinfo)
    : line_(linenumber),
      filename_(filename),
      errmsg_(errinfo)
{
    trace_stack();
}

Exception::Exception(const char* filename, int linenumber, const std::string& errinfo)
    : line_(linenumber)
    , filename_(filename)
    , errmsg_(errinfo)
{
    trace_stack();
}

Exception::~Exception() throw ()
{
}

void Exception::trace_stack()
{
#ifdef OS_WINDOWS
#else
    static const int len = 256;
    void* buffer[len];
    int nptrs = ::backtrace(buffer, len);
    char** strings = ::backtrace_symbols(buffer, nptrs);
    if (!strings)
        return;

    for (int i = 0; i < nptrs; ++i)
    {
#ifndef DO_NAME_DEMANGLE
        callStack_.append(strings[i]);
#else
        std::string line(strings[i]);  // ./test(_ZN6detail12c_do_nothingEfi+0x44) [0x401974]

        std::string unmangle;
        if (afl::util::demangleName(line.c_str(), unmangle))
        {
            callStack_.append(unmangle);
        }
        else
        {
            callStack_.append(strings[i]);
        }
#endif
        callStack_.push_back('\n');
    }
    free(strings);
#endif
}

NAMESPACE_AFL_UTIL_END
