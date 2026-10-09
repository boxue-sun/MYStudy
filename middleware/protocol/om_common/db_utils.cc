/*********************************************************************************
* @file		    db_utils.h
* @brief		db_utils.h belongs to CICTCI
* @details
* @author		alfred
* @email        zhangenwei64@gmail.com
* @date		    24-8-24
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-8-24 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#include "db_utils.h"
NAMESPACE_START_OM_COMPONENT_COMMON
// 静态成员变量定义
std::unique_ptr<DeviceStatusMsgDB> DBUtils::m_DeviceStatusMsgDBPtr = nullptr;

bool DBUtils::enableDebugPrint = false;
void DBUtils::setEnableDebugPrint(bool enableDebugPrintTemp)
{
    enableDebugPrint = enableDebugPrintTemp;
}

std::string DBUtils::extractVersion(const std::string& input)
{
    std::regex versionPattern(R"(v(\d+\.\d+))");
    std::smatch matches;
    
    if (std::regex_search(input, matches, versionPattern)) {
        return matches[0]; // returns the complete match (e.g., "v3.1")
    }
    return ""; // return empty string if no match found
}
std::string DBUtils::getGatewayAddress()
{
    // 执行 netstat 命令并获取输出
    FILE* pipe = popen("netstat -nr | grep ^0.0.0.0", "r");
    if (!pipe)
    {
//        OM_MEC_ERROR_PRINT << "[error]popen error！";
        std::cerr << "[error]popen error！" << std::endl;
        return ""; // 命令执行失败
    }
    
    char buffer[128];
    std::string output = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
    }
    pclose(pipe);
    // 从输出中提取网关地址
    std::size_t startPos = output.find("0.0.0.0") + 8;
    std::size_t endPos = output.find("  ", startPos+8);
    std::string gateWayStr = output.substr(startPos, endPos - startPos);
    return trim(gateWayStr);
}
std::string DBUtils::trim(const std::string& str)
{
    std::string::const_iterator start = str.begin();
    while (start != str.end() && std::isspace(*start))
    {
        ++start;
    }

    std::string::const_iterator end = str.end();
    while (end != start && std::isspace(*(end - 1)))
    {
        --end;
    }

    return std::string(start, end);
}
void DBUtils::initializeDB(const std::string &DbFilePath)
{
    m_DeviceStatusMsgDBPtr.reset(new DeviceStatusMsgDB(DbFilePath, enableDebugPrint));
};
bool DBUtils::isDBInitialized()
{
    return m_DeviceStatusMsgDBPtr != nullptr;
};
void DBUtils::getDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, std::vector<MsgDeviceStatus>& statusList, std::string interface,
                        std::string DbFilePath)
{
    if (!DBUtils::isDBInitialized())
    {
        DBUtils::initializeDB();
    }
    m_DeviceStatusMsgDBPtr->getAllDeviceStatus(tableInfo, statusList, interface);
  //  m_DeviceStatusMsgDBPtr->CloseDB();
//    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtrTemp;
//    m_DeviceStatusMsgDBPtrTemp.reset(new DeviceStatusMsgDB(DbFilePath));
//    m_DeviceStatusMsgDBPtrTemp->getAllDeviceStatus(tableInfo, statusList);
//    m_DeviceStatusMsgDBPtrTemp->CloseDB();
//    m_DeviceStatusMsgDBPtrTemp = nullptr;
}
void DBUtils::updateDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface,
                           std::string DbFilePath)
{
    if (!DBUtils::isDBInitialized())
    {
        DBUtils::initializeDB();
    }
    m_DeviceStatusMsgDBPtr->insertOrUpdateDeviceStatusSomeField(tableInfo, msgDeviceStatus, interface);
  //  m_DeviceStatusMsgDBPtr->CloseDB();
//    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtrTemp;
//    m_DeviceStatusMsgDBPtrTemp.reset(new DeviceStatusMsgDB(DbFilePath));
//    m_DeviceStatusMsgDBPtrTemp->insertOrUpdateDeviceStatusSomeField(tableInfo, msgDeviceStatus);
//    m_DeviceStatusMsgDBPtrTemp->CloseDB();
//    m_DeviceStatusMsgDBPtrTemp = nullptr;
}
void DBUtils::initDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface,
                         std::string DbFilePath)
{
    if (!DBUtils::isDBInitialized())
    {
        DBUtils::initializeDB();
    }
    m_DeviceStatusMsgDBPtr->insertOrUpdateDeviceStatus(tableInfo, msgDeviceStatus, interface);
   // m_DeviceStatusMsgDBPtr->CloseDB();
//    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtrTemp;
//    m_DeviceStatusMsgDBPtrTemp.reset(new DeviceStatusMsgDB(DbFilePath));
//    m_DeviceStatusMsgDBPtrTemp->insertOrUpdateDeviceStatus(tableInfo, msgDeviceStatus);
//    m_DeviceStatusMsgDBPtrTemp->CloseDB();
//    m_DeviceStatusMsgDBPtrTemp = nullptr;
}
void DBUtils::delDBInfo(std::pair<std::string, TABLE_TYPE> tableInfo, MsgDeviceStatus msgDeviceStatus, std::string interface,
                        std::string DbFilePath)
{
    if (!DBUtils::isDBInitialized())
    {
        DBUtils::initializeDB();
    }
    m_DeviceStatusMsgDBPtr->deleteTableSomeLine(tableInfo, msgDeviceStatus, interface);
   // m_DeviceStatusMsgDBPtr->CloseDB();
   //
//    std::unique_ptr<DeviceStatusMsgDB> m_DeviceStatusMsgDBPtrTemp;
//    m_DeviceStatusMsgDBPtrTemp.reset(new DeviceStatusMsgDB(DbFilePath));
//    m_DeviceStatusMsgDBPtrTemp->deleteTableSomeLine(tableInfo, msgDeviceStatus);
//    m_DeviceStatusMsgDBPtrTemp->CloseDB();
//    m_DeviceStatusMsgDBPtrTemp = nullptr;
}
std::string DBUtils::getLastPartOfPath(const std::string& path)
{
    // 从后向前查找最后一个'/'的位置
    size_t lastSlashPos = path.find_last_of('/');
    
    // 如果找不到'/'则返回整个字符串
    if (lastSlashPos == std::string::npos)
    {
        return path;
    }
    
    // 返回'/'后面的部分
    return path.substr(lastSlashPos + 1);
}
NetworkInfo DBUtils::getNetworkInfo(const std::string& interface_name)
{
    NetworkInfo info;
    std::string command = "ifconfig " + interface_name;
    std::string output;
    
    // 执行 ifconfig 命令并获取输出
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to execute command: " << command << std::endl;
        return info;
    }
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    pclose(pipe);
    
    // 解析 ifconfig 输出
    std::regex ip_regex(R"(inet\s+(\d+\.\d+\.\d+\.\d+))");
    std::regex gateway_regex(R"(inet\s+(\d+\.\d+\.\d+\.\d+).+\s+gw\s+(\d+\.\d+\.\d+\.\d+))");
    std::regex netmask_regex(R"(inet\s+\d+\.\d+\.\d+\.\d+\s+netmask\s+(\d+\.\d+\.\d+\.\d+))");

    std::smatch match;
    if (std::regex_search(output, match, ip_regex)) {
        info.ip_address = match[1];
    }

    if (std::regex_search(output, match, gateway_regex)) {
        info.ip_address = match[1];
        info.gateway = match[2];
    }

    if (std::regex_search(output, match, netmask_regex)) {
        info.netmask = match[1];
    }

    return info;
}

bool DBUtils::executeCommand(ssh_session session, const std::string &command, std::string& execRet, bool flagReboot)
{
    ssh_channel channel = ssh_channel_new(session);
    if (channel == nullptr)
    {
        std::cerr << "创建通道时出错" << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    if (ssh_channel_open_session(channel) != SSH_OK)
    {
        std::cerr << "打开通道时出错" << std::endl;
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    if (ssh_channel_request_exec(channel, command.c_str()) != SSH_OK)
    {
        std::cerr << "请求执行命令时出错" << std::endl;
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    // 等待 2 秒钟
    std::this_thread::sleep_for(std::chrono::seconds(2));

    char buffer[256];
    int nbytes;
    std::string output;

//    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0)
//    {
//        output.append(buffer, nbytes);
//    }
    // 连续读取数据直到 EOF
    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0 ) {
        if (nbytes > 0) {
            output.append(buffer, nbytes);  // 追加读取到的内容
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 防止忙等
    }
    if(!flagReboot)
    {
        if (nbytes < 0)
        {
            std::cerr << "读取通道时出错" << std::endl;
            return false;
        }
    }
    execRet = output;
    std::cout << "命令输出:\n" << output << std::endl;

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return true;
}
bool DBUtils::sshExec(SshInfo &sshInfo)
{
    const std::string host = sshInfo.host;    // 从命令行参数获取宿主机IP
    const std::string user = sshInfo.user;    // 从命令行参数获取用户名
    const std::string password = sshInfo.password; // 从命令行参数获取密码
    
    ssh_session session = ssh_new();
    if (session == nullptr)
    {
        std::cerr << "创建SSH会话时出错" << std::endl;
        return false;
    }
    else
    {
        std::cout << "创建SSH会话成功:" << std::endl;
    }
    
    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, user.c_str());
    // 设置主机密钥的验证选项
    ssh_options_set(session, SSH_OPTIONS_STRICTHOSTKEYCHECK, "no");
    if (ssh_connect(session) != SSH_OK)
    {
        std::cerr << "连接宿主机时出错: " << ssh_get_error(session) << std::endl;
        ssh_free(session);
        return false;
    }
    else
    {
        std::cout << "连接宿主机成功:" << std::endl;
    }
    
    if (ssh_userauth_password(session, nullptr, password.c_str()) != SSH_AUTH_SUCCESS)
    {
        std::cerr << "认证时出错: " << ssh_get_error(session) << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    else
    {
        std::cout << "认证成功" << std::endl;
    }

    // 执行 jtop 命令
    if(!executeCommand(session, sshInfo.execCmd, sshInfo.execRet, sshInfo.flagReboot))
    {
        std::cout << "执行错误" << std::endl;
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }

    ssh_disconnect(session);
    ssh_free(session);
    
    return true;

}
NAMESPACE_ENDED_OM_COMPONENT_COMMON