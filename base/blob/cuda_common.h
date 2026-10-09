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

#include <assert.h>
#ifndef PERCEPTION_CPU_ONLY

#include <cublas_v2.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <curand.h>
#include <driver_types.h>  // cuda driver types

#include <nvtx3/nvToolsExt.h>

#endif

namespace airos {
namespace base {

#define NO_GPU assert(false)

#ifndef PERCEPTION_CPU_ONLY

#define BASE_CUDA_CHECK(condition) \
  { airos::base::GPUAssert((condition), __FILE__, __LINE__); }

inline void GPUAssert(
    cudaError_t code, const char *file, int line, bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(
        stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
    if (abort) {
      exit(code);
    }
  }
}

struct NvTxRangeContext {
  explicit NvTxRangeContext(const char *name) {
    nvtxRangePush(name);
  }
  ~NvTxRangeContext() {
    nvtxRangePop();
  }
};

#define CUDA_MARK_CONTEXT(name)                                \
  auto const LOG_EVERY_N_VARNAME(nvtx_range_mark_, __LINE__) = \
      airos::base::NvTxRangeContext(name)

#else

#define CUDA_MARK_CONTEXT(name) (void)(name)

#endif

}  // namespace base
}  // namespace airos
