/*********************************************************************************
 * @file		httpClient.cpp
 * @brief		httpClient belongs to libapp
 * @details		httpClient belongs to libapp
 * @author		huyang
 * @date		2020/08/05
 * @copyright	Copyright (c) 2020 Gohigh V2X Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2020/08/05 huyang       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#include "httpClient.h"
NAMESPACE_AFL_UTIL_START

httpClient::httpClient()
{
    // TODO Auto-generated constructor stub

}

httpClient::~httpClient()
{
    // TODO Auto-generated destructor stub
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ((std::string *) userdata)->append(ptr, nmemb);
    return nmemb * size;
}

int httpClient::postMethod()
{

    if (initCurlHttp() > 0)
    {
        return CURLE_FAILED_INIT;
    }
    CURLcode res;
    struct curl_slist *headers = NULL;

    printf("HTTP URL : %s, Post data : %s\n", m_strUrl.c_str(), m_strPost.c_str());

    for (auto str: m_httpHeaders)
    {
        headers = curl_slist_append(headers, str.c_str());
    }
    //http request head
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //http request url
    curl_easy_setopt(curl, CURLOPT_URL, m_strUrl.c_str());
    //set http request method as post and set post data
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    //set call back function and response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &m_strResponse);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    //http transfer start
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed : %s!!!\n", curl_easy_strerror(res));
    } else
    {
        // get response code
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("HTTP Transmit success! response code: %ld, response data : %s\n", response_code, m_strResponse.c_str());
    }

    curl_slist_free_all(headers);
    cleanUp();
    return res;
}

int httpClient::getMethod()
{

    if (initCurlHttp() > 0)
    {
        return CURLE_FAILED_INIT;
    }
    CURLcode res;
    struct curl_slist *headers = NULL;
    printf("HTTP URL : %s, Post data : %s\n", m_strUrl.c_str(), m_strPost.c_str());

    for (auto str: m_httpHeaders)
    {
        headers = curl_slist_append(headers, str.c_str());
    }
    //http request head
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, m_strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &m_strResponse);
    /**
     * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
     * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
     */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    //http transfer start
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        printf("curl_easy_perform() failed : %s!!!\n", curl_easy_strerror(res));
    } else
    {
        // get response code
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("HTTP Transmit success! response code: %ld, response data : %s\n",
                response_code, m_strResponse.c_str());
    }
    curl_slist_free_all(headers);
    cleanUp();
    return res;
}

int httpClient::initCurlHttp()
{
    curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
        printf("curl_easy_init() failed : %s!!!\n", curl_easy_strerror(CURLE_FAILED_INIT));
    }
    return 0;
}

void httpClient::cleanUp()
{
    curl_easy_cleanup(curl);
}
NAMESPACE_AFL_UTIL_END