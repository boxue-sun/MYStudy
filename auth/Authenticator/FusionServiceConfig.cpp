#include "FusionServiceConfig.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <sstream>
#include <vector>

namespace FusionService {

bool FusionServiceConfig::loadConfigFromFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open config file: " << file_path << std::endl;
        return false;
    }

    try {
        nlohmann::json json_data;
        file >> json_data;

        project = json_data.at("project").get<std::string>();
        if (json_data.at("expired").is_string()) {
            expired = toUTCTimestampMs(json_data.at("expired").get<std::string>());
        } else if (json_data.at("expired").is_number_unsigned()) {
            expired = json_data.at("expired").get<uint64_t>();
        } else {
            std::cerr << "Error: Invalid type for 'expired' field" << std::endl;
            return false;
        }
        limit_period = json_data.at("limit_period").get<bool>();
        limit_hardware = json_data.at("limit_hardware").get<bool>();
        license_key = json_data.at("license_key").get<std::string>();
        license = json_data.at("license").get<std::string>();
        verify_code = json_data.at("verify_code").get<std::string>();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        return false;
    }
}

bool FusionServiceConfig::loadConfigFromString(const std::string& config_str) {
    std::istringstream iss(config_str);
    std::vector<std::string> fields;
    std::string field;

    // 使用 '|' 分割字符串
    while (std::getline(iss, field, '|')) {
        fields.push_back(field);
    }

    // 检查字段数量是否正确（预期 6 个字段）
    if (fields.size() != 7) {
        std::cerr << "Error: Invalid config string format, expected 6 fields, got " << fields.size() << std::endl;
        return false;
    }

    try {
        // 按顺序填充字段
        project = fields[0];
        expired = std::stoull(fields[1]); // expired 作为 uint64_t 解析
        limit_period = (fields[2] == "1" || fields[2] == "true"); // 支持 "1" 或 "true" 表示 true
        limit_hardware = (fields[3] == "1" || fields[3] == "true");
        license_key = fields[4];
        verify_code = fields[5];
        license = fields[6];
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config string: " << e.what() << std::endl;
        return false;
    }
}

uint64_t FusionServiceConfig::toUTCTimestampMs(const std::string& date_str) {
    std::tm tm{};
    std::istringstream iss(date_str);
    iss >> std::get_time(&tm, "%Y-%m-%d");
    if (iss.fail()) {
        throw std::runtime_error("Failed to parse date string: " + date_str);
    }
    tm.tm_hour = 23;
    tm.tm_min = 59;
    tm.tm_sec = 59;
    tm.tm_isdst = 0;
    time_t time = mktime(&tm);
    if (time == -1) {
        throw std::runtime_error("Invalid date: " + date_str);
    }
    return static_cast<uint64_t>(time) * 1000ULL;
}

} // namespace FusionService
