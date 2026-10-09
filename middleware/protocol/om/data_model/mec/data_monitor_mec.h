/*********************************************************************************
* @file		maintenance_management_ota_data.h
 * @brief		maintenance_management_ota_data belongs to CICTCI
 * @details
 * @author		alfred
 * @email       zhangenwei64@gmail.com
 * @date		24-4-21
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  24-4-21 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MONITOR_MEC_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_DATA_MODEL_MEC_DATA_MONITOR_MEC_H
#include "middleware/protocol/om_common/data_model/data_common.h"
#include "middleware/protocol/proto/monitor_mec.pb.h"
NAMESPACE_START_OM_COMPONENT_MEC
using namespace os::v2x::protocol::om::common;

// 监控项的运行时状态
struct MonitorState
{
    // ==========================================
    // 1. 配置部分 (Configuration) - 构造时确定，通常不变
    // ==========================================
    airos::monitor_mec::MonitorMecTag   tag;
    std::string                                             name;                   // 规则名称/提示信息
    MecAlarmTypeErrorCodeEnum           alarmType;    // 对应的告警类型
    uint64_t                                            timeoutMs;                  // 超时触发阈值 (毫秒)
    uint64_t                                            recoveryMs;                 // 恢复所需持续时间 (毫秒)
    bool        isLinkCheck;                // true: 链路状态检查, false: 数据超时检查
    bool        enable;                     // 是否检测
    int         selfCheckFaultType;         // 故障类型ID
    int maxTimeoutTolerance;     // 容忍的最大连续超时次数（例如设为3，即连续3秒超时才告警）
    std::string topic; //消息
    // ==========================================
    // 2. 运行时状态 (Runtime State) - 随时间变化
    // ==========================================
    // 内部状态变量 (保持默认值)
    uint64_t lastReceivedTime = 0;  //最后接收时间
    uint64_t recoveryStartTime = 0;  //回复开始时间
    uint64_t alarmOccurredTime = 0;  //告警产生时间
    uint64_t lastAlarmPublishTime = 0;  //最后推送时间
    bool isAlarmActive = false;     //告警是否激活
    bool lastLinkStatus = true;   //最后链路状态
    // 【新增】是否为首次检测标志，默认为 true
    bool isFirstCheck = true;
    mutable std::mutex stateMutex;

    // 【新增】防抖相关字段
    int continuousTimeoutCount = 0;  // 连续检测到超时的次数

    // ==========================================
    // [修复] 必须添加默认构造函数
    // ==========================================
    MonitorState() = default;

    // ==========================================
    // [修复] 添加带参数的构造函数 (解决编译错误的关键)
    // ==========================================
    // ==========================================
    // 包含所有参数的构造函数
    // ==========================================
    MonitorState(airos::monitor_mec::MonitorMecTag t,
                 std::string n,
                 MecAlarmTypeErrorCodeEnum at,
                 uint64_t tm,
                 uint64_t rm,
                 bool ilc,
                 bool en,
                 int sft, uint64_t mtt)
        : tag(t),
          name(std::move(n)),
          alarmType(at),
          timeoutMs(tm),
          recoveryMs(rm),
          isLinkCheck(ilc),
          enable(en),
          selfCheckFaultType(sft),
          maxTimeoutTolerance(mtt)
    {
    }
    // 删除拷贝构造和赋值（强制使用指针，防止误拷贝）
    MonitorState(const MonitorState&) = delete;
    MonitorState& operator=(const MonitorState&) = delete;

    // to_string 保持不变
    std::string to_string() const
    {
        std::ostringstream oss;
        oss << "MonitorState {"
            << "name: '" << name << "', "
            << "alarmType: " << static_cast<int>(alarmType) << ", "
            << "timeoutMs: " << timeoutMs << ", "
            << "recoveryMs: " << recoveryMs << ", "
            << "isLinkCheck: " << (isLinkCheck ? "true" : "false") << ", "
            << "enable: " << (enable ? "true" : "false") << ", "
            << "selfCheckFaultType: " << selfCheckFaultType << ", "
            << "lastReceivedTime: " << lastReceivedTime << ", "
            << "recoveryStartTime: " << recoveryStartTime << ", "
            << "alarmOccurredTime: " << alarmOccurredTime << ", "
            << "lastAlarmPublishTime: " << lastAlarmPublishTime << ", "
            << "isAlarmActive: " << (isAlarmActive ? "true" : "false") << ", "
            << "lastLinkStatus: " << (lastLinkStatus ? "true" : "false")
            << "}";
        return oss.str();
    }
};
// 【关键】定义智能指针类型
using MonitorStatePtr = std::shared_ptr<MonitorState>;

// ==========================================
// [新增] 1. 定义枚举索引 (放在类外或类内 public)
// ==========================================
enum CcindexTmTopicIndex {
    TM_TOPIC_TRAJECTORIES = 0,
    TM_TOPIC_VEHICLE_PASS,
    TM_TOPIC_QUEUE_UP,
    TM_TOPIC_AREA_STATE,
    TM_TOPIC_OVERFLOW,
    TM_TOPIC_OUTLANE,
    TM_TOPIC_STATISTICS,
    TM_TOPIC_EVALUATIONS,
    TM_TOPIC_NONMOTOR,
    TM_TOPIC_DEVICE_STATUS,
    TM_TOPIC_COUNT // 总数
};

enum CcindexStTopicIndex {
    ST_TOPIC_QUERY_ACK = 0,
    ST_TOPIC_UPDATE,
    ST_TOPIC_COUNT // 总数
};

// ==========================================
// [新增] 2. 定义设备上下文 (替代内层 Map)
// ==========================================
struct CcindexTmDeviceContext {
    // 使用 vector 替代 map，索引为 CcindexTmTopicIndex
    std::vector<MonitorStatePtr> states;
    CcindexTmDeviceContext() {
        states.resize(TM_TOPIC_COUNT, nullptr);
    }
};
using CcindexTmDeviceContextPtr = std::shared_ptr<CcindexTmDeviceContext>;

struct CcindexStDeviceContext {
    std::vector<MonitorStatePtr> states;
    CcindexStDeviceContext() {
        states.resize(ST_TOPIC_COUNT, nullptr);
    }
};
using CcindexStDeviceContextPtr = std::shared_ptr<CcindexStDeviceContext>;

/**
 * @brief 故障类型定义
 * 对应协议字段: faultType
 */
enum OmFaultType  {
    UNKNOWN = 0,                            // 预留未知类型
    OFT_SPAT_SRC_DATA = 1,                    // 1：信号机原始数据
    OFT_SENSOR_OBJS_DATA = 2,                    // 2：感知数据
    OFT_SENSOR_EVENTS_DATA = 3,                      // 3：事件
    OFT_CCINDEX_TC_DATA = 4,                 // 4：v2x信控指标
    OFT_CCINDEX_TM_DATA = 5,       // 5：雷达信控指标-动态
    OFT_CCINDEX_ST_DATA = 6,        // 6：雷达信控指标-静态
    OFT_RADAR_RAW_DATA = 7,              // 7：毫米波雷达原始数据
    OFT_LIDAR_RAW_DATA = 8,                     // 8：激光雷达原始数据
    OFT_SENSOR_CLOUD_LINK_STATUS = 9,                  // 9：云控链路状态
    OFT_CCINDEX_ST_LINK_STATUS = 10,        // 10：信控链路状态-静态
    OFT_CCINDEX_TM_LINK_STATUS = 11,       // 11：信控链路状态-动态
    OFT_CLOUD_POINT_LINK_STATUS = 12            // 12：点云链路状态
};

/**
 * @brief 故障类别定义
 * 对应协议字段: faultCategory
 */
enum OmFaultCategory  {
    DATA_EXCEPTION = 0,    // 0：数据异常
    LINK_EXCEPTION = 1     // 1：链路异常
};

/**
 * @brief 故障状态定义
 * 对应协议字段: faultStatus
 */
enum OmFaultStatus  {
    DISAPPEARED = 0,       // 0：故障消失 (恢复正常)
    OCCURRED = 1           // 1：故障产生
};


struct MecSelfCheckResult : public afl::base::SerializableData
{
    long        timestamp;
    std::string seqNum;
    std::string rscuEsn;
    OmFaultType         faultType;
    OmFaultCategory     faultCategory;
    OmFaultStatus       faultStatus;
    long        faultStartTime;
    long        faultStopTime;
    std::string faultDescription;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                      "timestamp",                      j, false);
        JsonSerialize(seqNum,                         "seqNum",                         j, false);
        JsonSerialize(rscuEsn,                        "rscuEsn",                        j, false);
        JsonSerialize(faultType,                      "faultType",                      j, false);
        JsonSerialize(faultCategory,                    "faultCategory",                    j, false);
        JsonSerialize(faultStatus,                    "faultStatus",                    j, false);
        JsonSerialize(faultStartTime,                 "faultStartTime",                 j, false);
        JsonSerialize(faultStopTime,                  "faultStopTime",                  j, false);
        JsonSerialize(faultDescription,               "faultDescription",               j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(timestamp,                    "timestamp",                      j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                       "seqNum",                         j, noUse_isEmptyFlag);
        JsonDeserialize(rscuEsn,                      "rscuEsn",                        j, noUse_isEmptyFlag);
        JsonDeserialize(faultType,                    "faultType",                      j, noUse_isEmptyFlag);
        JsonDeserialize(faultCategory,                    "faultCategory",                    j, noUse_isEmptyFlag);
        JsonDeserialize(faultStatus,                  "faultStatus",                    j, noUse_isEmptyFlag);
        JsonDeserialize(faultStartTime,               "faultStartTime",                 j, noUse_isEmptyFlag);
        JsonDeserialize(faultStopTime,                "faultStopTime",                  j, noUse_isEmptyFlag);
        JsonDeserialize(faultDescription,             "faultDescription",               j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "           << timestamp           << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "              << seqNum              << std::endl;
        ss << std::left << std::setw(40) << "rscuEsn: "             << rscuEsn             << std::endl;
        ss << std::left << std::setw(40) << "faultType: "           << faultType           << std::endl;
        ss << std::left << std::setw(40) << "faultCategory: "       << faultCategory       << std::endl;
        ss << std::left << std::setw(40) << "faultStatus: "         << faultStatus         << std::endl;
        ss << std::left << std::setw(40) << "faultStartTime: "      << faultStartTime      << std::endl;
        ss << std::left << std::setw(40) << "faultStopTime: "       << faultStopTime       << std::endl;
        ss << std::left << std::setw(40) << "faultDescription: "    << faultDescription    << std::endl;
        return ss.str();
    }
};


NAMESPACE_ENDED_OM_COMPONENT_MEC
#endif
