/*********************************************************************************
 * @file		channel.h
 * @brief		channel belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-10-26
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-10-26 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _CHANNEL_H
#define _CHANNEL_H
#include "define.h"
#include "socket_util.h"
#include "time_stamp.h"
#include "non_copy.h"
#include "event_loop.h"
#include "non_copy.h"
NAMESPACE_AFL_NET_START

class EventLoop;
using afl::util::TimeStamp;
/**
 * @brief 事件类型常量定义
 *
 * 定义了可以被poll监听的事件类型常量，这些位可以设置在events中表示感兴趣的事件类型;
 * 它们将出现在revents中表示文件描述符的状态。
 */
// Event types that can be polled for.  These bits may be set in `events'
// to indicate the interesting event types; they will appear in `revents'
// to indicate the status of the file descriptor.  */

/**
 * @brief 无事件
 * @note 没有任何事件发生
 */
#define FDEVENT_NONE    0x000       /* nothing */

// these are the POLL* values from <bits/poll.h> (linux poll)
/**
 * @brief 读事件
 * @note 有数据可以读取
 */
#define FDEVENT_IN      0x001       /* There is data to read.  */

/**
 * @brief 高优先级读事件
 * @note 有紧急数据可以读取
 */
#define FDEVENT_PRI     0x002       /* There is urgent data to read.  */
#define FDEVENT_OUT     0x004       /* Writing now will not block.  */

// Event types always implicitly polled for.  These bits need not be set in `events',
// but they will appear in `revents' to indicate the status of the file descriptor.
#define FDEVENT_ERR     0x008       /* Error condition.  */

/**
 * @brief 写事件
 * @note 写操作不会阻塞
 */
#define FDEVENT_HUP     0x010       /* Hung up.  */

/**
 * @brief 错误事件
 * @note 发生错误条件
 * @warning 必须检查错误事件
 */
#define FDEVENT_NVAL    0x020       /* Invalid polling request.  */


/**
 * @brief 挂起事件
 * @note 连接中断
 */
#define FDEVENT_RDHUP   0x2000      /* gnu extendsion */

/**
 * 枚举事件类型
*/
enum
{
    kEventNone    = FDEVENT_NONE,  //< 无事件
    kEventRead    = FDEVENT_IN | FDEVENT_PRI,  //< 读事件，优先级事件
    kEventWrite   = FDEVENT_OUT,  //< 写事件
    kEventError   = FDEVENT_ERR  //< 错误事件
};
/**
* @brief Channel类，用于描述一个IO事件的Channel
*/

class Channel : afl::base::NonCopy
{
public:
    /**
     * @brief 事件回调函数类型定义
     * @param void() 回调函数无参数
     */
    typedef std::function<void()>          EventCallback;

    /**
     * @brief 读事件回调函数类型定义
     * @param TimeStamp 回调函数参数为时间戳
     */
    typedef std::function<void(TimeStamp)> ReadEventCallback;

public:
    /**
     * @brief Channel构造函数
     * @param loop EventLoop对象
     * @param fd 文件描述符
     */
    Channel(EventLoop* loop, int fd);

    /**
     * @brief Channel析构函数
     */
    ~Channel();

public:
    /**
     * @brief 获取文件描述符
     * @return 文件描述符
     */
    int fd() const;
    /**
     * @brief 获取所属的EventLoop对象
     * @return 事件循环对象
     */
    EventLoop* ownerLoop();
    /**
     * @brief 设置读事件回调函数
     * @param cb 读事件回调函数
     */
    void setReadCallback(const ReadEventCallback& cb);
    /**
     * @brief 设置写事件回调函数
     * @param cb 写事件回调函数
     */
    void setWriteCallback(const EventCallback& cb);
    /**
     * @brief 设置关闭事件回调函数
     * @param cb 关闭事件回调函数
     */
    void setCloseCallback(const EventCallback& cb);
    /**
     * @brief 设置错误事件回调函数
     * @param cb 错误事件回调函数
     */
    void setErrorCallback(const EventCallback& cb);
    /**
     * @brief 获取事件掩码
     * @return 事件掩码
     */
    int events() const;

    /**
	* @brief 设置事件掩码
	* @param revt 事件掩码
	*/
    void set_revents(int revt);
    /**
     * @brief 获取触发的事件
     * @return 触发的事件掩码
     */
    int revents() const;
    /**
     * @brief 启用读事件
     */
    void enableReading();
    /**
     * @brief 禁用读事件
     */
    void disableReading();
    /**
     * @brief 启用写事件
     */
    void enableWriting();
    /**
     * @brief 禁用写事件
     */
    void disableWriting();
    /**
     * @brief 禁用所有事件
     */
    void disableAll();
    /**
     * @brief 判断是否没有任何事件
     * @return true-没有事件 false-有事件
     */
    bool isNoneEvent() const;

    /**
     * @brief 判断是否有写事件
     * @return true-有写事件 false-没有写事件
     */
    bool isWriting() const;


    /**
     * @brief 处理事件
     * @param receiveTime 接收时间
     */
    void handleEvent(TimeStamp receiveTime);

    /**
     * @brief 删除Channel
     */
    void remove();

    /**
     * @brief 将revents转换为字符串
     * @return 事件字符串
     */
    std::string reventsToString() const;

private:
    /**
     * @brief 更新Channel状态
     */
    void update();

    /**
     * @brief 处理事件，持有对象
     * @param receiveTime 接收时间
     */
    void handleEventWithHold(TimeStamp receiveTime);

private:
    EventLoop* loop_;  //< 所属的EventLoop对象
    int        fd_;       //< 文件描述符 fd_ may be socket\signal\timerfd
    int        events_;  //< 事件掩码
    int        revents_;  //< 触发的事件掩码 events of the poller returned

    ReadEventCallback readCallback_;
    EventCallback     writeCallback_;
    EventCallback     closeCallback_;
    EventCallback     errorCallback_;
};

NAMESPACE_AFL_NET_END
#endif  /* _CHANNEL_H */
