/*********************************************************************************
 * @file		event_fd.h
 * @brief		event_fd belongs to CICTCI
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
#include "event_fd.h"

NAMESPACE_AFL_NET_START
/**
 * @brief EventfdHandler构造函数
 *
 * 构造一个EventfdHandler对象，创建一个eventfd文件描述符
 *
 * 1. 初始化eventfd_文件描述符为-1
 * 2. 调用createEventfd方法创建eventfd文件描述符
 * 3. 使用eventfd_initval初始化值和eventfd_flags标志作为参数
 *
 * @param initval eventfd初始化值，默认为0
 * @param flags eventfd标志，默认为EFD_NONBLOCK | EFD_CLOEXEC
 */
EventfdHandler::EventfdHandler(unsigned int initval/* = 0 */, int flags/* = EFD_NONBLOCK | EFD_CLOEXEC */)
{
    /**
     * 函数内部流程：
     *
     * 1. 定义eventfd_文件描述符变量
     * 2. 初始化eventfd_文件描述符为-1
     * 3. 调用createEventfd方法创建eventfd文件描述符
     * 4. 使用initval和flags作为createEventfd的参数
     *
     * 使用的C++知识：
     *
     * 1. 类构造函数
     * 2. 文件描述符
     * 3. eventfd系统调用
     * 4. 默认参数
     * 5. C++注释
     */
    eventfd_ = -1;
    createEventfd(initval, flags);
}
/**
 * @brief EventfdHandler的析构函数
 *
 * 此函数负责在EventfdHandler对象销毁时关闭与之关联的eventfd文件描述符
 *

*/
EventfdHandler::~EventfdHandler()
{
	/**
	 * 1. 检查eventfd_文件描述符是否为-1,若为-1则无需关闭
	 * 2. 否则使用close系统调用关闭eventfd_文件描述符
	 * 3. 将eventfd_文件描述符设置为-1,表示已关闭
	 *
	 * 使用的C++知识：
	 * 1. 析构函数定义
	 * 2. if语句
	 * 3. close系统调用
	 * 4. 文件描述符
	 * 5. eventfd机制
	 */
    if (eventfd_ != -1)
    {
        ::close(eventfd_);
        eventfd_ = -1;
    }
}

int EventfdHandler::createEventfd(unsigned int initval, int flags)
{
    int efd = ::eventfd(initval, flags);
    if (efd < 0)
    {
//        LOG_ALERT("create eventfd failed in EventfdHandler::createEventfd()");
        return efd;
    }
//    LOG_INFO("EventfdHandler::createEventfd [%d]", efd);

    eventfd_ = efd;
    return efd;
}

ssize_t EventfdHandler::write(uint64_t value/* = 1 */)
{
    ssize_t n = ::write(eventfd_, &value, sizeof(value));
    if (n != sizeof(value))  // just write one uint64_t
    {
//        LOG_ERROR("EventfdHandler::write(): write error[%d][%d][%d]", eventfd_, n, errno);
    }
    return n;
}

ssize_t EventfdHandler::read(uint64_t* value/* = NULL*/)
{
    ssize_t n;
    if (value == NULL)
    {
        uint64_t tmp;
        n = ::read(eventfd_, &tmp, sizeof(uint64_t));
    }
    else
    {
        n = ::read(eventfd_, value, sizeof(uint64_t));
    }

    if (n != sizeof(uint64_t)) //always return 8 byte
    {
//        LOG_ERROR("EventfdHandler::read(): read error[%d][%d][%d][%s]", eventfd_, n, errno, strerror(errno));
    }

    return n;
}

NAMESPACE_AFL_NET_END
