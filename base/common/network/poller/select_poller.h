/*********************************************************************************
 * @file		select_poller.h
 * @brief		select_poller belongs to CICTCI
 * @details
 * @author		cs
 * @date		2015-01-13
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2015-01-13 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _SELECT_POLLER_H
#define _SELECT_POLLER_H
#include "base/common/network/define.h"
#include "poller.h"
#include <set>
#include <sys/select.h>
#include "base/common/network/channel.h"
NAMESPACE_AFL_NET_START

class SelectPoller : public Poller
{
public:
    explicit SelectPoller(EventLoop* loop);

    ~SelectPoller();

public:
    virtual bool updateChannel(Channel* channel);

    virtual bool removeChannel(Channel* channel);

    virtual TimeStamp pollOnce(int timeoutMs, ChannelList& activeChannels);

    virtual const char* ioMultiplexerName() const
    {
        return "select";
    }

private:
    void fireActiveChannels(int numEvents, ChannelList& activeChannels) const;

private:
    fd_set readfds_;
    fd_set writefds_;
    fd_set exceptfds_;

    fd_set select_readfds_;
    fd_set select_writefds_;
    fd_set select_exceptfds_;

    std::set< int, std::greater<int> >  fdlist_;
};

NAMESPACE_AFL_NET_END
#endif  /* _SELECTPOLLER_H */
