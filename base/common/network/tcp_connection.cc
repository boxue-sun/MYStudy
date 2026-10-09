#include "tcp_connection.h"

NAMESPACE_AFL_NET_START

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
//  LOG_INFO("defaultConnectionCallback : [%s]<->[%s] [%s]\n", conn->localAddress().ipPort().c_str(),
//        conn->peerAddress().ipPort().c_str(), conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr& conn, ByteBuffer* buf, TimeStamp receiveTime)
{
//    LOG_INFO("defaultMessageCallback : [%d][%s]", conn->fd(), buf->toString().c_str());
}

TcpConnection::TcpConnection(EventLoop* loop, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
    : loop_(loop)
    , state_(kConnecting)
    , localAddr_(localAddr.getSockAddrInet())
    , peerAddr_(peerAddr.getSockAddrInet())
{
    socket_ = new Socket(sockfd);
    socket_->setKeepAlive(true);
    socket_->setNoDelay(true);
    socket_->setNonBlocking();

    channel_ = new Channel(loop, sockfd);
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
//    LOG_INFO("TcpConnection::TcpConnection(), [%0x] [%d][%0x][%0x]", this, socket_->fd(), socket_, channel_);
}

TcpConnection::~TcpConnection()
{
    //LOG_INFO("TcpConnection::~TcpConnection(),[%0x] [%d][%0x][%0x]", this, socket_->fd(), socket_, channel_);
    //AFL_ASSERT(state_ == kDisconnected)(state_);
	SAFE_DELETE(socket_);
	SAFE_DELETE(channel_);
}

const char* TcpConnection::getState(StateE state)
{
    switch (state)
    {
    case kDisconnected:
        return "TcpDisconnected";  break;
    case kConnecting:
        return "TcpConnecting";    break;
    case kConnected:
        return "TcpConnected";     break;
    case kDisconnecting:
        return "TcpDisconnecting"; break;
    default:
        assert(0); break;
    }
    return "null";
}

void TcpConnection::send(const void* data, size_t len)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(data, len);
        }
        else
        {
            loop_->runInLoop(std::bind(static_cast<void(TcpConnection::*)(const void*, size_t)>
                            (&TcpConnection::sendInLoop), this, data, len));
        }
    }
}

void TcpConnection::send(const std::string& buffer)
{
      send(buffer.data(), buffer.size());
}

void TcpConnection::send(ByteBuffer* buffer)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buffer->peek(), buffer->readableBytes());
            buffer->retrieveAll();
        }
        else
        {
            loop_->runInLoop(std::bind(static_cast<void(TcpConnection::*)(const std::string&)>(&TcpConnection::sendInLoop),
                shared_from_this(), buffer->retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& buffer)
{
    sendInLoop(buffer.data(), buffer.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    if (state_ == kDisconnected)
    {
       APP_FRAMEWORK_LOG_WARN << "[error] [fd]"<<  socket_->fd()<< " disconnected, give up writing!";
        return;
    }
    
    int nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        if(!tlsHandle_)
        {
            nwrote = socket_->send((const char*)data, len);
        }
        else
        {
            nwrote = SSL_write(tlsHandle_, data, len);
        }

        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                AIROS_ERRORPRINT("[nwrote]%d", nwrote)
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
//                LOG_ERROR("TcpConnection::sendInLoop error, fd[%d], error[%d]", socket_->fd(), errno);
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    AFL_ASSERT(remaining <= len)(remaining)(len)(socket_->fd());

    if(len > 65535)
    {
    	return;
    }


    if((outputBuffer_.readableBytes() + len) > 65535)
    {
        if(onServerDisconnectCallBack_)
        {
            onServerDisconnectCallBack_(false);
            outputBuffer_.retrieveAll();
        }

    	return;
    }

    if (!faultError && remaining > 0)
    {
        outputBuffer_.write(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        SocketUtil::shutdownWrite(socket_->fd());
    }
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    AFL_ASSERT(state_ == kConnecting)(state_);
    setState(kConnected);
    channel_->enableReading();

    TcpConnectionPtr sp_this(shared_from_this());
    connectionCallback_(sp_this);
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        TcpConnectionPtr sp_this(shared_from_this());
        connectionCallback_(sp_this);
    }
    channel_->disableAll();
    channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime)
{
    loop_->assertInLoopThread();
    size_t n = 0;
    if(!tlsHandle_)
    {
        std::string data;
        n = socket_->recv(data);
        inputBuffer_.write(data);
        if (n > 0)
        {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
        else if (n == 0)
        {
            handleClose();
        }
        else
        {
            handleError();
        }
    }else
    {
        afl::net::ByteBuffer buff;
        n = SSL_read(tlsHandle_, (void *)buff.peek(), buff.writableBytes());
        int nRes = SSL_get_error(tlsHandle_, n);
        if (nRes == SSL_ERROR_NONE)
        {
            buff.hasWritten(n);
            inputBuffer_.write(buff.peek(), buff.readableBytes());
        }
        else if (nRes == SSL_ERROR_WANT_READ)
        {
            return;
        }
        else
        {
            return;
        }

        if (n > 0)
        {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
            return;
        }
        else if (n == 0)
        {
            handleClose();
        }
        else
        {
            handleError();
        }

    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();

    if (channel_->isWriting())
    {
        if(!tlsHandle_)
        {
            size_t n = socket_->send(outputBuffer_.peek(), outputBuffer_.readableBytes());
            if (n > 0)
            {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                    channel_->disableWriting();
                    if (writeCompleteCallback_)
                    {
                        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                    }
                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop();
                    }
                }
            }
            else
            {
               APP_FRAMEWORK_LOG_WARN << "[error]send fail [fd]" << socket_->fd() <<" [state]" << getState(state_) << "[send]" <<  n;
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            size_t n = SSL_write(tlsHandle_, outputBuffer_.peek(), outputBuffer_.readableBytes());
            int nRes = SSL_get_error(tlsHandle_, n);
            if(nRes == SSL_ERROR_NONE)
            {
                outputBuffer_.retrieve(n);
                if (outputBuffer_.readableBytes() == 0)
                {
                    channel_->disableWriting();
                    if (writeCompleteCallback_)
                    {
                        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                    }
                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop();
                    }
                }
            }
            else
            {
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }

    }
    else
    {
       APP_FRAMEWORK_LOG_WARN << "[error]no more writing, [fd]"<< socket_->fd() << "[state]" <<  getState(state_);
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    AFL_ASSERT(state_ == kConnected || state_ == kDisconnecting)(state_)(socket_->fd());
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = SocketUtil::getSocketError(channel_->fd());
    err++;
    if(onServerDisconnectCallBack_)
    {
       APP_FRAMEWORK_LOG_WARN << "onServerDisconnectCallBack_!";
        onServerDisconnectCallBack_(false);
        outputBuffer_.retrieveAll();
    }
}
void TcpConnection::initTls(TlsCtxInfo tlsCtxInfo)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    tlsCtxInfo_ = tlsCtxInfo;
}
void TcpConnection::deinitTls()
{
    if(tlsHandle_)
    {
        SSL_shutdown (tlsHandle_);
        //shutdown(fd,2);
        SSL_free(tlsHandle_);
    }
    if(tlsCtx_)
    {
        SSL_CTX_free(tlsCtx_);
    }
}

bool TcpConnection::tlsConnect()
{
    tlsHandle_ = SSL_new(tlsCtx_);
    if (!tlsHandle_)
    {
       APP_FRAMEWORK_LOG_WARN << "[client][tlsHandle_] failed to create SSL object";
        return false;
    }

    if (SSL_set_fd(tlsHandle_, socket_->fd()) != 1)
    {
       APP_FRAMEWORK_LOG_WARN << "[error][tlsHandle_] " << tlsHandle_ << "[fd]" << socket_->fd() << " failed to set SSL fd";
        SSL_free(tlsHandle_);
        tlsHandle_ = nullptr;
        return false;
    }

    SSL_set_connect_state(tlsHandle_);

    int ret;
    while ((ret = SSL_connect(tlsHandle_)) == -1)
    {
        int sslError = SSL_get_error(tlsHandle_, ret);
        if (sslError == SSL_ERROR_WANT_WRITE || sslError == SSL_ERROR_WANT_READ)
        {
            continue;
        }
        else
        {
           APP_FRAMEWORK_LOG_WARN << "[error][tlsHandle_] " << tlsHandle_ << "[fd]" << socket_->fd() << "SSL_connect " <<"failed to set SSL " << "[error]" << sslError;
            SSL_CTX_free(tlsCtx_);
            SSL_free(tlsHandle_);
            tlsCtx_ = nullptr;
            tlsHandle_ = nullptr;
            return false;
        }
    }
    return true;
}
bool TcpConnection::tlsAccept()
{
    tlsHandle_ = SSL_new(tlsCtx_);
    if (!tlsHandle_)
    {
        LOG_ERROR<< "SSL_new error!";
        return false;
    }

    if (SSL_set_fd(tlsHandle_, socket_->fd()) != 1)
    {
       APP_FRAMEWORK_LOG_WARN << "SSL_set_fd error!";
        SSL_free(tlsHandle_);
        tlsHandle_ = nullptr;
        return false;
    }

    SSL_set_accept_state(tlsHandle_);

    int ret;
    while ((ret = SSL_accept(tlsHandle_)) <= 0)
    {
        int err = SSL_get_error(tlsHandle_, ret);
        switch (err)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                continue;
            default:
               APP_FRAMEWORK_LOG_WARN << "SSL_accept error:" <<  err;
                SSL_CTX_free(tlsCtx_);
                SSL_free(tlsHandle_);
                tlsCtx_ = nullptr;
                tlsHandle_ = nullptr;
                return false;
        }
    }

    return true;
}

int passwordCallBack(char *buf, int bufsize, int verify, void *cbdata)
{
    if (cbdata != nullptr)
    {
        size_t cbdata_len = strlen((const char *)cbdata);
        strncpy(buf, static_cast<const char*>(cbdata), std::min(cbdata_len, static_cast<size_t>(bufsize - 1)));
        buf[bufsize - 1] = '\0';
        return static_cast<int>(cbdata_len);
    }
    else
    {
       APP_FRAMEWORK_LOG_WARN << "[error]cbdata is NULL";
        return 0;
    }
}

bool TcpConnection::tlsVerify()
{
    tlsHandle_ = nullptr;
    tlsCtx_ = nullptr;
    if( (tlsCtx_ = SSL_CTX_new (SSLv23_method())) == nullptr)
    {
       APP_FRAMEWORK_LOG_WARN << "[error]SSL_CTX_new error!";
        return false;
    }
    // 设置 TLS 版本为 TLS 1.2 和 TLS 1.3
    if (SSL_CTX_set_min_proto_version(tlsCtx_, tlsCtxInfo_.minVersion) != 1)
    {
       APP_FRAMEWORK_LOG_WARN << "[error]Failed to set minimum TLS version to " << tlsCtxInfo_.minVersion;
        return false;
    }
    if (SSL_CTX_set_max_proto_version(tlsCtx_, tlsCtxInfo_.maxVersion) != 1)
    {
       APP_FRAMEWORK_LOG_WARN << "[error]Failed to set minimum TLS version to " << tlsCtxInfo_.maxVersion;
        return false;
    }
    bool existSslCaCertificateFile = afl::FileUtil::isFileExist(tlsCtxInfo_.tlsCaCertificateFile.c_str());
    if(tlsCtxInfo_.tlsClient)
    {
        bool existSslClientCertificateFile = afl::FileUtil::isFileExist(tlsCtxInfo_.tlsClientCertificateFile.c_str());
        bool existSslClientPrivateKeyFile =  afl::FileUtil::isFileExist(tlsCtxInfo_.tlsClientPrivateKeyFile.c_str());
        if(existSslCaCertificateFile && existSslClientCertificateFile&&existSslClientPrivateKeyFile)
        {
            bool ret = ctxBidirectionalVerify();
            if (!ret)
            {
               APP_FRAMEWORK_LOG_WARN << "[error]client ctxBidirectionalVerify error!";
                return false;
            }
        }
        else
        {
            std::string missingCertFile;
            if (!existSslCaCertificateFile)
            {
                missingCertFile = "SSL CA Certificate";
            }
            else if (!existSslClientCertificateFile)
            {
                missingCertFile = "TLS Server Certificate";
            }
            else if (!existSslClientPrivateKeyFile)
            {
                missingCertFile = "TLS Server Private Key";
            }
           APP_FRAMEWORK_LOG_WARN << "[error]client please add"  << missingCertFile.c_str() <<"file!";
            return false;
        }
    }
    else
    {

    }

    return true;
}

bool TcpConnection::ctxBidirectionalVerify()
{
    SSL_CTX_set_verify(tlsCtx_, SSL_VERIFY_NONE, nullptr);
    if(tlsCtxInfo_.tlsClient)
    {
        if(tlsCtxInfo_.tlsClientPrivateKeyPassword.empty())
        {
        }
        else
        {
            SSL_CTX_set_default_passwd_cb(tlsCtx_, passwordCallBack);
            SSL_CTX_set_default_passwd_cb_userdata(tlsCtx_, const_cast<char *>(tlsCtxInfo_.tlsClientPrivateKeyPassword.c_str()));
        }
        if (!SSL_CTX_load_verify_locations(tlsCtx_, tlsCtxInfo_.tlsCaCertificateFile.c_str(), nullptr))
        {
            SSL_CTX_free(tlsCtx_);
           APP_FRAMEWORK_LOG_WARN << "[error]client SSL_CTX_load_verify_locations error!" ;
            return false;
        }

        if (SSL_CTX_use_certificate_file(tlsCtx_, tlsCtxInfo_.tlsClientCertificateFile.c_str(), SSL_FILETYPE_PEM) <= 0)
        {
            SSL_CTX_free(tlsCtx_);
           APP_FRAMEWORK_LOG_WARN << "[error]client SSL_CTX_use_certificate_file error!";
            return false;
        }
        if (SSL_CTX_use_PrivateKey_file(tlsCtx_, tlsCtxInfo_.tlsClientPrivateKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0 )
        {
           APP_FRAMEWORK_LOG_WARN << "SSL_CTX_use_PrivateKey_file error!";
            SSL_CTX_free(tlsCtx_);
            return false;
        }
        if (!SSL_CTX_check_private_key(tlsCtx_))
        {
           APP_FRAMEWORK_LOG_WARN << "[error]client SSL_CTX_check_private_key error!";
            SSL_CTX_free(tlsCtx_);
            return false;
        }
        if (!tlsCtxInfo_.tlsClientPrivateKeyPassword.empty())
        {
            if (!loadPrivateKeyWithPassword())
            {
               APP_FRAMEWORK_LOG_WARN << "[error]client loadPrivateKeyWithPassword error!";
                SSL_CTX_free(tlsCtx_);
                return false;
            }
        }
        else
        {

        }
    }
    else
    {

    }

    return true;
}

bool TcpConnection::loadPrivateKeyWithPassword()
{
    BIO *bio = nullptr;
    EVP_PKEY *pkey = nullptr;
    if(tlsCtxInfo_.tlsClient)
    {
        bio = BIO_new_file(tlsCtxInfo_.tlsClientPrivateKeyFile.c_str(), "rb");
        if (bio == nullptr)
        {
           APP_FRAMEWORK_LOG_WARN << "[error]client BIO_new_file failed";
            return false;
        }

        pkey = PEM_read_bio_PrivateKey(bio, nullptr, passwordCallBack, (void *)tlsCtxInfo_.tlsClientPrivateKeyPassword.c_str());
        if (pkey == nullptr)
        {
           APP_FRAMEWORK_LOG_WARN << "[error]client failed to load private key";
            BIO_free(bio);
            return false;
        }

    }
    else
    {

    }

    if (SSL_CTX_use_PrivateKey(tlsCtx_, pkey) <= 0 || !SSL_CTX_check_private_key(tlsCtx_))
    {
       APP_FRAMEWORK_LOG_WARN << "[error]failed to set private key in SSL context";
        EVP_PKEY_free(pkey);
        BIO_free(bio);
        return false;
    }

    EVP_PKEY_free(pkey);
    BIO_free(bio);
    return true;
}
int TcpConnection::getState()
{
    return state_;
}
NAMESPACE_AFL_NET_END
