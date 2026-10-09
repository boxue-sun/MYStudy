/*********************************************************************************
 * @file		signal_handler.h
 * @brief		signal_handler belongs to CICTCI
 * @details
 * @author		cwy
 * @date		2018-2-12
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2018-2-12 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef NETWORK_SIGNAL_HANDLER_H_
#define NETWORK_SIGNAL_HANDLER_H_

#include "define.h"
#include "call_backs.h"
#include "event_loop.h"
#include "channel.h"
#include <sys/signalfd.h>
#include <unordered_map>
#include <signal.h>
#include <sys/epoll.h>
#include "channel.h"
#include "semaphore.h"
#include <signal.h>
#include <sys/signal.h>
#include <assert.h>
NAMESPACE_AFL_NET_START

class SignalHandler
{
public:
    using SigInfo = struct signalfd_siginfo;
    using SignalCB = std::function<void(SigInfo&)>;

private:
    using SignalHandlers = std::unordered_map<int, SignalCB>;

public:
    SignalHandler(afl::net::EventLoop& loop);
    virtual ~SignalHandler();

    bool addSignal(int sigo,  SignalCB cb);

private:
    void procSignals();

private:
    afl::net::EventLoop& m_Loop;
    afl::net::Channel*   m_SigChnl;
    int                  m_Signalfd;
    sigset_t             m_SigMask;
    SignalHandlers       m_SigHdls;
};


NAMESPACE_AFL_NET_END

#endif /* NETWORK_SIGNAL_HANDLER_H_ */
