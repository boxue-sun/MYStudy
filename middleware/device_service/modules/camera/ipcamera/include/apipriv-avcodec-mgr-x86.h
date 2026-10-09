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

#ifndef __cplusplus
#error "This is a c++ header!"
#endif

#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-avcodec-mgr.h"
#include <fstream>
#include <iostream>

#include "base/common/log.h"
#include "cuda.h"
#include "cuda_runtime_api.h"
#include "gflags/gflags.h"
#include "ipcamera/include/apigpu-yuv2rgb.h"
#include "libavutil/hwcontext_cuda.h"

namespace airos {
namespace base {
namespace device {

class AVCodecCtxManagerX86 final : public AVCodecCtxManager {
 public:
  AVCodecCtxManagerX86(const AVCodecCtxManagerX86 &)                  = delete;
  const AVCodecCtxManagerX86 &operator=(const AVCodecCtxManagerX86 &) = delete;
  AVCodecCtxManagerX86(AVCodecCtxManagerX86 &&)                       = delete;
  AVCodecCtxManagerX86 &operator=(AVCodecCtxManagerX86 &&)            = delete;

  /**
   * 获取驱动单例，线程安全
   * @return Instance
   */
  static AVCodecCtxManagerX86 *getInstance();

  virtual int create_avctx(int devid = 0) override;

  virtual void decode_and_convert(
      int ctx_idx, bool need_rebuild_ctx, const AVPacket *pkt,
      airos::base::Color imgmode, const GetMeatimeCallbackT &cb,
      std::list<std::shared_ptr<GPUImage>> *image_list,
      std::string channel_name = "",
      bool use_system_time = false) override;

 private:
  AVCodecCtxManagerX86();
  ~AVCodecCtxManagerX86();
  bool __rebuild_ctx(int devid, int ctx_idx);

  static AVCodecCtxManagerX86 *_S_instance;
  static std::mutex _S_lock_instance;

  const AVCodec *_M_codec;
  const int64_t _M_devcount;
  std::vector<std::mutex> _M_gpulocks;
  std::vector<std::map<size_t, GPUBuffers>> _M_gpuptrs;
  std::mutex _M_lock_avctxes;
  std::vector<std::shared_ptr<AVCodecContext>> _M_avctxes;
  std::vector<bool> _M_is_get_resolution;
};

DEFINE_bool(cpu_yuv2rgb, true, "use cpu realize yuv2rgb");
AVCodecCtxManagerX86 *AVCodecCtxManagerX86::_S_instance = nullptr;
std::mutex AVCodecCtxManagerX86::_S_lock_instance;

#define gpuErrchk(ans) \
  { gpuAssert((ans), __FILE__, __LINE__); }

inline void gpuAssert(
    cudaError_t code, const char *file, int line, bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(
        stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
    if (abort) exit(code);
  }
}

AVCodecCtxManagerX86::AVCodecCtxManagerX86()
    : _M_codec(avcodec_find_decoder_by_name("h264"))
    , _M_devcount([]() {
      int ret = cuInit(0);  // CI libcuda.so不支持
      if (CUDA_SUCCESS != ret) {
        LOG_FATAL << "<CODEC-CTX-MGR> Failed to init CUDA!";
        abort();
      }
      int res = -1;
      if (cudaSuccess != cudaGetDeviceCount(&res) || res < 1) {
        LOG_FATAL << "<CODEC-CTX-MGR> Failed to get CUDA dev count!";
        abort();
      }
      return res;
    }())
    , _M_gpulocks(_M_devcount)
    , _M_gpuptrs(_M_devcount) {
  if (!_M_codec) {
    LOG_FATAL << "<CODEC-CTX-MGR> Failed to find decoder!";
    abort();
  }
  if (AV_HWDEVICE_TYPE_CUDA != av_hwdevice_find_type_by_name("cuda")) {
    LOG_FATAL << "<CODEC-CTX-MGR> Failed to find cuda support!";
    abort();
  }

  int cfg_idx = 0;
  while (true) {
    const AVCodecHWConfig *config = avcodec_get_hw_config(
        _M_codec, cfg_idx);  // h264_codec:从0开始找，直到找到cuda
    if (!config) {
      LOG_FATAL << " <CODEC-CTX-MGR> Failed to find codec cuda support!";
      abort();
    }
    if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
        config->device_type == AV_HWDEVICE_TYPE_CUDA) {
      break;
    }
    cfg_idx++;
  }
}

AVCodecCtxManagerX86::~AVCodecCtxManagerX86() {}

AVCodecCtxManagerX86 *AVCodecCtxManagerX86::getInstance() {
  if (!_S_instance) {
    std::lock_guard<std::mutex> g(_S_lock_instance);
    if (!_S_instance) {
      _S_instance = new AVCodecCtxManagerX86();
    }
  }
  return _S_instance;
}

bool AVCodecCtxManagerX86::__rebuild_ctx(int devid, int ctx_idx) {
  LOG_INFO << "<CODEC-CTX-MGR> Rebuilding CTX: " << ctx_idx;
  if (devid < 0 || devid > _M_devcount) {
    LOG_FATAL << "<CODEC-CTX-MGR> Invalid devid: " << devid << " in rebuild!";
    abort();
  }
  std::shared_ptr<AVCodecContext> ctx = nullptr;
  ctx.reset(avcodec_alloc_context3(_M_codec), [](AVCodecContext *ctx) {
    if (avcodec_is_open(ctx)) {
      avcodec_close(ctx);
    }
    avcodec_free_context(&ctx);
  });

  if (!ctx) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to alloca codec context!";
    return false;
  }

  if (0 > av_hwdevice_ctx_create(
              &ctx->hw_device_ctx,
              AV_HWDEVICE_TYPE_CUDA,
              std::to_string(devid).c_str(),
              nullptr,
              0)) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to create  "
                 "AV_HWDEVICE_TYPE_CUDA hwdevice context!";
    return false;
  }

  if (0 > avcodec_open2(ctx.get(), _M_codec, nullptr)) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to open codec context!";
    return false;
  }
  std::lock_guard<std::mutex> g(_M_lock_avctxes);
  ctx->opaque         = _M_avctxes[ctx_idx]->opaque;
  _M_avctxes[ctx_idx] = ctx;
  return true;
}

int AVCodecCtxManagerX86::create_avctx(int devid) {
  if (devid < 0 || devid > _M_devcount) {  // gpu_id
    LOG_ERROR << "<CODEC-CTX-MGR> Invalid devid: " << devid;
    return -1;
  }
  std::shared_ptr<AVCodecContext> ctx = nullptr;
  ctx.reset(avcodec_alloc_context3(_M_codec), [](AVCodecContext *ctx) {
    if (avcodec_is_open(ctx)) {
      avcodec_close(ctx);
    }
    avcodec_free_context(&ctx);
  });
  if (!ctx) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to alloca codec context!";
    return -1;
  }

  if (0 > av_hwdevice_ctx_create(
              &ctx->hw_device_ctx,
              AV_HWDEVICE_TYPE_CUDA,
              std::to_string(devid).c_str(),
              nullptr,
              0)) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to create hwdevice context!";
    return -1;
  }

  if (0 > avcodec_open2(ctx.get(), _M_codec, nullptr)) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to open codec context!";
    return -1;
  }

  ctx->opaque = reinterpret_cast<void *>(devid);
  // LOCK, add to vector & return
  std::lock_guard<std::mutex> g(_M_lock_avctxes);
  int res = _M_avctxes.size();
  _M_avctxes.push_back(ctx);

  return res;
}

void AVCodecCtxManagerX86::decode_and_convert(
    int ctx_idx, bool need_rebuild_ctx, const AVPacket *pkt,
    airos::base::Color imgmode, const GetMeatimeCallbackT &cb,
    std::list<std::shared_ptr<GPUImage>> *image_list,
    std::string channel_name,
    bool use_system_time) {
  if (image_list == nullptr) return;
  std::shared_ptr<AVCodecContext> avctx = nullptr;
  {
    std::lock_guard<std::mutex> g(_M_lock_avctxes);
    if (0 > ctx_idx || _M_avctxes.size() <= (unsigned int)ctx_idx) {
      LOG_ERROR << "<CODEC-CTX-MGR> Invalid ctx_idx!";
      return;
    }
    avctx = _M_avctxes[ctx_idx];
  }
  if (need_rebuild_ctx) {
    while (!__rebuild_ctx(reinterpret_cast<int64_t>(avctx->opaque), ctx_idx)) {
    }
    return;
  }

  struct timespec ts_bgn = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &ts_bgn);

  int ret_send = avcodec_send_packet(avctx.get(), pkt);

  if (AVERROR(ENOMEM) == ret_send) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to send packet! (NOMEM)";
    while (!__rebuild_ctx(reinterpret_cast<int64_t>(avctx->opaque), ctx_idx)) {
    }
    return;
  } else if (0 > ret_send) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to send packet!";
    char buf[1024] = {0};
    av_strerror(ret_send, buf, 1024);
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to send "
              << (pkt->flags & AV_PKT_FLAG_KEY ? "I frame:" : "P frame:")
              << ret_send << "-" << std::string(buf);
    if (AVERROR(EAGAIN) != ret_send) {
      // AVERROR(EAGAIN) need resend frame after avcodec_receive_frame
      return;
    }
  }

  if (!avctx->hwaccel) {
    LOG_ERROR << "<CODEC-CTX-MGR> Failed to get HWAccel!";
    while (!__rebuild_ctx(reinterpret_cast<int64_t>(avctx->opaque), ctx_idx)) {
    }
    return;
  }

  int devid = reinterpret_cast<int64_t>(avctx->opaque);
  // static int n = 0;

  // static int x = 0;
  while (true) {
    // Create AVFrame
    std::shared_ptr<AVFrame> frame;
    frame.reset(av_frame_alloc(), [](AVFrame *f) {
      av_frame_unref(f);
      av_frame_free(&f);
    });

    // Receive AVFrame
    int ret = avcodec_receive_frame(avctx.get(), frame.get());
    if (!avctx.get()) {
      LOG_ERROR << "avctx.get() is null";
    }

    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
      // EAGAIN需要继续送帧，无其他影响
      if (AVERROR(EAGAIN) != ret) {
        char buf[1024] = {0};
        av_strerror(ret, buf, 1024);
        LOG_ERROR << "<CODEC-CTX-MGR> Failed to receive "
                  << (pkt->flags & AV_PKT_FLAG_KEY ? "I frame:" : "P frame:")
                  << ret << "-" << std::string(buf);
      }
      break;
    }
    if (AVERROR(EAGAIN) == ret_send) {
      ret_send = avcodec_send_packet(avctx.get(), pkt);
      if (0 != ret_send) {
        LOG_ERROR << "<CODEC-CTX-MGR> Failed to resend pkt:" << ret << "!";
      }
    }

    struct timespec ts_yuv = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts_yuv);
    struct timeval tv_yuv;
    gettimeofday(&tv_yuv, NULL);
    // Create GPUImage
    auto gpuimg        = std::make_shared<GPUImage>();
    gpuimg->dev_id     = devid;
    gpuimg->width      = frame->width;
    gpuimg->height     = frame->height;
    gpuimg->meatime_us = cb(frame.get());
    gpuimg->mode       = imgmode;
    /**
     * @brief 目前外部未使用yuvtime_us,
     * 现使用该字段记录数据送入解码器前后的对应关系
     *
     * @note 1表示 送入解码器与送出解码器的数据是对应的;
     *       0表示 送入解码器与送出解码器的数据是不对应
     */
    gpuimg->yuvtime_us = (pkt->dts == frame->pkt_dts) ? 1 : 0;
    if (gpuimg->yuvtime_us == 0) {
      LOG_ERROR << "<CODEC-CTX-MGR> ERROR: pkt->dts!=frame->pkt_dts,dts:"
                << pkt->dts << ",pkt_dts:" << frame->pkt_dts;
    }
    // Get gpuptr
    size_t gpu_bufsize = 3LL * frame->width * frame->height;
    {
      void *ptr = nullptr;
      std::lock_guard<std::mutex> g(_M_gpulocks[devid]);
      if (_M_gpuptrs[devid].find(gpu_bufsize) == _M_gpuptrs[devid].cend()) {
        // Create new buffers mgr
        _M_gpuptrs[devid][gpu_bufsize];
      }
      // Judge buffer enough
      if (_M_gpuptrs[devid][gpu_bufsize]._M_ptrs.empty()) {
        if (_M_gpuptrs[devid][gpu_bufsize]._M_allocated >=
            GPUBuffers::GPU_BUF_MAX_ALLOC) {
          LOG_ERROR << "<CODEC-CTX-MGR> No gpu memory allowed!";
          break;
        }
        // Allocate a gpu buffer
        int tmpdev = 0;
        cudaGetDevice(&tmpdev);
        cudaSetDevice(devid);
        void *__new_ptr = nullptr;

        if (cudaMalloc(&__new_ptr, gpu_bufsize) != cudaSuccess) {
          cudaError_t code = cudaGetLastError();
          LOG_ERROR << cudaGetErrorString(code);
          abort();
        }

        if (!__new_ptr) {
          LOG_FATAL << "<CODEC-CTX-MGR> Failed to alloc GPU memory!";
          abort();
        }
        _M_gpuptrs[devid][gpu_bufsize]._M_ptrs.emplace_back(__new_ptr);
        _M_gpuptrs[devid][gpu_bufsize]._M_allocated++;
        __new_ptr = nullptr;
        cudaSetDevice(tmpdev);
      }
      // USE Allocated buffer
      ptr = _M_gpuptrs[devid][gpu_bufsize]._M_ptrs.front();
      _M_gpuptrs[devid][gpu_bufsize]._M_ptrs.pop_front();
      gpuimg->gpu_ptr.reset(ptr, [this, devid, gpu_bufsize](void *p) {
        std::lock_guard<std::mutex> g(_M_gpulocks[devid]);
        _M_gpuptrs[devid][gpu_bufsize]._M_ptrs.push_back(p);
      });
    }

    // Change CUDA Context
    auto *hwdevctx =
        reinterpret_cast<AVHWDeviceContext *>(avctx->hw_device_ctx->data);
    if (!hwdevctx) {
      break;
    }
    auto *hwctx = reinterpret_cast<AVCUDADeviceContext *>(hwdevctx->hwctx);
    if (!hwctx) {
      break;
    }
    CUcontext c = hwctx->cuda_ctx;
    if (CUDA_SUCCESS != cuCtxPushCurrent(c)) {
      LOG_FATAL << "<CODEC-CTX-MGR> Failed to push CUDA context!";
      abort();
    }
    if (airos::base::device::gpu::yuv2rgb(
            imgmode, frame.get(), gpu_bufsize, gpuimg->gpu_ptr.get())) {
      image_list->push_back(gpuimg);
    } else {
      LOG_ERROR << "<CODEC-CTX-MGR> Failed to convert:["
                << static_cast<void *>(frame.get()) << ","
                << static_cast<void *>(frame->data[0]) << ","
                << static_cast<void *>(frame->data[1]) << "," << gpu_bufsize
                << "," << frame->width << "," << frame->height << "].";
    }
    if (CUDA_SUCCESS != cuCtxPopCurrent(nullptr)) {
      LOG_FATAL << "<CODEC-CTX-MGR> Failed to pop CUDA context!";
      abort();
    }

    struct timespec ts_rgb = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts_rgb);
    // CAL COST
    double cost_yuv_ms = ts_yuv.tv_sec - ts_bgn.tv_sec;
    cost_yuv_ms *= 1000;
    cost_yuv_ms += 1. * (ts_yuv.tv_nsec - ts_bgn.tv_nsec) / 1000 / 1000;
    double cost_rgb_ms = ts_rgb.tv_sec - ts_yuv.tv_sec;
    cost_rgb_ms *= 1000;
    cost_rgb_ms += 1. * (ts_rgb.tv_nsec - ts_yuv.tv_nsec) / 1000 / 1000;
    LOG_INFO << "<CODEC-CTX-MGR> <GPU-COST> H264_YUV: " << cost_yuv_ms
             << " YUV_RGB: " << cost_rgb_ms;
  }
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
