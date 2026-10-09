/*********************************************************************************
 * @file		atomic.h
 * @brief		atomic belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-08-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-08-21 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _ATOMIC_H
#define _ATOMIC_H
#include "define.h"
#include <pthread.h>

#define ATOMIC_ADD(ptr, v)            __sync_add_and_fetch(ptr, v)
#define ATOMIC_SUB(ptr, v)            __sync_sub_and_fetch(ptr, v)
#define ATOMIC_ADD_AND_FETCH(ptr, v)  __sync_add_and_fetch(ptr, v)
#define ATOMIC_SUB_AND_FETCH(ptr, v)  __sync_sub_and_fetch(ptr, v)
#define ATOMIC_FETCH_AND_ADD(ptr, v)  __sync_fetch_and_add(ptr, v)
#define ATOMIC_FETCH_AND_SUB(ptr, v)  __sync_fetch_and_sub(ptr, v)
#define ATOMIC_FETCH(ptr)             __sync_add_and_fetch(ptr, 0)
#define ATOMIC_SET(ptr, v)            __sync_val_compare_and_swap(ptr, *(ptr), v)
#define ATOMIC_CAS(ptr, cmp, v)       __sync_bool_compare_and_swap(ptr, cmp, v)


NAMESPACE_AFL_THREAD_START

template <typename T>
class Atomic
{
public:
    Atomic()
    {
        memset((void*)&atomic_, 0x00, sizeof(atomic_));
        ATOMIC_SET(&atomic_, 0);
    }

public:
    void add(T n = 1)
    {
        ATOMIC_ADD(&atomic_, n);
    }

    T getAndAdd(T n)
    {
        return ATOMIC_FETCH_AND_ADD(&atomic_, n);
    }

    T addAndGet(T n)
    {
        //return ATOMIC_FETCH_AND_ADD(&atomic_, n) + n;   // This is OK!
        //return getAndAdd(n) + n;                        // This is OK!
#ifdef OS_LINUX
        return ATOMIC_ADD_AND_FETCH(&atomic_, n);
#else
        return ATOMIC_FETCH_AND_ADD(&atomic_, n) + n;
#endif
    }

    void sub(T n = 1)
    {
        ATOMIC_SUB(&atomic_, n);
    }

    T getAndSub(T n)
    {
        return ATOMIC_FETCH_AND_SUB(&atomic_, n);
    }

    T subAndGet(T n)
    {
        //return ATOMIC_FETCH_AND_SUB(&atomic_, n) - n;   // This is OK!
        //return getAndSub(n) - n;                        // This is OK!
#ifdef OS_LINUX
        return ATOMIC_SUB_AND_FETCH(&atomic_, n);
#else
        return ATOMIC_FETCH_AND_SUB(&atomic_, n) + n;
#endif
    }

    T increment()
    {
        return addAndGet(1);
    }

    T decrement()
    {
        return addAndGet(-1);
    }

    T value()
    {
        return ATOMIC_FETCH(&atomic_);
    }

public:
    T operator++()
    {
        return addAndGet(1);
    }

    T operator--()
    {
        return subAndGet(1);
    }

    T operator++(int)
    {
        return getAndAdd(1);
    }

    T operator--(int)
    {
        return getAndSub(1);
    }

    T operator+=(T n)
    {
        return addAndGet(n);
    }

    T operator-=(T n)
    {
        return subAndGet(n);
    }

    void operator=(T n)
    {
        ATOMIC_SET(&atomic_, n);
    }
    bool operator==(T n)
    {
        return (value() == n);
    }

    operator T()
    {
        return value();
    }

private:
    volatile T atomic_;
};

template<>
class Atomic<bool>
{
public:
    Atomic()
    {
        memset((void*)&atomic_, 0x00, sizeof(atomic_));
        ATOMIC_SET(&atomic_, 0);
    }

    Atomic(bool value)
    {
        memset((void*)&atomic_, 0x00, sizeof(atomic_));
        ATOMIC_SET(&atomic_, value ? 1 : 0);
    }

    Atomic& operator=(bool value)
    {
        ATOMIC_SET(&atomic_, value ? 1 : 0);
        return *this;
    }

    bool clear()     // set false
    {
        return ATOMIC_SET(&atomic_, 0);
    }

    bool test_and_set()  //set true and return old value
    {
        return ATOMIC_SET(&atomic_, 1);
    }

    bool value()
    {
        return ATOMIC_FETCH(&atomic_);
    }

    operator bool()
    {
        return ATOMIC_FETCH(&atomic_);
    }

private:
    volatile int  atomic_;
};


typedef Atomic<int32_t>     AtomicInt32;
typedef Atomic<int64_t>     AtomicInt64;
typedef Atomic<bool>        AtomicBool;

NAMESPACE_AFL_THREAD_END
#endif /* _ATOMIC_H */
