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

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "middleware/device_service/modules/camera/ipcamera/include/api-structs.h"

namespace airos {
namespace base {
namespace device {

class AVCodecCtxManager {
protected:
    class GPUBuffers final {
    public:
        const static size_t GPU_BUF_MAX_ALLOC = 64ULL;
        size_t _M_allocated = 0;
        std::list<void *> _M_ptrs;
    };

protected: // FUNCTIONS
    AVCodecCtxManager() = default;

    virtual ~AVCodecCtxManager() = default;

public: // FUNCTIONS
    /**
     * 获取驱动单例，线程安全
     * @return Instance
     */
    static AVCodecCtxManager *getInstance();

    typedef std::function<
            int64_t(const AVFrame *)
    > GetMeatimeCallbackT;

    virtual int create_avctx(int devid = 0) = 0;

    virtual void decode_and_convert(
            int ctx_idx,
            bool need_rebuild_ctx,
            const AVPacket *pkt,
            airos::base::Color imgmode,
            const GetMeatimeCallbackT &cb,
            std::list<std::shared_ptr<GPUImage>>* image_list,
            std::string channel_name = "",
            bool use_system_time = false
    ) = 0;

public: // DELETED
    AVCodecCtxManager(const AVCodecCtxManager &) = delete;
    const AVCodecCtxManager &operator=(const AVCodecCtxManager &) = delete;
    AVCodecCtxManager(AVCodecCtxManager &&) = delete;
    AVCodecCtxManager &operator=(AVCodecCtxManager &&) = delete;
};

} // END namespace device
} // END namespace base
} // END namespace airos
