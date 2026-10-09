/*********************************************************************************
 * @file		cs_singleton.h
 * @brief		cs_singleton belongs to CICTCI
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
#ifndef BASE_COMMON_CS_SINGLETON_H
#define BASE_COMMON_CS_SINGLETON_H

#include "define.h"
#include "mutex.h"

NAMESPACE_AFL_BASE_START

#define DECLARE_SINGLETON_CLASS(type)  friend class afl::base::Singleton< type >
/**
 * @brief 单例模式类模板
 *
 * @tparam T 单例类模板参数
 */
template <class T>
class Singleton
{
public:
   /**
	* @brief 返回实例指针
	*
	* @return T* 指向实例的指针
	*/
    static T* getInstancePtr()
    {
    	  // 如果实例未创建，调用createInstance创建
        if (0 == proxy_.instance_)
        {
            createInstance();
        }
        return proxy_.instance_;
    }
    /**
     * @brief 返回实例引用
     *
     * @return T& 对实例的引用
     */
    static T& getInstanceRef()
    {
    	// 如果实例未创建，调用createInstance创建
        if (0 == proxy_.instance_)
        {
            createInstance();
        }
        return *(proxy_.instance_);
    }
    /**
     * @brief 静态创建实例函数
     *
     * @return T* 创建的实例指针
     */
    static T* createInstance()
    {
        return proxy_.createInstance();
    }
    /**
     * @brief 静态删除实例函数
     */
    static void deleteInstance()
    {
        proxy_.deleteInstance();
    }

private:
    /**
     * @brief 代理类，管理实例的创建和销毁
     */
    struct Proxy
    {
        /**
         * @brief 默认构造函数
         */
        Proxy() : instance_(0)
        {
        }
        /**
         * @brief 析构函数
         *
         * @note 如果实例存在，释放它
         */
        ~Proxy()
        {
            if (instance_)
            {
                delete instance_;
                instance_ = 0;
            }
        }
        /**
         * @brief 实例的创建函数
         *
         * @note 使用锁保护并判断是否已创建
         *
         * @return T* 创建的实例指针
         */
        T* createInstance()
        {
        	// 实例指针，加锁创建
            T* p = instance_;
            if (p == 0)
            {
                afl::thread::LockGuard<afl::thread::Mutex> guard(lock_);
                if ((p = instance_) == 0)
                {
                    instance_ = p = new T;
                }
            }
            return instance_;
        }
        /**
         * @brief 实例的删除函数
         *
         * @note 使用锁删除实例
         */
        void deleteInstance()
        {
            if (proxy_.instance_)
            {
                delete proxy_.instance_;
                proxy_.instance_ = 0;
            }
        }
        T* instance_; //!< 实例指针
        afl::thread::Mutex lock_; //!< 锁
    };
protected:
    /**
     * @brief 默认构造函数
     */
    Singleton()  {  }

    /**
     * @brief 析构函数
     */
    ~Singleton() {  }
private:
    static Proxy proxy_;  //< 代理对象
};

template < class T >
typename Singleton<T>::Proxy Singleton<T>::proxy_;

NAMESPACE_AFL_BASE_END

#endif
