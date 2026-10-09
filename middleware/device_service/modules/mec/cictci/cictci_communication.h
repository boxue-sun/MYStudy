/*
 * @Author: zhangenwei
 * @Date: 2024-01-22 14:25:27
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-22 15:15:24
 * @Description:
 * @FilePath: /airos2.0/middleware/device_service/modules/mec/cictci/cictci_communication.h
 */

#pragma once
#include <arpa/inet.h>
#include <stdint.h>

#include <string>
#include "base/common/network/byte_buffer.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include "glog/logging.h"
namespace os {
namespace v2x {
namespace device {

/**
 * @brief Mec通信类
 * @note 和Mec设备的通信
 */
class CictciCommunication
{
public:
    enum class ProtocolType
    {
        UDP = 0, TCP = 1
    };

    CictciCommunication();

    ~CictciCommunication();

    bool
    Init(const std::string &remote_ip, const uint16_t remote_port, const std::string &host_ip, const uint16_t host_port,
         const ProtocolType protocol);

    bool Connect();

    ssize_t SendData(uint8_t *send_data, size_t send_len);

    ssize_t RecvData(afl::net::ByteBuffer& buffer);

private:
    bool UdpInit();

    bool TcpInit();

private:
    int socket_fd_ = -1;
    ProtocolType protocol_type_ = ProtocolType::UDP;
    struct sockaddr_in remote_addr_;
    struct sockaddr_in host_addr_;
};

}  // namespace device
}  // namespace v2x
}  // namespace os