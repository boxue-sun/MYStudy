/*********************************************************************************
 * @file		call_backs.h
 * @brief		call_backs belongs to CICTCI
 * @details
 * @author		cs
 * @date
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-10-31 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _CALLBACKS_H
#define _CALLBACKS_H
#include  "define.h"
#include "time_stamp.h"
#include <memory>            //for std::shared_ptr

NAMESPACE_AFL_NET_START

class ByteArray;
class EventLoop;
class TcpConnection;
class InetAddress;
class TcpAcceptor;
class ByteBuffer;
using afl::util::TimeStamp;
/**
 * @brief 字符串缓冲区
*/
//typedef std::string Buffer;
/**
 * @brief TCP连接智能指针
*/
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;


/**
 * @brief 默认的连接回调函数
 * @param conn TCP连接智能指针
*/
void defaultConnectionCallback(const TcpConnectionPtr& conn);


/**
 * @brief 默认的消息回调函数
 * @param conn TCP连接智能指针
 * @param buffer 消息缓冲区
 * @param receiveTime 收到消息的时间戳
*/
void defaultMessageCallback(const TcpConnectionPtr& conn, ByteBuffer* buffer,
                            TimeStamp receiveTime);
/**
 * @brief 连接回调函数类型
 * @param conn TCP连接智能指针
*/
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;

/**
 * @brief 关闭回调函数类型
 * @param conn TCP连接智能指针
*/
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

/**
 * @brief 写完成回调函数类型
 * @param conn TCP连接智能指针
*/
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;

/**
 * @brief 消息回调函数类型
 * @param conn TCP连接智能指针
 * @param buffer 消息缓冲区
 * @param receiveTime 收到消息的时间戳
*/
typedef std::function<void (const TcpConnectionPtr&, ByteBuffer*, TimeStamp)> MessageCallback;

/**
 * @brief 定时器ID类型
*/
typedef int  TimerId;

/**
 * @brief 定时器回调函数类型
*/
typedef std::function<void ()>  TimerCallback;


/**
 * @brief 信号回调函数类型
 * @param signalId 信号ID
*/
typedef std::function<void (int)> SignalCallback;

NAMESPACE_AFL_NET_END
#endif  /* _CALLBACKS_H */
