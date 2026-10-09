#include "middleware/protocol/rtsp_tool/channel_data_processor.h"

#include <iostream>
#include <chrono>
#include <iomanip>

ChannelDataProcessor::ChannelDataProcessor(const std::string& rtsp_url, double fps, bool debug)
    : debug_(debug),
      rtsp_pusher_(std::make_unique<RTSPPusher>(rtsp_url, fps)) {
    if (!rtsp_pusher_->Start()) {
        throw std::runtime_error("Failed to start RTSP pusher");
    }
}

ChannelDataProcessor::~ChannelDataProcessor() {
    if (rtsp_pusher_) {
        rtsp_pusher_->Stop();
    }
}

void ChannelDataProcessor::ProcessData(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg) {
    if (!msg) {
        return;
    }
    PushToRTSP(msg);

    if (debug_) {
        PrintInfo(msg);
        try {
            current_frame_id_ = std::stoull(msg->frame_id());
            has_frame_id_ = true;
        } catch (const std::exception& e) {
            current_frame_id_ = 0;
            last_frame_id_ = 0;
            has_frame_id_ = false;
            std::cout << "Failed to parse frame ID: " << msg->frame_id() << std::endl;
        }
        if(received_first_key_frame) {
            total_frames_++;
            if(has_frame_id_) {
                if(current_frame_id_ ==last_frame_id_ + 1){
                    // DEBUGPRINT("normal frame, current_frame_id: %lu, last_frame_id: %lu", current_frame_id_, last_frame_id_);
                } else if(current_frame_id_ > last_frame_id_ + 1) {
                    dropped_frames_ += (current_frame_id_ - last_frame_id_ - 1);
                } else {
                    if(current_frame_id_ == last_frame_id_) {
                        DEBUGPRINT("duplicate frame, current_frame_id: %lu, last_frame_id: %lu", current_frame_id_, last_frame_id_);
                    } else {
                        DEBUGPRINT("out of order frame, current_frame_id: %lu, last_frame_id: %lu", current_frame_id_, last_frame_id_);
                        out_of_order_frames_++;
                    }
                }
                last_frame_id_ = current_frame_id_;
            }

            if (total_frames_ % 100 == 0) {
                PrintStatistics();
            }
        } else {
            if(has_frame_id_) {
                last_frame_id_ = current_frame_id_;
            } 
        }
    }
}

void ChannelDataProcessor::PrintInfo(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg) {

    std::string frame_type = "普通帧";
    if(msg->frame_type() == 1) {
        received_first_key_frame = true;
        frame_type = "关键帧";
    }
    // DEBUGPRINT("Processing frame: format:%s, type:%s, size:%lu Bytes", msg->format().c_str(), frame_type.c_str(), msg->data().size());
}

void ChannelDataProcessor::PrintStatistics() {
    DEBUGPRINT("===++++++++++++ 统计信息 ++++++++++++++===");
    DEBUGPRINT("total_frames: %lu, dropped_frames: %lu, out_of_order_frames: %lu",
               total_frames_, dropped_frames_, out_of_order_frames_);
    DEBUGPRINT("lost rate: %.2f%%, out_of_order_rate: %.2f%%",
               (dropped_frames_ * 100.0) / total_frames_,
               (out_of_order_frames_ * 100.0) / total_frames_);
    DEBUGPRINT("==========================================");
}

void ChannelDataProcessor::PushToRTSP(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg) {
    if (rtsp_pusher_) {
        const uint8_t* frame_data = reinterpret_cast<const uint8_t*>(msg->data().data());
        int frame_size = msg->data().size();
        
        // 获取相机时间戳（纳秒转换为毫秒）
        int64_t camera_timestamp = 0;
        if (msg->header().has_camera_timestamp()) {
            camera_timestamp = msg->header().camera_timestamp() / 1000000;  // 纳秒转毫秒
        }
        
        // 使用相机时间戳作为 dts 进行帧间隔计算
        if (!rtsp_pusher_->PushFrame(frame_data, frame_size, camera_timestamp, camera_timestamp)) {
            std::cout << "Failed to push frame to RTSP" << std::endl;
        }
    }
}