/*********************************************************************************
 * @file		exception.h
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
#ifndef _EXCEPTION_H
#define _EXCEPTION_H
#include "define.h"
#include <exception>
#include <stdlib.h>
#include <execinfo.h>
NAMESPACE_AFL_UTIL_START

class Exception : public std::exception
{
public:
    explicit Exception(const char* errinfo);
    Exception(const char* filename, int linenumber, const char* errinfo);
    Exception(const char* filename, int linenumber, const std::string& errinfo);
    virtual ~Exception() throw();

    virtual const char* what() const throw()
    {
        return errmsg_.c_str();
    }

    const char* stackTrace() const throw()
    {
        return callStack_.c_str();
    }

    const char* filename() const throw()
    {
        return filename_.c_str();
    }

    int line() const throw()
    {
        return line_;
    }
private:
    void trace_stack();

    int line_;
    std::string filename_;
    std::string errmsg_;
    std::string callStack_;
};

NAMESPACE_AFL_UTIL_END
#endif  /* _EXCEPTION_H */
