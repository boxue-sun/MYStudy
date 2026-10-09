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
*  24-8-24 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#include "log_utils.h"
NAMESPACE_START_OM_COMPONENT_COMMON
// 静态成员变量定义

std::string LogUtils::mqtt                             = "[mqtt]";
std::string LogUtils::om_mec_register                  = "[om_mec_register]";

std::string LogUtils::om_mec_device_info_query         = "[om_mec_device_info_query]";
std::string LogUtils::om_mec_infoid0_basic_info        = "[om_mec_infoid0_basic_info]";
std::string LogUtils::om_mec_infoid1_running_status    = "[om_mec_infoid1_running_status]";
std::string LogUtils::om_mec_infoid2_basic_info        = "[om_mec_infoid2_basic_info]";
std::string LogUtils::om_mec_infoid3_running_status    = "[om_mec_infoid2_running_status]";
std::string LogUtils::om_mec_infoid4_config            = "[om_mec_infoid4_config]";
std::string LogUtils::om_mec_infoid5_running_info      = "[om_mec_infoid5_running_info]";
std::string LogUtils::om_mec_infoid6_alarm             = "[om_mec_infoid6_alarm]";
std::string LogUtils::om_mec_infoid7_version           = "[om_mec_infoid7_version]";
std::string LogUtils::om_mec_timer_ptp_pub             = "[om_mec_timer_ptp_pub]";
std::string LogUtils::om_mec_timer_heartbeat_pub       = "[om_mec_timer_heartbeat_pub]";
std::string LogUtils::om_mec_timer_running_status_pub  = "[om_mec_timer_running_status_pub]";
std::string LogUtils::om_mec_ota                       = "[om_mec_ota]";
std::string LogUtils::om_mec_timer_running_info_pub    = "[om_mec_timer_running_info_pub]";
std::string LogUtils::om_mec_timer_version_pub         = "[om_mec_timer_running_info_pub]";
std::string LogUtils::om_mec_timer_alarm_pub           = "[om_mec_timer_alarm_pub]";
std::string LogUtils::om_mec_timer_basic_info_pub      = "[om_mec_timer_basic_info_pub]";
std::string LogUtils::om_mec_config_modify_sub         = "[om_mec_config_modify_sub]";
std::string LogUtils::om_mec_power_sub                 = "[om_mec_power_sub]";

std::string LogUtils::om_mec_bs_ptp_pub                = "[om_mec_bs_ptp_pub]";
std::string LogUtils::om_mec_bs_spat_src_pub           = "[om_mec_bs_spat_src_pub]";
std::string LogUtils::om_mec_bs_cd_scenario_pub        = "[om_mec_bs_cd_scenario_pub]";
std::string LogUtils::om_mec_bs_angle_offset_pub       = "[om_mec_bs_angle_offset_pub]";
std::string LogUtils::om_mec_bs_v2xdata_bsm_pub        = "[om_mec_bs_v2xdata_bsm_pub]";
std::string LogUtils::om_mec_bs_v2xdata_map_pub        = "[om_mec_bs_v2xdata_map_pub]";
std::string LogUtils::om_mec_bs_v2xdata_spat_pub       = "[om_mec_bs_v2xdata_spat_pub]";
std::string LogUtils::om_mec_bs_v2xdata_rsm_pub        = "[om_mec_bs_v2xdata_rsm_pub]";
std::string LogUtils::om_mec_bs_v2xdata_rsi_pub        = "[om_mom_mec_bs_v2xdata_rsi_pub]";
std::string LogUtils::om_mec_bs_v2xdata_rsc_pub        = "[om_mec_bs_v2xdata_rsc_pub]";
std::string LogUtils::om_mec_bs_v2xdata_ssm_pub        = "[om_mec_bs_v2xdata_ssm_pub]";
std::string LogUtils::om_mec_bs_v2xdata_rtcm_pub       = "[om_mec_bs_v2xdata_rtcm_pub]";
std::string LogUtils::om_mec_bs_v2xdata_vir_pub        = "[om_mec_bs_v2xdata_vir_pub]";
std::string LogUtils::om_mec_bs_v2xdata_pam_pub        = "[om_mec_bs_v2xdata_pam_pub]";
std::string LogUtils::om_mec_bs_v2xdata_perception_pub = "[om_mec_bs_v2xdata_perception_pub]";
NAMESPACE_ENDED_OM_COMPONENT_COMMON