/*********************************************************************************
 * @file		semaphore.h
 * @brief		semaphore belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-05-16
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-05-16 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef AFL_NET_SEMAPHORE_H_
#define AFL_NET_SEMAPHORE_H_

#include "non_copy.h"
#include <exception>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>


int sem_init (sem_t *__sem, int __pshared, unsigned int __value) __attribute__ ((weak));
int sem_destroy (sem_t *__sem) __attribute__ ((weak));
int sem_wait (sem_t *__sem) __attribute__ ((weak));
int sem_post (sem_t *__sem) __attribute__ ((weak));

NAMESPACE_AFL_THREAD_START

class Semaphore : public afl::base::NonCopy
{
public:
    explicit Semaphore(int initialcount = 0, int maxcount = 0x7fffffff)
    {
        if(nullptr == sem_init)
        {
            fprintf(stderr, "Using Semaphore, but not used -lpthread link flag!");
            abort();
        }
        sem_init(&sem_, false, initialcount);
    }

    ~Semaphore()
    {
        sem_destroy(&sem_);
    }

public:
    bool wait()
    {
        // see http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
        int rc;
        do
        {
            rc = sem_wait(&sem_);
        }
        while (rc == -1 && errno == EINTR);
        return rc == 0;
    }

    bool wait(int64_t timeoutMs)
    {
        struct timespec ts;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int64_t usec = tv.tv_usec + timeoutMs * 1000LL;
        ts.tv_sec = tv.tv_sec + usec / 1000000;
        ts.tv_nsec = (usec % 1000000) * 1000;
        return sem_timedwait(&sem_, &ts) == 0;
    }

    bool try_wait()
    {
        return sem_trywait(&sem_) == 0;
    }

    bool post(long rc = 1)
    {
        while (rc-- > 0)
        {
            sem_post(&sem_);
        }
        return true;
    }

private:
    sem_t sem_;
};

NAMESPACE_AFL_THREAD_END
#endif  /* _SEMAPHORE_H */

