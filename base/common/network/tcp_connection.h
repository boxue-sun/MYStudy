/*********************************************************************************
 * @file		tcp_connection.h
 * @brief		tcp_connection belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-10-31
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  [create]2014-10-31 cs       1.0       ————             Create this file
 *  [modify]2024-04-23 zhangenwei       1.0       ————     add tls
 * @endverbatim
 ********************************************************************************/
#ifndef _TCPCONNECTION_H
#define _TCPCONNECTION_H

#include "define.h"
#include "time_stamp.h"
#include "any.h"
#include "call_backs.h"
#include "inet_address.h"
#include "socket.h"
#include "byte_buffer.h"
#include "channel.h"
#include "event_loop.h"
#include <memory>     //for enable_shared_from_this
#include "print.h"
#include "exception"
#include "file_util.h"
#include "event_loop.h"
#include "smart_assert.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "base/common/log.h"
NAMESPACE_AFL_NET_START
class Channel;
class EventLoop;
class Socket;
class InetAddress;
using afl::util::TimeStamp;
enum TlsAuthMode
{
    NoAuthentication,     // 无认证
    ClientAuthentication, // 单向认证
    MutualAuthentication  // 双向认证
};
typedef struct TlsCtxInfo
{
    bool tlsClient = true;
    TlsAuthMode tlsAuthMode = MutualAuthentication;

    std::string tlsCaCertificateFile;
    std::string tlsClientCertificateFile;
    std::string tlsClientPrivateKeyFile;
    std::string tlsClientPrivateKeyPassword;
    int minVersion = TLS1_2_VERSION;
    int maxVersion = TLS1_2_VERSION;
}CtxInfo;
using OnServerDisconnectCallBack = std::function<void(bool serverDis)>;
class TcpConnection : afl::base::NonCopy, public std::enable_shared_from_this< TcpConnection >
{
public:
    TcpConnection(EventLoop* loop, int sockfd, const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

public:
    void setConnectStateCallBack(const OnServerDisconnectCallBack cb)
    {
        onServerDisconnectCallBack_ = cb;
    }
    EventLoop* getLoop() const
    {
        return loop_;
    }
    AFL_SOCKET fd() const
    {
        return socket_->fd();
    }
    const InetAddress& localAddress() const
    {
        return localAddr_;
    }
    const InetAddress& peerAddress() const
    {
        return peerAddr_;
    }
    bool connected() const
    {
        return state_ == kConnected;
    }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    void enableReading()
    {
        channel_->enableReading();
    }
    void disableReading()
    {
        channel_->disableReading();
    }
    void enableWriting()
    {
        channel_->enableWriting();
    }
    void disableWriting()
    {
        channel_->disableWriting();
    }
    void disableAll()
    {
        channel_->disableAll();
    }

    void setNoDelay(bool on)
    {
        socket_->setNoDelay(on);
    }

    void setContext(const afl::util::any& context)
    {
        context_ = context;
    }
    const afl::util::any getContext() const
    {
        return context_;
    }
    afl::util::any* getMutableContext()
    {
        return &context_;
    }

    void connectEstablished();   // called when TcpServer accepts a new connection
    void connectDestroyed();     // called when TcpServer has removed me from its map

    void send(const void* data, size_t len);
    void send(const std::string& buffer);
    void send(ByteBuffer* buffer);

    void shutdown();

    void initTls(TlsCtxInfo tlsCtxInfo);
    void deinitTls();
    bool tlsConnect();
    bool tlsAccept();
    bool getCertsInfo();
    bool tlsVerify();

    bool ctxNoVerify();
    bool ctxBidirectionalVerify();
    bool ctxSignleVerify();
    bool loadPrivateKeyWithPassword();
    int getState();
private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    const char* getState(StateE);

    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const void* data, size_t len);
    void sendInLoop(const std::string& buffer);
    void shutdownInLoop();
    void setState(StateE s)
    {
        state_ = s;
    }

private:
    bool serverDisconnected_ = false;
    OnServerDisconnectCallBack onServerDisconnectCallBack_;
    EventLoop*            loop_;
    StateE                state_;
    Socket*               socket_;
    Channel*              channel_;
    const InetAddress     localAddr_;
    const InetAddress     peerAddr_;

    afl::util::any        context_;

    ByteBuffer            inputBuffer_;
    ByteBuffer            outputBuffer_; // FIXME: use list<Buffer> as output buffer.

    ConnectionCallback    connectionCallback_;
    MessageCallback       messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback         closeCallback_;
    TlsCtxInfo            tlsCtxInfo_;
    SSL *                 tlsHandle_ = nullptr;
    SSL_CTX *             tlsCtx_ = nullptr;

};

NAMESPACE_AFL_NET_END
#endif  /* _TCPCONNECTION_H */
