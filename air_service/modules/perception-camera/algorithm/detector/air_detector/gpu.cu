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
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>

#include <algorithm>
#include <fstream>
#include <iostream>

#include "gpu.h"

namespace airos {
namespace perception {
namespace algorithm {
void codeCheck(cudaError_t code, const char *file, int line, bool abort) {
  if (code != cudaSuccess) {
    LOG(ERROR) << "GPUassert: " << cudaGetErrorString(code) << ", " << file
               << "," << line;
    if (abort) {
      exit(code);
    }
  }
}
void CodeCheck(cudaError_t code) {
  codeCheck(code, __FILE__, __LINE__);
}
void DeviceEnable(int device_id) {
  CodeCheck(cudaSetDevice(device_id));
}

__global__ void resize_nearest_kernel_mean(
    const unsigned char *src, float *dst, int channel, int height, int width,
    int stepwidth, int dst_height, int dst_width, int dst_step1, double fx,
    double fy, float mean_b, float mean_g, float mean_r, bool channel_axis,
    float scale_b, float scale_g, float scale_r) {
  const int dst_x = blockDim.x * blockIdx.x + threadIdx.x;
  const int dst_y = blockDim.y * blockIdx.y + threadIdx.y;

  if (dst_x < dst_width && dst_y < dst_height) {
    double src_x = dst_x * fx;
    double src_y = dst_y * fy;
    const int x1 = __float2int_rz(src_x);
    const int y1 = __float2int_rz(src_y);

    for (int c = 0; c < channel; c++) {
      int idx11 = y1 * stepwidth + x1 * channel;
      float out = src[idx11 + c];

      if (out < 0) {
        out = 0;
      }
      if (out > 255) {
        out = 255;
      }

      int dst_idx;
      if (channel_axis) {
        dst_idx = dst_y * dst_step1 + dst_x * channel + c;
      } else {
        dst_idx = (c * dst_height + dst_y) * dst_step1 + dst_x;
      }

      if (c == 0) {
        dst[dst_idx] = (out - mean_b) * scale_b;
      } else if (c == 1) {
        dst[dst_idx] = (out - mean_g) * scale_g;
      } else if (c == 2) {
        dst[dst_idx] = (out - mean_r) * scale_r;
      }
    }
  }
}

__global__ void resize_linear_kernel_mean2(
    const unsigned char *src, float *dst, int channel, int height, int width,
    int stepwidth, int dst_height, int dst_width, int dst_step1, double fx,
    double fy, float mean_b, float mean_g, float mean_r, bool channel_axis,
    float scale_b, float scale_g, float scale_r) {
  const int dst_x = blockDim.x * blockIdx.x + threadIdx.x;
  const int dst_y = blockDim.y * blockIdx.y + threadIdx.y;
  if (dst_x < dst_width && dst_y < dst_height) {
    double src_x      = (dst_x + 0.5) * fx - 0.5;
    double src_y      = (dst_y + 0.5) * fy - 0.5;
    const int x1      = __float2int_rd(src_x);
    const int y1      = __float2int_rd(src_y);
    const int x1_read = max(x1, 0);
    const int y1_read = max(y1, 0);
    const int x2      = x1 + 1;
    const int y2      = y1 + 1;
    const int x2_read = min(x2, width - 1);
    const int y2_read = min(y2, height - 1);
    // (h*width+w)*channel+c
    int src_reg = 0;
    for (int c = 0; c < channel; c++) {
      float out = 0;

      int idx11 = y1_read * stepwidth + x1_read * channel;
      src_reg   = src[idx11 + c];
      out       = out + (x2 - src_x) * (y2 - src_y) * src_reg;
      int idx12 = y1_read * stepwidth + x2_read * channel;
      src_reg   = src[idx12 + c];
      out       = out + src_reg * (src_x - x1) * (y2 - src_y);

      int idx21 = y2_read * stepwidth + x1_read * channel;
      src_reg   = src[idx21 + c];
      out       = out + src_reg * (x2 - src_x) * (src_y - y1);

      int idx22 = y2_read * stepwidth + x2_read * channel;
      src_reg   = src[idx22 + c];
      out       = out + src_reg * (src_x - x1) * (src_y - y1);
      if (out < 0) {
        out = 0;
      }
      if (out > 255) {
        out = 255;
      }

      int dst_idx;
      if (channel_axis) {
        dst_idx = dst_y * dst_step1 + dst_x * channel + c;
      } else {
        dst_idx = (c * dst_height + dst_y) * dst_step1 + dst_x;
      }

      if (c == 0) {
        dst[dst_idx] = (out - mean_b) * scale_b;
      } else if (c == 1) {
        dst[dst_idx] = (out - mean_g) * scale_g;
      } else if (c == 2) {
        dst[dst_idx] = (out - mean_r) * scale_r;
      }
    }
  }
}

int divup(int a, int b) {
  if (a % b) {
    return a / b + 1;
  } else {
    return a / b;
  }
}

int GPUResizeReshape(
    const unsigned char *gpu_src, int origin_channel, int origin_height,
    int origin_width, int origin_step, float *gpu_dst, cv::Size target_size,
    int des_step1, cv::InterpolationFlags resize_type, float mean_b,
    float mean_g, float mean_r, float scale_b, float scale_g, float scale_r) {
  int width  = target_size.width;
  int height = target_size.height;

  // channel_axis: false
  // SRC: 1 H W C
  // DST: 1 C H W

  double fx = static_cast<double>(origin_width) / static_cast<double>(width);
  double fy = static_cast<double>(origin_height) / static_cast<double>(height);
  const dim3 block(32, 8);
  const dim3 grid(divup(width, block.x), divup(height, block.y));

  if (resize_type == cv::InterpolationFlags::INTER_NEAREST) {
    resize_nearest_kernel_mean<<<grid, block>>>(
        gpu_src, gpu_dst, origin_channel, origin_height, origin_width,
        origin_step, height, width, des_step1, fx, fy, mean_b, mean_g, mean_r,
        false, scale_b, scale_g, scale_r);
  } else {
    // 使用双线性插值进行 resize
    resize_linear_kernel_mean2<<<grid, block>>>(
        gpu_src, gpu_dst, origin_channel, origin_height, origin_width,
        origin_step, height, width, des_step1, fx, fy, mean_b, mean_g, mean_r,
        false, scale_b, scale_g, scale_r);
  }
  return origin_channel * width * height;
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
