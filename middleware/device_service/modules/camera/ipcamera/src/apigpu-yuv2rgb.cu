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

#include <iostream>
#include <cuda_runtime_api.h>

#include "base/cv_image/image.h"
#include "middleware/device_service/modules/camera/ipcamera/include/apigpu-yuv2rgb.h"
#include "base/common/log.h"

namespace airos {
namespace base {
namespace device {
namespace gpu {

__global__ static void __convert(airos::base::Color imgmode, int width,
                                 int height, int linesize, const void *y_data,
                                 const void *uv_data, void *result) {
  const int x = blockIdx.x * blockDim.x + threadIdx.x;
  const int y = blockIdx.y * blockDim.y + threadIdx.y;
  if (x >= width || y > height) {
    return;
  }
  int y_idx = y * linesize + x;
  int uv_idx = y / 2 * linesize + x & (INT_MAX - 1);
  int out_idx = (y * width + x) * 3;
  // GET yuv
  float _y = ((unsigned char *)y_data)[y_idx];
  float _u = ((unsigned char *)uv_data)[uv_idx] - 128;
  float _v = ((unsigned char *)uv_data)[uv_idx + 1] - 128;
  // Convert
  float _r = _y + 1.402f * _v;
  float _g = _y - 0.34413f * _u - 0.71414f * _v;
  float _b = _y + 1.772f * _u;
  // Result
  if (imgmode == airos::base::Color::BGR) {
    ((unsigned char *)result)[out_idx + 2] = (unsigned char)(_r < 0     ? 0
                                                             : _r > 255 ? 255
                                                                        : _r);
    ((unsigned char *)result)[out_idx + 1] = (unsigned char)(_g < 0     ? 0
                                                             : _g > 255 ? 255
                                                                        : _g);
    ((unsigned char *)result)[out_idx] = (unsigned char)(_b < 0     ? 0
                                                         : _b > 255 ? 255
                                                                    : _b);
  } else {  // airos::base::Color::RGB
    ((unsigned char *)result)[out_idx] = (unsigned char)(_r < 0     ? 0
                                                         : _r > 255 ? 255
                                                                    : _r);
    ((unsigned char *)result)[out_idx + 1] = (unsigned char)(_g < 0     ? 0
                                                             : _g > 255 ? 255
                                                                        : _g);
    ((unsigned char *)result)[out_idx + 2] = (unsigned char)(_b < 0     ? 0
                                                             : _b > 255 ? 255
                                                                        : _b);
  }
}

bool yuv2rgb(airos::base::Color imgmode, const AVFrame *frame, size_t bufsize,
             void *gpubuf, unsigned int block_x, unsigned int block_y) {
  if (!gpubuf || !frame || !frame->data[0] || !frame->data[1]) {
    return false;
  }
  // Prepare
  dim3 block(block_x, block_y);

  if (bufsize < 3ULL * frame->width * frame->height) {
    return false;
  }
  int gridx = (frame->width + 2 * block.x - 1) / (block.x);
  int gridy = (frame->height + 2 * block.y - 1) / (block.y);
  dim3 grid(gridx, gridy);

  // gpu yuv2rgb
  /* YUV420 In-memory arrangement output from by h264_cuda: NV12
  Y Y Y Y Y Y
  Y Y Y Y Y Y
  Y Y Y Y Y Y
  Y Y Y Y Y Y
  Y Y Y Y Y Y
  Y Y Y Y Y Y
  U V U V U V
  U V U V U V
  U V U V U V
  */
  __convert<<<grid, block>>>(imgmode, frame->width, frame->height,
                             frame->linesize[0], frame->data[0], frame->data[1],
                             gpubuf);
  cudaDeviceSynchronize();

  return true;
}

}  // END namespace gpu
}  // END namespace device
}  // END namespace base
}  // namespace airos
