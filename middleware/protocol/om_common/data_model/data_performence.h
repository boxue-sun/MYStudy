/*********************************************************************************
 * @file		performence_management_data.h
 * @brief		performence_management_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-23
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-23 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef MQTT_CLIENT_MEC_OM_PERFORMENCE_DATA_H
#define MQTT_CLIENT_MEC_OM_PERFORMENCE_DATA_H
#include "data_common.h"
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace afl::base;
// struct DiskStat {
//     unsigned long readsCompleted;
//     unsigned long writesCompleted;
//     unsigned long blocksRead;
//     unsigned long blocksWritten;
// };
// struct DiskStat {
//     std::string deviceName;
//     unsigned long readsCompleted;
//     unsigned long readsMerged;
//     unsigned long sectorsRead;
//     unsigned long readTime;
//     unsigned long writesCompleted;
//     unsigned long writesMerged;
//     unsigned long sectorsWritten;
//     unsigned long writeTime;
//     unsigned long ioInProgress;
//     unsigned long ioTime;
//     unsigned long weightedIoTime;
//     unsigned long blocksRead;
//     unsigned long blocksWritten;
// };
// struct DisksInfoSt {
//     std::string deviceName;
//     unsigned long totalCapacity;   // 总容量（以字节为单位）
//     unsigned long usedCapacity;     // 已用容量（以字节为单位）
//     unsigned long availableCapacity; // 可用容量（以字节为单位）
//     std::vector<DiskStat> diskStats;
// };

struct DiskStat {
    std::string deviceName;
    unsigned long readsCompleted;
    unsigned long readsMerged;
    unsigned long sectorsRead;
    unsigned long readTime;
    unsigned long writesCompleted;
    unsigned long writesMerged;
    unsigned long sectorsWritten;
    unsigned long writeTime;
    unsigned long ioInProgress;
    unsigned long ioTime;
    unsigned long weightedIoTime;
    unsigned long blocksRead;
    unsigned long blocksWritten;
};

struct MountPointInfo {
    std::string mountPoint;
    unsigned long totalCapacity;   // 总容量（以字节为单位）
    unsigned long usedCapacity;    // 已用容量（以字节为单位）
    unsigned long availableCapacity; // 可用容量（以字节为单位）
};

struct DisksInfoSt {
    std::string deviceName;
    std::vector<MountPointInfo> mountPoints; // 存储不同挂载点的信息
    std::vector<DiskStat> diskStats;
};
struct NetworkStats {
    unsigned long bytesReceived;
    unsigned long packetsReceived;
    unsigned long errs;
    unsigned long drop;
    unsigned long fifo;
    unsigned long frame;
    unsigned long compressed;
    unsigned long multicast;
    unsigned long bytesTransmitted;
    unsigned long packetsTransmitted;
    
    // 重载加法运算符以便于累加
    NetworkStats operator+(const NetworkStats& other) const {
        return {bytesReceived + other.bytesReceived,
                packetsReceived + other.packetsReceived,
                errs + + other.errs,
                drop + + other.drop,
                fifo + + other.fifo,
                frame + + other.frame,
                compressed + + other.compressed,
                multicast + + other.multicast,
                bytesTransmitted + other.bytesTransmitted,
                packetsTransmitted + other.packetsTransmitted};
    }

    // 提供一个方法以便于清零
    void reset() {
        bytesReceived = 0;
        packetsReceived = 0;
        errs = 0;
        drop = 0;
        fifo = 0;
        frame = 0;
        compressed = 0;
        multicast = 0;
        bytesTransmitted = 0;
        packetsTransmitted = 0;
    }
};

struct CpuInfo : public afl::base::SerializableData
{
    std::string load;
    bool loadEmpty = true;

    float loadUp;
    bool loadUpEmpty = true;
    float temp;
    std::string uti;
    virtual void serialize(json& j) override
    {
        JsonSerialize(load, "loadinner", j, loadEmpty);
        JsonSerialize(loadUp, "load", j, loadUpEmpty);
        JsonSerialize(temp, "temp", j, false);
        JsonSerialize(uti, "uti", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(load, "loadinner", j, loadEmpty);
        JsonDeserialize(loadUp, "load", j, loadUpEmpty);
        JsonDeserialize(temp, "temp", j, noUse_isEmptyFlag);
        JsonDeserialize(uti, "uti", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "Load: " << load << std::endl;
        ss << std::left << std::setw(40) << "Temp: " << temp << std::endl;
        ss << std::left << std::setw(40) << "UTI: " << uti << std::endl;
        return ss.str();
    }
} ;
struct GpuInfo : public afl::base::SerializableData
{
    float load = 0;
    float smem = 0;
    float pmen = 0;
    float temp = 0;
    std::string uti;

    virtual void serialize(json& j) override
    {
        JsonSerialize(load, "load", j, false);
        JsonSerialize(smem, "smem", j, false);
        JsonSerialize(pmen, "pmen", j, false);
        JsonSerialize(temp, "temp", j, false);
        JsonSerialize(uti, "uti", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(load, "load", j, noUse_isEmptyFlag);
        JsonDeserialize(smem, "smem", j, noUse_isEmptyFlag);
        JsonDeserialize(pmen, "pmen", j, noUse_isEmptyFlag);
        JsonDeserialize(temp, "temp", j, noUse_isEmptyFlag);
        JsonDeserialize(uti, "uti", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "Load: " << load << std::endl;
        ss << std::left << std::setw(40) << "Smemory: " << smem << std::endl;
        ss << std::left << std::setw(40) << "Pmem: " << pmen << std::endl;
        ss << std::left << std::setw(40) << "Temp: " << temp << std::endl;
        ss << std::left << std::setw(40) << "Uti: " << uti << std::endl;

        return ss.str();
    }
} ;
struct MemInfo : public afl::base::SerializableData
{
    float total;
    float used;
    float free;
    virtual void serialize(json& j) override
    {
        JsonSerialize(total, "total", j, false);
        JsonSerialize(used, "used", j, false);
        JsonSerialize(free, "free", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(total, "total", j, noUse_isEmptyFlag);
        JsonDeserialize(used, "used", j, noUse_isEmptyFlag);
        JsonDeserialize(free, "free", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(20) << "Total: " << total << std::endl;
        ss << std::left << std::setw(20) << "Used: " << used << std::endl;
        ss << std::left << std::setw(20) << "Free: " << free << std::endl;
        return ss.str();
    }
} ;

struct DiskInfo : public afl::base::SerializableData
{
    float total;
    float used;
    float free;
    uint64_t tps;
    float write;
    float read;
    std::vector<MountPointInfo> mountPoints; // 存储不同挂载点的信息
    virtual void serialize(json& j) override
    {
        JsonSerialize(total, "total", j, false);
        JsonSerialize(used, "used", j, false);
        JsonSerialize(free, "free", j, false);

        JsonSerialize(tps, "tps", j, false);
        JsonSerialize(write, "write", j, false);
        JsonSerialize(read, "read", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(total, "total", j, noUse_isEmptyFlag);
        JsonDeserialize(used, "used", j, noUse_isEmptyFlag);
        JsonDeserialize(free, "free", j, noUse_isEmptyFlag);

        JsonDeserialize(tps, "tps", j, noUse_isEmptyFlag);
        JsonDeserialize(write, "write", j, noUse_isEmptyFlag);
        JsonDeserialize(read, "read", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "Total: " << total << std::endl;
        ss << std::left << std::setw(40) << "Used: " << used << std::endl;
        ss << std::left << std::setw(40) << "Free: " << free << std::endl;
        ss << std::left << std::setw(40) << "TPS: " << tps << std::endl;
        ss << std::left << std::setw(40) << "Write: " << write << std::endl;
        ss << std::left << std::setw(40) << "Read: " << read << std::endl;
        return ss.str();
    }
} ;

struct NetInfo : public afl::base::SerializableData
{
    int rx;
    int tx;
    float rxByte;
    float txByte;

    virtual void serialize(json& j) override
    {
        JsonSerialize(rx, "rx", j, false);
        JsonSerialize(tx, "tx", j, false);
        JsonSerialize(rxByte, "rxByte", j, false);
        JsonSerialize(txByte, "txByte", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(rx, "rx", j, noUse_isEmptyFlag);
        JsonDeserialize(tx, "tx", j, noUse_isEmptyFlag);
        JsonDeserialize(rxByte, "rxByte", j, noUse_isEmptyFlag);
        JsonDeserialize(txByte, "txByte", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(20) << "rx: " << rx << std::endl;
        ss << std::left << std::setw(20) << "tx: " << tx << std::endl;
        ss << std::left << std::setw(20) << "rxByte: " << rxByte << std::endl;
        ss << std::left << std::setw(20) << "txByte: " << txByte << std::endl;
        return ss.str();
    }
} ;
struct RunningInfo : public afl::base::SerializableData
{
    CpuInfo cpuInfo;
    GpuInfo gpuInfo;
    MemInfo memInfo;
    DiskInfo diskInfo;
    NetInfo netInfo;
    virtual void serialize(json& j) override
    {
        JsonSerialize(cpuInfo, "cpu", j, false);
        JsonSerialize(gpuInfo, "gpu", j, false);
        JsonSerialize(memInfo, "mem", j, false);
        JsonSerialize(diskInfo, "disk", j, false);
        JsonSerialize(netInfo, "net", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(cpuInfo, "cpu", j, noUse_isEmptyFlag);
        JsonDeserialize(gpuInfo, "gpu", j, noUse_isEmptyFlag);
        JsonDeserialize(memInfo, "mem", j, noUse_isEmptyFlag);
        JsonDeserialize(diskInfo, "disk", j, noUse_isEmptyFlag);
        JsonDeserialize(netInfo, "net", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "CpuInfo:\n" << cpuInfo.to_string() << std::endl;
        ss << std::left << std::setw(40) << "GpuInfo:\n" << gpuInfo.to_string() << std::endl;
        ss << std::left << std::setw(40) << "MemInfo:\n" << memInfo.to_string() << std::endl;
        ss << std::left << std::setw(40) << "DiskInfo:\n" << diskInfo.to_string() << std::endl;
        ss << std::left << std::setw(40) << "NetInfo:\n" << netInfo.to_string() << std::endl;
        return ss.str();
    }
};

struct PerformanceData : public afl::base::SerializableData
{
    uint64_t timeStamp;
    std::string seqNum;
    std::string rscuEsn;
    std::string protocolVersion;
    RunningInfo runningInfo;

    virtual void serialize(json& j) override
    {
        JsonSerialize(timeStamp, "timestamp", j, false);
        JsonSerialize(seqNum, "seqNum", j, false);
        JsonSerialize(rscuEsn, "rscuEsn", j, false);
        JsonSerialize(protocolVersion, "protocolVersion", j, false);
        JsonSerialize(runningInfo, "runningInfo", j, false);
    }

    virtual void deserialize(const json& j) override
    {
        JsonDeserialize(timeStamp, "timestamp", j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum, "seqNum", j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn, "rscuEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(protocolVersion, "protocolVersion", j, noUse_isEmptyFlag);
        JsonDeserialize(runningInfo, "runningInfo", j, noUse_isEmptyFlag);
    }
public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "Time Stamp: " << timeStamp << std::endl;
        ss << std::left << std::setw(40) << "Seq Num: " << seqNum << std::endl;
        ss << std::left << std::setw(40) << "RSCU ESIM SN: " << rscuEsn << std::endl;
        ss << std::left << std::setw(40) << "Protocol Version: " << protocolVersion << std::endl;
        ss << std::left << std::setw(40) << "Running Info:\n" << runningInfo.to_string() << std::endl;
        return ss.str();
    }
};

NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif //NCS_MEC_PERFORMENCE_MANAGEMENT_DATA_H
