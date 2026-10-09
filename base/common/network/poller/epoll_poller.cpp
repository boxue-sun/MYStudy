/*********************************************************************************
 * @file		epoll_poller.cpp
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
 *  2014-09-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#include "epoll_poller.h"

NAMESPACE_AFL_NET_START

AFL_STATIC_ASSERT(EPOLLIN == POLLIN, "must equal");
AFL_STATIC_ASSERT(EPOLLPRI == POLLPRI, "must equal");
AFL_STATIC_ASSERT(EPOLLOUT == POLLOUT, "must equal");
AFL_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP, "must equal");
AFL_STATIC_ASSERT(EPOLLERR == POLLERR, "must equal");
AFL_STATIC_ASSERT(EPOLLHUP == POLLHUP, "must equal");

EpollPoller::EpollPoller(EventLoop* loop, bool enableET/* = false*/)
    : Poller(loop)
    , enableET_(enableET)
    , events_(64)
{
    epollfd_ = epoll_create(1024);
    assert(epollfd_ > 0 && " epoll create failure!");
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

bool EpollPoller::updateChannel(Channel* channel)
{
    AFL_SOCKET fd = channel->fd();
//    LOG_INFO("EpollPoller::updateChannel[%d]", fd);
    if (hasChannel(channel))   //exist, update
    {
        assert(getChannel(fd) == channel);
        if (channel->isNoneEvent())
        {
//            LOG_INFO("EpollPoller::updateChannel [%d][%0x] NoneEvent", fd, channel);
            channelMap_.erase(fd);
            return update(channel, EPOLL_CTL_DEL);
        }
        else
        {
            return update(channel, EPOLL_CTL_MOD);
        }
    }
    else                       //new, add
    {
        assert(getChannel(fd) == NULL);
        channelMap_[fd] = channel;
        return update(channel, EPOLL_CTL_ADD);
    }
}

bool EpollPoller::removeChannel(Channel* channel)
{
    if (!hasChannel(channel))  // 娉ㄦ剰 updateChannel 鍑芥暟涓篃鏈変竴澶剅emoveChannel鐨勯�昏緫
        return true;

    AFL_SOCKET fd = channel->fd();
//    LOG_INFO("EpollPoller::removeChannel [%d][%0x]", fd, channel);
    assert(hasChannel(channel) && "the remove socket must be already exist");
    assert(getChannel(fd) == channel && "the remove socket must be already exist");
    assert(channel->isNoneEvent());
    size_t n = channelMap_.erase(fd);
    AFL_UNUSED(n);
    assert(n == 1);

    return update(channel, EPOLL_CTL_DEL);
}

bool EpollPoller::update(Channel* channel, int operation)
{
    AFL_SOCKET fd = channel->fd();
    struct epoll_event ev = { 0, { 0 } };
    //ev.events = channel->events();
    int events = channel->events();
    if (events & FDEVENT_IN)  ev.events |= EPOLLIN;
    if (events & FDEVENT_OUT) ev.events |= EPOLLOUT;
    ev.events |= EPOLLERR | EPOLLHUP;

    if (enableET_) ev.events |= EPOLLET;

    ev.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &ev) < 0)
    {
//        LOG_CRITICA("EpollPoller::update error, [socket %d][op %d]", fd, operation);
        return false;
    }
    return true;
}

TimeStamp EpollPoller::pollOnce(int timeoutMs, ChannelList& activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    TimeStamp now(TimeStamp::now());
    if (numEvents > 0)
    {
//        LOG_DEBUG("EpollPoller::pollOnce: [%d] events happended", numEvents);
        fireActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
//        LOG_DEBUG("EpollPoller::pollOnce: nothing happended");
    }
    else
    {
        // error happens, log uncommon ones
        // TODO : should return -1 if EINTR, else return 0
        if (savedErrno != SOCK_ERR_EINTR)
        {
            errno = savedErrno;
//            LOG_DEBUG("EpollPoller::pollOnce: error [%d]", savedErrno);
        }
    }

    return now;
}

void EpollPoller::fireActiveChannels(int numEvents, ChannelList& activeChannels) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        assert(hasChannel(channel) && "the channel must be already exist");

        //channel->set_revents(events_[i].events);
        int revents = FDEVENT_NONE;
        if (events_[i].events & EPOLLIN)    revents |= FDEVENT_IN;
        if (events_[i].events & EPOLLPRI)   revents |= FDEVENT_PRI;
        if (events_[i].events & EPOLLOUT)   revents |= FDEVENT_OUT;
        if (events_[i].events & EPOLLERR)   revents |= FDEVENT_ERR;
        if (events_[i].events & EPOLLHUP)   revents |= FDEVENT_HUP;
        channel->set_revents(revents);

        activeChannels.push_back(channel);
    }
}

NAMESPACE_AFL_NET_END
