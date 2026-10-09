/*********************************************************************************
* @file		    log_utils.h
* @brief		log_utils.h belongs to CICTCI
* @details
* @author		alfred
* @email        zhangenwei64@gmail.com
* @date		    24-8-24
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  24-9-20 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/
#ifndef AIROS2_0_OM_COMMON_LOG_UTILS_H
#define AIROS2_0_OM_COMMON_LOG_UTILS_H
#include "namespace.h"
#include "base/common/network/exception.h"
#include "data_model/data_common.h"
#include "sqlite_device_status.h"
#include <string>   // 用于 std::string
#include <regex>    // 用于 std::regex 和 std::smatch
#include <iostream> // (如果需要) 用于 std::cout 或 std::cerr
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <memory>
#include <thread> // 用于 sleep_for
#include <chrono> // 用于高精度时间
NAMESPACE_START_OM_COMPONENT_COMMON
using namespace std;
using namespace afl::util;
using namespace os::v2x::protocol::om::db;
using namespace os::v2x::protocol::om::common;
//class DeviceStatusMsgDB;
class LogUtils
{
public:
    LogUtils() = default;
    virtual ~LogUtils() = default;// 这里使用 default 表示使用默认析构函数  ;
public:
    static std::string mqtt                             ;
    static std::string om_mec_register                  ;
    static std::string om_mec_device_info_query         ;
    static std::string om_mec_infoid0_basic_info        ;
    static std::string om_mec_infoid1_running_status    ;
    static std::string om_mec_infoid2_basic_info        ;
    static std::string om_mec_infoid3_running_status    ;
    static std::string om_mec_infoid4_config            ;
    static std::string om_mec_infoid5_running_info      ;
    static std::string om_mec_infoid6_alarm             ;
    static std::string om_mec_infoid7_version           ;

    static std::string om_mec_timer_ptp_pub             ;
    static std::string om_mec_timer_heartbeat_pub       ;
    static std::string om_mec_timer_running_status_pub  ;
    static std::string om_mec_ota                       ;
    static std::string om_mec_timer_running_info_pub    ;
    static std::string om_mec_timer_version_pub         ;
    static std::string om_mec_timer_alarm_pub           ;
    static std::string om_mec_timer_basic_info_pub      ;

    static std::string om_mec_config_modify_sub         ;
    static std::string om_mec_power_sub                 ;

    static std::string om_mec_bs_ptp_pub                ;
    static std::string om_mec_bs_spat_src_pub           ;
    static std::string om_mec_bs_cd_scenario_pub        ;
    static std::string om_mec_bs_angle_offset_pub       ;
    static std::string om_mec_bs_v2xdata_bsm_pub        ;
    static std::string om_mec_bs_v2xdata_map_pub        ;
    static std::string om_mec_bs_v2xdata_spat_pub       ;
    static std::string om_mec_bs_v2xdata_rsm_pub        ;
    static std::string om_mec_bs_v2xdata_rsi_pub        ;
    static std::string om_mec_bs_v2xdata_rsc_pub        ;
    static std::string om_mec_bs_v2xdata_ssm_pub        ;
    static std::string om_mec_bs_v2xdata_rtcm_pub       ;
    static std::string om_mec_bs_v2xdata_vir_pub        ;
    static std::string om_mec_bs_v2xdata_pam_pub        ;
    static std::string om_mec_bs_v2xdata_perception_pub ;
};
NAMESPACE_ENDED_OM_COMPONENT_COMMON
#endif
