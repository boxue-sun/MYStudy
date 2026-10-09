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

#include "cuda_util.h"

#include <cuda_runtime_api.h>

#include <vector>

#include "base/common/log.h"

namespace airos {
namespace base {

CudaUtil::CudaUtil() {}

bool CudaUtil::set_device_id(int device_id) {
  int now_device  = -1;
  auto cuda_error = cudaGetDevice(&now_device);
  Vaild(cuda_error);
  if (now_device == device_id) {
    return true;
  } else {
    cuda_error = cudaSetDevice(device_id);
    Vaild(cuda_error);
  }
  return true;
}
void* CudaUtil::malloc(int size) {
  void* gpu_data       = nullptr;
  cudaError cuda_error = cudaMalloc(&gpu_data, size);
  Vaild(cuda_error);
  APP_FRAMEWORK_LOG_INFO << "gpu data ptr " << gpu_data << ", size " << size;
  cudaMemset(gpu_data, 0, size);
  return gpu_data;
}

void CudaUtil::free(void* ptr) {
  cudaError cuda_error = cudaFree(ptr);
  Vaild(cuda_error);
}

void CudaUtil::memset(void* gpu_data, int value, int size) {
  cudaError cuda_error = cudaMemset(gpu_data, value, size);
  Vaild(cuda_error);
  return;
}

CudaUtil::~CudaUtil() {}

bool CudaUtil::CopyHostToDevice(
    unsigned char* des_data, int max_size, int des_step,
    const unsigned char* src_data, int height, int width, int src_step) {
  if (max_size < des_step * height) {
    LOG_ERROR << "destination space is not enough";
    return false;
  }
  cudaError err = cudaMemcpy2D(
      des_data,
      des_step,
      src_data,
      src_step,
      width,
      height,
      cudaMemcpyHostToDevice);
  if (err != cudaError::cudaSuccess) {
    LOG_ERROR << "CopyHostToDevice cudaMemcpy2D " << err << ", msg "
              << cudaGetErrorString(err);
    return false;
  }
  return true;
}

bool CudaUtil::CopyVecDeviceToDevice(
    float* des_data, int max_size, const float* src_data, int len) {
  if (max_size < len) {
    LOG_ERROR << "destination space is not enough";
    return false;
  }
  cudaError err = cudaMemcpy(
      des_data, src_data, len * sizeof(float), cudaMemcpyDeviceToDevice);
  if (err != cudaError::cudaSuccess) {
    LOG_ERROR << "CopyDeviceToDevice cudaMemcpy " << err << ", msg "
              << cudaGetErrorString(err);
    return false;
  }
  return true;
}

bool CudaUtil::CopyDeviceToDevice(
    void* des_data, int max_size, const void* src_data, int len) {
  if (max_size < len) {
    LOG_ERROR << "destination space is not enough";
    return false;
  }
  cudaError err = cudaMemcpy(
      des_data, src_data, len * sizeof(char), cudaMemcpyDeviceToDevice);
  if (err != cudaError::cudaSuccess) {
    LOG_ERROR << "CopyDeviceToDevice cudaMemcpy " << err << ", msg "
              << cudaGetErrorString(err);
    return false;
  }
  return true;
}

std::vector<float> GpuImgToVec(
    const float* gpu_data, int channel, int width, int height) {
  std::vector<float> fval;
  fval.resize(channel * width * height);
  cudaError res = cudaMemcpy(
      fval.data(),
      gpu_data,
      channel * width * height * sizeof(float),
      cudaMemcpyKind::cudaMemcpyDeviceToHost);
  if (res != cudaSuccess) {
    LOG_ERROR << "cudaMemcpy res " << res;
  }
  return fval;
}

}  // namespace base
}  // namespace airos
