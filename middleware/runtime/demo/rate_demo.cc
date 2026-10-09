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

#include <chrono>
#include <iostream>

#include "middleware/runtime/src/air_middleware_rate.h"

using namespace airos::middleware;

int main() {
  double x = 10.0;
  AirMiddlewareRate rate(x);

  int n = 0;
  while (1) {
    rate.Sleep();
    std::cout << ++n << ": current time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count()
              << " ms" << std::endl;
  }

  return 0;
}
