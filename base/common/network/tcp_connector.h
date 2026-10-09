/*********************************************************************************
 * @file		tcp_connector.h
 * @brief		tcp_connector belongs to CICTCI
 * @details		客户端连接器，连接远程Socket
 * @author		cs
 * @date		2014-10-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-10-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _TCPCONNECTOR_H
#define _TCPCONNECTOR_H

//#include "socket_util.h"
#include "inet_address.h"
#include "define.h"
#include "non_copy.h"
#include "event_loop.h"
#include "channel.h"
NAMESPACE_AFL_NET_START
class Socket;
class Channel;
class EventLoop;
class InetAddress;

class TcpConnector : afl::base::NonCopy
{
public:
    typedef std::function<void (AFL_SOCKET)> NewConnectionCallback;

public:
    TcpConnector(EventLoop* loop, const InetAddress& serverAddr);
    ~TcpConnector();

    void setNewConnectionCallback(const NewConnectionCallback& callback)
    {
        newConnCallBack_ = callback;
    }

    const InetAddress& serverAddress() const
    {
        return serverAddr_;
    }

    void connect();
    void reconnect();
    void stop();

private:
    void connectInLoop();
    void connectServer();
    void connectEstablished(AFL_SOCKET sock);
    void stopInLoop();

    void handleWrite();
    void handleError();
    AFL_SOCKET disableChannel();
    void retry(AFL_SOCKET sockfd);
    void cancelPendingRetry();

    enum States { kDisconnected, kConnecting, kConnected };
    void setState(States s)
    {
        state_ = s;
    }

private:
    States                     state_;
    bool                       connect_;
    EventLoop*                 loop_;
    const InetAddress          serverAddr_;
    Channel*                   connChannel_;
    NewConnectionCallback      newConnCallBack_;

    int retryDelayMs;
    int retryTimer;

    static const int kMaxRetryDelayMs;
    static const int kInitRetryDelayMs;
};

typedef TcpConnector* TcpConnectorPtr;

NAMESPACE_AFL_NET_END
#endif  /* _TCPCONNECTOR_H */
