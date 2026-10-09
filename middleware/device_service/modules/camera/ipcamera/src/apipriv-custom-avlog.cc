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

#include "ipcamera/include/apipriv-custom-avlog.h"

#include "base/common/log.h"

#define BUF_LEN (65536)

namespace airos {
namespace base {
namespace device {

void apipriv_custom_avlog(void *ptr, int level, const char *fmt, va_list vl) {
  char buf[BUF_LEN] = {0};
  int pnt           = 1;
  if (level > AV_LOG_INFO) {
    return;
  }
  std::string info = "<FFMPEG> ";
  av_log_format_line(ptr, level, fmt, vl, buf, BUF_LEN, &pnt);
  info += buf;
  if (level > AV_LOG_WARNING) {
    LOG_INFO << info;
  } else if (level > AV_LOG_ERROR) {
    LOG_WARN << info;
  } else if (level > AV_LOG_FATAL) {
    LOG_ERROR << info;
  } else {
    LOG_FATAL << info;
  }
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
