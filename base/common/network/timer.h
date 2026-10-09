/*********************************************************************************
 * @file		timer.h
 * @brief		timer belongs to CICTCI
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
#ifndef _TIMER_H
#define _TIMER_H
#include "define.h"
#include "mutex.h"
#include "call_backs.h"
NAMESPACE_AFL_NET_START
class EventLoop;

class Timer
{
public:
    Timer(TimerId id, const TimerCallback& cb, const TimeStamp& when, double interval)
        : id_(id)
        , callback_(cb)
        , when_(when)
        , interval_(interval)
    {
    }

    TimerId id() const
    {
        return id_;
    }

    TimeStamp expires_at() const
    {
        return when_;
    }

    bool repeat() const
    {
        return interval_ > 0;
    }

    void trigger() const
    {
        callback_();
    }

    void restart(const TimeStamp& now)
    {
        if (repeat())
        {
            when_ = now + interval_;
        }
        else
        {
            when_ = TimeStamp::invalid();
        }
    }

private:
    TimerId       id_;
    TimerCallback callback_;
    TimeStamp     when_;
    double        interval_;  // second
};

inline bool operator<(const Timer& lhs, const Timer& rhs)
{
    return lhs.expires_at() < rhs.expires_at();
}

NAMESPACE_AFL_NET_END
#endif  /* _TIMER_H */
