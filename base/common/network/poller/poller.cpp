/*********************************************************************************
 * @file		poller.cpp
 * @brief		poller belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-09-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-09-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#include "poller.h"
#include "epoll_poller.h"
#include "poll_poller.h"
#include "select_poller.h"
#include "base/common/network/channel.h"
NAMESPACE_AFL_NET_START

Poller::Poller(EventLoop* loop) : loop_(loop)
{
}

Poller::~Poller()
{
}

bool Poller::hasChannel(const Channel* channel) const
{
    ChannelMap::const_iterator itr = channelMap_.find(channel->fd());
    return itr != channelMap_.end() && itr->second == channel;
}

Channel* Poller::getChannel(AFL_SOCKET sock) const
{
    ChannelMap::const_iterator itr = channelMap_.find(sock);
    if (itr == channelMap_.end())
        return NULL;
    return itr->second;
}

/*static*/ Poller* Poller::createPoller(EventLoop* loop)
{
#if defined(USE_POLLER_EPOLL)
    return new EpollPoller(loop);
#elif defined(USE_POLLER_SELECT)
    return new SelectPoller(loop);
#elif defined(USE_POLLER_POLL)
    return new PollPoller(loop);
#else
    return NULL;
#endif
}

NAMESPACE_AFL_NET_END
