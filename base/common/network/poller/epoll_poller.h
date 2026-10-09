/*********************************************************************************
 * @file		epoll_poller.h
 * @brief		epoll_poller belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-09-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *   cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _EPOLL_POLLER_H
#define _EPOLL_POLLER_H
#include "base/common/network/define.h"
#include "poller.h"
#include "base/common/network/socket.h"
#include "base/common/network/channel.h"
#include "base/common/network/smart_assert.h"
#include <string.h>
#include <sys/epoll.h>
struct epoll_event;
NAMESPACE_AFL_NET_START

class EpollPoller : public Poller
{
public:
    explicit EpollPoller(EventLoop* loop, bool enableET = false);

    ~EpollPoller();

public:
    virtual bool updateChannel(Channel* channel);

    virtual bool removeChannel(Channel* channel);

    virtual TimeStamp pollOnce(int timeoutMs, ChannelList& activeChannels);

    virtual const char* ioMultiplexerName() const
    {
        return "linux_epoll";
    }

private:
    bool update(Channel* channel, int operation);

    void fireActiveChannels(int numEvents, ChannelList& activeChannels) const;

private:
    typedef std::vector<struct epoll_event> EpollEventList;

    int  epollfd_;
    bool enableET_;
    EpollEventList events_;
};

NAMESPACE_AFL_NET_END
#endif  /* _EPOLLPOLLER_H */
