/*********************************************************************************
* @file		    performence_utils.h
* @brief		performence_utils.h belongs to CICTCI
* @details
* @author		alfred
* @email        zhangenwei64@gmail.com
* @date		    24-8-22
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-8-22 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/
#ifndef AIROS2_0_OM_COMMON_PERFORMENCE_H
#define AIROS2_0_OM_COMMON_PERFORMENCE_H
#include "namespace.h"
#include "data_model/data_performence.h"
#include "base/common/network/exception.h"
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/file.h>
#include <unistd.h>
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace std;
using namespace afl::util;
using namespace os::v2x::protocol::om::common;
class PerformenceUtils
{
public:
    PerformenceUtils() = default;
    virtual ~PerformenceUtils()= default; // 这里使用 default 表示使用默认析构函数  ;
    static bool    getCpuInfo(CpuInfo& cpuInfo);
    static bool    getMemInfo(MemInfo& memInfo);
    static bool    getDiskInfo(DiskInfo &diskInfo);
    static bool    getNetInfo(NetInfo& netInfo, std::string nic = "eth0");
    static bool getDiskCapacityInfo(DisksInfoSt &disksInfoSt);
    static bool getDiskIOStats(DisksInfoSt &disksInfoSt);
    static std::string formatBytes(double bytes);

    static bool getGpuInfo(GpuInfo& gpuInfo);

    public:
    static bool    printInfo ;
    static int     ByteUnit;
    static std::string m_RootDirName;
    static std::string m_WorkDirName;
};
NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif
