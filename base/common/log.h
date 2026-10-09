/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#pragma once

#ifndef CONFIG_USE_CYBER_LOG

#include "glog/logging.h"

#define LOG_DEBUG DLOG(INFO)
#define LOG_INFO LOG(INFO)
#define LOG_WARN LOG(WARNING)
#define LOG_ERROR LOG(ERROR)
#define LOG_FATAL LOG(FATAL)
#define LOG_V(log_severity) VLOG(log_severity)
#define LOG_DATA_RECYCLE LOG(INFO)

// LOG_IF (使用LOG_IF 跳过不输出的日志，避免浪费资源) 
#define LOG_INFO_IF LOG_IF(INFO, FLAGS_minloglevel == 0)
#define LOG_WARN_IF LOG_IF(WARNING, FLAGS_minloglevel <= 1)
#define LOG_ERROR_IF LOG_IF(ERROR, FLAGS_minloglevel <= 2)
#define LOG_FATAL_IF LOG_IF(FATAL, FLAGS_minloglevel <= 3)

// LOG_EVERY_N
#define LOG_INFO_EVERY(freq) LOG_EVERY_N(INFO, freq)
#define LOG_WARN_EVERY(freq) LOG_EVERY_N(WARNING, freq)
#define LOG_ERROR_EVERY(freq) LOG_EVERY_N(ERROR, freq)

#define GLOG_TIMESTAMP(timestamp) std::to_string(timestamp)

#else

#include "cyber/common/log.h"
#define LOG_DEBUG ADEBUG
#define LOG_INFO AINFO
#define LOG_WARN AWARN
#define LOG_ERROR AERROR
#define LOG_FATAL AFATAL

#endif

// 为了将不同模块的日志进行区分, 方便管理。定义宏对下列模块的日志进行重定向

// airos_app_framework 模块的日志
#define LOG_KEY_APP_FRAMEWORK "[airos_app_framework]"
#define APP_FRAMEWORK_LOG_INFO LOG_INFO_IF << LOG_KEY_APP_FRAMEWORK
#define APP_FRAMEWORK_LOG_WARN LOG_WARN_IF << LOG_KEY_APP_FRAMEWORK
#define APP_FRAMEWORK_LOG_ERROR LOG_ERROR_IF << LOG_KEY_APP_FRAMEWORK
#define APP_FRAMEWORK_LOG_FATAL LOG_FATAL_IF << LOG_KEY_APP_FRAMEWORK

// app 模块的日志
#define LOG_KEY_APP "[airos_app]"
#define APP_LOG_INFO LOG_INFO_IF << LOG_KEY_APP
#define APP_LOG_WARN LOG_WARN_IF << LOG_KEY_APP
#define APP_LOG_ERROR LOG_ERROR_IF << LOG_KEY_APP
#define APP_LOG_FATAL LOG_FATAL_IF << LOG_KEY_APP

// v2x codec 模块的日志
#define LOG_KEY_V2X_CODEC "[v2x_codec]"
#define V2X_CODEC_LOG_INFO LOG_INFO_IF << LOG_KEY_V2X_CODEC
#define V2X_CODEC_LOG_WARN LOG_WARN_IF << LOG_KEY_V2X_CODEC
#define V2X_CODEC_LOG_ERROR LOG_ERROR_IF << LOG_KEY_V2X_CODEC
#define V2X_CODEC_LOG_FATAL LOG_FATAL_IF << LOG_KEY_V2X_CODEC

// 接入层 device_service mec 模块
#define LOG_KEY_MEC_SERVICE "[mec_service]"
#define MEC_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_MEC_SERVICE
#define MEC_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_MEC_SERVICE
#define MEC_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_MEC_SERVICE
#define MEC_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_MEC_SERVICE

// 接入层 device_service rsu 模块
#define LOG_KEY_RSU_SERVICE "[rsu_service]"
#define RSU_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_RSU_SERVICE
#define RSU_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_RSU_SERVICE
#define RSU_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_RSU_SERVICE
#define RSU_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_RSU_SERVICE

// 接入层 device_service traffic light 模块
#define LOG_KEY_TRAFFIC_LIFGT_SERVICE "[traffic_light_service]"
#define TRAFFIC_LIFGT_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE
#define TRAFFIC_LIFGT_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE
#define TRAFFIC_LIFGT_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE
#define TRAFFIC_LIFGT_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_TRAFFIC_LIFGT_SERVICE

// 接入层 device_service ipcamera 模块
#define LOG_KEY_IPCAMERA_SERVICE "[ipcamera_service]"
#define IPCAMERA_SERVICE_LOG_INFO LOG_INFO_IF << LOG_KEY_IPCAMERA_SERVICE
#define IPCAMERA_SERVICE_LOG_WARN LOG_WARN_IF << LOG_KEY_IPCAMERA_SERVICE
#define IPCAMERA_SERVICE_LOG_ERROR LOG_ERROR_IF << LOG_KEY_IPCAMERA_SERVICE
#define IPCAMERA_SERVICE_LOG_FATAL LOG_FATAL_IF << LOG_KEY_IPCAMERA_SERVICE


#define LOG_KEY_RASP "[rsap]"
#define RSAP_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_RASP
#define RSAP_WARN_PRINT LOG_WARN_IF << LOG_KEY_RASP
#define RSAP_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_RASP
#define RSAP_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_RASP
#define RSAP_FATAL_PRINT LOG_FATAL_IF << LOG_KEY_RASP