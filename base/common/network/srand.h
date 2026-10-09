/*********************************************************************************
 * @file		srand.h
 * @brief		srand belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		2024-05-17
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024-05-17 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _SRAND_H
#define _SRAND_H
#include "define.h"
#include <string>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
NAMESPACE_AFL_UTIL_START

class Srand
{
public:
    static std::string srandStr(uint32_t num)
    {
        static bool initialized = false;
        if (!initialized) {
            srand(time(0));
            initialized = true;
        }
        string ret;
        char m[64] = {0};
        char s[10] = {0};

        for (uint32_t i = 0; i < num; i++)
        {
            int x, type;
            type = rand() % 3;
            if (type == 0)//判断随机类型生成大小写或者字母
            {
                x = rand() % ('Z' - 'A' + 1) + 'A';
            } else if (type == 1)
            {
                x = rand() % ('z' - 'a' + 1) + 'a';
            } else if (type == 2)
            {
                x = rand() % ('9' - '0' + 1) + '0';
            }
            sprintf(s, "%c", x);
            strcat((char *) m, (const char *) s);
        }
        ret = m;
//        printf("%s\n", ret.c_str());
        return m;
    }

};
NAMESPACE_AFL_UTIL_END

#endif /* _DATETIME_H */
