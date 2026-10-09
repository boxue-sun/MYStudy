/*********************************************************************************
 * @file		tcp_acceptor.h
 * @brief		tcp_acceptor belongs to CICTCI
 * @details
 * @author		cs
 * @date		 2014-10-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *   2014-10-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef BASE_COMMON_NETWORK_TCPACCEPTOR_H
#define BASE_COMMON_NETWORK_TCPACCEPTOR_H
#include "define.h"
#include "time_stamp.h"
#include "non_copy.h"

NAMESPACE_AFL_NET_START

class Socket;
class InetAddress;
class Channel;
class EventLoop;
using afl::util::TimeStamp;

class TcpAcceptor : afl::base::NonCopy
{
public:
    //typedef std::function<void(Socket *)> NewConnectionCallback;
    typedef std::function<void (int , const InetAddress&)> NewConnectionCallback;
public:
    TcpAcceptor(EventLoop *loop, const InetAddress& listenAddr);
    ~TcpAcceptor();

    void setNewConnectionCallback(const NewConnectionCallback& callback)
    {
        newConnCallBack_ = callback;
    }

    void listen();

private:
    void onAccept(TimeStamp now);

private:
    EventLoop *loop_;
    Socket    *accept_socket;
    Channel   *accept_channel_;
    NewConnectionCallback newConnCallBack_;
};

NAMESPACE_AFL_NET_END
#endif  /* _TCPACCEPTOR_H */
