#include "middleware/protocol/rtsp_tool/rtsp_pusher.h"

RTSPPusher::RTSPPusher(const std::string& rtsp_url, double fps)
    : rtsp_url_(rtsp_url),
      fps_(fps),
      output_ctx(nullptr),
      pCodecCtx_(nullptr),
      pCodecParserCtx_(nullptr),
      running_(false) {

    // 初始化 FFmpeg 库
    // av_register_all();
    avformat_network_init();
    DEBUGPRINT("RTSPPusher initialized");
}

void RTSPPusher::InitializeDecoderAndParser() {
    if (!pCodecCtx_ || !pCodecParserCtx_) {
        AVCodec *pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!pCodec) {
            DEBUGPRINT("Failed to find decoder");
            return;
        }
        pCodecCtx_ = avcodec_alloc_context3(pCodec);
        if (!pCodecCtx_) {
            DEBUGPRINT("Failed to allocate codec context");
            return;
        }
        pCodecParserCtx_ = av_parser_init(AV_CODEC_ID_H264);
        if (!pCodecParserCtx_) {
            DEBUGPRINT("Failed to allocate codec parser context");
            avcodec_free_context(&pCodecCtx_);
            pCodecCtx_ = nullptr;
            return;
        }
    }
}


bool RTSPPusher::Start() {
    DEBUGPRINT("Starting RTSP pusher...");
    if (avformat_alloc_output_context2(&output_ctx, nullptr, "rtsp", rtsp_url_.c_str()) < 0) {
        DEBUGPRINT("Failed to create output context");
        return false;
    }

    running_ = true;
    push_thread_ = std::thread(&RTSPPusher::PushLoop, this);
    DEBUGPRINT("RTSP pusher started");

    return true;
}

int64_t RTSPPusher::GetCurrentTimeMs() {
    auto now = std::chrono::high_resolution_clock::now();
    // 转换为毫秒时间戳
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = ms.time_since_epoch().count();
    DEBUGPRINT("Current timestamp:%ld ms", value);
    return value;
}

// 帧率很重要的参数，需要准确，不然容易导致花屏、卡顿等问题
double RTSPPusher::GetFrameRate(const std::vector<int64_t>& frame_timestamps) {
    if (frame_timestamps.size() < 2) {
        return 0.0;
    }

    // 获取最后两帧的时间戳，防止前面包批量涌入
    int64_t last_ts = frame_timestamps.back();
    int64_t second_last_ts = frame_timestamps[frame_timestamps.size() - 2];
    int64_t duration_ms = last_ts - second_last_ts;
    double duration_s = duration_ms / 1000.0; // 毫秒转秒

    // 计算帧率
    double calculated_fps = 1.0 / duration_s;

    // 找到最接近的标准帧率
    double closest_fps = STANDARD_FPS[0];
    for (double fps : STANDARD_FPS) {
        if (std::abs(fps - calculated_fps) < std::abs(closest_fps - calculated_fps)) {
            closest_fps = fps;
        }
    }

    DEBUGPRINT("Duration: %.2f s, Calculated FPS: %.2f, Closest FPS: %.2f", duration_s, calculated_fps, closest_fps);

    return closest_fps;
}

bool RTSPPusher::PushFrame(const uint8_t* frame_data, int frame_size, int64_t dts, int64_t camera_timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_) {
        DEBUGPRINT("Pusher is not running, cannot push frame");
        return false;
    }
    if(!find_idr_) {
        if(frame_rs_.size() >= MAX_QUEUE_SIZE) {
            DEBUGPRINT("frame_rs queue is full, clearing old");
            frame_rs_.clear();
        }
        if (dts > 0) {
            frame_rs_.push_back(dts);
        } else {  // 如果没有提供 dts，则使用当前时间戳
            frame_rs_.push_back(GetCurrentTimeMs());
        }
    }
    if(frame_queue_.size() >= MAX_QUEUE_SIZE) {
        DEBUGPRINT("Frame queue is full, clearing old frames, size: %lu", frame_queue_.size());
        frame_queue_ = std::queue<std::unique_ptr<Frame>>(); // 清空队列
    }
    frame_queue_.push(std::make_unique<Frame>(frame_data, frame_size, camera_timestamp));
    cond_var_.notify_one();
    // DEBUGPRINT("Frame pushed to queue (size: %d bytes), queue size:%lu", frame_size, frame_queue_.size());
    return true;
}

void RTSPPusher::PrintCodecParserInfo(AVCodecParserContext* pCodecParserCtx) {
    if (!pCodecParserCtx) {
        DEBUGPRINT("Codec parser context is null");
        return;
    }

    std::ostringstream oss;

    oss << "Packet Seq Num:" << pCodecParserCtx->output_picture_number;
    switch (pCodecParserCtx->pict_type) {
        case AV_PICTURE_TYPE_I:
            oss << ", Packet Type:I";
            break;
        case AV_PICTURE_TYPE_P:
            oss << ", Packet Type:P";
            break;
        case AV_PICTURE_TYPE_B:
            oss << ", Packet Type:B";
            break;
        default:
            oss << ", Packet Type:error:" << static_cast<int>(pCodecParserCtx->pict_type);
            break;
    }
    oss << "Frame Width:" << pCodecParserCtx->width
        << ", Height:" << pCodecParserCtx->height
        << ", format:" << static_cast<int>(pCodecParserCtx->format)
        << ", pic_type:" << static_cast<int>(pCodecParserCtx->pict_type);
    std::string out = oss.str();
    DEBUGPRINT("%s", out.c_str());
}

void RTSPPusher::HandleFrame(std::unique_ptr<Frame> frame) {
    if (!frame) {
        DEBUGPRINT("Received empty frame");
        return;
    }
    
    AVPacket pkt;
    if (!find_idr_) {
        if(!pCodecCtx_ || !pCodecParserCtx_) {
            InitializeDecoderAndParser();
            if (!pCodecCtx_ || !pCodecParserCtx_) {
                std::cerr << "Failed to initialize decoder or parser" << std::endl;
                return;
            }
        }
        const uint8_t* data = frame->data.data();
        int size = frame->size;
        while (size > 0 && find_idr_ == false) {
            // record 中的数据每个元素都是一个完整的 H.264 NALU
			int parsed_size = av_parser_parse2(pCodecParserCtx_, pCodecCtx_, 
					&pkt.data, &pkt.size,      // 输出
					data, size,                // 输入
				   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);  // 后面3参数不重要,用来填充当前frame时间位置信息
            if (pkt.size == 0) {
                DEBUGPRINT("No data parsed, skipping");
                break; // 没有数据可解析，跳出循环
            }
            DEBUGPRINT("parsed_size size: %d, pkt.size:%d", parsed_size, pkt.size);
            PrintCodecParserInfo(pCodecParserCtx_);

			data += parsed_size;    // 移动缓冲读指针
			size -= parsed_size;    // 剩余缓冲数据长度
            if(pCodecParserCtx_->pict_type == AV_PICTURE_TYPE_I) {
                format_ = pCodecParserCtx_->format;
                width_ = pCodecParserCtx_->width;
                height_ = pCodecParserCtx_->height;
                if(fps_ > 0){
                    find_idr_ = true; // 找到 IDR 帧
                } else {
                    fps_ = GetFrameRate(frame_rs_); // 计算帧率
                    if(fps_ > 0) {
                        find_idr_ = true;
                    } else {
                        DEBUGPRINT("Failed to calculate frame rate, using default FPS");
                        break;
                    }
                }
            }
            if(find_idr_){
                DEBUGPRINT("Parsed frame data, size: %d bytes, format: %d, width: %d, height: %d, fps: %.2f", 
                        pkt.size, format_, width_, height_, fps_);              
                
                // 只在第一次找到IDR帧时创建输出流
                if (!output_ctx->streams || output_ctx->nb_streams == 0) {
                    // 创建输出视频流
                    AVStream* out_stream = avformat_new_stream(output_ctx, nullptr);
                    if (!out_stream) {
                        DEBUGPRINT("Failed to create output stream");
                        find_idr_ = false; // 重置状态
                        return;
                    }
                    
                    // 设置H.264编码参数
                    out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
                    out_stream->codecpar->codec_id = AV_CODEC_ID_H264;
                    out_stream->codecpar->codec_tag = 0;
                    out_stream->codecpar->format = format_;
                    out_stream->codecpar->width = width_;
                    out_stream->codecpar->height = height_;
                    out_stream->time_base = {1, 90000};
                    
                    // 打开输出
                    if (!(output_ctx->oformat->flags & AVFMT_NOFILE)) {
                        if (avio_open(&output_ctx->pb, rtsp_url_.c_str(), AVIO_FLAG_WRITE) < 0) {
                            DEBUGPRINT("Failed to open output URL: %s", rtsp_url_.c_str());
                            find_idr_ = false; // 重置状态
                            break;
                        }
                    }
                    
                    // 写入头部, 默认使用UDP传输
                    // 如果需要强制使用TCP，可以使用以下代码
                    // AVDictionary *options = NULL;
                    // av_dict_set(&options, "rtsp_transport", "tcp", 0);  // 强制使用 TCP
                    // avformat_write_header(output_ctx, &options);
                    if (avformat_write_header(output_ctx, nullptr) < 0) {
                        DEBUGPRINT("Failed to write header");
                        ResetOutputContext(); // 重置输出上下文
                        break;
                    }else{
                        DEBUGPRINT("Header written successfully");
                        average_interval_ = static_cast<int64_t>((1000.0 / fps_) * 90); // 计算平均间隔时间
                    }
                }
            }
		}
        if (!find_idr_) {
            DEBUGPRINT("No IDR frame found, cannot push frame");
            return;
        }
    } 
    
    // SEI 帧处理
    uint8_t* final_data = frame->data.data();
    int final_size = frame->size;
    bool need_free = false;
    
    if (enable_dahua_sei_ && frame->timestamp > 0) {
        // 检测是否包含 SEI 帧
        if (!DetectSEIFrame(frame->data.data(), frame->size)) {
            // 生成大华 SEI 帧
            size_t sei_size = 0;
            uint8_t* sei_data = CreateDahuaSEIFrame(&sei_size, frame->timestamp);
            if (sei_data) {
                // 插入 SEI 帧到数据前
                uint8_t* new_data = nullptr;
                int new_size = 0;
                if (InsertSEIFrame(frame->data.data(), frame->size, sei_data, sei_size, 
                                 &new_data, &new_size)) {
                    final_data = new_data;
                    final_size = new_size;
                    need_free = true;
                    DEBUGPRINT("Generated and inserted Dahua SEI frame, timestamp: %ld", frame->timestamp);
                } else {
                    DEBUGPRINT("Failed to insert SEI frame");
                }
                av_free(sei_data);
            } else {
                DEBUGPRINT("Failed to generate Dahua SEI frame");
            }
        }
    }
    
    // 发送帧
    av_init_packet(&pkt);
    pkt.data = final_data;
    pkt.size = final_size;
    pkt.stream_index = 0;   // 视频目前只有一个流，索引为0
    // 生成时间戳
    pkt.dts = last_dts_ + average_interval_;
    pkt.pts = pkt.dts;
    last_dts_ = pkt.dts;

    // DEBUGPRINT("Pushing frame (size: %d bytes, dts: %ld, pts: %ld)", pkt.size, pkt.dts, pkt.pts);

    if (av_interleaved_write_frame(output_ctx, &pkt) < 0) {
        DEBUGPRINT("Failed to write frame");
        ResetOutputContext(); // 重置输出上下文
        if (need_free) {
            av_free(final_data);
        }
        return;
    }
    
    // 释放临时分配的内存
    if (need_free) {
        av_free(final_data);
    }
}

void RTSPPusher::PushLoop() {
    try {
        while (running_) {
            std::unique_ptr<Frame> frame;
            std::unique_lock<std::mutex> lock(mutex_);

            cond_var_.wait(lock, [this]() { return !running_ || !frame_queue_.empty(); });

            if (!running_) {
                break;
            }

            frame = std::move(frame_queue_.front());
            frame_queue_.pop();
            lock.unlock();

            if (!frame) {
                continue;
            }

            // DEBUGPRINT("Processing frame (size: %d bytes)", frame->size);
            HandleFrame(std::move(frame));
        }
    } catch (const std::exception& e) {
        DEBUGPRINT("Exception in PushLoop: %s", e.what())
    } catch (...) {
        DEBUGPRINT("Unknown exception in PushLoop");
    }

    DEBUGPRINT("Push loop finished");
}

void RTSPPusher::ResetOutputContext() {
    find_idr_ = false; // 重置状态
    if (output_ctx->pb) {
        avio_closep(&output_ctx->pb);
    }
    // 1. 清理旧的 output_ctx
    if (output_ctx->pb) avio_closep(&output_ctx->pb); {
        avformat_free_context(output_ctx);
        output_ctx = nullptr;
    }
    // 2. 重新初始化 output_ctx
    if (avformat_alloc_output_context2(&output_ctx, nullptr, "rtsp", rtsp_url_.c_str()) < 0) {
        DEBUGPRINT("Failed to recreate output context after failure");
        return;
    }
    DEBUGPRINT("Output context reset");
}

void RTSPPusher::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(running_) {
            DEBUGPRINT("Stopping RTSP pusher...");
            running_ = false;
            frame_queue_ = std::queue<std::unique_ptr<Frame>>(); // 清空队列
        } 
    }
    cond_var_.notify_all();
    
    if (push_thread_.joinable()) {
        push_thread_.join(); // 先等待线程结束
    }

    // 再释放RTSP资源
    if (output_ctx) {
        av_write_trailer(output_ctx);
        avformat_free_context(output_ctx);
        output_ctx = nullptr;
    }
}


RTSPPusher::~RTSPPusher() {
    Stop();
    avformat_network_deinit();
    // 释放解码器上下文和解析器上下文
    if (pCodecParserCtx_) {
        av_parser_close(pCodecParserCtx_);
        pCodecParserCtx_ = nullptr;
    }
    if (pCodecCtx_) {
        avcodec_free_context(&pCodecCtx_);
        pCodecCtx_ = nullptr;
    }
    DEBUGPRINT("RTSPPusher destroyed");
}


bool RTSPPusher::DetectSEIFrame(const uint8_t* data, int size) {
    if (!data || size < 4) {
        return false;
    }
    
    size_t pos = 0;
    while (pos < size - 3) {
        // 检查 3 字节起始码
        if (data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
            uint8_t nal_type = data[pos + 3] & 0x1F;
            if (nal_type == 6) {  // SEI NALU 类型
                return true;
            }
            pos += 4;
        }
        // 检查 4 字节起始码
        else if (pos < size - 4 && data[pos] == 0x00 && data[pos + 1] == 0x00 && 
                 data[pos + 2] == 0x00 && data[pos + 3] == 0x01) {
            uint8_t nal_type = data[pos + 4] & 0x1F;
            if (nal_type == 6) {  // SEI NALU 类型
                return true;
            }
            pos += 5;
        }
        else {
            pos++;
        }
    }
    return false;
}

uint8_t* RTSPPusher::CreateDahuaSEIFrame(size_t* out_size, int64_t timestamp) {
    if (!out_size) {
        return nullptr;
    }
    
    // 大华 SEI 帧格式（参考 rtsp_server.cpp）
    const uint8_t start_code[] = {0x00, 0x00, 0x00, 0x01};
    const uint8_t nal_type = 0x06;           // NALU 类型 6 (SEI)
    const uint8_t payload_type1 = 0x05;      // 负载类型1
    const uint8_t payload_type2 = 0x00;      // 负载类型2
    const uint8_t payload_size = 0x30;       // 负载大小 (48字节)
    const char uuid[] = "DahuaTechnology\0"; // UUID (16字节)
    char timestamp_str[14];                  // 时间戳字符串 (14字节)
    const uint8_t reserved[] = {0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
                               0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A}; // 保留字段 (18字节)
    const uint8_t rbsp_stop_bit = 0x80;      // 停止位
    
    // 格式化时间戳为 13 位字符串（左补零）
    snprintf(timestamp_str, sizeof(timestamp_str), "%013ld", timestamp);
    timestamp_str[13] = '\0';
    
    // 计算总大小
    *out_size = 4 + 4 + 48 + 1;  // 起始码 + NALU头 + 负载 + 停止位 = 57字节
    
    // 分配内存
    uint8_t* sei = (uint8_t*)av_malloc(*out_size);
    if (!sei) {
        DEBUGPRINT("Failed to allocate SEI frame memory");
        return nullptr;
    }
    
    // 构建 SEI 帧
    size_t pos = 0;
    
    // 起始码
    memcpy(sei + pos, start_code, 4);
    pos += 4;
    
    // NALU 头
    sei[pos++] = nal_type;
    sei[pos++] = payload_type1;
    sei[pos++] = payload_type2;
    sei[pos++] = payload_size;
    
    // UUID
    memcpy(sei + pos, uuid, 16);
    pos += 16;
    
    // 时间戳
    memcpy(sei + pos, timestamp_str, 14);
    pos += 14;
    
    // 保留字段
    memcpy(sei + pos, reserved, 18);
    pos += 18;
    
    // 停止位
    sei[pos++] = rbsp_stop_bit;
    
    
    return sei;
}

bool RTSPPusher::InsertSEIFrame(const uint8_t* original_data, int original_size,
                               const uint8_t* sei_data, size_t sei_size,
                               uint8_t** new_data, int* new_size) {
    if (!original_data || !sei_data || !new_data || !new_size) {
        return false;
    }
    
    // 计算新数据大小
    *new_size = original_size + sei_size;
    
    // 分配新内存
    *new_data = (uint8_t*)av_malloc(*new_size);
    if (!*new_data) {
        DEBUGPRINT("Failed to allocate memory for new data with SEI");
        return false;
    }
    
    // 先复制 SEI 帧
    memcpy(*new_data, sei_data, sei_size);
    
    // 再复制原始数据
    memcpy(*new_data + sei_size, original_data, original_size);
    
    
    return true;
}
