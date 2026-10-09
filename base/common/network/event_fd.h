/*********************************************************************************
 * @file		event_fd.h
 * @brief		event_fd belongs to CICTCI
 * @details
 * @author		cs
 * @date		2015-01-14
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2015-01-14 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _EVENTFD_H
#define _EVENTFD_H

#include "define.h"

#include "socket_util.h"
NAMESPACE_AFL_NET_START

/**
 * @brief 事件文件描述符操作类
 *
 * 该类封装了eventfd的创建和读写操作
*/
class EventfdHandler
{
public:
    /**
     * @brief 构造函数
     *
     * @param initval 初始化值
     * @param flags 标志位，支持EFD_NONBLOCK和EFD_CLOEXEC
     */
    EventfdHandler(unsigned int initval = 0, int flags = EFD_NONBLOCK | EFD_CLOEXEC);

    /**
     * @brief 析构函数
     */
    ~EventfdHandler();

public:
    /**
     * @brief 获取文件描述符
     *
     * @return 文件描述符
     */
    int fd()
    {
        return eventfd_;
    }
    /**
     * @brief 通知其他线程/进程
     *
     * 通过写1来通知
     */
    void notify()
    {
        write(1);
    }
    /**
     * @brief 写操作
     *
     * @param value 写入的值
     * @return 实际写入字节数
     */
    ssize_t write(uint64_t value = 1);
    /**
     * @brief 读操作
     *
     * @param value 用于返回读出的值
     * @return 实际读出字节数
     */
    ssize_t read(uint64_t* value = NULL);

private:
    /// iff success, return eventfd_, else return -1;
    /**
     * @brief 创建事件文件描述符
     *
     * @param initval 初始化值
     * @param flags 标志位
     * @return 成功返回文件描述符，失败返回-1
     */
    int createEventfd(unsigned int initval, int flags);

private:
    int   eventfd_;  //< 事件文件描述符
};

NAMESPACE_AFL_NET_END
#endif  /* ZL_EVENTFD_H */
