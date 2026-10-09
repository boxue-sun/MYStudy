/*********************************************************************************
 * @file		poller.h
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
#ifndef _POLLER_H
#define _POLLER_H
#include <vector>
#include "base/common/network/define.h"
#include "base/common/network/socket_util.h"
#include "base/common/network/time_stamp.h"

NAMESPACE_AFL_NET_START

#define POLL_WAIT_INDEFINITE
#define USE_POLLER_EPOLL
#define USE_POLLER_SELECT
#define USE_POLLER_POLL


using afl::util::TimeStamp;
class Socket;
class Channel;
class EventLoop;

class Poller
{
public:
    typedef std::vector<Channel*>           ChannelList;
    typedef std::map<AFL_SOCKET, Channel*>  ChannelMap;

public:
    explicit Poller(EventLoop* loop);
    virtual ~Poller();
    static Poller* createPoller(EventLoop* loop);

public:
    virtual bool updateChannel(Channel* channel) = 0;

    virtual bool removeChannel(Channel* channel) = 0;

    virtual TimeStamp pollOnce(int timeoutMs, ChannelList& activeChannels) = 0;

    virtual const char* ioMultiplexerName() const = 0;

public:
    virtual bool hasChannel(const Channel* channel) const;
    virtual Channel* getChannel(AFL_SOCKET sock) const;

protected:
    ChannelMap  channelMap_;
    EventLoop*  loop_;
};

NAMESPACE_AFL_NET_END
#endif  /* _POLLER_H */
