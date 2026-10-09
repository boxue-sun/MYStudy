/*********************************************************************************
 * @file		date_time.h
 * @brief		date_time belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		2014-09-17
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-09-17 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef _DATETIME_H
#define _DATETIME_H
#include "define.h"
#include <string>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>   // for snprintf
NAMESPACE_AFL_UTIL_START

class DateTime
{
public:
    /**
    * @brief           鍒ゆ柇鎸囧畾骞翠唤鏄惁涓洪棸年
    * @param year      鎸囧畾鐨勫勾浠�
    * @return          杩斿洖鏃ユ湡鍜屾椂闂存牸寮忓寲鍚庣殑瀛楃涓�: YYYY-MM-DD HH:MM:SS
    */
    static bool        isLeapYear(int year);

    /**
    * @brief           鑾峰彇褰撳墠鏃ユ湡鍜屾椂闂�
    * @param ptm       鎸囧悜褰撳墠鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    */
    static void        currentDateTime(struct tm* ptm);
    /**
    * @brief           鑾峰彇褰撳墠鏃ユ湡鍜屾椂闂�: YYYY-MM-DD HH:MM:SS
    * @param buf       鐢ㄦ潵瀛樺偍褰撳墠鏃ユ湡鍜屾椂闂寸殑缂撳啿鍖�
    * @param size      buf鐨勫ぇ灏忥紝len > sizeof("YYYY-MM-DD HH:MM:SS")
    */
    static void        currentDateTime(char* buf, size_t size);
    /**
    * @brief           鑾峰彇褰撳墠鏃ユ湡鍜屾椂闂�: YYYY-MM-DD HH:MM:SS
    * @return          杩斿洖鏃ユ湡鍜屾椂闂存牸寮忓寲鍚庣殑瀛楃涓�
    */
    static std::string currentDateTime();

    /**
    * @brief           鑾峰彇褰撳墠鏃ユ湡: YYYY-MM-DD
    * @param buf       鐢ㄦ潵瀛樺偍褰撳墠鏃ユ湡鐨勭紦鍐插尯
    * @param size      buf鐨勫ぇ灏忥紝len > sizeof("YYYY-MM-DD")
    */
    static void        currentDate(char* buf, size_t size);
    /**
    * @brief           鑾峰彇褰撳墠鏃ユ湡: YYYY-MM-DD
    * @return          杩斿洖鏃ユ湡鍜屾椂闂存牸寮忓寲鍚庣殑瀛楃涓�
    */
    static std::string currentDate();

    /**
    * @brief           鑾峰彇褰撳墠鏃堕棿: HH:SS:MM
    * @param buf       鐢ㄦ潵瀛樺偍褰撳墠鏃堕棿鐨勭紦鍐插尯
    * @param size      buf鐨勫ぇ灏忥紝len > sizeof("HH:SS:MM")
    */
    static void        currentTime(char* buf, size_t size);
    /**
    * @brief           鑾峰彇褰撳墠鏃堕棿: HH:SS:MM
    * @return          杩斿洖鏃堕棿鏍煎紡鍖栧悗鐨勫瓧绗︿覆
    */
    static std::string currentTime();

    /**
    * @brief           灏嗕竴涓瓧绗︿覆杞崲鎴愭棩鏈熸椂闂存牸寮忥紝瑕佹眰鍘熷瓧绗︿覆鏍煎紡涓�: YYYY-MM-DD HH:MM:SS
    * @param strTime   鍖呭惈鏃堕棿鏍煎紡鐨勫瓧绗︿覆
    * @param datetime  鐢ㄦ潵瀛樺偍瀛楃涓茶浆鎹㈡垚鏃堕棿鐨勭粨鏋勪綋鎸囬拡
    * @return          杞崲鎴愬姛杩斿洖true锛屽惁鍒欒繑鍥瀎alse
    */
    static bool        stringToDataTime(const char* strTime, struct tm* datetime);
    /**
    * @brief           灏嗕竴涓瓧绗︿覆杞崲鎴愭棩鏈熸椂闂存牸寮忥紝瑕佹眰鍘熷瓧绗︿覆鏍煎紡涓�: YYYY-MM-DD HH:MM:SS
    * @param strTime   鍖呭惈鏃堕棿鏍煎紡鐨勫瓧绗︿覆
    * @param datetime  鐢ㄦ潵瀛樺偍瀛楃涓茶浆鎹㈡垚鏃堕棿鐨勭粨鏋勪綋鎸囬拡
    * @return          杞崲鎴愬姛杩斿洖true锛屽惁鍒欒繑鍥瀎alse
    */
    static bool        stringToDataTime(const char* strTime, time_t* datetime);

    /**
    * @brief           灏嗕竴涓瓧绗︿覆杞崲鎴愭棩鏈熸椂闂存牸寮忥紝瑕佹眰鍘熷瓧绗︿覆鏍煎紡涓�: YYYY-MM-DD HH:MM:SS
    * @param datetime  鎸囧悜鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @param buf       鐢ㄦ潵瀛樺偍瀛楃涓茶浆鎹㈡垚鏃堕棿鐨勭紦鍐插尯
    * @param size      缂撳啿鍖哄ぇ灏�
    */
    static void        dateTimeToString(struct tm* datetime, char* buf, size_t size);
    /**
    * @brief           灏嗕竴涓棩鏈熷拰鏃堕棿杞崲涓哄瓧绗︿覆: YYYY-MM-DD HH:MM:SS
    * @param datetime  瀛樺偍鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @return          杩斿洖鏃堕棿鏍煎紡鍖栧悗鐨勫瓧绗︿覆
    */
    static std::string dateTimeToString(struct tm* datetime);

    /**
    * @brief           灏嗕竴涓棩鏈熻浆鎹负瀛楃涓�: YYYY-MM-DD
    * @param datetime  鎸囧悜鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @param buf       鐢ㄦ潵瀛樺偍瀛楃涓茶浆鎹㈡垚鏃堕棿鐨勭紦鍐插尯
    * @param size      缂撳啿鍖哄ぇ灏�
    */
    static void        dateToString(struct tm* datetime, char* buf, size_t size);
    /**
    * @brief           灏嗕竴涓棩鏈熻浆鎹负瀛楃涓�: YYYY-MM-DD
    * @param datetime  鎸囧悜鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @return          杩斿洖鏃堕棿鏍煎紡鍖栧悗鐨勫瓧绗︿覆
    */
    static std::string dateToString(struct tm* datetime);

    /**
    * @brief           灏嗕竴涓椂闂磋浆鎹负瀛楃涓�: HH:MM:SS
    * @param datetime  鎸囧悜鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @param buf       鐢ㄦ潵瀛樺偍瀛楃涓茶浆鎹㈡垚鏃堕棿鐨勭紦鍐插尯
    * @param size      缂撳啿鍖哄ぇ灏�
    */
    static void        timeToString(struct tm* datetime, char* buf, size_t size);
    /**
    * @brief           灏嗕竴涓椂闂磋浆鎹负瀛楃涓�: HH:MM:SS
    * @param datetime  鎸囧悜鏃ユ湡鍜屾椂闂寸殑缁撴瀯浣撴寚閽�
    * @return          杩斿洖鏃堕棿鏍煎紡鍖栧悗鐨勫瓧绗︿覆
    */
    static std::string timeToString(struct tm* datetime);
};

NAMESPACE_AFL_UTIL_END

#endif /* _DATETIME_H */
