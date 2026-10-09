/*********************************************************************************
 * @file		concurrent_queue.h
 * @brief		concurrent_queue belongs to CICTCI
 * @details
 * @author		cs
 * @date		2024-12-05
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
#ifndef _CONCURRENT_QUEUE_H
#define _CONCURRENT_QUEUE_H

#include "define.h"
#include "mutex.h"
#include <vector>
#include <queue>
#include <stack>


NAMESPACE_AFL_THREAD_START

namespace ConcurrentQueueTraits
{
    struct tagFIFO {};  /// first-in first-out
    struct tagFILO {};  /// first-in last-out
    struct tagPRIO {};  /// priority
}

template<typename T, typename Queue = std::queue<T>, typename Order = ConcurrentQueueTraits::tagFIFO>
struct ConcurrentQueue
{
public:
    typedef ConcurrentQueue<T, Queue, Order>    this_type;
    typedef Queue                               QueueType;
    typedef afl::thread::Mutex                   MutexType;
    typedef afl::thread::LockGuard<MutexType>    LockGuard;

    explicit ConcurrentQueue(size_t capacity = size_t(0))
            : stopFlag_(false)
            , capacity_(capacity)
            , mutex_()
    {
    }

    inline bool push(const T& v)
    {
        LockGuard lock(mutex_);
        if (stopFlag_) return false;
        if (capacity_ > 0 && queue_.size() >= capacity_) return false;
        queue_.push(v);
        return true;
    }

    inline bool push(T&& v)
    {
        LockGuard lock(mutex_);
        if (stopFlag_) return false;
        if (capacity_ > 0 && queue_.size() >= capacity_) return false;
        queue_.push(std::move(v));
        return true;
    }

    // std::back_inserter support
    inline void push_back(const T& data)
    {
        push(data);
    }

    inline void push_back(T&& data)
    {
        push(std::move(data));
    }

    /// Push some items with no more lock, return the remained iterator
    template<typename InIterator>
    inline InIterator push_some(InIterator first, InIterator last)
    {
        LockGuard lock(mutex_);
        if (stopFlag_)
        {
            return first;
        }
        for (; first != last && (capacity_ == 0 || queue_.size() < capacity_); ++first)
        {
            queue_.push(*first);
        }
        return first;
    }

    inline bool pop(T& v)
    {
        LockGuard lock(mutex_);
        if (stopFlag_ || queue_.empty()) return false;

        pop(v, Order());
        return true;
    }

    // Pop at most nelem items
    template<typename OutIterator>
    inline OutIterator pop_some(OutIterator oi, size_t count = size_t(-1))
    {
        LockGuard lock(mutex_);
        if (stopFlag_ || queue_.empty()) return false;

        count = std::min(queue_.size(), count);
        for (size_t i = 0; i < count; i++)
        {
            pop(*oi, Order());
            oi++;
        }
        return oi;
    }

    inline bool empty() const
    {
        LockGuard lock(mutex_);
        return queue_.empty();
    }

    inline bool full() const
    {
        LockGuard lock(mutex_);
        return (capacity_ > 0 && queue_.size() >= capacity_) ? true : false;
    }

    inline size_t size() const
    {
        LockGuard lock(mutex_);
        return queue_.size();
    }

    inline size_t capacity() const
    {
        return capacity_;
    }

    inline void stop()
    {
        LockGuard lock(mutex_);
        stopFlag_ = true;
    }

private:
    template <typename Tag>
    bool pop(T& v, Tag tag);

    //template <>
    bool pop(T& v, ConcurrentQueueTraits::tagFIFO tag)
    {
        if (queue_.empty())
            return false;
        v = queue_.front();
        queue_.pop();
        return true;
    }

    //template <>
    bool pop(T& v, ConcurrentQueueTraits::tagFILO tag)
    {
        if (queue_.empty())
            return false;
        v = queue_.top();
        queue_.pop();
        return true;
    }

    //template <>
    bool pop(T& v, ConcurrentQueueTraits::tagPRIO tag)
    {
        if (queue_.empty())
            return false;
        v = queue_.top();
        queue_.pop();
        return true;
    }

private:
    ConcurrentQueue(const ConcurrentQueue&) = delete;
    ConcurrentQueue(ConcurrentQueue&&) = delete;
    void operator=(const ConcurrentQueue&) = delete;

    bool stopFlag_;
    const size_t capacity_;
    mutable MutexType mutex_;
    QueueType queue_;
};

template<typename T, typename Queue = std::queue<T> >
using FifoConcurrentQueue = ConcurrentQueue<T, Queue, ConcurrentQueueTraits::tagFIFO>;

template<typename T, typename Queue = std::stack<T> >
using FiloConcurrentQueue = ConcurrentQueue<T, Queue, ConcurrentQueueTraits::tagFILO>;

template<typename T, typename Queue = std::priority_queue<T> >
using PrioConcurrentQueue = ConcurrentQueue<T, Queue, ConcurrentQueueTraits::tagPRIO>;

NAMESPACE_AFL_THREAD_END

#endif