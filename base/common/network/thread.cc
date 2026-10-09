/*********************************************************************************
 * @file		thread.cpp
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
#include "thread.h"


int pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) __attribute__ ((weak));

int pthread_attr_setstacksize(pthread_attr_t* __attr, size_t __stacksize) __attribute__ ((weak));

int pthread_join(pthread_t __th, void** __thread_return) __attribute__ ((weak));

int pthread_detach (pthread_t __th) __attribute__ ((weak));

NAMESPACE_AFL_THREAD_START
//#define DO_NOT_USE_TRY_CATCH
namespace detail
{
struct ThreadImplDataInfo
{
    typedef afl::thread::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;

    ThreadImplDataInfo(const ThreadFunc& func, const std::string& threadName)
        : func_(func)
        , name_(threadName)
    {

    }

    void runThread()
    {
#ifdef DO_NOT_USE_TRY_CATCH
        func_();
#else
        try
        {
            func_();
        }
        catch (const afl::util::Exception& ex)
        {
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            std::abort();
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            std::abort();
        }
        catch (...)
        {
            fprintf(stderr, "uncaught exception caught in Thread %s\n", name_.c_str());
            // Uncaught exceptions will terminate the application (default behavior according to C++11)
            std::terminate();
        }
#endif
    }
};

void* startThread(void* arg)
{
    ThreadImplDataInfo* data = static_cast<ThreadImplDataInfo*>(arg);
    data->runThread();
    delete data;
    return 0;
}
}

Thread::Thread(const ThreadFunc& func, const std::string& name/* = unknown*/, int policy /*= SCHED_OTHER*/, int priority/*= 0*/)
    : threadId_(0)
    //, threadFunc_(func)
    , threadName_(name)
    , notAThread_(true)
    , joined_(false)
{
    detail::ThreadImplDataInfo* data = new detail::ThreadImplDataInfo(func, name);
    // Create the thread


    if(nullptr == pthread_create)
    {
        fprintf(stderr, "Using afl::thread::thread, but not used -lpthread link flag!");
        abort();
    }

    struct sched_param param;
    pthread_attr_t pAttr;
    memset(&pAttr, 0x00, sizeof(pAttr));
	pthread_attr_init(&pAttr);
	pthread_attr_setscope(&pAttr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setstacksize(&pAttr, 128 * 1024);

	if(policy == SCHED_FIFO || policy == SCHED_RR)
	{
		pthread_attr_setschedpolicy(&pAttr, policy);
		pthread_attr_getschedparam(&pAttr, &param);

		priority = ((priority > sched_get_priority_max(policy)) || (priority < 0))? 0 : priority;
		param.sched_priority = (0 == geteuid()) ? (priority) : 0;
		pthread_attr_setschedparam(&pAttr, &param);
	}

	if (pthread_create(&threadId_, &pAttr, detail::startThread, data) != 0)
		threadId_ = 0;

    if (!threadId_)
    {
        delete data;
        std::abort();
    }
    notAThread_ = false;  // The thread is now alive
}

Thread::Thread(const ThreadFunc& func, const std::string& name/* = unknown*/, int policy /*= SCHED_OTHER*/, int priority/*= 0*/, int ssz)
    : threadId_(0)
    //, threadFunc_(func)
    , threadName_(name)
    , notAThread_(true)
    , joined_(false)
{
    detail::ThreadImplDataInfo* data = new detail::ThreadImplDataInfo(func, name);
    // Create the thread
    if(nullptr == pthread_create)
    {
        fprintf(stderr, "Using afl::thread::thread, but not used -lpthread link flag!");
        abort();
    }

    struct sched_param param;
    pthread_attr_t pAttr;
    memset(&pAttr, 0x00, sizeof(pAttr));
	pthread_attr_init(&pAttr);
	pthread_attr_setscope(&pAttr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setstacksize(&pAttr, ssz);

	if(policy == SCHED_FIFO || policy == SCHED_RR)
	{
		pthread_attr_setschedpolicy(&pAttr, policy);
		pthread_attr_getschedparam(&pAttr, &param);

		priority = ((priority > sched_get_priority_max(policy)) || (priority < 0))? 0 : priority;
		param.sched_priority = (0 == geteuid()) ? (priority) : 0;
		pthread_attr_setschedparam(&pAttr, &param);
	}

	if (pthread_create(&threadId_, &pAttr, detail::startThread, data) != 0)
		threadId_ = 0;

    if (!threadId_)
    {
        delete data;
        std::abort();
    }
    notAThread_ = false;  // The thread is now alive
}

Thread::~Thread()
{
    if (joinable())
        std::terminate();
}

bool Thread::joinable() const
{
    return !joined_ && !notAThread_;
}

void Thread::join()
{
    if (joinable())
    {
        pthread_join(threadId_, NULL);
        joined_ = true;
    }
}

void Thread::detach()
{
    // It's not use joinable(), so iff you call detach() more than once, then terimnate
    if (!joined_/*joinable()*/)
    {
        pthread_detach(threadId_);
        notAThread_ = true;
    }
}

Thread::id Thread::get_id() const
{
    if (!joinable())
        return id();
    return id(threadId_);
}

/*static*/ unsigned int Thread::hardware_concurrency()
{
#if defined(OS_WINDOWS)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int) si.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_ONLN)
    return (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
    return (int) sysconf(_SC_NPROC_ONLN);
#else
    // The standard requires this function to return zero if the number of
    // hardware cores could not be determined.
    return 0;
#endif
}

//------------------------------------------------------------------------------
// this_thread
//------------------------------------------------------------------------------
namespace this_thread
{
thread_local int g_currentTid = 0;

void cacheThreadTid()
{
    g_currentTid = static_cast<int>(::syscall(SYS_gettid));
}

int gettid()
{
    if (g_currentTid == 0)
    {
        cacheThreadTid();
    }
    return g_currentTid;
}

Thread::id get_id()
{
    return Thread::id(pthread_self());
}
}

NAMESPACE_AFL_THREAD_END
