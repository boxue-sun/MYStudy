/*********************************************************************************
 * @file		thread_loop.cpp
 * @brief		thread_loop belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-12-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-12-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#include "thread_loop.h"


int ThreadLoop::m_ThreadloopCnt = 0;

ThreadLoop::ThreadLoop(SCHED_POLICY policy, SCHED_PRIORITY priority, std::function<void()> func)
	: m_SchedPolicy(policy)
	, m_SchedPriority(priority)
	, m_Thread(nullptr)
    , m_Loop(nullptr)
    , m_Func(func)
{
	std::stringstream ss;
	switch(m_SchedPolicy)
	{
	case OTHER:
		ss << "ThreadLoop" << m_ThreadloopCnt++ << " Policy:SCHED_OTHER";
		break;

	case FIFO:
		ss << "ThreadLoop" << m_ThreadloopCnt++ << " Policy:SCHED_FIFO, Priority:" << priority;
		break;

	case RR:
		ss << "ThreadLoop" << m_ThreadloopCnt++ << " Policy:SCHED_RR, Priority:" << priority;
		break;

	default:
		ss << "ThreadLoop" << m_ThreadloopCnt++ << " Policy:unknown";
		break;
	}
	m_Name = ss.str();
}

ThreadLoop::~ThreadLoop()
{
}

void ThreadLoop::start()
{
	if(m_Thread == nullptr)
	{
		m_Thread = new afl::thread::Thread(std::bind(&ThreadLoop::createEventLoop, this),
		        m_Name, m_SchedPolicy, m_SchedPriority, 256 * 1024);
		m_Semaphore.wait();
	}
}

void ThreadLoop::stop()
{
    fprintf(stderr, "Thread Loop %s quit\n", this->getThreadName().c_str());
	m_Loop->quit();
	m_Thread->join();
	SAFE_DELETE(m_Thread);
}

static void ThreadTraceBack(int signo)
{
//    gLogW("Thread catch signal %d!", signo);

    std::stringstream ss;
    ss << "xds exit by signal " << signo;
    afl::util::Exception e(ss.str().c_str());
//    gLogE("%s", e.what());
//    gLogE("%s", e.stackTrace());
    _exit(signo);
}

void ThreadLoop::createEventLoop()
{
    afl::net::EventLoop eloop;
    afl::net::SignalHandler sigHdr(eloop);
    m_Loop = &eloop;

    if(m_Func)
        m_Func();
    else
    {
		std::set<int> sigTrace = 
			{SIGILL, SIGABRT, SIGBUS, SIGFPE, SIGSEGV, SIGSYS, SIGSTKFLT, 
			 SIGXCPU, SIGXFSZ};
		std::set<int> sigMonitor =
			{SIGHUP,  SIGUSR1, SIGUSR2, SIGPIPE, SIGTERM, SIGCHLD, SIGTSTP,
			 SIGTTIN, SIGTTOU, SIGIO,   SIGPWR };
		std::set<int> sigDefault = 
			{SIGINT,  SIGQUIT, SIGTRAP, SIGKILL, SIGALRM, SIGCONT, SIGSTOP,
			 SIGVTALRM, SIGPROF, SIGURG, SIGWINCH};

        for(auto sig : sigTrace)
        {
			if(sigDefault.find(sig) == sigDefault.end())
				signal(sig, ThreadTraceBack);
        }

        sigset_t mask;
        sigemptyset(&mask);
        for(auto sig : sigMonitor)
		{
			if(sigDefault.find(sig) == sigDefault.end())
				sigaddset(&mask, sig);
		}
        pthread_sigmask(SIG_BLOCK, &mask, NULL);

        signal(SIGIO,   SIG_IGN);
        signal(SIGPIPE, SIG_IGN);

    }

    m_Semaphore.post();
    eloop.loop();
}
