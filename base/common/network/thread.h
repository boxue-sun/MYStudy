/*********************************************************************************
 * @file		thread.h
 * @brief		thread belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-09-04
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-09-04 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _THREAD_H
#define _THREAD_H
#include "define.h"
#include "non_copy.h"
#include "mutex.h"
#include <string>
#include "exception.h"
#include <syscall.h>
NAMESPACE_AFL_THREAD_START

#include <unistd.h>
#include <errno.h>
typedef pthread_t native_thread_handle;

#if !defined(_TTHREAD_CPP11_) && !defined(thread_local)
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
#define thread_local __thread
#else
#define thread_local __declspec(thread)
#endif
#endif

class Thread : afl::base::NonCopy
{
public:
    class id;
    typedef std::function<void ()> ThreadFunc;

public:
    explicit Thread(const ThreadFunc& func, const std::string& name = "unknown", int policy = SCHED_OTHER, int priority = 0);

	Thread(const ThreadFunc& func, const std::string& name, int policy, int priority, int ssz);
    ~Thread();

public:
    void join();
    void detach();

    bool joinable() const;

    id get_id() const;  /// Return the thread ID of a thread object.

    native_thread_handle threadHandle() const
    {
        return threadId_;
    }

    const std::string& threadName() const
    {
        return threadName_;
    }

    static unsigned int hardware_concurrency();

private:
    friend struct ThreadImplDataInfo;
    native_thread_handle   threadId_;
    std::string            threadName_;
    bool                   notAThread_;     ///< True iff this object is not a thread of execution
    bool                   joined_;         ///< True iff this thread called join
};

/// Thread ID. Refer : http://tinythreadpp.bitsnbites.eu/
/// The thread ID is a unique identifier for each thread.
/// @see thread::get_id()
class Thread::id
{
public:
    /// The default constructed ID is that of thread without a thread of execution.
    id() : mId(0) {}

    id(unsigned long int aId) : mId(aId) {}

    id(const id& aId) : mId(aId.mId) {}

    inline id& operator=(const id& aId)
    {
        mId = aId.mId;
        return *this;
    }

    inline friend bool operator==(const id& aId1, const id& aId2)
    {
        return (aId1.mId == aId2.mId);
    }

    inline friend bool operator!=(const id& aId1, const id& aId2)
    {
        return (aId1.mId != aId2.mId);
    }

    inline friend bool operator<=(const id& aId1, const id& aId2)
    {
        return (aId1.mId <= aId2.mId);
    }

    inline friend bool operator<(const id& aId1, const id& aId2)
    {
        return (aId1.mId < aId2.mId);
    }

    inline friend bool operator>=(const id& aId1, const id& aId2)
    {
        return (aId1.mId >= aId2.mId);
    }

    inline friend bool operator>(const id& aId1, const id& aId2)
    {
        return (aId1.mId > aId2.mId);
    }

    inline friend std::ostream& operator <<(std::ostream& os, const id& obj)
    {
        os << obj.mId;
        return os;
    }

    unsigned long int value() const
    {
        return mId;
    }
private:
    unsigned long int mId;
};


// Related to <ratio> - minimal to be able to support chrono.
typedef long long __intmax_t;

/// Minimal implementation of the @c ratio class. This class provides enough
/// functionality to implement some basic @c chrono classes.
template <__intmax_t N, __intmax_t D = 1>
class ratio
{
public:
    static double _as_double()
    {
        return double(N) / double(D);
    }
};

/// Minimal implementation of the @c chrono namespace.
/// The @c chrono namespace provides types for specifying time intervals.
namespace chrono
{
/// Duration template class. This class provides enough functionality to
/// implement @c this_thread::sleep_for().
template < class _Rep, class _Period = ratio<1> >
class duration
{
private:
    _Rep rep_;
public:
    typedef _Rep rep;
    typedef _Period period;

    /// Construct a duration object with the given duration.
    template <class _Rep2>
    explicit duration(const _Rep2& r) : rep_(r) {}

    /// Return the value of the duration object.
    rep count() const
    {
        return rep_;
    }
};

// Standard duration types.
typedef duration<__intmax_t, ratio<1, 1000000000> >
nanoseconds; ///< Duration with the unit nanoseconds.
typedef duration<__intmax_t, ratio<1, 1000000> >
microseconds;   ///< Duration with the unit microseconds.
typedef duration<__intmax_t, ratio<1, 1000> >
milliseconds;      ///< Duration with the unit milliseconds.
typedef duration<__intmax_t>
seconds;                            ///< Duration with the unit seconds.
typedef duration<__intmax_t, ratio<60> >
minutes;                ///< Duration with the unit minutes.
typedef duration<__intmax_t, ratio<3600> > hours;                ///< Duration with the unit hours.
}

/// The namespace @c this_thread provides methods for dealing with the
/// calling thread.
namespace this_thread
{
/// Return the thread ID of the calling thread.
Thread::id get_id();

extern thread_local int g_currentTid;
void cacheThreadTid();

/// return thread id of kernel
inline int tid()
{
    if (g_currentTid == 0)
        cacheThreadTid();

    return g_currentTid;
}

/// Yield execution to another thread.
/// Offers the operating system the opportunity to schedule another thread
/// that is ready to run on the current processor.
inline void yield()
{
    sched_yield();
}

/// Blocks the calling thread for a period of time.
/// @param[in] aTime Minimum time to put the thread to sleep.
/// Example usage:
/// @code
/// // Sleep for 100 milliseconds
/// this_thread::sleep_for(chrono::milliseconds(100));
/// @endcode
/// @note Supported duration types are: nanoseconds, microseconds,
/// milliseconds, seconds, minutes and hours.
template <class _Rep, class _Period>
void sleep_for(const chrono::duration<_Rep, _Period>& aTime)
{
    while (usleep(int(double(aTime.count()) * (1000000.0 * _Period::_as_double()) + 0.5)) != 0
            && errno == EINTR);
}

inline void sleep(uint32_t millsecond)
{
    //#if defined(OS_WINDOWS)
    //    Sleep(millsecond);
    //#else
    //    usleep(millsecond / 1000);
    //#endif
    sleep_for(chrono::milliseconds(millsecond));
}
}

NAMESPACE_AFL_THREAD_END
#endif  /* _THREAD_H */
