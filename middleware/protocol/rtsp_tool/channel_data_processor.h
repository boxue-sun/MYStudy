#ifndef CHANNEL_DATA_PROCESSOR_H
#define CHANNEL_DATA_PROCESSOR_H

#include "base/device_connect/proto/ipcamera.pb.h"
#include "middleware/protocol/rtsp_tool/rtsp_pusher.h"

#include <memory>
#include <string>
#include <map>

class ChannelDataProcessor {
public:
    ChannelDataProcessor(const std::string& rtsp_url, double fps = 25.0, bool debug = false);
    ~ChannelDataProcessor();

    /**
     * @brief 处理接收到的压缩图像数据
     * @param msg 接收到的压缩图像消息
     */
    void ProcessData(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg);

    /**
     * @brief 停止RTSP推流器
     */
    void Stop() {
        if (rtsp_pusher_) {
            rtsp_pusher_->Stop();
        }
    }

private:
    /**
     * @brief 打印当前时间和图像信息
     * @param frame_id 帧ID
     * @param msg 压缩图像消息
     */
    void PrintInfo(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg);
    /**
     * @brief 打印统计信息
     */
    void PrintStatistics();
    /**
     * @brief 推送压缩图像数据到RTSP服务器
     * @param msg 压缩图像消息
     */
    void PushToRTSP(const std::shared_ptr<const os::v2x::device::ipcamera::CompressedImage>& msg);

    bool debug_ = false;                        ///< 是否启用调试模式
    std::unique_ptr<RTSPPusher> rtsp_pusher_;   ///< RTSP推流器
    bool has_frame_id_ = false;                 ///< 是否有帧ID
    uint64_t current_frame_id_ = 0;             ///< 当前帧ID
    uint64_t last_frame_id_ = 0;                ///< 上一帧ID
    uint64_t total_frames_ = 0;                 ///< 总帧数
    uint64_t dropped_frames_ = 0;               ///< 丢包数
    uint64_t out_of_order_frames_ = 0;          ///< 乱序帧数
    bool received_first_key_frame = false;      ///< 收到关键帧开始计数
};

#endif // CHANNEL_DATA_PROCESSOR_H