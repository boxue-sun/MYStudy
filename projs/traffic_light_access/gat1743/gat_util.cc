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

#include "gat_util.h"

#include <sys/time.h>

#include <iostream>
#include <sstream>

#include "base/common/log.h"

namespace os {
namespace v2x {
namespace device {

std::string GatUtil::Bin2Str(const void *data, const ssize_t len) {
  std::ostringstream oss;
  char tmp_hex[3] = {0};
  uint8_t *p_data = (uint8_t *)data;
  ssize_t i       = 0;
  while (i < len) {
    sprintf(tmp_hex, "%02X", p_data[i++]);
    oss << tmp_hex;
  }
  return oss.str();
}

void GatUtil::DbgPrintBinary(
    const void *data, const ssize_t len, std::string prefix) {
  TRAFFIC_LIFGT_SERVICE_LOG_WARN << prefix << "(" << len << ")" << GatUtil::Bin2Str(data, len);
}

uint64_t GatUtil::GetCurTimestampMsec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return uint64_t(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

}  // namespace device
}  // namespace v2x
}  // namespace os
