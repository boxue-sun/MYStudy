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

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <driver_types.h>

#include <opencv2/core/cuda.hpp>
#include <opencv2/opencv.hpp>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

int divup(int a, int b);
void codeCheck(cudaError_t code, const char* file, int line, bool abort = true);

void CodeCheck(cudaError_t code);
void DeviceEnable(int device_id);

int GPUResizeReshape(
    const unsigned char* gpu_src, int origin_channel, int origin_height,
    int origin_width, int origin_step, float* gpu_dst, cv::Size target_size,
    int des_step1, cv::InterpolationFlags resize_type, float mean_b,
    float mean_g, float mean_r, float scale_b, float scale_g, float scale_r);
}  // namespace algorithm
}  // namespace perception
}  // namespace airos
