#include "tcp_acceptor.h"
#include "socket.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "exception.h"

NAMESPACE_AFL_NET_START

TcpAcceptor::TcpAcceptor(EventLoop *loop, const InetAddress& listenAddr)
    : loop_(loop)
{
    accept_socket = new Socket(SocketUtil::createSocket());

    accept_socket->setNoDelay();
    accept_socket->setNonBlocking();

    if (!accept_socket->setReuseAddr(true))
    {
        throw afl::util::Exception("Could not reuse socket address.");
    }
    if (!accept_socket->bind(listenAddr))
    {
        throw afl::util::Exception("Could not bind to port.");
    }

    accept_channel_ = new Channel(loop, accept_socket->fd());
    accept_channel_->setReadCallback(std::bind(&TcpAcceptor::onAccept, this, std::placeholders::_1));
}

TcpAcceptor::~TcpAcceptor()
{
    accept_channel_->disableAll();
    accept_channel_->remove();
    SAFE_DELETE(accept_channel_);
    SAFE_DELETE(accept_socket);
}

void TcpAcceptor::listen()
{
    loop_->assertInLoopThread();
    if (!accept_socket->listen(128)) //may be bigger, see 'cat /proc/sys/net/core/somaxconn'
    {
        throw afl::util::Exception("Could not listen to port.");
    }
//    LOG_INFO("TcpAcceptor::listen on [%s]", SocketUtil::getLocalIpPort(accept_socket->fd()).c_str());

    accept_channel_->enableReading();
}

void TcpAcceptor::onAccept(TimeStamp now)
{
    loop_->assertInLoopThread();
    int count = 0;
    while(count < 100)
    {
        InetAddress peerAddr;
        AFL_SOCKET newfd = accept_socket->accept(&peerAddr);
        if(newfd > 0)
        {
            if (newConnCallBack_)
            {  
//                LOG_INFO("TcpAcceptor::OnAccept accept one client from[%d][%s]", newfd, peerAddr.ipPort().c_str());
                newConnCallBack_(newfd, peerAddr);
            }
            else
            {
//                LOG_ALERT("TcpAcceptor::OnAccept() no callback , and close the coming connection![%d]", newfd);
                SocketUtil::closeSocket(newfd);
            }
            count++;
        }
        else
        {
            if(AFL_SOCKET_ERROR == SOCK_ERR_EAGAIN || AFL_SOCKET_ERROR == SOCK_ERR_EWOULDBLOCK)
            {
                //We have processed all incoming  connections.
            }
            else if(AFL_SOCKET_ERROR == SOCK_ERR_EMFILE)
            {
                // TODO 姝ゆ椂鍥犱负杈惧埌月澶ф枃浠舵弿杩扮鑰屾帴鏀跺け璐ワ紝鍥犱负poller浣跨敤鐨勬槸姘村钩瑙﹀彂妯″紡锛�
                // 浼氬鑷磒oller鎸佺画閫氱煡鍙浜嬩欢锛屽洜姝ら�犳垚acceptor棰戠箒鍘籥ccept锛岀洿鑷宠繘绋嬩腑鍏抽棴浜�
                // 鍏朵粬杩炴帴鑰屾湁绌轰綑鎻忚堪绗︽墠鍋滄銆傝繖鏍蜂細瀵艰嚧CPU 100% loop銆�
                // 瑙ｅ喅鏂规瑙� 锛� http://blog.csdn.net/solstice/article/details/6365666
                // http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_accept_ing_wh
            }
            else
            {
//                LOG_ALERT("TcpAcceptor::OnAccept() accept connection error![%d][%d]", newfd, errno);
            }
            break;
        }
    }
}

NAMESPACE_AFL_NET_END
