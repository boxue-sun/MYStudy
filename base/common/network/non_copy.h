/*********************************************************************************
 * @file		non_copy.h
 * @brief		non_copy belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-05-16
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 * 2014-05-16  cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _NONCOPY_H
#define _NONCOPY_H

#include "define.h"

NAMESPACE_AFL_BASE_START
/**
 * @brief 禁止拷贝的类
 *
 * NonCopy类禁止拷贝构造和赋值操作。
 */
class NonCopy
{
protected:
   /**
	* @brief 构造函数
	*
	*/
    NonCopy() {}
    /**
     * @brief 析构函数
     *
     */
    ~NonCopy() {}
private:
    /**
     * @brief 拷贝构造函数
     *
     * @param[in] other 要拷贝的对象
     *
     * 拷贝构造函数声明但未定义，禁止调用
     */
    NonCopy(const NonCopy&);
    /**
     * @brief 赋值操作符
     *
     * @param[in] other 要赋值的对象
     *
     * 赋值操作符声明但未定义，禁止调用
     */
    const NonCopy& operator=(const NonCopy&);
};

NAMESPACE_AFL_BASE_END

#endif /* _NONCOPY_H */
