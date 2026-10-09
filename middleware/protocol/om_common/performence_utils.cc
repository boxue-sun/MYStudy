/*********************************************************************************
* @file		    performence_utils.h
* @brief		performence_utils.h belongs to CICTCI
* @details
* @author		alfred
* @email        zhangenwei64@gmail.com
* @date		    24-8-22
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-8-22 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#include "performence_utils.h"
#include <cuda_runtime.h>
#include <cstdio>
#include <cstring>
NAMESPACE_START_OM_COMPONENT_COMMON
bool PerformenceUtils::printInfo = true;
std::string PerformenceUtils::m_RootDirName = "/media";
std::string PerformenceUtils::m_WorkDirName = "/etc/hosts";
int  PerformenceUtils::ByteUnit = 1024;
bool PerformenceUtils::getNetInfo(NetInfo &netInfo, std::string nic)
{
    std::string netFile = "/proc/net/dev";
    std::ostringstream   ss;
    int fd = open(netFile.c_str(), O_RDONLY);
    if (fd == -1)
    {
        OM_DS_ERROR_PRINT << "Unable to open /proc/net/dev";
        return false;
    }
    
    // 尝试获取共享锁
    if (flock(fd, LOCK_SH) == -1)
    {
        OM_DS_ERROR_PRINT << "Unable to lock /proc/net/dev";
        close(fd);
        return false;
    }
    // 生成时间戳
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
    // 构造拷贝文件名
    std::string copyFile = "/tmp/net_dev_" + std::string(timestamp) + ".txt";
    
    // 复制 /proc/net/dev 到新文件
    {
        std::ifstream src(netFile, std::ios::binary);
        std::ofstream dst(copyFile, std::ios::binary);
        dst << src.rdbuf();
    }

    // 释放锁并关闭文件描述符
    flock(fd, LOCK_UN);
    close(fd);
    
    std::ifstream file(copyFile.c_str());
    if (!file.is_open()) {
        OM_DS_ERROR_PRINT << "Unable to open /proc/net/dev";
        flock(fd, LOCK_UN); // 释放锁
        close(fd);
        return false;
    }
    
    std::string line;
    // 跳过前两行标题
    std::getline(file, line);
    std::getline(file, line);
    
    std::map<std::string, NetworkStats> currentStats;  // 当前的网络统计
    NetworkStats totalStats{0, 0, 0, 0}; // 初始化总统计

    // 读取网络接口数据
    while (std::getline(file, line))
    {
        NetworkStats stats{0, 0, 0, 0};
        //        parseNetworkStats(line, stats);
        
        std::istringstream iss(line);
        std::string interfaceName;
        
        // 读取并丢弃接口名称，提取统计数据
        iss >> interfaceName; // 读取接口名称
        interfaceName = interfaceName.substr(0, interfaceName.size() - 1); // 去掉冒号

        // 读取接收和发送的数据
        iss >> stats.bytesReceived >> stats.packetsReceived;
        iss >>  stats.errs >> stats.drop >> stats.fifo >> stats.frame >> stats.compressed >> stats.multicast;
        iss >> stats.bytesTransmitted >> stats.packetsTransmitted; // 直接读取发送的字节和包数
       // 忽略中间的错误、丢失、fifo、帧等字段
        iss.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // 忽略不必要的字段
        
        
        if (!nic.empty())
        {
            // 只处理 eth0 接口的统计信息
            if (interfaceName == nic) {
                currentStats[interfaceName] = stats; // 存储当前接口的统计信息
                totalStats = totalStats + stats; // 对 eth0 接口信息进行累加
            }
        }
        else
        {
            // 忽略接口名称为 "lo" 的统计信息
            if (interfaceName.find("lo") == 0 ||
                interfaceName.find("usb") == 0 ||
                interfaceName.find("can") == 0 ||
                interfaceName.find("docker") == 0 ||
                interfaceName.find("veth") == 0 ||
                interfaceName.find("dummy") == 0) {
                continue;
            }
            currentStats[interfaceName] = stats; // 存储当前接口的统计信息
            // 对各个接口信息进行累加
            totalStats = totalStats + stats;
        }
    }
    
    // 关闭读取文件
    file.close();
    
    // 删除拷贝的文件
    if (std::remove(copyFile.c_str()) != 0)
    {
        OM_DS_ERROR_PRINT << "Error deleting temporary file: " << copyFile;
    }
    
    netInfo.rx = totalStats.packetsReceived;
    netInfo.rxByte = totalStats.bytesReceived;
    netInfo.tx = totalStats.packetsTransmitted;
    netInfo.txByte = totalStats.bytesTransmitted;
    if(printInfo)
    {
        ss << std::left << std::setw(20) << "net-rx: " << totalStats.packetsReceived << std::endl;
        ss << std::left << std::setw(20) << "net-rxByte: "   << totalStats.bytesReceived << std::endl;
        ss << std::left << std::setw(20) << "net-tx: "   << totalStats.packetsTransmitted << std::endl;
        ss << std::left << std::setw(20) << "net-txByte: "   << totalStats.bytesTransmitted << std::endl;
        // OM_DS_DEBUG_PRINT << ss.str() << std::endl;
        // OM_DS_WARN_PRINT << "----------------------------------------" << std::endl;
    }
    
    return true;
}

// bool  PerformenceUtils::getDiskInfo(DiskInfo &diskInfo)
// {
//     std::ostringstream  ss;
//     DisksInfoSt disksInfoSt;
//
//     {
//         FILE* pipe = popen("df --block-size=1", "r"); // 设置为 1 字节
//         if (!pipe)
//         {
//             OM_DS_WARN_PRINT << "popen() failed!";
//             return false;
//         }
//
//         char buffer[256];
//         std::string line;
//         disksInfoSt.totalCapacity = 0;
//         disksInfoSt.usedCapacity = 0;
//         disksInfoSt.availableCapacity = 0;
//
//         // 跳过第一行标题
//         fgets(buffer, sizeof(buffer), pipe);
//
//         // 读取各个磁盘的信息
//         while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
//             line = buffer;
//             std::istringstream iss(line);
//             std::string filesystem; // 文件系统名称
//             unsigned long capacity, used, available;
//             // 解析行中的信息
//             iss >> filesystem >> capacity >> used >> available;
//
//             // 检查文件系统名称是否包含 "overlay"
//             if (line.find("overlay") == std::string::npos) {
//                 disksInfoSt.totalCapacity += capacity;
//                 disksInfoSt.usedCapacity += used;
//                 disksInfoSt.availableCapacity += available;
//             }
//         }
//         pclose(pipe);
//     }
//
//     {
//         std::string diskFile = "/proc/diskstats";
//         int fd = open(diskFile.c_str(), O_RDONLY);
//         if (fd == -1) {
//             OM_DS_WARN_PRINT << "Unable to open /proc/diskstats";
//             return false;
//         }
//
//         // 尝试获取共享锁
//         if (flock(fd, LOCK_SH) == -1)
//         {
//             OM_DS_WARN_PRINT << "Unable to lock /proc/diskstats";
//             close(fd);
//             return false;
//         }
//
//         // 生成时间戳
//         std::time_t now = std::time(nullptr);
//         char timestamp[20];
//         std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
//         // 构造拷贝文件名
//         std::string copyFile = "/tmp/diskstats_" + std::string(timestamp) + ".txt";
//
//         // 复制 /proc/diskstats 到新文件
//         {
//             std::ifstream src(diskFile, std::ios::binary);
//             std::ofstream dst(copyFile, std::ios::binary);
//             dst << src.rdbuf();
//         }
//
//         // 释放锁并关闭文件描述符
//         flock(fd, LOCK_UN);
//         close(fd);
//
//         std::ifstream file(copyFile.c_str());
//         if (!file.is_open())
//         {
//             OM_DS_WARN_PRINT << "Unable to open /proc/diskstats";
//             flock(fd, LOCK_UN); // 释放锁
//             close(fd);
//             return false;
//         }
//
//         std::string line;
//         while (std::getline(file, line))
//         {
//             std::istringstream iss(line);
//             DiskStat stat;
//             int major, minor;
//
//             iss >> major >> minor >> disksInfoSt.deviceName
//                     >> stat.readsCompleted >> std::ws // 使用 std::ws 跳过空白
//                     >> std::ws >> stat.writesCompleted
//                     >> std::ws >> stat.blocksRead
//                     >> std::ws >> stat.blocksWritten;
//
//             // 只记录 mmcblk0、nvme0n1、loop等设备的统计信息，其他可以根据需要
//             if (disksInfoSt.deviceName.find("overlay") == std::string::npos)
//             {
//                 disksInfoSt.diskStats.push_back(stat);
//             }
//         }
//
//         // 关闭读取文件
//         file.close();
//
//         // 删除拷贝的文件
//         if (std::remove(copyFile.c_str()) != 0) {
//             OM_DS_WARN_PRINT << "Error deleting temporary file: " << copyFile;
//         }
//     }
//
//     {
//
//         float total = disksInfoSt.totalCapacity ;
//         float used = disksInfoSt.usedCapacity ;
//         float free = disksInfoSt.availableCapacity;
//
//         unsigned long totalIORequests = 0;
//         unsigned long totalBytesWritten = 0;
//         unsigned long totalBytesRead = 0;
//
//         const unsigned long blockSize = 512; // 每块的字节数
//
//         for (const auto& stat : disksInfoSt.diskStats) {
//             totalIORequests += (stat.readsCompleted + stat.writesCompleted);
//             totalBytesWritten += (stat.blocksWritten * blockSize);
//             totalBytesRead += (stat.blocksRead * blockSize);
//         }
//         // 转换为 MB
//         double totalMBWritten = totalBytesWritten;
//         double totalMBRead = totalBytesRead;
//
//         if(printInfo)
//         {
//             ss << std::left << std::setw(20) << "disk-total: " << total << " B" << std::endl;
//             ss << std::left << std::setw(20) << "disk-used: " << used << " B" << std::endl;
//             ss << std::left << std::setw(20) << "disk-free: " << free << " B" << std::endl;
//             ss << std::left << std::setw(20) << "disk-tps: " << totalIORequests << std::endl;
//             ss << std::left << std::setw(20) << "disk-write: " << totalMBWritten << " B" << std::endl;
//             ss << std::left << std::setw(20) << "disk-read: " << totalMBRead << " B" << std::endl;
//         }
//         diskInfo.total =  total;
//         diskInfo.used =  used;
//         diskInfo.free =  free;
//         diskInfo.tps =  totalIORequests;
//         diskInfo.write =  totalMBWritten;
//         diskInfo.read =  totalMBRead;
//     }
//     if(printInfo) {
//         std::cout << ss.str() << std::endl;
//         std::cout << std::left << std::setw(20) << "----------------------------------------" << std::endl;
//     }
//     OM_DS_WARN_PRINT << "[perf-disk]" <<  ss.str();
//     return true;
// }
bool PerformenceUtils::getDiskInfo(DiskInfo& diskInfo)
{
	std::ostringstream ss;
	DisksInfoSt disksInfoSt;

	// 1. 获取磁盘容量信息
	if (!getDiskCapacityInfo(disksInfoSt))
	{
		OM_DS_ERROR_PRINT << "[error]获取磁盘容量信息";
		return false;
	}

	// 2. 获取磁盘I/O统计信息
	if (!getDiskIOStats(disksInfoSt))
	{
		OM_DS_ERROR_PRINT << "[error]获取磁盘I/O统计信息";
		return false;
	}

	// 3. 计算并填充结果
	// 查找根目录和/work目录的信息
	MountPointInfo rootInfo;
	MountPointInfo workInfo;
	bool rootFound = false;
	bool workFound = false;

	for (const auto& mp : disksInfoSt.mountPoints)
	{
		if (mp.mountPoint == m_RootDirName)
		{
			rootInfo = mp;
			rootFound = true;
		}
		else if (mp.mountPoint == m_WorkDirName)
		{
			workInfo = mp;
			workFound = true;
		}
	}

	// 如果找不到某个目录，使用默认值
	if (!rootFound)
	{
		rootInfo.mountPoint = m_RootDirName;
		rootInfo.totalCapacity = 0;
		rootInfo.usedCapacity = 0;
		rootInfo.availableCapacity = 0;
	}

	if (!workFound)
	{
		workInfo.mountPoint = m_WorkDirName;
		workInfo.totalCapacity = 0;
		workInfo.usedCapacity = 0;
		workInfo.availableCapacity = 0;
	}

	// 计算I/O统计
	unsigned long totalIORequests = 0;
	unsigned long totalBytesWritten = 0;
	unsigned long totalBytesRead = 0;

	const unsigned long blockSize = 512; // 每块的字节数

	for (const auto& stat : disksInfoSt.diskStats)
	{
		totalIORequests += (stat.readsCompleted + stat.writesCompleted);
		totalBytesWritten += (stat.blocksWritten * blockSize);
		totalBytesRead += (stat.blocksRead * blockSize);
	}

	// 填充结果 - 使用根目录的信息作为主要信息
	diskInfo.total = rootInfo.totalCapacity + workInfo.totalCapacity;
	diskInfo.used = rootInfo.usedCapacity + workInfo.usedCapacity;
	diskInfo.free = rootInfo.availableCapacity + workInfo.availableCapacity;
	diskInfo.tps = totalIORequests;
	diskInfo.write = totalBytesWritten;
	diskInfo.read = totalBytesRead;
	diskInfo.mountPoints = disksInfoSt.mountPoints;
	// 输出信息
	if (printInfo)
	{
		// 根目录信息
		ss << "根目录(/)信息:" << std::endl;
		ss << std::left << std::setw(20) << "  disk-total: " << formatBytes(rootInfo.totalCapacity) << "(" <<
			rootInfo.totalCapacity << ")" << std::endl;
		ss << std::left << std::setw(20) << "  disk-used: " << formatBytes(rootInfo.usedCapacity) << "(" << rootInfo
			.usedCapacity << ")" << std::endl;
		ss << std::left << std::setw(20) << "  disk-free: " << formatBytes(rootInfo.availableCapacity) << "(" <<
			rootInfo.availableCapacity << ")" << std::endl;

		// /work目录信息（如果存在）
		if (workFound)
		{
			ss << "/work目录信息:" << std::endl;
			ss << std::left << std::setw(20) << "  disk-total: " << formatBytes(workInfo.totalCapacity) << "(" <<
				workInfo.totalCapacity << ")" <<
				std::endl;
			ss << std::left << std::setw(20) << "  disk-used: " << formatBytes(workInfo.usedCapacity) << "(" <<
				workInfo.usedCapacity << ")" << std::endl;
			ss << std::left << std::setw(20) << "  disk-free: " << formatBytes(workInfo.availableCapacity) << "(" <<
				workInfo.availableCapacity << ")" <<
				std::endl;
		}
		ss << "总的磁盘信息:" << std::endl;
		ss << std::left << std::setw(20) << "  disk-total: " << formatBytes(diskInfo.total) <<
			std::endl;
		ss << std::left << std::setw(20) << "  disk-used: " << formatBytes(diskInfo.used) << std::endl;
		ss << std::left << std::setw(20) << "  disk-free: " << formatBytes(diskInfo.free) <<
			std::endl;

		// I/O统计信息
		ss << "I/O统计信息:" << std::endl;
		ss << std::left << std::setw(20) << "  disk-tps: " << totalIORequests << std::endl;
		ss << std::left << std::setw(20) << "  disk-write: " << formatBytes(totalBytesWritten) << std::endl;
		ss << std::left << std::setw(20) << "  disk-read: " << formatBytes(totalBytesRead) << std::endl;

		// std::cout << ss.str() << std::endl;
		// std::cout << std::left << std::setw(20) << "----------------------------------------" << std::endl;
	}

	// OM_DS_DEBUG_PRINT << "[perf-disk]" << ss.str();
	return true;
}

// 获取磁盘容量信息
bool PerformenceUtils::getDiskCapacityInfo(DisksInfoSt& disksInfoSt)
{
	// 使用RAII方式管理资源
	struct PipeDeleter
	{
		void operator()(FILE* pipe) { if (pipe) pclose(pipe); }
	};
	std::unique_ptr<FILE, PipeDeleter> pipe(popen("df --block-size=1", "r"));

	if (!pipe)
	{
		OM_DS_ERROR_PRINT << "popen() failed!";
		return false;
	}
	char buffer[512]; // 增大缓冲区以处理长行
	std::string line;
	// 跳过第一行标题
	if (!fgets(buffer, sizeof(buffer), pipe.get()))
	{
		OM_DS_ERROR_PRINT << "Failed to fgets";
		return false;
	}
	// 读取各个磁盘的信息
	while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
	{
		line = buffer;
		// 跳过overlay文件系统
		if (line.find("overlay") != std::string::npos)
		{
			continue;
		}

		// 处理根目录和/work目录
		if (line.find(m_RootDirName) != std::string::npos || line.find(m_WorkDirName) != std::string::npos)
		{
			// OM_DS_WARN_PRINT << "[notice]find dir";
			std::istringstream iss(line);
			std::string filesystem;
			unsigned long capacity, used, available;
			std::string usagePercent, mountPoint;
			// 解析行中的信息 (文件系统 容量 已用 可用 使用率 挂载点)
			if (iss >> filesystem >> capacity >> used >> available >> usagePercent >> mountPoint)
			{
				// 创建新的挂载点信息
				MountPointInfo mpInfo;
				mpInfo.mountPoint = mountPoint;
				mpInfo.totalCapacity = capacity;
				mpInfo.usedCapacity = used;
				mpInfo.availableCapacity = available;

				// 添加到挂载点列表
				disksInfoSt.mountPoints.push_back(mpInfo);
			}
		}
		else
		{
			// OM_DS_WARN_PRINT << "[notice]line.find(m_RootDirName) != std::string::npos || line.find(m_WorkDirName) != std::string::npos";
		}
	}

	return !disksInfoSt.mountPoints.empty();
}

// 获取磁盘I/O统计信息
bool PerformenceUtils::getDiskIOStats(DisksInfoSt& disksInfoSt)
{
	const std::string diskFile = "/proc/diskstats";

	// 直接打开文件
	int fd = open(diskFile.c_str(), O_RDONLY);
	if (fd == -1)
	{
		OM_DS_ERROR_PRINT << "Unable to open " << diskFile;
		return false;
	}

	// 确保在函数结束时关闭文件描述符
	struct FDGuard
	{
		int fd;

		~FDGuard()
		{
			if (fd != -1)
			{
				flock(fd, LOCK_UN);
				close(fd);
			}
		}
	} fdGuard{fd};

	// 尝试获取共享锁
	if (flock(fd, LOCK_SH) == -1)
	{
		OM_DS_ERROR_PRINT << "Unable to lock " << diskFile;
		return false;
	}

	// 使用临时文件API创建临时文件
	char tmpfile[] = "/tmp/diskstats_XXXXXX";
	int tmp_fd = mkstemp(tmpfile);
	if (tmp_fd == -1)
	{
		OM_DS_ERROR_PRINT << "Failed to create temporary file";
		return false;
	}
	close(tmp_fd); // 关闭文件描述符，我们只需要文件名

	// 确保在函数结束时删除临时文件
	struct TempFileGuard
	{
		const char* filename;
		~TempFileGuard() { if (filename) std::remove(filename); }
	} tempGuard{tmpfile};

	// 复制 /proc/diskstats 到临时文件
	{
		std::ifstream src(diskFile, std::ios::binary);
		std::ofstream dst(tmpfile, std::ios::binary);
		if (!src || !dst)
		{
			OM_DS_ERROR_PRINT << "Failed to copy " << diskFile;
			return false;
		}
		dst << src.rdbuf();
	}

	// 读取临时文件
	std::ifstream file(tmpfile);
	if (!file.is_open())
	{
		OM_DS_ERROR_PRINT << "Unable to open temporary file: " << tmpfile;
		return false;
	}

	std::string line;
	std::vector<std::string> relevantDevices = {"mmcblk0", "nvme0n1"};

	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		int major, minor;
		std::string deviceName;
		DiskStat stat;

		// 读取设备信息
		if (!(iss >> major >> minor >> deviceName))
		{
			continue;
		}

		// 检查是否为相关设备
		bool isRelevantDevice = false;
		for (const auto& device : relevantDevices)
		{
			if (deviceName.find(device) != std::string::npos)
			{
				isRelevantDevice = true;
				break;
			}
		}

		if (!isRelevantDevice)
		{
			continue;
		}

		// 读取统计信息
		if (!(iss >> stat.readsCompleted >> stat.readsMerged >> stat.sectorsRead >> stat.readTime
			>> stat.writesCompleted >> stat.writesMerged >> stat.sectorsWritten >> stat.writeTime
			>> stat.ioInProgress >> stat.ioTime >> stat.weightedIoTime))
		{
			continue;
		}

		// 存储块读写数据
		stat.blocksRead = stat.sectorsRead;
		stat.blocksWritten = stat.sectorsWritten;

		// 保存设备名称和统计信息
		stat.deviceName = deviceName;
		disksInfoSt.diskStats.push_back(stat);
	}

	return !disksInfoSt.diskStats.empty();
}

// 格式化字节数为可读形式
std::string PerformenceUtils::formatBytes(double bytes)
{
	const char* units[] = {" B", " KB", " MB", " GB", " TB"};
	int unitIndex = 0;

	while (bytes >= 1024.0 && unitIndex < 4)
	{
		bytes /= 1024.0;
		unitIndex++;
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << bytes << units[unitIndex];
	return oss.str();
}
bool PerformenceUtils::getMemInfo(MemInfo &memInfo)
{
    std::ostringstream  ss;

    std::string memFile = "/proc/meminfo";
    int fd = open(memFile.c_str(), O_RDONLY);
    if (fd == -1)
    {
        OM_DS_ERROR_PRINT << "Unable to open /proc/meminfo";
        return false;
    }
    
    // 尝试获取共享锁
    if (flock(fd, LOCK_SH) == -1)
    {
        OM_DS_ERROR_PRINT << "Unable to lock /proc/meminfo";
        close(fd);
        return false;
    }
    // 生成时间戳
    std::time_t now = std::time(nullptr);
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
    
    // 构造拷贝文件名
    std::string copyFile = "/tmp/meminfo_" + std::string(timestamp) + ".txt";

    // 复制 /proc/meminfo 到新文件
    {
        std::ifstream src(memFile, std::ios::binary);
        std::ofstream dst(copyFile, std::ios::binary);
        dst << src.rdbuf();
    }

    // 释放锁并关闭文件描述符
    flock(fd, LOCK_UN);
    close(fd);
    std::ifstream file(copyFile.c_str());
    if (!file.is_open()) {
        OM_DS_ERROR_PRINT << "Unable to open /proc/meminfo";
        flock(fd, LOCK_UN); // 释放锁
        close(fd);
        return false;
    }
    std::string line;
    unsigned long totalMem = 0;
    unsigned long freeMem = 0;
    unsigned long availableMem = 0;
    
    while (std::getline(file, line))
    {
        if (line.find("MemTotal:") == 0)
        {
            sscanf(line.c_str(), "MemTotal: %lu kB", &totalMem);
        } else if (line.find("MemFree:") == 0) {
            sscanf(line.c_str(), "MemFree: %lu kB", &freeMem);
        } else if (line.find("MemAvailable:") == 0) {
            sscanf(line.c_str(), "MemAvailable: %lu kB", &availableMem);
        }
    }

    file.close();

    // 计算已用内存
    unsigned long usedMem = totalMem - freeMem;
    
    memInfo.total = (double) totalMem * ByteUnit;
    memInfo.used = (double) usedMem * ByteUnit;
    memInfo.free = (double) availableMem * ByteUnit;
    if(printInfo) {
        ss << std::left << std::setw(20) << "mem-total:" << memInfo.total << "B" << std::endl;
        ss << std::left << std::setw(20) << "mem-used:" << memInfo.used << "B" << std::endl;
        ss << std::left << std::setw(20) << "mem-free:" << memInfo.free << "B" << std::endl;

        // 输出到标准输出
        OM_DS_DEBUG_PRINT << ss.str() ;
        // std::cout << std::left << std::setw(20) << "----------------------------------------" << std::endl;
    }
    // 删除拷贝的文件
    if (std::remove(copyFile.c_str()) != 0)
    {
        OM_DS_ERROR_PRINT << "Error deleting temporary file: " << copyFile;
    }
    return true;
}

bool PerformenceUtils::getCpuInfo(CpuInfo &cpuInfo)
{
    std::ostringstream  ss;
    try {
        {
            std::string cpuStatFile = "/proc/stat";
            int fd = open(cpuStatFile.c_str(), O_RDONLY);
            if (fd == -1)
            {
                OM_DS_ERROR_PRINT << "Unable to open /proc/stat";
                return false;
            }
    
            // 尝试获取共享锁
            if (flock(fd, LOCK_SH) == -1)
            {
                OM_DS_ERROR_PRINT << "Unable to lock /proc/stat";
                close(fd);
                return false;
            }
            // 生成时间戳
            std::time_t now = std::time(nullptr);
            char timestamp[20];
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
            
            // 构造拷贝文件名
            std::string copyFile = "/tmp/stat_" + std::string(timestamp) + ".txt";

            // 复制 /proc/stat 到新文件
            {
                std::ifstream src(cpuStatFile, std::ios::binary);
                std::ofstream dst(copyFile, std::ios::binary);
                dst << src.rdbuf();
            }

            // 释放锁并关闭文件描述符
            flock(fd, LOCK_UN);
            close(fd);
            
            std::ifstream file(copyFile.c_str());
            if (!file.is_open())
            {
                OM_DS_ERROR_PRINT << "Unable to open /proc/stat";
                flock(fd, LOCK_UN); // 释放锁
                close(fd);
                return false;
            }
    
            std::string line;
            std::getline(file, line);// 读取第一行以获取 CPU 信息
            file.close();
    
            // 解析 CPU 使用信息
            std::istringstream iss(line);
            std::string cpu;
            iss >> cpu;// 获取 "cpu" 字符串
    
            std::vector<long> cpuTimes;
            long time;
            while (iss >> time)
            {
                cpuTimes.push_back(time);// 将剩余的数字存入向量
            }
    
            // 计算总使用时间和总时间
            long user = cpuTimes[0];                               // user
            long nice = cpuTimes[1];                               // nice
            long system = cpuTimes[2];                             // system
            long idle = cpuTimes[3];                               // idle
            long iowait = (cpuTimes.size() > 4) ? cpuTimes[4] : 0; // iowait (如果存在)
            long irq = (cpuTimes.size() > 5) ? cpuTimes[5] : 0;    // irq
            long softirq = (cpuTimes.size() > 6) ? cpuTimes[6] : 0;// softirq
            long steal = (cpuTimes.size() > 7) ? cpuTimes[7] : 0;  // steal
    
            long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;
            long totalUsageTime = user + nice + system + irq + softirq + steal;
    
            // 计算 CPU 利用率
            float cpuUsage = (static_cast<float>(totalUsageTime) / totalTime);
            cpuInfo.uti = std::to_string(cpuUsage);
            ss << std::left << std::setw(20) << "cpu-uti:" << cpuInfo.uti<< std::endl;
            // 删除拷贝的文件
            if (std::remove(copyFile.c_str()) != 0)
            {
                OM_DS_ERROR_PRINT << "Error deleting temporary file: " << copyFile;
            }
            
        }
    
        {
            //cpu负载
            std::string cpuLoadavgFile = "/proc/loadavg";
            int fd = open(cpuLoadavgFile.c_str(), O_RDONLY);
            if (fd == -1)
            {
                OM_DS_ERROR_PRINT << "Unable to open /proc/loadavg";
                return false;
            }
    
            // 尝试获取共享锁
            if (flock(fd, LOCK_SH) == -1)
            {
                OM_DS_ERROR_PRINT << "Unable to lock /proc/loadavg";
                close(fd);
                return false;
            }
            // 生成时间戳
            std::time_t now = std::time(nullptr);
            char timestamp[20];
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
            
            // 构造拷贝文件名
            std::string copyFile = "/tmp/loadavg_" + std::string(timestamp) + ".txt";

            // 复制 /proc/loadavg 到新文件
            {
                std::ifstream src(cpuLoadavgFile, std::ios::binary);
                std::ofstream dst(copyFile, std::ios::binary);
                dst << src.rdbuf();
            }

            // 释放锁并关闭文件描述符
            flock(fd, LOCK_UN);
            close(fd);
            
            std::ifstream file(copyFile.c_str());
            if (!file.is_open())
            {
                OM_DS_ERROR_PRINT << "[error]Unable to open /proc/loadavg";
                flock(fd, LOCK_UN); // 释放锁
                close(fd);
                return false;
            }
    
            std::string line;
            std::getline(file, line);// 读取第一行以获取负载信息
            file.close();
            // 解析负载信息
            std::istringstream iss(line);
            std::vector<std::string> loadValues;
    
            std::string load1, load5, load15, running, total;
            iss >> load1 >> load5 >> load15 >> running >> total;
    
            // 获取 CPU 核心数
            long nproc = sysconf(_SC_NPROCESSORS_ONLN);
            // 输出负载信息
            // 使用宽字符字符串
            if(printInfo)
            {
                ss << std::left << std::setw(20) << "load1:" << load1 << std::endl;
                ss << std::left << std::setw(20) << "load5:" << load5 << std::endl;
                ss << std::left << std::setw(20) << "load15:" << load15 << std::endl;
            }
            //            ss << std::left << std::setw(40)  << "当前运行的进程数:" <<  running << std::endl;
//            ss << std::left << std::setw(40)  << "总进程数:" <<  total << std::endl;
//            ss << std::left << std::setw(40)  << "CPU核心数:" <<  nproc << std::endl;
            // 解析负载值
            float loadAvg1 = std::stof(load1);
            float loadAvg5 = std::stof(load5);
            float loadAvg15 = std::stof(load15);
            //要求:百分比0,5=50%
            cpuInfo.load = std::to_string(loadAvg1/100);
            // 删除拷贝的文件
            if (std::remove(copyFile.c_str()) != 0)
            {
                OM_DS_ERROR_PRINT << "Error deleting temporary file: " << copyFile;
            }
        }

        {
            //cpu温度
            std::string cpuTempFile = "/sys/class/thermal/thermal_zone0/temp";
            int fd = open(cpuTempFile.c_str(), O_RDONLY);
            if (fd == -1) {
                OM_DS_ERROR_PRINT << "Unable to open /sys/class/thermal/thermal_zone0/temp";
                return false;
            }
    
            // 尝试获取共享锁
            if (flock(fd, LOCK_SH) == -1)
            {
                OM_DS_ERROR_PRINT << "Unable to lock /sys/class/thermal/thermal_zone0/temp";
                close(fd);
                return false;
            }
            // 生成时间戳
            std::time_t now = std::time(nullptr);
            char timestamp[20];
            std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&now));
            
            // 构造拷贝文件名
            std::string copyFile = "/tmp/cpu_temp_" + std::string(timestamp) + ".txt";

            // 复制 /sys/class/thermal/thermal_zone0/temp 到新文件
            {
                std::ifstream src(cpuTempFile, std::ios::binary);
                std::ofstream dst(copyFile, std::ios::binary);
                dst << src.rdbuf();
            }

            // 释放锁并关闭文件描述符
            flock(fd, LOCK_UN);
            close(fd);
            
            
            std::ifstream file(copyFile.c_str());
            if (!file.is_open())
            {
                OM_DS_ERROR_PRINT << "Unable to open /sys/class/thermal/thermal_zone0/temp";
                flock(fd, LOCK_UN); // 释放锁
                close(fd);
                return false;
            }

            long temp;
            file >> temp;// 读取温度值
            file.close();
            // 转换为摄氏度（温度以千摄氏度为单位）
            cpuInfo.temp = float(temp / 1000.0f);// 返回摄氏度
            if(printInfo)
            {
                ss << std::left << std::setw(20) << "cpu-temp:" << cpuInfo.temp << std::endl;
                // 输出到标准输出
                OM_DS_DEBUG_PRINT << ss.str();
                // std::cout << std::left << std::setw(20) << "----------------------------------------" << std::endl;
            }
            // 删除拷贝的文件
            if (std::remove(copyFile.c_str()) != 0)
            {
                OM_DS_ERROR_PRINT << "Error deleting temporary file: " << copyFile;
            }
        }
    }catch (afl::util::Exception &e)
    {
        OM_DS_ERROR_PRINT << "[what]" << e.what();
    	return false;
    }
  
    return true;
}

bool PerformenceUtils::getGpuInfo(GpuInfo& gpuInfo)
{   //初始化默认值
    gpuInfo.load = -1.0;
    gpuInfo.smem= -1.0;
    gpuInfo.pmen = -1.0;
    gpuInfo.temp = -1.0;
    gpuInfo.uti = "";

   {//获取负载率
        const char* load_paths[] = {
            "/sys/devices/gpu.0/load",
            "/sys/devices/57000000.gpu/load"
        };
        for (auto p : load_paths) {
            std::ifstream load_file(p);
            if (load_file.is_open()) {
                int load_val = 0;
                load_file >> load_val;
                gpuInfo.load = (double)load_val / 1000.0 * 100.0;
                load_file.close();
                break;
            }
        }

        if (gpuInfo.load >= 0) {
            gpuInfo.uti = std::to_string(gpuInfo.load) + "%";
        }

        if (printInfo) {
            OM_DS_DEBUG_PRINT << "[GPU] load=" << gpuInfo.load << "%, uti=" << gpuInfo.uti;
        }
    }

    {//获取共享内存占用量
        FILE* fp = popen("cat /sys/kernel/debug/nvmap/iovmm/maps 2>/dev/null", "r");
        if (fp) {
            char line[512];
            long long total_kb = -1;

            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "total") != NULL) {
                    sscanf(line, "%*s %lldK", &total_kb);
                    break;
                }
                if (strstr(line, "CLIENT") != NULL || strstr(line, "BASE") != NULL) {
                    continue;
                }
                char user[64] = {0};
                char process[128] = {0};
                int pid = 0;
                long long size_kb = 0;
                int matched = sscanf(line, "%s %s %d %lldK", user, process, &pid, &size_kb);
                if (matched == 4 && size_kb > 0) {
                    total_kb += size_kb;
                }
            }
            pclose(fp);

            if (total_kb > 0) {
                gpuInfo.smem = total_kb / 1024.0;
                if (printInfo) {
                    OM_DS_DEBUG_PRINT << "[GPU] smem=" << gpuInfo.smem << " MB (from nvmap)";
                }
            } else {
                if (printInfo) {
                    OM_DS_WARN_PRINT << "[GPU] nvmap: no valid data found";
                }
            }
        } else {
            if (printInfo) {
                OM_DS_WARN_PRINT << "[GPU] nvmap: popen failed, maybe need sudo or not available";
            }
        }
    }

    

    {//获取共享内存占用率
          if (gpuInfo.smem >= 0) {
            size_t free_mem = 0, total_mem = 0;
            cudaError_t err = cudaMemGetInfo(&free_mem, &total_mem);
            if (err == cudaSuccess && total_mem > 0) {
                double total_mb = total_mem / (1024.0 * 1024.0);
                gpuInfo.pmen = (gpuInfo.smem / total_mb) * 100.0;
                if (printInfo) {
                    OM_DS_DEBUG_PRINT << "[GPU] pmen=" << gpuInfo.pmen
                                      << "% (smem=" << gpuInfo.smem
                                      << "MB, total=" << total_mb << "MB)";
                }
            } else {
                if (printInfo) {
                    OM_DS_WARN_PRINT << "[GPU] pmen: cudaMemGetInfo failed, pmen remains -1";
                }
            }
        }
    }

    {

    //获取GPU温度：
        int gpu_zone = -1;
        //遍历所有的thermal_zone 

        for(int i = 0;i<10;i++){
            std::string type_path = "/sys/class/thermal/thermal_zone"+std::to_string(i)+"/type";
            std::ifstream type_file(type_path);
            if(!type_file.is_open()){
                
                continue;
            }

            std::string type;
            //读出来
            std::getline(type_file,type);

            //再检查一遍是不是gpu的
            if(type.find("GPU") != std::string::npos){
                gpu_zone = i;
                break;
            }

            //关闭
            type_file.close();
        }

        //找到了GPU zone 就读温度值
        if(gpu_zone>=0){
            std::string temp_path =  "/sys/class/thermal/thermal_zone"+std::to_string(gpu_zone)+"/temp";
            std::ifstream temp_file(temp_path);

           if (temp_file.is_open()) {
                int temp_milli_celsius = 0;  // 千分之一摄氏度
                temp_file >> temp_milli_celsius;
                
                gpuInfo.temp = temp_milli_celsius / 1000.0f;  // 转换成摄氏度
                
                temp_file.close();
                
                if(printInfo) {
                    OM_DS_DEBUG_PRINT << "[GPU] Temperature found: " << gpuInfo.temp << "℃ (zone" << gpu_zone << ")";
                }
            } else {
                if(printInfo) {
                    OM_DS_ERROR_PRINT << "[GPU] Cannot open temp file at zone " << gpu_zone;
                }
            }
        } else {
            if(printInfo) {
                OM_DS_WARN_PRINT << "[GPU] GPU thermal zone not found!";
            }
        }

     
    // 温度读取成功就返回true
    return (gpuInfo.temp >= 0.0);

    }
    


    }
NAMESPACE_ENDED_OM_COMPONENT_COMMON