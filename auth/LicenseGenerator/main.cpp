#include <iostream>
#include "Authenticator/Authenticator.h"
#include "Authenticator/HardwareCode.h"
#include <string>


int main(int argc, char* argv[]) {

    std::string hard = FusionService::inner::GetHardDriveSerialNumber();
    std::string mac = FusionService::inner::GetMACAddress();
    std::string hard_ward_license =  hard + "@" + mac;
    FusionService::FusionServiceConfig config;

    if (!config.loadConfigFromFile("config.json")) {
        fprintf(stderr, "Unable to parse config file config.json.\n");
        return -2;
    }
    std::string license = FusionService::Authenticator::GetInstance().Generate(config, hard_ward_license);
    // 调用生成授权信息函数
    std::cout << "License: " << license << std::endl;
    return 0;
}