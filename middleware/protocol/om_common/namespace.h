/*
 * @Author: zhangenwei
 * @Date: 2024-02-19 10:16:15
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-19 10:16:15
 * @Description:
 */
#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_COMPONENT_NAMESPACE
#define AIROS_MIDDLEWARE_PROTOCOL_OM_COMPONENT_NAMESPACE
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <thread>
#include <mutex>
#include <future>
#include <dirent.h>
#include <regex.h>
#include <limits.h>
#include <sys/statfs.h>
#include <MQTTAsync.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <map>
#include <cstring>
#include <memory>
#include "yaml-cpp/yaml.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <iostream>
#include <linux/if.h>
#include "sys/socket.h"
#include <netinet/in.h>
#include "base/common/network/serializable_data.h"
#include "base/common/network/md5.h"
#include "base/common/network/print.h"
#include "base/common/log.h"
#include "base/common/network/byte_buffer.h"
#include "base/common/network/exception.h"
#include <boost/endian/conversion.hpp>
#include "base/common/configer/configer_data.h"
#include "base/common/network/serializable_data.h"
#include "base/common/network/print.h"
#include "base/common/network/channel.h"
#include "base/common/network/json.h"
#include "base/common/network/inet_address.h"
#include "base/common/log.h"
#include "base/common/network/byte_buffer.h"
#include "base/common/configer/configurable.h"
#include "base/common/configer/configer_data.h"
#include "base/common/network/srand.h"
#include "air_service/framework/proto/airos_usecase.pb.h"
#include "base/work_param/configer_om_work_param.h"
#include <sys/statvfs.h>
#include <regex>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

#include "base/common/network/date_time.h"
#include "base/common/network/httplib.h"
#include "base/common/network/exception.h"
using namespace afl::util;
#define NAMESPACE_OS_START        namespace os {
#define NAMESPACE_OS_END          }

#define NAMESPACE_V2X_BASE_START   NAMESPACE_OS_START namespace v2x {
#define NAMESPACE_V2X_BASE_END     } NAMESPACE_OS_END

#define NAMESPACE_PROTOCOL_THREAD_START NAMESPACE_V2X_BASE_START namespace protocol {
#define NAMESPACE_PROTOCOL_THREAD_END   } NAMESPACE_V2X_BASE_END
////////////////////////////////////////////////////////////////////////////////////////////


#define NAMESPACE_START_OM          NAMESPACE_PROTOCOL_THREAD_START  namespace om {
#define NAMESPACE_ENDED_OM          } NAMESPACE_PROTOCOL_THREAD_END


#define NAMESPACE_START_OM_COMPONENT_COMMON   NAMESPACE_START_OM namespace common {
#define NAMESPACE_ENDED_OM_COMPONENT_COMMON   } NAMESPACE_ENDED_OM

#define NAMESPACE_START_OM_COMPONENT_CLOUD   NAMESPACE_START_OM namespace cloud {
#define NAMESPACE_ENDED_OM_COMPONENT_CLOUD   } NAMESPACE_ENDED_OM
/////////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_OM_COMPONENT_CAMERA   NAMESPACE_START_OM namespace camera {
#define NAMESPACE_ENDED_OM_COMPONENT_CAMERA   } NAMESPACE_ENDED_OM

/////////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_OM_COMPONENT_RADAR   NAMESPACE_START_OM namespace radar {
#define NAMESPACE_ENDED_OM_COMPONENT_RADAR   } NAMESPACE_ENDED_OM

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_RADAR_CCINDEX   NAMESPACE_PROTOCOL_THREAD_START namespace ccindex {
#define NAMESPACE_ENDED_RADAR_CCINDEX    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_RADAR_RADAR_CLOUD   NAMESPACE_PROTOCOL_THREAD_START namespace radar_cloud {
#define NAMESPACE_ENDED_RADAR_RADAR_CLOUD    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_RADAR_RADAR_STATIC   NAMESPACE_PROTOCOL_THREAD_START namespace radar_static {
#define NAMESPACE_ENDED_RADAR_RADAR_STATIC    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_RADAR_RADAR_TRAFFIC_METRICS   NAMESPACE_PROTOCOL_THREAD_START namespace radar_traffic_metrics {
#define NAMESPACE_ENDED_RADAR_RADAR_TRAFFIC_METRICS    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_RADAR_RADAR_TC   NAMESPACE_PROTOCOL_THREAD_START namespace radar_tc {
#define NAMESPACE_ENDED_RADAR_RADAR_TC    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_RADAR_OM_CCINDEX   NAMESPACE_PROTOCOL_THREAD_START namespace om_ccindex {
#define NAMESPACE_ENDED_RADAR_OM_CCINDEX    } NAMESPACE_PROTOCOL_THREAD_END
//////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_OM_COMPONENT_DEVICE_STATUS  NAMESPACE_START_OM namespace db {
#define NAMESPACE_ENDED_OM_COMPONENT_DEVICE_STATUS     } NAMESPACE_ENDED_OM

//////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_OM_COMPONENT_MEC   NAMESPACE_START_OM namespace mec {
#define NAMESPACE_ENDED_OM_COMPONENT_MEC      } NAMESPACE_ENDED_OM

//////////////////////////////////////////////////////////////////////////////////
#define NAMESPACE_START_CAMERA_EVENT   NAMESPACE_PROTOCOL_THREAD_START namespace cameraevent {
#define NAMESPACE_ENDED_CAMERA_EVENT    } NAMESPACE_PROTOCOL_THREAD_END

#define NAMESPACE_START_OM_COMPONENT_MONITOR  NAMESPACE_START_OM namespace monitor {
#define NAMESPACE_ENDED_OM_COMPONENT_MONITOR     } NAMESPACE_ENDED_OM


#define NAMESPACE_START_SPILL_REPORTER_COMPONENT_RADAR   NAMESPACE_START_OM namespace spill_reporter {
#define NAMESPACE_ENDED_SPILL_REPORTER_COMPONENT_RADAR   } NAMESPACE_ENDED_OM

#define NAMESPACE_START_SOUND_PLAYER_COMPONENT  namespace sound_player {
#define NAMESPACE_ENDED_SOUND_PLAYER_COMPONENT   }

#define EXCP_TRY_BEGIN  try {
#define EXCP_CATCH(e)   } catch (afl::util::Exception& e) {
#define EXCP_CATCH_ALL  } catch (...) {
#define EXCP_CATCH_END  }

#define OM_TRY_BEGIN  try {

#define OM_CATCH_END  EXCP_CATCH(e) \
        std::cerr << "Caught exception: " << e.what() << std::endl; \
        EXCP_CATCH_ALL              \
        std::cerr << "Caught an unknown exception." << std::endl;   \
        EXCP_CATCH_END

#define LOG_KEY_OM_DS "[om_device_status]"
#define OM_DS_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_DS
#define OM_DS_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_DS
#define OM_DS_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_DS
#define OM_DS_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_DS
#define OM_DS_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_DS

#define LOG_KEY_OM_MONITOR "[om_monitor]"
#define OM_MONITOR_DEBUG_PRINT LOG_INFO_IF << LOG_KEY_OM_MONITOR
#define OM_MONITOR_WARN_PRINT LOG_WARN_IF << LOG_KEY_OM_MONITOR
#define OM_MONITOR_ERROR_PRINT LOG_ERROR_IF << LOG_KEY_OM_MONITOR
#define OM_MONITOR_SUCCESS_PRINT LOG_INFO_IF << LOG_KEY_OM_MONITOR
#define OM_MONITOR_FATAL_PRINT   LOG_FATAL_IF << LOG_KEY_OM_MONITOR

#endif

