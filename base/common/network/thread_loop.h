/*********************************************************************************
 * @file		thread_loop.h
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

#ifndef THREADLOOP_H_
#define THREADLOOP_H_

#include "cs_singleton.h"
#include "event_loop.h"
#include "semaphore.h"
#include "signal_handler.h"
#include <sstream>
#include "exception.h"
#include <sys/signal.h>
class ThreadLoop
{
public:
	enum SCHED_POLICY
	{
		OTHER = SCHED_OTHER,
		FIFO = SCHED_FIFO,
		RR   = SCHED_RR,
	};
	using SCHED_PRIORITY = int;

public:
	ThreadLoop(SCHED_POLICY policy = OTHER, SCHED_PRIORITY priority = 0, std::function<void()> func = nullptr);
	virtual ~ThreadLoop();

	virtual void start();
	virtual void stop();

	virtual afl::net::EventLoop* getEventLoop()
	{
		return m_Loop;
	}

	const std::string& getThreadName() const
	{
	    return m_Name;
	}

private:
	void createEventLoop();
	void procSignals(afl::net::SignalHandler::SigInfo& info);

private:
	const SCHED_POLICY   m_SchedPolicy;
	const SCHED_PRIORITY m_SchedPriority;

	afl::thread::Thread* m_Thread;
	afl::net::EventLoop* m_Loop;
	std::string m_Name;
	std::function<void()> m_Func;

	afl::thread::Semaphore m_Semaphore;

	static int m_ThreadloopCnt;
};

class NormalThreadLoop : public ThreadLoop, afl::base::Singleton<NormalThreadLoop>
{
	DECLARE_SINGLETON_CLASS(NormalThreadLoop);

public:
	NormalThreadLoop() : ThreadLoop()
	{}
};

class RRThreadLoop : public ThreadLoop, afl::base::Singleton<RRThreadLoop>
{
	DECLARE_SINGLETON_CLASS(RRThreadLoop);

public:
	RRThreadLoop() : ThreadLoop(ThreadLoop::RR, 95)
	{}
};

class FIFOThreadLoop : public ThreadLoop, afl::base::Singleton<FIFOThreadLoop>
{
	DECLARE_SINGLETON_CLASS(FIFOThreadLoop);

public:
	FIFOThreadLoop() :  ThreadLoop(ThreadLoop::FIFO, 95)
	{}
};


#endif /* THREADLOOP_H_ */
