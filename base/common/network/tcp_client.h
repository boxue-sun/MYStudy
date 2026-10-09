/*********************************************************************************
 * @file		tcp_client.h
 * @brief		tcp_client belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-11-06
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-11-06  cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H
#include "define.h"
#include "time_stamp.h"
#include "mutex.h"
#include "call_backs.h"
#include "inet_address.h"
#include "tcp_connection.h"
#include "event_loop.h"
#include "inet_address.h"
#include "tcp_connection.h"
#include "tcp_connector.h"
//#include "socket_util.h"
using afl::util::TimeStamp;
using afl::thread::Mutex;

NAMESPACE_AFL_NET_START

class EventLoop;
class InetAddress;
class TcpConnector;
class ByteBuffer;
class Tcpconnection;

class TcpClient
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& clientname = "TcpClient");
    ~TcpClient();

public:
    EventLoop* getLoop() const { return loop_; }
    AFL_SOCKET fd() const
    {
        assert(connection_);
        return connection_->fd();
    }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

public:
    void connect();
    void reconnect();
    void disconnect();
    void stop();

    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

private:
    EventLoop              *loop_;
    TcpConnector           *connector_;
    ConnectionCallback     connectionCallback_;
    MessageCallback        messageCallback_;
    WriteCompleteCallback  writeCompleteCallback_;
    bool                   retry_;
    bool                   connect_;
    TcpConnectionPtr       connection_;
    const std::string      clientName_;
};

NAMESPACE_AFL_NET_END
#endif  /* _TCPCLIENT_H */
