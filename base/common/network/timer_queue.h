/*********************************************************************************
 * @file		timer_queue.h
 * @brief		timer_queue belongs to CICTCI
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
#ifndef _TIMER_QUEUE_H
#define _TIMER_QUEUE_H

#include "define.h"
#include "mutex.h"
#include "atomic.h"
#include "timer.h"
#include "call_backs.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "timer.h"
#include "event_loop.h"
#include <limits.h>

using afl::util::TimeStamp;
NAMESPACE_AFL_NET_START

class Timer;
class EventLoop;

class TimerQueue
{
    typedef std::pair<TimeStamp, Timer*>         Entry;
    typedef std::set<Entry>                      TimerList;
    typedef std::unordered_map<TimerId, Timer*>  TimerMap;
    typedef std::unordered_set<TimerId>          CancelTimerList;

public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

public:
    TimerId addTimer(const TimerCallback& cb, const TimeStamp& when, double interval);
    void    cancelTimer(TimerId id);
    TimeStamp getNearestExpiration() const;
    void    runTimer(const TimeStamp& now);

private:
    void addTimerInLoop(Timer* timer);
    void cancelTimerInLoop(TimerId id);
    void addTimer(Timer* timer);
    std::vector<Entry> getExpiredTimers(const TimeStamp& now);

private:
    TimerList                timers_;
    TimerMap                 activeTimers_;
    CancelTimerList          cancelTimers_;

    EventLoop*                loop_;
    afl::thread::Atomic<int>  atomic_;
    afl::thread::Atomic<bool> callingTimesFunctor_;
};

NAMESPACE_AFL_NET_END
#endif  /* _TIMERQUEUE_H */
