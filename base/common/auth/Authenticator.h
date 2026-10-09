#ifndef _AUTHENTICATOR_AUTHENTICATOR_H_
#define _AUTHENTICATOR_AUTHENTICATOR_H_

#include "Common.h"
#include "FusionServiceConfig.h"
#include <string>

namespace FusionService {

	/*
	 * @brief 验证器类
	 */
	class Authenticator
	{
	public:

		Authenticator() : _project(), _expired(0), _init(false), _limit_period(true), _limit_hardware(true), _license_key(), _license() {}

		/**
		 * 初始化验证器
		 * 主要使用verify_code所有授权信息相关的字段，确定无篡改后再读取授权信息
		 * @param config 配置文件，包含授权信息
		 * @return 初始化成功返回0，否则返回错误码
		 */
		int Init(const FusionServiceConfig& config);

		/**
		 * 鉴权
		 * @return 鉴权成功返回0，否则返回错误码
		 */
		int Authorize();

		/**
		 * 鉴权
		 * @return 鉴权成功返回0，否则返回错误码
		 */
		int Authorize(const std::string& license);

		/**
		 * 生成授权信息，仅限生成授权信息时调用
		 * 需要用到的字段包括：project，expired，limit_period，limit_hardware。执行成功后会生成license_key，verify_code，license
		 * @param config 配置文件，包含上述需要用到的字段
		 */
		void Generate(const FusionServiceConfig& config);

		/**
		 * 获取验证器类实例
		 * @return 验证器类
		 */
		static Authenticator& GetInstance() { return _authenticator; }

		// 验证器静态实例
		static Authenticator _authenticator;

	protected:

		std::string Merge(const FusionServiceConfig& config);

		std::string VerifyKey(const std::string& from);

		std::string LicenseKey(const std::string& from);

		std::string LicenseKey();

		std::string HardwareCode();

		uint64_t GetTimestamp();

	private:

		std::string _project;
		uint64_t _expired;
		bool _init;
		bool _limit_period;
		bool _limit_hardware;
		std::string _license_key;
		std::string _license;


	};


}

#endif