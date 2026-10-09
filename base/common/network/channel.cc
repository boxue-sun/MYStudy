#include "channel.h"


NAMESPACE_AFL_NET_START

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
{
}

Channel::~Channel()
{
    if (loop_->isInLoopThread())
    {
        // assert(!loop_->hasChannel(this));
    }
}
/**
 * @brief 更新通道
 *
 * 调用消息循环的updateChannel方法更新当前通道
	该函数负责调用消息循环的updateChannel方法更新当前通道。
	函数内部首先获取消息循环对象的引用，然后调用消息循环的updateChannel方法，传入当前通道this作为参数完成更新操作。
	主要使用了类方法定义、智能指针、消息循环类、通道类以及类方法调用等C++知识。
 * @param 无
 * @return 无
 */
void Channel::update()
{
   /**
	* 函数内部流程：
	* 1. 获取消息循环对象的引用
	* 2. 调用消息循环的updateChannel方法，传入当前通道this作为参数
	*
	* 使用的C++知识：
	* 1. 类方法定义
	* 2. 智能指针
	* 3. 消息循环类
	* 4. 通道类
	* 5. 类方法调用
	*/
    loop_->updateChannel(this);
}
/**
 * @brief 从事件循环中移除Channel
 *
 * 从事件循环中移除Channel,删除Channel的引用计数
 *
 * 1. 断言Channel当前没有事件
 * 2. 调用事件循环的removeChannel方法移除Channel
 *
 * @return 无返回值
 */
void Channel::remove()
{
	/**
	* 函数内部流程：
	*
	* 1. 使用assert断言Channel当前没有事件(isNoneEvent())
	* 2. 获取事件循环的引用(loop_),调用它的removeChannel方法移除Channel
	*
	* 使用的C++知识：
	*
	* 1. 类定义
	* 2. 方法定义
	* 3. assert断言
	* 4. 智能指针
	* 5. 事件循环类
	* 6. Channel类
	* 7. C++注释
	*/
    assert(isNoneEvent());
    loop_->removeChannel(this);
}
/**
 * @brief 处理事件
 *
 * 此函数用于处理Channel接收到的事件
 * 1. 调用handleEventWithHold方法处理事件
 *
 * @param receiveTime 事件接收时间
 *
 * 使用的C++知识：
 * 1. 函数定义
 * 2. 方法调用
 * 3. 参数传递
 * 4. C++注释
*/
void Channel::handleEvent(TimeStamp receiveTime)
{
   /**
	* 函数内部流程：
	* 1. 调用Channel类中的handleEventWithHold方法，传递接收时间参数receiveTime
	*
    * 使用的C++知识：
    * 1. 类成员方法调用
    * 2. 参数传递
   */
    handleEventWithHold(receiveTime);
}
/**
 * @brief 处理带超时的事件
 * @param receiveTime 接收事件的时间戳
*/
void Channel::handleEventWithHold(TimeStamp receiveTime)
{
	/**
	 *  * 这个函数处理套接字的事件，会根据事件类型调用不同的回调函数
	 * 1. 检查是否有挂断事件，如果有且没有读事件，则调用closeCallback_
	 * 2. 检查是否有无效事件，如果有则打印日志
	 * 3. 检查是否有错误事件或无效事件，如果有则调用errorCallback_
	 * 4. 检查是否有读事件，如果有则调用readCallback_
	 * 5. 检查是否有写完成事件，如果有则调用writeCallback_
	 *
	 * 使用的C++知识：
	 * 1. 函数定义
	 * 2. if条件语句
	 * 3. 位运算判断事件类型
	 * 4. 日志打印
	 * 5. 回调函数调用
	 * 6. Channel类
	 * 7. C++注释
	 */
    if ((revents_ & FDEVENT_HUP) && !(revents_ & FDEVENT_IN))
    {
//        LOG_INFO("Channel::handleEventWithHold closeCallback, fd[%d]", fd_);
        if (closeCallback_)
            closeCallback_();
    }

    if (revents_ & FDEVENT_NVAL)
    {
//        LOG_WARN("Channel::handle_event() POLLNVAL, fd[%d]", fd_);
    }

    if (revents_ & (FDEVENT_ERR | FDEVENT_NVAL))
    {
//        LOG_INFO("Channel::handleEventWithHold closeCallback, fd[%d]", fd_);
        if (errorCallback_)
            errorCallback_();
    }
    if (revents_ & kEventRead)
    {
        if (readCallback_)
            readCallback_(receiveTime);
    }
    if (revents_ & FDEVENT_OUT)
    {
        if (writeCallback_)
            writeCallback_();
    }
}
/**
 * @brief 将事件类型转换为字符串
 * 将Channel对象的revents事件类型转换为以空格分隔的字符串表示
 * @return 事件字符串
 */
std::string Channel::reventsToString() const
{
	/**
	* 函数内部流程：
	* 1. 定义一个ostringstream对象oss用于存储结果字符串
	* 2. 使用<<操作符向oss中插入fd和"： "
	* 3. 使用if语句判断revents中的各个事件标志位，如果为真则向oss插入事件名称字符串
	* 4. 返回oss中的结果字符串
	*
	* 使用的C++知识：
	* 1. std::ostringstream字符串流
	* 2. <<操作符
	* 3. if语句
	* 4. 位运算符&
	* 5. Channel类和它的revents成员
	*/
    std::ostringstream oss;
    oss << fd_ << ": ";
    if (revents_ & FDEVENT_IN)
        oss << "IN ";
    if (revents_ & FDEVENT_PRI)
        oss << "PRI ";
    if (revents_ & FDEVENT_OUT)
        oss << "OUT ";
    if (revents_ & FDEVENT_HUP)
        oss << "HUP ";
    if (revents_ & FDEVENT_RDHUP)
        oss << "RDHUP ";
    if (revents_ & FDEVENT_ERR)
        oss << "ERR ";
    if (revents_ & FDEVENT_NVAL)
        oss << "NVAL ";

    return oss.str().c_str();
}

/**
 * @brief 获取通道的文件描述符
 *
 * 返回通道对应的文件描述符
 *
 * @return 文件描述符
 */
int Channel::fd() const
{
   /**
	* 函数内部流程：
	* 1. 返回类成员变量fd_对应的文件描述符
	*
	* 使用的C++知识：
	* 1. 类定义
	* 2. 成员变量
	* 3. const修饰符
	* 4. 返回值
	* 5. C++注释
	*/
   return fd_;
}

EventLoop* Channel::ownerLoop()
{
   return loop_;
}

void Channel::setReadCallback(const ReadEventCallback& cb)
{
   readCallback_ = cb;
}

void Channel::setWriteCallback(const EventCallback& cb)
{
   writeCallback_ = cb;
}

void Channel::setCloseCallback(const EventCallback& cb)
{
   closeCallback_ = cb;
}

void Channel::setErrorCallback(const EventCallback& cb)
{
   errorCallback_ = cb;
}

int Channel::events() const
{
   return events_;
}

void Channel::set_revents(int revt)
{
   revents_ = revt;
}

int Channel::revents() const
{
   return revents_;
}

void Channel::enableReading()
{
   events_ |= kEventRead;
   update();
}

void Channel::disableReading()
{
   events_ &= ~kEventRead;
   update();
}

void Channel::enableWriting()
{
   events_ |= kEventWrite;
   update();
}

void Channel::disableWriting()
{
   events_ &= ~kEventWrite;
   update();
}

void Channel::disableAll()
{
   events_ = kEventNone;
   update();
}

bool Channel::isNoneEvent() const
{
   return events_ == kEventNone;
}

bool Channel::isWriting() const
{
   return events_ & kEventWrite;
}

NAMESPACE_AFL_NET_END
