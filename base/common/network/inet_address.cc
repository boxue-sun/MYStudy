#include "inet_address.h"

NAMESPACE_AFL_NET_START

InetAddress::InetAddress(uint16_t port/* = 0*/)
{
    ::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = INADDR_ANY;
}

InetAddress::InetAddress(const char *ip, uint16_t port)
{
    ::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    int nIP = 0;
    if (!ip || '\0' == *ip || 0 == strcmp(ip, "0")
        || 0 == strcmp(ip, "0.0.0.0") || 0 == strcmp(ip, "*"))
    {
        nIP = htonl(INADDR_ANY);
    }
    else
    {
        nIP = inet_addr(ip);
    }
    addr_.sin_addr.s_addr = nIP;
}

InetAddress::InetAddress(const AFL_SOCKADDR_IN& addr) : addr_(addr)
{

}

std::string InetAddress::ip() const
{
    char ip[256], tmp[256];
    AFL_SNPRINTF(ip, 128, "%s", inet_ntop(AF_INET, (void*)&addr_.sin_addr, tmp, 256));
    return ip;
}

uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::ipPort() const
{
    char host[256], ip[256];
    AFL_SNPRINTF(host, 256, "%s:%d", inet_ntop(AF_INET, (void*)&addr_.sin_addr, ip, 256), ntohs(addr_.sin_port));
    return host;
}

/*static*/ bool InetAddress::resolve(const char *hostname, InetAddress *addr)
{
    static __thread char g_resolveBuffer[64 * 1024];
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    bzero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname, &hent, g_resolveBuffer, sizeof(g_resolveBuffer), &he, &herrno);
    if (ret == 0 && he != NULL)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        addr->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        return false;
    }
}

NAMESPACE_AFL_NET_END
