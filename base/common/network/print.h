/*********************************************************************************
 * @file		print.h
 * @brief		print belongs to CICTCI
 * @details
 * @author		alfred
 * @date		2024-02-01
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024-02-01 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AIROS_EDGE_PRINT_H
#define AIROS_EDGE_PRINT_H
#include "define.h"
#include "time_stamp.h"
#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "base/common/log.h"
#define AIROS_INFO(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);             \
        char buffer1[10240] = {0};              \
	    sprintf(buffer1, "[%s][%20s(%6d)]\033[1;31m" format "\033[0m\n", buffer, file, __LINE__, ##__VA_ARGS__); \
        google::LogMessage(__FILE__, __LINE__, google::INFO).stream() << buffer1 ;\
	}
#define AIROS_ERRORPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);                   \
	    char buffer1[10240] = {0};\
        sprintf(buffer1, "[%s][%20s(%6d)]\033[1;31m" format "\033[0m", buffer, file, __LINE__, ##__VA_ARGS__); \
        std::cout <<  buffer1 <<std::endl;  \
}
#define AIROS_SUCESSPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);                   \
	    char buffer1[10240] = {0};\
        sprintf(buffer1, "[%s][%20s(%6d)]\033[1;32m" format "\033[0m", buffer, file, __LINE__, ##__VA_ARGS__); \
        std::cout <<  buffer1 <<std::endl;  \
}
#define AIROS_NORMAL_DEBUGPRINT(format,...){ \
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
	    printf("[%s][%20s(%6d)]\033[1;32m" format "\033[0m\n", buffer, file, __LINE__, ##__VA_ARGS__);\
	}

/////////////////////////////////////////////////////////////
#define DEBUG_PRINT(format,...){\
         LOG_INFO << format << #__VA_ARGS__; \
	    }
#define WARN_PRINT(format,...){\
         LOG_WARN << format << #__VA_ARGS__; \
        }
#define ERROR_PRINT(format,...){\
         LOG_ERROR << format << #__VA_ARGS__; \
        }
#define SUCCESS_PRINT(format,...){\
         LOG_INFO << format << #__VA_ARGS__; \
        }
#define FATAL_PRINT(format,...){\
         LOG_FATAL << format << #__VA_ARGS__; \
        }

#define DEBUGPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
	    printf("[%s][%30s(%6ld)]\033[1;37m" format "\033[0m\n", buffer, file, (long int) __LINE__, ##__VA_ARGS__);\
	}
#define WARNPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
	    printf("[%s][%30s(%6ld)]\033[1;33m" format "\033[0m\n", buffer, file, (long int)__LINE__, ##__VA_ARGS__);\
}
#define ERRORPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
	    printf("[%s][%30s(%6ld)]\033[1;31m" format "\033[0m\n", buffer, file, (long int)__LINE__, ##__VA_ARGS__);\
}
#define SUCESSPRINT(format,...){\
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
        printf("[%s][%30s(%6ld)]\033[1;32m" format "\033[0m\n", buffer, file, (long int)__LINE__, ##__VA_ARGS__);\
}
#define NORMAL_DEBUGPRINT(format,...){ \
		char buffer[32] = {0};\
		sprintf(buffer, "%s",  afl::util::TimeStamp::now(true).toString(true).c_str());\
	    const char* file = __FILE__;\
	    file = basename(file);\
	    assert(file);\
	    printf("[%s][%30s(%6ld)]\033[1;32m" format "\033[0m\n", buffer, file, (long int)__LINE__, ##__VA_ARGS__);\
	}

#endif //AIROS_EDGE_PRINT_H
