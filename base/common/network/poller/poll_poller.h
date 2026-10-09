/*********************************************************************************
 * @file		poll_poller.h
 * @brief		poll_poller belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-12-22
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-12-22 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _POLLPOLLER_H
#define _POLLPOLLER_H
#include "poller.h"
#include <unordered_map>
#include "base/common/network/channel.h"
#include "base/common/network/define.h"
NAMESPACE_AFL_NET_START

class PollPoller : public Poller
{
public:
    explicit PollPoller(EventLoop* loop);

    ~PollPoller();

public:
    virtual bool updateChannel(Channel* channel);

    virtual bool removeChannel(Channel* channel);

    virtual TimeStamp pollOnce(int timeoutMs, ChannelList& activeChannels);

    virtual const char* ioMultiplexerName() const
    {
        return "linux_poll";
    }

private:
    void fireActiveChannels(int numEvents, ChannelList& activeChannels) const;

private:
    typedef std::vector<struct pollfd>         PollFdList;
    typedef std::unordered_map<Channel*, int>  ChannelIter;

    PollFdList    pollfds_;
    ChannelIter   channelIter_;
};

NAMESPACE_AFL_NET_END
#endif  /* _POLLPOLLER_H */
