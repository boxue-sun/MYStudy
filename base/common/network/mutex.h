/*********************************************************************************
 * @file		mutex.h
 * @brief		mutex belongs to CICTCI
 * @details
 * @author		cs
 * @date		2024-01-24
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024-01-24 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef BASE_COMMON_MUTEX_H_
#define BASE_COMMON_MUTEX_H_

#include "define.h"

#include <unistd.h>
#include <pthread.h>
#include <errno.h>

NAMESPACE_AFL_THREAD_START

#ifdef NDEBUG

#define THREAD_CHECK(func) \
do{ \
    int errnum = (func);       \
    if(errnum != 0)            \
    fprintf(stderr, "%s:%d : [%d]\n", __FILE__, __LINE__, errnum);  \
}while(0)

#else

#define THREAD_CHECK(func)  \
do { \
    int errnum = (func);       \
    if(errnum != 0)            \
    {                          \
        fprintf(stderr, "%s:%d : [%d]\n", __FILE__, __LINE__, errnum); \
        assert(errnum == 0);   \
    }                          \
}while(0)

#endif

class NullMutex
{
    DISALLOW_COPY_AND_ASSIGN(NullMutex);
public:
    NullMutex()
    {
    }
    ~NullMutex()
    {
    }

public:
    void lock()
    {
    }

    bool try_lock()
    {
        return true;
    }

    void unlock()
    {
    }
};

class SpinMutex
{
public:
    SpinMutex()
    {
        lock_ = 0;
    }

    ~SpinMutex()
    {
    }

    void lock()
    {
        while (__sync_lock_test_and_set(&lock_, 1))
        {
        }
    }

    void unlock()
    {
        __sync_lock_release(&lock_);
    }

private:
    volatile int lock_;
};

class Mutex
{
    DISALLOW_COPY_AND_ASSIGN(Mutex);
public:
    Mutex()
    {
        pthread_mutex_init(&mutex_, NULL);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&mutex_);
    }

public:
    void lock()
    {
        THREAD_CHECK(pthread_mutex_lock(&mutex_));
    }

    bool try_lock()
    {
        return pthread_mutex_trylock(&mutex_) == 0;
    }

    void unlock()
    {
        THREAD_CHECK(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t* getMutex()
    {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
};


class RecursiveMutex
{
    DISALLOW_COPY_AND_ASSIGN(RecursiveMutex);
public:
    RecursiveMutex()
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mutex_, &attr);
    }

    ~RecursiveMutex()
    {
        pthread_mutex_destroy(&mutex_);
    }

    void lock()
    {
        THREAD_CHECK(pthread_mutex_lock(&mutex_));
    }

    bool try_lock()
    {
        return pthread_mutex_trylock(&mutex_) == 0;
    }

    void unlock()
    {
        THREAD_CHECK(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t* getMutex()
    {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
};

template <class MutexType>
class LockGuard
{
    DISALLOW_COPY_AND_ASSIGN(LockGuard);
public:
    explicit LockGuard(MutexType& mutex) : mutex_(mutex)
    {
        mutex_.lock();
    }
    ~LockGuard()
    {
        mutex_.unlock();
    }
private:
    MutexType& mutex_;
};

template <class MutexType>
class MutexTryLockGuard
{
    DISALLOW_COPY_AND_ASSIGN(MutexTryLockGuard);
public:
    explicit MutexTryLockGuard(MutexType& mutex) : mutex_(mutex)
    {
        isLocked_ = mutex_.try_lock();
    }
    ~MutexTryLockGuard()
    {
        if (isLocked_)
            mutex_.unlock();
    }
    bool IsLocked()
    {
        return isLocked_;
    }

private:
    bool  isLocked_;
    MutexType& mutex_;
};

NAMESPACE_AFL_THREAD_END

#endif  /* _MUTEX_H */
