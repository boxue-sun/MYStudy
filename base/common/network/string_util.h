/*
 * @Author: cs
 * @Date: 2024-01-24 11:16:47
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-01-24 11:16:47
 * @Description:
 * @FilePath:
 */
#ifndef _STRING_UTIL_H
#define _STRING_UTIL_H
#include "base/common/network/define.h"
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>
NAMESPACE_AFL_UTIL_START

/// 鏍煎紡鍖栧瓧绗︿覆
size_t stringFormatAppend(std::string* dst, const char* format, ...);
size_t stringFormat(std::string* dst, const char* format, ...);
std::string stringFormat(const char* format, ...);

/// 浠绘剰绫诲瀷杞负瀛楃涓�
template <typename T>
inline std::string toStr(const T& t)
{
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

/// 瀛楃涓茶浆涓烘煇涓�绫诲瀷
template <typename T>
T strTo(const std::string& str)
{
    T t;
    std::istringstream iss(str);
    iss >> t;
    return t;
}

/// 瀛楃涓插拷鐣ュぇ灏忓啓姣旇緝, 鍙敤浣滃鍣紙姣斿map銆乻et锛夌殑姣旇緝瀛�
struct string_cmp_nocase : public std::binary_function<std::string, std::string, bool>
{
public:
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        std::string::const_iterator p = lhs.begin();
        std::string::const_iterator p2 = rhs.begin();

        while (p != lhs.end() && p2 != rhs.end())
        {
            if (toupper(*p) != toupper(*p2))
                return (toupper(*p) < toupper(*p2) ? 1 : 0);
            ++p;
            ++p2;
        }

        return (lhs.size() == rhs.size()) ? 0 : (lhs.size() < rhs.size()) ? 1 : 0;
    }
};

/// 灏嗗瓧绗︿覆杞负灏忓啓骞惰繑鍥�
inline std::string toLower(const std::string& str)
{
    std::string t = str;
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    return t;
}

/// 灏嗗瓧绗︿覆杞负灏忓啓骞惰繑鍥�
inline std::string toUpper(const std::string& str)
{
    std::string t = str;
    std::transform(t.begin(), t.end(), t.begin(), ::toupper);
    return t;
}

/// 鍒ゆ柇瀛楃涓叉槸鍚︿互鏌愪竴瀛愪覆涓哄紑濮�
inline bool startsWith(const std::string& str, const std::string& substr)
{
    return str.find(substr) == 0;
}

/// 鍒ゆ柇瀛楃涓叉槸鍚︿互鏌愪竴瀛愪覆涓虹粨灏�
inline bool endsWith(const std::string& str, const std::string& substr)
{
    return str.rfind(substr) == (str.length() - substr.length());
}

/// 姣旇緝涓や釜瀛楃涓叉槸鍚︾浉绛�
inline bool equals(const std::string& lhs, const std::string& rhs)
{
    return (lhs) == (rhs);
}

/// 鍘绘帀瀛楃涓蹭腑宸﹁竟灞炰簬瀛楃涓瞕elim涓换涓�瀛楃鐨勬墍鏈夊瓧绗�(榛樿鍘婚櫎绌烘牸)
inline std::string& trimLeft(std::string& str, const char* delim = " ")
{
    str.erase(0, str.find_first_not_of(delim));
    return str;
}

/// 鍘绘帀瀛楃涓蹭腑鍙宠竟灞炰簬瀛楃涓瞕elim涓换涓�瀛楃鐨勬墍鏈夊瓧绗�(榛樿鍘婚櫎绌烘牸)
inline std::string& trimRight(std::string& str, const char* delim = " ")
{
    str.erase(str.find_last_not_of(delim) + 1);
    return str;
}

/// 鍘绘帀瀛楃涓蹭腑涓ょ灞炰簬瀛楃涓瞕elim涓换涓�瀛楃鐨勬墍鏈夊瓧绗�(榛樿鍘婚櫎绌烘牸)
inline std::string& trim(std::string& str, const char* delim = " ")
{
    trimLeft(str, delim);
    trimRight(str, delim);
    return str;
}

/// 鍘绘帀瀛楃涓蹭腑鐨勬墍鏈夌壒瀹氬崟涓�瀛楃
inline std::string erase(std::string& str, char c = ' ')
{
    str.erase(std::remove_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), c)),
              str.end());
    return str;
}

/// 瀛楃涓叉浛鎹� 鍘绘帀瀛楃涓蹭腑鐨勬煇鐗瑰畾瀛楃涓瞕elim骞朵互鏂板瓧绗︿覆s浠ｆ浛
inline std::string replaceAll(std::string& str, const char* delim, const char* s = "")
{
    size_t len = strlen(delim);
    size_t pos = str.find(delim);
    while (pos != std::string::npos)
    {
        str.replace(pos, len, s);
        pos = str.find(delim, pos);
    }
    return str;
}

/// 瀛楃涓插垎闅旓紝insertEmpty : 濡傛灉鏈夎繛缁殑delim锛屾槸鍚︽彃鍏ョ┖涓�
inline void split(const std::string& str, std::vector<std::string>& result,
                  const std::string& delim = " ", bool insertEmpty = false)
{
    if (str.empty() || delim.empty())
        return;

    std::string::const_iterator substart = str.begin(), subend;
    while (true)
    {
        subend = std::search(substart, str.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);

        if (!temp.empty())
        {
            result.push_back(temp);
        }
        else if (insertEmpty)
        {
            result.push_back("");
        }

        if (subend == str.end())
            break;
        substart = subend + delim.size();
    }
}

/// 瀛楃涓插悎年
template< typename SequenceSequenceT, typename Range1T>
inline typename SequenceSequenceT::value_type join(const SequenceSequenceT& Input,
                                                   const Range1T& Separator)
{
    typedef typename SequenceSequenceT::value_type ResultT;
    typedef typename SequenceSequenceT::const_iterator InputIteratorT;

    InputIteratorT itBegin = Input.begin();
    InputIteratorT itEnd = Input.end();

    ResultT Result;

    if (itBegin != itEnd)
    {
        Result += *itBegin;
        ++itBegin;
    }

    for (; itBegin != itEnd; ++itBegin)
    {
        Result += Separator;    // Add separator
        Result += *itBegin;     // Add element
    }

    return Result;
}

template<typename SequenceSequenceT, typename Range1T, typename PredicateT>
inline typename SequenceSequenceT::value_type
join_if(const SequenceSequenceT& Input, const Range1T& Separator, PredicateT Pred)
{
    typedef typename SequenceSequenceT::value_type ResultT;
    typedef typename SequenceSequenceT::const_iterator InputIteratorT;

    InputIteratorT itBegin = Input.begin();
    InputIteratorT itEnd = Input.end();

    ResultT Result;

    while (itBegin != itEnd && !Pred(*itBegin)) ++itBegin;

    if (itBegin != itEnd)
    {
        Result += *itBegin;
        ++itBegin;
    }

    for (; itBegin != itEnd; ++itBegin)
    {
        if (Pred(*itBegin))
        {
            Result += Separator;    // Add separator
            Result += *itBegin;     // Add element
        }
    }

    return Result;
}

NAMESPACE_AFL_BASE_END

#endif  /* _STRING_UTIL_H */
