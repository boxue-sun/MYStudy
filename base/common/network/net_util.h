/*********************************************************************************
 * @file		net_util.h
 * @brief		net_util belongs to CICTCI
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
#ifndef BASE_COMMON_NETUTIL_H
#define BASE_COMMON_NETUTIL_H

#include "define.h"
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
#include <thread>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <ifaddrs.h>
#include <linux/icmp.h>
NAMESPACE_AFL_NET_START

class NetUtil
{
public:
    static bool        isBroadcastAddress(const char *str);
    static bool        isValidIp(const char *str);

    static bool        isValidIpv4(const char *str);

    static bool        isValidIpv6(const char *str);

    static bool        isLittleEndian();

    static void        reverseBytes(const void *source, void *result, size_t length);

    template <typename DataType>
    static DataType    reverseBytes(const DataType& source)
    {
        DataType result = 0;
        reverseBytes(&source, &result, sizeof(result));
        return result;
    }

    template <typename DataType>
    static void        reverseBytes(const DataType *source, DataType *result)
    {
        reverseBytes(source, result, sizeof(DataType));
    }

    static void        host2Net(const void *source, void *result, size_t length);

    template <typename DataType>
    static void        host2Net(const DataType& source, DataType& result)
    {
        host2Net(&source, &result, sizeof(result));
    }

    template <typename DataType>
    static DataType    host2Net(const DataType& source)
    {
        DataType result;
        host2Net(&source, &result, sizeof(result));
        return result;
    }

    static void        net2Host(const void *source, void *result, size_t length);

    template <typename DataType>
    static void        net2Host(const DataType& source, DataType& result)
    {
        host2Net<DataType>(source, result);
    }

    template <typename DataType>
    static DataType net2Host(const DataType& source)
    {
        return host2Net<DataType>(source);
    }
    static bool arpPing(const char* target_ip);
    static bool ping(const std::string& target_ip);

};

NAMESPACE_AFL_NET_END
#endif
