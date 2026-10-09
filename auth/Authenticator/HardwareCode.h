#ifndef _AUTHENTICATOR_HARDARECODE_H_
#define _AUTHENTICATOR_HARDARECODE_H_

#include <string>

namespace FusionService {

	namespace inner {

		/**
		 * 获取硬盘序列号
		 * @return 硬盘序列号
		 */
		std::string GetHardDriveSerialNumber();

		/**
		 * 获取网卡MAC地址
		 * @return MAC地址
		 */
		std::string GetMACAddress();

	}

}

#endif