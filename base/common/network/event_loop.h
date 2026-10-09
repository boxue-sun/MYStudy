/*********************************************************************************
 * @file		event_loop.h
 * @brief		event_loop belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-10-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-10-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H

#include "define.h"
#include "time_stamp.h"
#include "non_copy.h"
#include "mutex.h"
#include "thread.h"
#include "atomic.h"
#include "call_backs.h"
#include "non_copy.h"
NAMESPACE_AFL_NET_START

class Channel;
class Poller;
class Timer;
class TimerQueue;
class EventfdHandler;

class EventLoop : afl::base::NonCopy
{
public:
    typedef std::function<void()> Functor;
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

public:
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    void runInLoop(const Functor& func);

    void queueInLoop(const Functor& func);

    TimerId addTimer(const TimerCallback& cb, const TimeStamp& when);
    TimerId addTimer(const TimerCallback& cb, double delaySeconds, bool repeat = false);
    void    cancelTimer(TimerId id);

    bool isRunning()
    {
        return running_;
    }
    bool isInLoopThread() const
    {
        return currentThreadId_ == thread::this_thread::tid();
    }

    int getCurrentThreadId() const
    {
        return currentThreadId_;
    }

    void assertInLoopThread() const;

private:
    void wakeupPoller();          //wakeup the waiting poller
    void callPendingFunctors();   //call when loop() return

private:
    typedef std::vector<Channel*> ChannelList;

    const int                currentThreadId_;  // thread id of this object created

    ChannelList              activeChannels_;   // active channels when poll return
    Channel*                 currentActiveChannel_; // the current processing active channel
    Poller*                  poller_;          // I/O poller
    thread::Atomic<bool>     running_;          // status for eventloop running
    thread::Atomic<bool>     eventHandling_;    // status for active channel handling

    EventfdHandler*          wakeupfd_;        // wakeup poller::poll
    Channel*                 wakeupChannel_;   // channel of wakeupfd_

    thread::Atomic<bool>     callingPendingFunctors_;  // status for pending functors calling
    thread::Mutex            mutex_;            // for guard  pendingFunctors_
    std::vector<Functor>     pendingFunctors_;  // functors when polling, need mutex guard

    TimerQueue*              timerQueue_;

};

NAMESPACE_AFL_NET_END
#endif  /* _EVENTLOOP_H */
