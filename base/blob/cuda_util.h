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

#include <cublas_v2.h>
// #include <opencv2/opencv.hpp>
#include <cuda_runtime_api.h>

#include "base/common/log.h"
namespace airos {
namespace base {

class CudaUtil {
 public:
  static bool set_device_id(int device_id);
  static void* malloc(int size);
  static void free(void* ptr);

  static void memset(void* ptr, int value, int size);
  static void inline Vaild(cudaError err) {
    if (err != cudaSuccess) {
      LOG_ERROR << "cuda option error, err " << err << ", msg "
                << cudaGetErrorString(err);
    }
  }
  static bool CopyHostToDevice(
      unsigned char* des_data, int max_size, int des_step,
      const unsigned char* src_data, int height, int width, int src_step);
  static bool CopyDeviceToHost(
      unsigned char* des_data, int max_size, int des_step,
      const unsigned char* src_data, int height, int width, int src_step);
  static bool CopyVecDeviceToDevice(
      float* des_data, int max_size, const float* src_data, int len);
  static bool CopyDeviceToDevice(
      void* des_data, int max_size, const void* src_data, int len);
  static bool CopyVecHostToDevice(
      float* des_data, int max_size, const float* src_data, int len);
  ~CudaUtil();
  CudaUtil();
};

}  // namespace base
}  // namespace airos
