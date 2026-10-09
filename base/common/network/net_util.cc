/*********************************************************************************
 * @file		net_util.cc
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
#include "net_util.h"


NAMESPACE_AFL_NET_START

bool NetUtil::isBroadcastAddress(const char *str)
{
    return (NULL == str) ? false : (0 == strcmp(str, "255.255.255.255"));
}

bool NetUtil::isValidIp(const char *str)
{
    return isValidIpv4(str) || isValidIpv6(str);
}

bool NetUtil::isValidIpv4(const char *str)
{
    if((NULL == str) || (0 == str[0]) || ('0' == str[0])) return false;
    if(0 == strcmp(str, "*")) return true;
    if(0 == strcmp(str, "255.255.255.255")) return false;

    int dot = 0; // .的个数
    const char *strp = str;
    int num = 0; //计算每一个以'.'分开的字符串数值
    while(*strp)
    {
        if('.' == *strp)
        {
            ++dot;
            num = 0;
        }
        else if((*strp < '0') || (*strp > '9'))
        {
            return false;
        }
        else
        {
            num *= 10;
            num += (*strp - '0');
            if(num > 255)
                return false;
        }
        ++strp;
    }

    return (3 == dot);  // .的个数必须为3
}

bool NetUtil::isValidIpv6(const char *str)
{
    const char *pos = strchr(str, ':');
    if(NULL == pos) return false;
    return strchr(pos, ':') != NULL;
}

bool NetUtil::isLittleEndian()
{
    union w
    {
        int i;
        char c;
    } u;
    u.i = 1;

    return (u.c == 1);

    //int i = 0x1;
    //return *(char *)&i == 0x1;  // this is ok
}

void NetUtil::reverseBytes(const void *source, void *result, size_t length)
{
    char *source_begin = (char *)source;
    char *result_end = ((char *)result) + length;

    for(size_t i = 0; i < length; ++i)
        *(--result_end) = source_begin[i];
}

void NetUtil::host2Net(const void *source, void *result, size_t length)
{
    if(isLittleEndian())   //只有小字节序才需要转换，大字节序和网络字节序是一致的
        reverseBytes(source, result, length);
}

void NetUtil::net2Host(const void *source, void *result, size_t length)
{
    host2Net(source, result, length);
}
// 使用 ARP 来检测远端设备是否联网
bool NetUtil::arpPing(const char* target_ip) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return false;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip, &dest_addr.sin_addr);

    char arp_request[42];
    memset(arp_request, 0, sizeof(arp_request));
    arp_request[0] = 0xff; // ARP 转发
    arp_request[1] = 0xff; // ARP 转发
    arp_request[2] = 0x00; //  ARP Opcode (请求)
    arp_request[3] = 0x01; //  ARP Protocol (IPv4)
    arp_request[4] = 0x06; //  ARP Hardware Address Length (6 bytes for Ethernet)
    arp_request[5] = 0x04; //  ARP Protocol Address Length (4 bytes for IPv4)
    arp_request[6] = 0x00; //  ARP Sender Hardware Address (6 bytes)
    arp_request[12] = 0x00; //  ARP Target Hardware Address (6 bytes)
    arp_request[18] = 0x01; //  ARP Opcode (请求)
    arp_request[20] = 0x00; //  ARP Sender IP Address (4 bytes)
    arp_request[24] = 0x00; //  ARP Target IP Address (4 bytes)

    if (sendto(sockfd, arp_request, sizeof(arp_request), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Failed to send ARP request." << std::endl;
        close(sockfd);
        return false;
    }

    char buffer[1024];
    int recv_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, nullptr, nullptr);
    if (recv_size < 0) {
        std::cerr << "Failed to receive ARP response." << std::endl;
        close(sockfd);
        return false;
    }

    close(sockfd);
    return recv_size > 0;
}

bool NetUtil::ping(const std::string& target_ip)
{
    std::string command = "ping -c 1 " + target_ip;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to execute ping command." << std::endl;
        return false;
    }

    char buffer[128];
    std::string result;
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }

    pclose(pipe);
    return result.find("bytes from") != std::string::npos;
}

NAMESPACE_AFL_NET_END
