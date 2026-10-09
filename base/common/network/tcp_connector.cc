#include "tcp_connector.h"

NAMESPACE_AFL_NET_START

const int TcpConnector::kMaxRetryDelayMs = 5 * 1000;
const int TcpConnector::kInitRetryDelayMs = 500;

TcpConnector::TcpConnector(EventLoop* loop, const InetAddress& serverAddr)
    : state_(kDisconnected), connect_(false),
      loop_(loop), serverAddr_(serverAddr),
      connChannel_(NULL),
      retryDelayMs(kInitRetryDelayMs),
      retryTimer(-1)
{
}

TcpConnector::~TcpConnector()
{
}

void TcpConnector::cancelPendingRetry()
{
    if(retryTimer > 0)
    {
        loop_->cancelTimer(retryTimer);
        retryTimer = -1;
    }
}

void TcpConnector::connect()
{
    cancelPendingRetry();
    connect_ = true;
    loop_->runInLoop(std::bind(&TcpConnector::connectInLoop, this));
}


void TcpConnector::reconnect()
{
    loop_->assertInLoopThread();

    if(state_ == kConnecting)
        return;

    setState(kDisconnected);
    retryDelayMs = kInitRetryDelayMs;

    cancelPendingRetry();
    connect_ = true;
    connectInLoop();
}

void TcpConnector::connectInLoop()
{
    loop_->assertInLoopThread();
//    assert(state_ == kDisconnected);
    if (state_ == kDisconnected && connect_)
    {
        connectServer();
    }
}

void TcpConnector::connectServer()
{
    AFL_SOCKET sockfd = SocketUtil::createSocket(); //::createNonblockingOrDie();
    SocketUtil::setNonBlocking(sockfd);

    int ret = SocketUtil::connect(sockfd, serverAddr_.getSockAddrInet());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connectEstablished(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        SocketUtil::closeSocket(sockfd);
        break;

    default:
        SocketUtil::closeSocket(sockfd);
        // connectErrorCallback_();
        break;
    }
}

void TcpConnector::connectEstablished(AFL_SOCKET sock)
{
    setState(kConnecting);
    if (connChannel_)
        delete connChannel_;

    connChannel_ = new Channel(loop_, sock);
    connChannel_->setWriteCallback(std::bind(&TcpConnector::handleWrite, this));
    connChannel_->setErrorCallback(std::bind(&TcpConnector::handleError, this));

    connChannel_->enableWriting();
}

void TcpConnector::stop()
{
    cancelPendingRetry();
    connect_ = false;
    loop_->queueInLoop(std::bind(&TcpConnector::stopInLoop, this));
}

void TcpConnector::stopInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnecting)
    {
        setState(kDisconnected);
        AFL_SOCKET sockfd = disableChannel();
        retry(sockfd);
    }
}

AFL_SOCKET TcpConnector::disableChannel()
{
    connChannel_->disableAll();   // 从poller中移除，不再关注任何事件
    connChannel_->remove();
    AFL_SOCKET sockfd = connChannel_->fd();
    return sockfd;
}

//连接远端socket成功
void TcpConnector::handleWrite()
{
//    LOG_INFO("TcpConnector::handleWrite : [%d]", connChannel_->fd());
    if (state_ ==
            kConnecting) //连接建立时注册Channel可写事件，此时响应可写，将socket返回，并禁用Channel
    {
        AFL_SOCKET sockfd = disableChannel();
        int err = SocketUtil::getSocketError(sockfd);
        if (err)
        {
//            LOG_WARN("TcpConnector::handleWrite - SO_ERROR = [%d][%d][%s]", sockfd, err, strerror(err));
            retry(sockfd);
        }
        else if (SocketUtil::isSelfConnect(sockfd))
        {
//          LOG_WARN("TcpConnector::handleWrite - Self connect = [%d]", sockfd);
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnCallBack_(sockfd);
            }
            else
            {
                SocketUtil::closeSocket(sockfd);
            }
        }
    }
    else
    {
        assert(state_ == kDisconnected);
    }
}

void TcpConnector::handleError()
{
//    LOG_ERROR("TcpConnector::handleError(): fd = [%d], state = [%d]", connChannel_->fd(), state_);
    if (state_ == kConnecting)
    {
        AFL_SOCKET sockfd = disableChannel();
        int err = SocketUtil::getSocketError(sockfd);
//        LOG_ERROR("TcpConnector::handleError() SO_ERROR = [%d][%s]", err, strerror(err));
        err++; /* discard warning */
        retry(sockfd);
    }
}

void TcpConnector::retry(AFL_SOCKET sockfd)
{
    SocketUtil::closeSocket(sockfd);
    setState(kDisconnected);
    if (connect_)
    {
//        LOG_INFO("TcpConnector::retry; Retry connecting to [%s]", serverAddr_.ipPort().c_str());
        retryTimer = loop_->addTimer(std::bind(&TcpConnector::connectInLoop, this), retryDelayMs/1000.0);
        retryDelayMs = std::min(retryDelayMs * 2, kMaxRetryDelayMs);
    }
    else
    {
//        LOG_INFO("TcpConnector::retry: do not connect");
    }
}

NAMESPACE_AFL_NET_END
