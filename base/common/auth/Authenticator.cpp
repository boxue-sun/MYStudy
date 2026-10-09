#include "Authenticator.h"
#include <sstream>
#include <random>
#include <chrono>

#include "MD5.h"
#include "AES_Encryptor.h"
#include "HardwareCode.h"


#define MAGIC_CODE 77615216
#define KEY_LENGTH 16

namespace FusionService {

	// 初始化静态成员
	Authenticator Authenticator::_authenticator;

	int Authenticator::Init(const FusionServiceConfig& config)
	{
		if (config.license_key.empty() || config.verify_code.empty() || config.license.empty()) {
			return CC_ERR_LICENSE_MISSING_FIELD;
		}
		if (config.license_key.length() != KEY_LENGTH || config.verify_code.length() != 32) {
			return CC_ERR_LICENSE_INVALID_FIELD;
		}
		// 先获取组合后的授权信息
		std::string verify_content = Merge(config);
		// 使用license_key生成verify_key
		std::string verify_key = VerifyKey(config.license_key);
		// 使用verify_key对verify_content进行AES加密
		AesEncryptor aes_verify((unsigned char*)verify_key.c_str());
		std::string verify_encrypted = aes_verify.EncryptString(verify_content);
		// 使用MD5后的AES字符串与verify_code进行比对
		MD5 verify_md5(verify_encrypted);
		// 不相符则说明字符串被篡改，授权失败
		if (verify_md5.toStr() != config.verify_code) {
			return CC_ERR_LICENSE_VERIFY;
		}

		// 赋值
		_project = config.project;
		_expired = config.expired;
		_limit_period = config.limit_period;
		_limit_hardware = config.limit_hardware;
		_license_key = LicenseKey(config.license_key);

		if (_limit_hardware) {
			if (config.license.empty()) {
				return CC_ERR_LICENSE_MISSING_FIELD;
			}
			AesEncryptor aes_license((unsigned char*)_license_key.c_str());
			_license = aes_license.DecryptString(config.license);
		}

		_init = true;

		return CC_OK;
	}

	int Authenticator::Authorize()
	{
		if (!_init) {
			return CC_ERR_LICENSE;
		}
		if (_limit_period) {
			if (_expired < GetTimestamp()) {
				return CC_ERR_LICENSE_EXPIRED;
			}
		}
		if (_limit_hardware) {
			std::string hardware_code = HardwareCode();
			if (_license != hardware_code) {
				return CC_ERR_LICENSE_HARDWARE;
			}
		}
		return CC_OK;
	}

	int Authenticator::Authorize(const std::string& license)
	{
		FusionServiceConfig config;

		if (!config.loadConfigFromString(license)){
			return CC_INVALID_ARGUMENT;
		}

		if (config.license_key.empty() || config.verify_code.empty() || config.license.empty()) {
			return CC_ERR_LICENSE_MISSING_FIELD;
		}
		if (config.license_key.length() != KEY_LENGTH || config.verify_code.length() != 32) {
			return CC_ERR_LICENSE_INVALID_FIELD;
		}

		// 新增必须校验硬件信息
		if (config.limit_hardware == false) {
			return CC_ERR_LICENSE_INVALID_FIELD;
		}

		// 先获取组合后的授权信息
		std::string verify_content = Merge(config);
		// 使用license_key生成verify_key
		std::string verify_key = VerifyKey(config.license_key);
		// 使用verify_key对verify_content进行AES加密
		AesEncryptor aes_verify((unsigned char*)verify_key.c_str());
		std::string verify_encrypted = aes_verify.EncryptString(verify_content);
		// 使用MD5后的AES字符串与verify_code进行比对
		MD5 verify_md5(verify_encrypted);
		// 不相符则说明字符串被篡改，授权失败
		if (verify_md5.toStr() != config.verify_code) {
			return CC_ERR_LICENSE_VERIFY;
		}

		// 赋值
		_project = config.project;
		_expired = config.expired;
		_limit_period = config.limit_period;
		_limit_hardware = config.limit_hardware;
		_license_key = LicenseKey(config.license_key);

		if (_limit_hardware) {
			if (config.license.empty()) {
				return CC_ERR_LICENSE_MISSING_FIELD;
			}
			AesEncryptor aes_license((unsigned char*)_license_key.c_str());
			_license = aes_license.DecryptString(config.license);
		}

		if (_limit_period) {
			if (_expired < GetTimestamp()) {
				return CC_ERR_LICENSE_EXPIRED;
			}
		}
		if (_limit_hardware) {
			std::string hardware_code = HardwareCode();
			if (_license != hardware_code) {
				return CC_ERR_LICENSE_HARDWARE;
			}
		}
		return CC_OK;
	}

	void Authenticator::Generate(const FusionServiceConfig& config)
	{
		// 生成对外输出的授权码
		std::string output_license_key = LicenseKey();
		std::string real_license_key = LicenseKey(output_license_key);
		std::string hardware_code = HardwareCode();
		AesEncryptor aes_license((unsigned char*)real_license_key.c_str());
		FusionServiceConfig output_config = config;
		output_config.license_key = output_license_key;
		output_config.license = aes_license.EncryptString(hardware_code);
		std::string output_verify_content = Merge(output_config);

		std::string verify_key = VerifyKey(output_license_key);
		AesEncryptor aes_verify((unsigned char*)verify_key.c_str());
		std::string verify_encrypted = aes_verify.EncryptString(output_verify_content);
		MD5 verify_md5(verify_encrypted);
		
		printf("--------------------------------------------------------------\n");
		printf("Please check if there are any issues with the following information:\n");
		printf("Project: %s\n", config.project.c_str());
		printf("Expired: %zu\n", config.expired);
		printf("Limit Period: %s\n", config.limit_period ? "true" : "false");
		printf("Limit Hardware: %s\n", config.limit_hardware ? "true" : "false");
		printf("--------------------------------------------------------------\n");
		printf("Hardware Code: %s\n", hardware_code.c_str());
		printf("Integrated Content: %s\n", output_verify_content.c_str());
		printf("--------------------------------------------------------------\n");
		printf("Fill in the printed content below into the FusionService JSON configuration file:\n");
		printf("license_key: %s\n", output_license_key.c_str());
		printf("verify_code: %s\n", verify_md5.toStr().c_str());
		printf("license: %s\n", output_config.license.c_str());
		printf("--------------------------------------------------------------\n");
	}

	std::string Authenticator::Merge(const FusionServiceConfig& config)
	{
		std::stringstream ss;
		ss << MAGIC_CODE << "|" << config.project << "|" << config.expired << "|" << config.limit_period << "|" << config.limit_hardware << "|" << config.license_key << "|" << config.license;

		return ss.str();
	}

	std::string Authenticator::VerifyKey(const std::string& from)
	{
		std::string to;
		if (from.empty()) {
			return to;
		}
		if (from.length() != KEY_LENGTH) {
			return to;
		}

		to = std::string(from.c_str());

		std::swap(to[0], to[1]);
		std::swap(to[2], to[3]);
		std::swap(to[4], to[5]);
		std::swap(to[7], to[9]);
		std::swap(to[10], to[12]);
		std::swap(to[13], to[15]);

		for (size_t i = 1; i < KEY_LENGTH; i += 2) {
			to[i] = to[i] ^ 0x1F;
		}

		return to;
	}

	std::string Authenticator::LicenseKey(const std::string& from)
	{
		std::string to;
		if (from.empty()) {
			return to;
		}
		if (from.length() != KEY_LENGTH) {
			return to;
		}

		to = std::string(from.c_str());

		for (size_t i = 0; i < KEY_LENGTH; ++i) {
			to[i] = to[i] ^ 0x5A;
		}

		std::swap(to[0], to[15]);
		std::swap(to[1], to[14]);
		std::swap(to[3], to[12]);
		std::swap(to[4], to[11]);
		std::swap(to[6], to[9]);
		std::swap(to[7], to[8]);

		AesEncryptor aes((unsigned char*)to.c_str());
		MD5 md5(from);
		to = aes.EncryptString(md5.toStr());

		return to;
	}



	std::string Authenticator::LicenseKey()
	{
		std::string password;
		const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<int> distribution(0, characters.length() - 1);

		for (int i = 0; i < KEY_LENGTH; ++i) {
			password += characters[distribution(generator)];
		}

		return password;
	}

	std::string Authenticator::HardwareCode()
	{
		std::string hard_drive_sn = inner::GetHardDriveSerialNumber();
		std::string mac_address = inner::GetMACAddress();
		std::stringstream ss;
		ss << hard_drive_sn << "@" << mac_address;
		return ss.str();
	}

	uint64_t Authenticator::GetTimestamp()
	{
		return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count());
	}

}

