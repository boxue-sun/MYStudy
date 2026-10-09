/*********************************************************************************
 * @file		demangle.cpp
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
#include "demangle.h"


NAMESPACE_AFL_UTIL_START

bool demangleName(const char* mangled, char* unmangled, size_t buf_size)
{
    int status;

    static const size_t max_size = 1024;
    char temp[max_size];

    if(1 == sscanf(mangled, "%*[^(]%*[^_]%127[^)+]", temp))
    {
        unmangled = abi::__cxa_demangle(temp, unmangled, &buf_size, &status);
        if (status == 0)
        {
//            printf("Name after  Mangled : %s ; Name before Mangled : %s\n", unmangled, temp);
            return true;
        }
    }

//    printf("Name after  Mangled fail: %s ; Name before Mangled : %s\n", unmangled, temp);
    return false;
}

bool demangleName(const char* mangled, std::string& unmangled)
{
    static const size_t max_size = 1024;
    char result[max_size];
    if (demangleName(mangled, result, max_size))
    {
        unmangled = result;
        return true;
    }
    return false;
}

NAMESPACE_AFL_UTIL_END
