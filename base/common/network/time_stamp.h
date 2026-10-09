/*********************************************************************************
 * @file		time_stamp.h
 * @brief		time_stamp belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-01-24
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-01-24 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _TimeStamp_H
#define _TimeStamp_H
#include <string>
#include "define.h"
#include <stdio.h>
#include <time.h>
#  include <sys/time.h>
#define AFL_MSEC_PER_SEC   (1000)
#define AFL_USEC_PER_SEC   (1000 * 1000)
#define AFL_TIME_SEC(time) ((time) / AFL_USEC_PER_SEC)

NAMESPACE_AFL_UTIL_START

class TimeStamp
{
public:
    TimeStamp();
    explicit TimeStamp(int64_t ms);

public:

    /**
     *
     * @param isSystemTime  true return system time and false return the time starts from the moment the system starts,default use false
     * @return
     */
    static TimeStamp now(bool isSystemTime = false);
    static TimeStamp invalid();

    /// return seconds
    static double timeDiffS(const TimeStamp& end, const TimeStamp& start)
    {
        int64_t delta = end.microSeconds() - start.microSeconds();
        return AFL_TIME_SEC(delta * 1.0);
    }

    /// return mill seconds
    static int64_t timeDiffMS(const TimeStamp& end, const TimeStamp& start)
    {
        int64_t delta = end.microSeconds() - start.microSeconds();
        return (delta / AFL_MSEC_PER_SEC);
    }

    /// return micro seconds
    static int64_t timeDiffUS(const TimeStamp& end, const TimeStamp& start)
    {
        return end.microSeconds() - start.microSeconds();
    }

public:
    int64_t microSeconds() const
    {
        return microSeconds_;
    }

    int64_t millSeconds() const
    {
        return microSeconds_ / AFL_MSEC_PER_SEC;
    }

    int64_t seconds()     const
    {
        return microSeconds_ / AFL_USEC_PER_SEC;
    }

    bool valid() const
    {
        return microSeconds_ > 0;
    }

    void swap(TimeStamp& that)
    {
        std::swap(microSeconds_, that.microSeconds_);
    }

    struct tm getTm(bool showlocaltime = true) const;
    std::string toString(bool showlocaltime = true) const;

private:
    int64_t  microSeconds_;
};


inline std::ostream& operator<<(std::ostream& out, const TimeStamp& ts)
{
    out << ts.toString();
    return out;
}

inline bool operator<(const TimeStamp& lhs, const TimeStamp& rhs)
{
    return lhs.microSeconds() < rhs.microSeconds();
}

inline bool operator==(const TimeStamp& lhs, const TimeStamp& rhs)
{
    return lhs.microSeconds() == rhs.microSeconds();
}

/// return micro seconds
inline int64_t operator-(const TimeStamp& end, const TimeStamp& start)
{
    return end.microSeconds() - start.microSeconds();
}

inline TimeStamp operator+(const TimeStamp& lhs, double seconds)
{
    int64_t delta = seconds * AFL_USEC_PER_SEC;
    return TimeStamp(lhs.microSeconds() + delta);
}

inline TimeStamp operator+(double seconds, const TimeStamp& rhs)
{
    return (rhs + seconds);
}

inline TimeStamp operator+=(TimeStamp& lhs, double seconds)
{
    return lhs = lhs + seconds;
}

inline TimeStamp operator-(const TimeStamp& lhs, double seconds)
{
    return (lhs + (-seconds));
}

inline TimeStamp operator-=(TimeStamp& lhs, double seconds)
{
    return (lhs = (lhs + (-seconds)));
}

NAMESPACE_AFL_BASE_END

#endif  /* _TimeStamp_H */
