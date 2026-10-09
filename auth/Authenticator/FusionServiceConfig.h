#ifndef FUSION_SERVICE_CONFIG_H
#define FUSION_SERVICE_CONFIG_H

#include <string>
#include <cstdint>

namespace FusionService {

class FusionServiceConfig {
public:
    // 默认构造函数
    FusionServiceConfig()
        : expired(0), limit_period(false), limit_hardware(false) {}

    // 从文件加载配置到当前对象
    bool loadConfigFromFile(const std::string& file_path);

    // 新增：从字符串加载配置到当前对象
    bool loadConfigFromString(const std::string& config_str);

    // 成员变量（保持公有）
    std::string project;       // 项目名称
    uint64_t expired;          // 授权过期时间（UTC 时间戳，毫秒）
    bool limit_period;         // 是否限制授权周期
    bool limit_hardware;       // 是否限制硬件绑定
    std::string license_key;   // 授权密钥
    std::string license;       // 加密后的硬件绑定信息
    std::string verify_code;   // 验证码，用于校验授权信息

private:
    // 内部工具函数：解析日期字符串到 UTC 时间戳（毫秒）
    static uint64_t toUTCTimestampMs(const std::string& date_str);
};

} // namespace FusionService

#endif // FUSION_SERVICE_CONFIG_H
