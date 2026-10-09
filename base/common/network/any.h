/*********************************************************************************
 * @file		any.h
 * @brief		any belongs to CICTCI
 * @details
 * @author		cs
 * @date		2014-08-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-08-21 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _ANY_H
#define _ANY_H
#include "define.h"

NAMESPACE_AFL_UTIL_START

class any
{
public:
    any() : content_(0)
    {
    }

    any(const any& that) : content_(that.clone())
    {
    }

    any(any&&  that) : content_(std::move(that.content_))
    {
    }

    template<typename U>
    any(U&&  value) : content_(new holder <typename std::decay<U>::type>(std::forward<U>(value)))
    {
    }

    any& operator=(const any& rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    template<typename ValueType>
    any& operator=(const ValueType& rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    ~any()
    {
        delete content_;
    }

public:
    any& swap(any& rhs)
    {
        std::swap(content_, rhs.content_);
        return *this;
    }

    bool empty() const
    {
        return !content_;
    }

    void clear()
    {
        any().swap(*this);
    }

    const std::type_info& type() const
    {
        return content_ ? content_->type() : typeid(void);
    }

private:
    struct placeholder
    {
        virtual ~placeholder() { }
        virtual const std::type_info& type() const = 0;
        virtual placeholder* clone() const = 0;
    };

    template <typename ValueType>
    class holder : public placeholder
    {
    public:
        holder(const ValueType& value) : held_(value)
        {
        }
    public:
        virtual const std::type_info& type() const
        {
            return typeid(ValueType);
        }

        virtual placeholder* clone() const
        {
            return new holder(held_);
        }
    public:
        ValueType held_;
    private:
        holder& operator=(const holder&);
    };

    placeholder* clone() const
    {
        if (content_ != 0)
            return content_->clone();

        return 0;
    }

private:
    template<typename ValueType>
    friend ValueType* any_cast(any*);

private:
    placeholder* content_;
};

inline void swap(any& lhs, any& rhs)
{
    lhs.swap(rhs);
}

class bad_any_cast :  public std::bad_cast
{
public:
    virtual const char* what() const throw()
    {
        return "afl::bad_any_cast: failed conversion using afl::any_cast";
    }
};

template<typename ValueType>
inline ValueType* any_cast(any* operand)
{
    //any::holder<ValueType>* derived = static_cast<any::holder<ValueType>*> (operand->content_);
    //return &derived->held_;
    return  operand && operand->type() == typeid(ValueType)
            //? &static_cast<any::holder<std::remove_cv<ValueType>::type> *>(operand->content_)->held_
            ? &static_cast<any::holder<ValueType>*>(operand->content_)->held_
            : 0;
}

template<typename ValueType>
inline const ValueType* any_cast(const any* operand)
{
    return afl::util::any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
inline ValueType any_cast(any& operand)
{
    //typedef std::remove_reference<ValueType>::type nonref;
    typedef ValueType nonref;

    nonref* result = any_cast<nonref>(&operand);
    if (!result)
        throw bad_any_cast();

    return static_cast<ValueType>(*result);
}

template<typename ValueType>
inline ValueType any_cast(const any& operand)
{
    //typedef std::remove_reference<ValueType>::type nonref;
    typedef ValueType nonref;
    return any_cast<const nonref>(const_cast<any&>(operand));
}

NAMESPACE_AFL_UTIL_END
#endif  /* _ANY_H */
