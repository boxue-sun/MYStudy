/*********************************************************************************
 * @file		httpClient.h
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
 *  2020/08/05 huyang       1.0       ����             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef HTTP_HTTPCLIENT_H_
#define HTTP_HTTPCLIENT_H_
#include <vector>
#include <string>
#include "curl/curl.h"
#include <stdio.h>
#include "base/common/network/define.h"
#include "base/common/network/print.h"
NAMESPACE_AFL_UTIL_START

class httpClient
{
public:
    httpClient();
    virtual ~httpClient();

    int postMethod();
    int getMethod();

    void setHeaders(std::string header){
        m_httpHeaders.push_back(header);
    }
    void setUrl(std::string url){
        m_strUrl = url;
    }
    void setPostData(std::string data){
        m_strPost = data;
    }
    std::string getResponse(){
        return m_strResponse;
    }

private:
    int initCurlHttp();
    void cleanUp();

private:
    CURL* curl = NULL;
    std::vector<std::string> m_httpHeaders;
    std::string m_strUrl;
    std::string m_strPost;
    std::string m_strResponse;
};
NAMESPACE_AFL_UTIL_END
#endif /* HTTP_HTTPCLIENT_H_ */
