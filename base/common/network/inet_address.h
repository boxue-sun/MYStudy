/*********************************************************************************
 * @file		inet_address.h
 * @brief		inet_address belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-09-06
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-09-06 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef BASE_COMMON_NETWORK_INETADDRESS_H
#define BASE_COMMON_NETWORK_INETADDRESS_H

#include "define.h"
#include "socket_util.h"


NAMESPACE_AFL_NET_START

class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0);
    InetAddress(const char *ip, uint16_t port);
    InetAddress(const AFL_SOCKADDR_IN& addr);

    static bool resolve(const char *hostname, InetAddress *addr);

public:
    uint16_t port() const;
    std::string ip() const;
    std::string ipPort() const;

    size_t addressLength() const { return sizeof(addr_); }
    operator struct sockaddr *() const{ return (struct sockaddr*)&addr_; }

    const AFL_SOCKADDR_IN& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const AFL_SOCKADDR_IN& addr) { addr_ = addr; }

    uint32_t ipNetEndian() const { return addr_.sin_addr.s_addr; }
    uint16_t portNetEndian() const { return addr_.sin_port; }

private:
    AFL_SOCKADDR_IN  addr_;
};

NAMESPACE_AFL_NET_END
#endif  /* ZL_INETADDRESS_H */
