/*********************************************************************************
 * @file		standard_ipcamera.h
 * @brief		标准IP Camera设备头文件
 * @details		标准IP Camera设备的头文件定义，包含设备类声明和接口定义
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               标准IP Camera设备头文件
 *
 * @endverbatim
 ********************************************************************************/

#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <cfloat>
#include <fstream>

#include "base/device_connect/ipcamera/device_base.h"
#include "glog/logging.h"
#include "yaml-cpp/yaml.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

namespace os {
namespace v2x {
namespace device {

class StandardIpCamera : public IpCameraDevice {
 public:
  StandardIpCamera(const IpCameraCallBack& cb)
      : IpCameraDevice(cb) {}
  ~StandardIpCamera() {
    Stop();
  }

  bool Init(const std::string& conf) override;
  void Start() override;
  void WriteToDevice(
      const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) override;
  IpCameraDeviceState GetState() override;

 private:
  void Stop();
  bool InitFFmpeg();
  void CleanupFFmpeg();
  uint64_t GetCurrentTimestamp();

 private:
  std::atomic<bool> stop_ = ATOMIC_VAR_INIT(false);
  
  // 配置参数
  std::vector<std::string> rtsp_urls_;  // 多路RTSP URL
  std::vector<std::string> output_topics_;  // 输出话题列表
  int reconnect_interval_;  // 重连间隔（秒）
  int connection_timeout_;  // 连接超时时间（秒）
  bool enable_sei_timestamp_;  // 是否启用SEI时间戳解析
  bool enable_h264_save_;  // 是否保存H.264文件到磁盘
  
  // 单个流信息结构体
  struct StreamInfo {
    std::string rtsp_url;
    std::string output_topic;  // 输出话题
    std::atomic<bool> stop;
    std::atomic<bool> connected;
    std::unique_ptr<std::thread> pull_thread;
    AVFormatContext* format_ctx;
    int video_stream_index;
    uint32_t sequence_num;
    uint64_t frame_counter;  // 每路流的独立帧计数器 (0-18446744073709551615)
    double last_send_time;   // 上一帧发送时间（秒）
    
    // 发送间隔统计
    double total_interval_sec;  // 总间隔时间（秒）
    double min_interval_sec;    // 最小间隔（秒）
    double max_interval_sec;    // 最大间隔（秒）
    int64_t interval_count;     // 间隔统计次数
    
    // H.264文件保存相关
    std::ofstream h264_file;  // H.264文件流
    std::string h264_filename;  // H.264文件名
    int64_t saved_frame_count;  // 已保存帧数
    
    StreamInfo() : stop(false), connected(false), format_ctx(nullptr), 
                   video_stream_index(-1), sequence_num(0), frame_counter(0),
                   last_send_time(0.0), total_interval_sec(0.0), min_interval_sec(DBL_MAX), 
                   max_interval_sec(0.0), interval_count(0), saved_frame_count(0) {}
  };
  
  // 多路流相关
  std::map<std::string, std::shared_ptr<StreamInfo>> streams_;
  std::mutex streams_mutex_;
  
  void StartStream(const std::string& stream_id);
  void StopStream(const std::string& stream_id);
  void TaskPullRtspStream(const std::string& stream_id);
  bool ConnectRtspStream(const std::string& stream_id);
  
  // SEI时间戳解析
  int64_t ExtractExposureTimestamp(const uint8_t* data, size_t size);
  
  // 关键帧检测
  bool IsKeyFrame(const uint8_t* data, size_t size);
  
  // H.264文件保存
  void SaveH264ToFile(const std::string& stream_id, const uint8_t* data, size_t size);
  void CloseH264File(const std::string& stream_id);
};

}  // namespace device
}  // namespace v2x
}  // namespace os 
