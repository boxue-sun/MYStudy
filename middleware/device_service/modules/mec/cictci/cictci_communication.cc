/*
 * @Author: zhangenwei
 * @Date: 2024-01-22 14:35:17
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-22 15:15:08
 * @Description:
 * @FilePath: /airos2.0/middleware/device_service/modules/mec/cictci/cictci_communication.cc
 */

#include "cictci_communication.h"
#include "base/common/log.h"


namespace os {
namespace v2x {
namespace device {

CictciCommunication::CictciCommunication()
{
}

CictciCommunication::~CictciCommunication()
{
    if (socket_fd_ > 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}
/**
 * @brief 初始化通信模块
 *
 * 此函数用于初始化CictciCommunication通信模块，完成TCP或UDP通信的套接字绑定设置
 *
 * 1. 设置远程地址结构remote_addr_,根据传入的远程IP和端口号进行初始化
 * 2. 设置本地地址结构host_addr_,根据传入的本地IP和端口号进行初始化
 *   - 如果本地IP为空，则使用INADDR_ANY表示绑定所有IP
 * 3. 检查协议类型protocol是否支持，只支持TCP和UDP
 * 4. 调用Connect()函数完成套接字连接
 *
 * 返回初始化是否成功
*/
bool CictciCommunication::Init(const std::string &remote_ip, const uint16_t remote_port, const std::string &host_ip,
                               const uint16_t host_port, const ProtocolType protocol)
{
    memset(&remote_addr_, 0, sizeof(remote_addr_));
    remote_addr_.sin_family = AF_INET;
    remote_addr_.sin_addr.s_addr = inet_addr(remote_ip.c_str());
    remote_addr_.sin_port = htons(remote_port);

    memset(&host_addr_, 0, sizeof(host_addr_));
    host_addr_.sin_family = AF_INET;
    if (host_ip.empty())
    {
        host_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    } else
    {
        host_addr_.sin_addr.s_addr = inet_addr(host_ip.c_str());
    }
    host_addr_.sin_port = htons(host_port);

    if ((protocol != ProtocolType::UDP) && (protocol != ProtocolType::TCP))
    {
        MEC_SERVICE_LOG_ERROR << "not support protocol type: " << unsigned(protocol);
        return false;
    }

    return this->Connect();
}

bool CictciCommunication::Connect()
{
    switch (protocol_type_)
    {
        case ProtocolType::UDP:
            return this->UdpInit();
            break;
        case ProtocolType::TCP:
            return this->TcpInit();
            break;
    }

    return false;
}

ssize_t CictciCommunication::SendData(uint8_t *send_data, size_t send_len)
{
    if (socket_fd_ < 0)
    {
        return 0;
    }
    ssize_t ret_val = -1;
    switch (protocol_type_)
    {
        case ProtocolType::UDP:
            ret_val = sendto(socket_fd_, send_data, send_len, 0, (struct sockaddr *) &remote_addr_,
                             sizeof(remote_addr_));
            break;
        case ProtocolType::TCP:
            ret_val = send(socket_fd_, send_data, send_len, 0);
            break;
    }

    if (ret_val > 0)
    {
//        GatUtil::DbgPrintBinary(send_data, send_len, "socket send : ");
    }
    return ret_val;
}
/**
 * @brief 从通信信道中接收数据
 *
 * 此函数从通信信道中接收数据，并将其存储在recv_buf中。
 * @param recv_buf 接收数据的缓冲区
 * @param recv_buf_len 接收数据的缓冲区长度
 * @return 接收到的数据长度，如果接收失败，则返回0
 */
ssize_t CictciCommunication::RecvData(afl::net::ByteBuffer& buffer)
{
    if (socket_fd_ < 0)
    {
        return 0;
    }
    ssize_t recv_len = 0;
    struct sockaddr_in recv_addr;
    socklen_t sock_len;

    switch (protocol_type_)
    {
        case ProtocolType::UDP:
            sock_len = sizeof(recv_addr);
            recv_len = recvfrom(socket_fd_, buffer.beginWrite(), buffer.writableBytes(), 0, (struct sockaddr *) &recv_addr, &sock_len);
            break;
        case ProtocolType::TCP:
            recv_len = recv(socket_fd_, buffer.beginWrite(), buffer.writableBytes(), 0);
            break;
    }

    if (recv_len < 0)
    {
        buffer.retrieveAll();
//        MEC_SERVICE_LOG_ERROR << "socket recv error, errno=" << unsigned(errno) << " " << strerror(errno);
    } else
    {
        //修改
        buffer.hasWritten(recv_len);
//        V2X_CODEC_LOG_INFO << "recvive data sucess!";
    }

    return recv_len;
}


bool CictciCommunication::UdpInit()
{
    struct timeval timeout = {2, 0};

    if (socket_fd_ >= 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    int tmp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (tmp_fd < 0)
    {
        MEC_SERVICE_LOG_ERROR << "udp socket create error, errno=" << strerror(errno);
        return false;
    }

    int ret_val = 1;
    if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, &ret_val, sizeof(ret_val)) < 0)
    {
        MEC_SERVICE_LOG_ERROR << "udp socket setsockopt SO_REUSEADDR error, " << strerror(errno);
        close(tmp_fd);
        return false;
    }

    ret_val = 1;
    if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEPORT, &ret_val, sizeof(ret_val)) < 0)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEPORT error, " << strerror(errno);
    }

    ret_val = bind(tmp_fd, (const struct sockaddr *) &host_addr_, sizeof(host_addr_));
    if (ret_val < 0)
    {
        MEC_SERVICE_LOG_ERROR << "udp bind socket error, errno=" << strerror(errno);
        close(tmp_fd);
        return false;
    }

    ret_val = setsockopt(tmp_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));
    if (ret_val < 0)
    {
        MEC_SERVICE_LOG_ERROR << "udp socket setsockopt SO_RCVTIMEO error, errno=" << strerror(errno);
        close(tmp_fd);
        return false;
    }

    socket_fd_ = tmp_fd;
    return true;
}

bool CictciCommunication::TcpInit()
{
    struct timeval timeout = {2, 0};

    if (socket_fd_ >= 0)
    {
        close(socket_fd_);
        socket_fd_ = -1;
    }

    int tmp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tmp_fd < 0)
    {
        MEC_SERVICE_LOG_ERROR << "tcp socket create error, errno=" << strerror(errno);
        return false;
    }

    // disable Nagle
    int retval = 0;
    int enable = 1;
    retval = setsockopt(tmp_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void *>(&enable), sizeof(enable));
    if (retval == -1)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket setsockopt TCP_NODELAY error, errno=" << strerror(errno);
        return false;
    }

    retval = 1;
    if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, &retval, sizeof(retval)) < 0)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEADDR error, " << strerror(errno);
        return false;
    }

    retval = 1;
    if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEPORT, &retval, sizeof(retval)) < 0)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_REUSEPORT error, " << strerror(errno);
    }

    retval = connect(tmp_fd, (struct sockaddr *) &remote_addr_, sizeof(remote_addr_));
    if (retval < 0)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket connect error, errno=" << strerror(errno);
        return false;
    }

    retval = setsockopt(tmp_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));
    if (retval < 0)
    {
        close(tmp_fd);
        MEC_SERVICE_LOG_ERROR << "tcp socket setsockopt SO_RCVTIMEO error, errno=" << strerror(errno);
        return false;
    }

    socket_fd_ = tmp_fd;
    return true;
}

}  // namespace device
}  // namespace v2x
}  // namespace os
