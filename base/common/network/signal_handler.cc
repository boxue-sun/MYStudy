/*
 * signal_handler.cpp
 *
 *  Created on: 2018年2月12日
 *      Author: cwy
 */

#include "signal_handler.h"

NAMESPACE_AFL_NET_START


SignalHandler::SignalHandler(afl::net::EventLoop& loop)
    : m_Loop(loop)
{
    // TODO Auto-generated constructor stub
    sigemptyset(&m_SigMask);

    m_Signalfd = signalfd(-1, &m_SigMask, SFD_NONBLOCK);
    m_SigChnl = new afl::net::Channel(&m_Loop, m_Signalfd);
    m_SigChnl->setReadCallback(std::bind(&SignalHandler::procSignals, this));

    m_Loop.runInLoop([=](){m_SigChnl->enableReading();});
}

SignalHandler::~SignalHandler()
{
    // TODO Auto-generated destructor stub
    close(m_Signalfd);
    m_SigHdls.clear();

    m_Loop.runInLoop([=](){m_SigChnl->disableAll(); m_SigChnl->remove(); SAFE_DELETE(m_SigChnl);});
}

bool SignalHandler::addSignal(int sigo, SignalCB cb)
{
    bool retb = true;
    afl::thread::Semaphore sema;

    m_Loop.runInLoop(
        [&]()
        {
            if(sigaddset(&m_SigMask, sigo) < 0
             || sigprocmask(SIG_BLOCK, &m_SigMask, NULL) < 0
             || signalfd(m_Signalfd, &m_SigMask, SFD_NONBLOCK) < 0)
                retb = false;
            else
                m_SigHdls[sigo] = cb;

            sema.post();
        }
    );

    sema.wait();
    return retb;
}

void SignalHandler::procSignals()
{
    struct signalfd_siginfo siginfo;
    int rlen = ::read(m_Signalfd, &siginfo, sizeof(siginfo));
    if(rlen != sizeof(siginfo))
    {
        perror("read signal error!\n");
        return;
    }

    auto hdl = m_SigHdls.find(siginfo.ssi_signo);
    if(hdl != m_SigHdls.end())
        hdl->second(siginfo);
}

NAMESPACE_AFL_NET_END
