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

#include "system_cmd.h"

#include <signal.h>

#include "base/common/log.h"

namespace airos {
namespace monitor {

bool SystemCmd::System(const std::string& cmd) {
  if (cmd.empty()) {
    APP_LOG_ERROR << "system cmd is empty!";
    return false;
  }
  int ret = 0;
  typedef void (*sighandler_t)(int);
  sighandler_t old_handler;
  old_handler = signal(SIGCHLD, SIG_DFL);
  ret         = system(cmd.c_str());
  signal(SIGCHLD, old_handler);

  if (ret != 0) {
    APP_LOG_INFO << cmd << "fail!, errno: " << strerror(errno);
    return false;
  }

  if (WIFEXITED(ret)) {
    if (0 == WEXITSTATUS(ret)) {
      return true;
    }
    APP_LOG_ERROR << "cmd exec fail status: " << WEXITSTATUS(ret);
    return false;
  }

  if (WIFSIGNALED(ret)) {
    APP_LOG_ERROR << "abnormal termination, status: " << WTERMSIG(ret);
    return false;
  }

  if (WIFSTOPPED(ret)) {
    APP_LOG_ERROR << "process stopped, status: " << WSTOPSIG(ret);
    return false;
  }

  APP_LOG_ERROR << "shell_system run shell unknown error";
  return false;
}

}  // namespace monitor
}  // namespace airos
