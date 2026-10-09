#include "event_loop.h"
#include "time_stamp.h"
#include "thread.h"
#include "channel.h"
#include "poller/poller.h"
#include "timer_queue.h"
#include "event_fd.h"

#include <assert.h>
#include <sys/eventfd.h>     // for eventfd
#include <signal.h>          // for ::signal

using namespace afl::thread;
NAMESPACE_AFL_NET_START

namespace
{

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
} _dont_use_this_class_;
}

thread_local EventLoop* g_evloopInThisThread = 0;

EventLoop::EventLoop()
    : currentThreadId_(this_thread::tid())
    , currentActiveChannel_(NULL)
    , running_(false)
    , eventHandling_(false)
    , callingPendingFunctors_(false)
    , mutex_()
{
    assert(!g_evloopInThisThread);

    poller_ = Poller::createPoller(this);

    wakeupfd_ = new EventfdHandler();
    wakeupChannel_ = new Channel(this, wakeupfd_->fd());
    //wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->setReadCallback(std::bind(&EventfdHandler::read, wakeupfd_,
                                              static_cast<uint64_t*>(NULL)));
    wakeupChannel_->enableReading();  // ready for read event of wakeupfd_

    timerQueue_ = new TimerQueue(this);

    g_evloopInThisThread = this;
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    SAFE_DELETE(wakeupfd_);
    SAFE_DELETE(wakeupChannel_);
    SAFE_DELETE(poller_);
}

void EventLoop::loop()
{
    assertInLoopThread();
    running_ = true;

    TimeStamp now;
    while (running_)
    {
        activeChannels_.clear();

        int timeoutMs = 0;
        TimeStamp nextExpired = timerQueue_->getNearestExpiration();
        if (nextExpired.valid())
        {
            now = TimeStamp::now();
            double seconds = TimeStamp::timeDiffS(nextExpired, now);
//            LOG_INFO("nextExpired.valid() [%s][%s][%lf]", nextExpired.toString().c_str(), now.toString().c_str(), seconds);
            if (seconds <= 0)
                timeoutMs = 0;
            else
                timeoutMs = seconds * 1000;
        }
        else
        {
#if defined(POLL_WAIT_INDEFINITE)
            timeoutMs = -1;
#else
            timeoutMs = 0;
#endif
        }

        now = poller_->pollOnce(timeoutMs, activeChannels_);
//        LOG_INFO("EventLoop::loop [%s][%d]", now.toString().c_str(), activeChannels_.size());

        eventHandling_ = true;
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            currentActiveChannel_ = *it;
            currentActiveChannel_->handleEvent(now);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;

        timerQueue_->runTimer(now);

        callPendingFunctors();
    }
}

void EventLoop::quit()
{
    running_ = false;
    if (!isInLoopThread())
    {
        wakeupPoller();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if (eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
               std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::runInLoop(const Functor& func)
{
    if (isInLoopThread())
    {
        func();
    }
    else
    {
        queueInLoop(func);
    }
}

void EventLoop::queueInLoop(const Functor& func)
{
    {
        LockGuard<Mutex> lock(mutex_);
        pendingFunctors_.push_back(func);
    }

    if (!isInLoopThread() || !callingPendingFunctors_) // may be should wakeup at any time
    {
        wakeupPoller();
    }
}

TimerId EventLoop::addTimer(const TimerCallback& cb, const TimeStamp& when)
{
    return timerQueue_->addTimer(cb, when, 0);
}

TimerId EventLoop::addTimer(const TimerCallback& cb, double delaySeconds, bool repeat/* = false*/)
{
    TimeStamp when(TimeStamp::now());
    when += delaySeconds;
    return timerQueue_->addTimer(cb, when, repeat ? delaySeconds : 0);
}

void EventLoop::cancelTimer(TimerId id)
{
    timerQueue_->cancelTimer(id);
}

void EventLoop::callPendingFunctors()
{
    std::vector<Functor> tmp_functors;
    callingPendingFunctors_ = true;
    {
        LockGuard<Mutex> lock(mutex_);
        tmp_functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < tmp_functors.size(); ++i)
    {
        tmp_functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::assertInLoopThread() const
{
    if (!isInLoopThread())
    {
        assert("EventLoop::assertInLoopThread()" && 0);
    }
}

void EventLoop::wakeupPoller()
{
//    LOG_INFO("EventLoop::wakeupPoller()");
    uint64_t value = 1;
    ssize_t n = wakeupfd_->write(value);
    if (n != sizeof(value))
    {
//        LOG_ERROR("EventLoop::wakeupPoller() wakeupfd_ write error[%d][%d]", n, errno);
    }
}

NAMESPACE_AFL_NET_END
