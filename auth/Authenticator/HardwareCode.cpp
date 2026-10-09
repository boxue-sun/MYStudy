#include "HardwareCode.h"
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#endif

namespace FusionService {

	namespace inner {

#ifdef _WIN32

		std::string GetHardDriveSerialNumber() {
            DWORD dwSize = MAX_PATH;
            char szLogicalDrives[MAX_PATH] = { 0 };
            DWORD dwResult = GetLogicalDriveStrings(dwSize, szLogicalDrives);

            if (dwResult > 0 && dwResult <= MAX_PATH) {
                char* szSingleDrive = szLogicalDrives;
                while (*szSingleDrive) {
                    // 获取驱动类型
                    UINT nDriveType = GetDriveType(szSingleDrive);
                    if (nDriveType == DRIVE_FIXED) {
                        // 准备CreateFile函数需要的驱动器名称格式
                        std::string strDrivePath = "\\\\.\\";
                        strDrivePath.append(szSingleDrive, 0, 2);

                        // 打开驱动器
                        HANDLE hDevice = CreateFile(strDrivePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                        if (hDevice != INVALID_HANDLE_VALUE) {
                            DWORD dwBytesReturned = 0;
                            std::unique_ptr<BYTE[]> buffer(new BYTE[1024]);
                            STORAGE_PROPERTY_QUERY storagePropertyQuery;
                            memset(&storagePropertyQuery, 0, sizeof(STORAGE_PROPERTY_QUERY));
                            storagePropertyQuery.PropertyId = StorageDeviceProperty;
                            storagePropertyQuery.QueryType = PropertyStandardQuery;

                            // 获取设备描述信息
                            if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
                                buffer.get(), 1024, &dwBytesReturned, NULL)) {
                                STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer.get();
                                if (deviceDescriptor->SerialNumberOffset != 0) {
                                    // 输出硬盘序列号
                                    std::string serialNumber = (char*)buffer.get() + deviceDescriptor->SerialNumberOffset;
                                    CloseHandle(hDevice);
                                    return serialNumber;
                                }
                            }
                            CloseHandle(hDevice);
                        }
                    }
                    // 移动到下一个驱动器
                    szSingleDrive += strlen(szSingleDrive) + 1;
                }
            }

            return std::string();
		}

        std::string GetMACAddress() {
            IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information for up to 16 NICs
            DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

            DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
                AdapterInfo,                 // [out] buffer to receive data
                &dwBufLen);                  // [in] size of receive data buffer
            if (dwStatus != ERROR_SUCCESS) {
                return std::string(); // Failed to get network adapter information
            }

            PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to current adapter info
            std::ostringstream macAddressStream;
            while (pAdapterInfo) {    // Iterate through all of the adapters
                macAddressStream << std::hex; // Format as hexadecimal
                for (BYTE i = 0; i < pAdapterInfo->AddressLength; i++) {
                    if (i != 0) {
                        macAddressStream << "-";
                    }
                    macAddressStream << std::setw(2) << std::setfill('0') << static_cast<int>(pAdapterInfo->Address[i]);
                }
                return macAddressStream.str(); // Return the MAC address of the first adapter
                pAdapterInfo = pAdapterInfo->Next; // Move to next adapter
            }

            return std::string(); // In case no adapter is found
        }

#else

        std::string GetHardDriveSerialNumber() {
            
            const std::vector<std::string> drives = { "sda", "sda1", "sda2", "sdb", "mmcblk0", "nvme0n1" };
            
            std::stringstream ss;

            for (size_t i = 0; i < drives.size(); ++i) {
                std::string serialNumberPath = "/sys/class/block/" + drives[i] + "/device/serial";
                std::ifstream file(serialNumberPath.c_str());
                std::string serialNumber;
                if (file.good()) {
					std::getline(file, serialNumber);
					file.close();
					ss << serialNumber << ".";
				}
			}

            return ss.str();
        }

        std::string GetMACAddress() {

            const std::vector<std::string> ifaces = { "eth0", "eth1", "eth2", "eno1", "enp4s0"};

            std::stringstream ss;

            for (size_t i = 0; i < ifaces.size(); ++i) {

                int fd;
                struct ifreq ifr;
                unsigned char mac[6];

                fd = socket(AF_INET, SOCK_DGRAM, 0);
                ifr.ifr_addr.sa_family = AF_INET;
                strncpy(ifr.ifr_name, ifaces[i].c_str(), IFNAMSIZ - 1);

                if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
                    close(fd);
                    continue; // Unable to get MAC address
                }

                close(fd);

                memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);

                std::ostringstream macAddressStream;
                macAddressStream << std::hex << std::setfill('0');
                for (int i = 0; i < 6; ++i) {
                    macAddressStream << std::setw(2) << (int)mac[i];
                    if (i != 5)
                        macAddressStream << ":";
                }
                ss << macAddressStream.str() << ".";
            }

            return ss.str();
        }





#endif

	}

}