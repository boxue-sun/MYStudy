/*********************************************************************************
 * @file		standard_ipcamera.cc
 * @brief		标准IP Camera设备实现
 * @details		标准IP Camera设备的具体实现，支持多路RTSP流处理和H264解码
 * @author		ChangXuhui
 * @date		2025/7/4
 * @copyright	Copyright (c) 2025 The Airos Authors.
 * @verbatim
 *
 *  Change History:
 *  Date         Author        Version      ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2025/7/4  ChangXuhui      1.0          ————               标准IP Camera设备实现
 *
 * @endverbatim
 ********************************************************************************/

#include "standard_ipcamera.h"
#include "base/device_connect/ipcamera/device_factory.h"
#include "base/common/time_util.h"
#include "base/common/log.h"
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <regex>
#include <string>
#include <fstream>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>

namespace os {
namespace v2x {
namespace device {

// 文件写入相关全局变量
static std::map<std::string, std::ofstream> g_h264_files;
static std::mutex g_file_mutex;
static std::map<std::string, int64_t> g_frame_counts;

bool StandardIpCamera::Init(const std::string& conf) {
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Init config file: " << conf;
  
  try {
    YAML::Node config = YAML::LoadFile(conf);
    
    // 读取配置参数
    reconnect_interval_ = config["reconnect_interval"].as<int>(5);
    connection_timeout_ = config["connection_timeout"].as<int>(10);
    enable_sei_timestamp_ = config["enable_sei_timestamp"].as<bool>(true);
    enable_h264_save_ = config["enable_h264_save"].as<bool>(false);
    
    // 读取多路RTSP URL
    if (config["rtsp_urls"] && config["rtsp_urls"].IsSequence()) {
      for (const auto& url : config["rtsp_urls"]) {
        rtsp_urls_.push_back(url.as<std::string>());
      }
    }
    
    // 读取输出话题配置
    if (config["output_topics"] && config["output_topics"].IsSequence()) {
      for (const auto& topic : config["output_topics"]) {
        output_topics_.push_back(topic.as<std::string>());
      }
    }
    
    // 验证配置一致性
    if (rtsp_urls_.size() != output_topics_.size()) {
      IPCAMERA_SERVICE_LOG_ERROR << "RTSP URLs count (" << rtsp_urls_.size() 
                 << ") does not match output topics count (" << output_topics_.size() << ")";
      return false;
    }

    IPCAMERA_SERVICE_LOG_INFO << "RTSP URLs count: " << rtsp_urls_.size();
    for (size_t i = 0; i < rtsp_urls_.size(); ++i) {
      IPCAMERA_SERVICE_LOG_INFO << "RTSP URL " << i << ": " << rtsp_urls_[i];
      IPCAMERA_SERVICE_LOG_INFO << "Output Topic " << i << ": " << output_topics_[i];
    }
    IPCAMERA_SERVICE_LOG_INFO << "Reconnect interval: " << reconnect_interval_ << "s";
    IPCAMERA_SERVICE_LOG_INFO << "Connection timeout: " << connection_timeout_ << "s";
    IPCAMERA_SERVICE_LOG_INFO << "SEI timestamp parsing: " << (enable_sei_timestamp_ ? "enabled" : "disabled");
    IPCAMERA_SERVICE_LOG_INFO << "H.264 file save: " << (enable_h264_save_ ? "enabled" : "disabled");
    
    // 初始化 FFmpeg
    if (!InitFFmpeg()) {
      IPCAMERA_SERVICE_LOG_ERROR << "Failed to initialize FFmpeg";
      return false;
    }
    
    // 为每个URL创建流信息
    for (size_t i = 0; i < rtsp_urls_.size(); ++i) {
      auto stream_info = std::make_shared<StreamInfo>();
      stream_info->rtsp_url = rtsp_urls_[i];
      stream_info->output_topic = output_topics_[i];
      
      std::lock_guard<std::mutex> lock(streams_mutex_);
      streams_[output_topics_[i]] = stream_info;  // 使用输出话题作为流ID
      
      IPCAMERA_SERVICE_LOG_INFO << "Created stream for topic: " << output_topics_[i] 
                << " with URL: " << rtsp_urls_[i];
    }
    
    IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera initialized successfully";
    return true;
    
  } catch (const std::exception& e) {
    IPCAMERA_SERVICE_LOG_ERROR << "Failed to parse config file: " << e.what();
    return false;
  }
}

void StandardIpCamera::Start() {
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Start";
  
  if (stop_.load()) {
    IPCAMERA_SERVICE_LOG_WARN << "Camera already stopped, cannot start";
    return;
  }
  
  // 启动所有流
  std::lock_guard<std::mutex> lock(streams_mutex_);
  for (auto& pair : streams_) {
    StartStream(pair.first);
  }
  
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera started with " << streams_.size() << " streams";
}

void StandardIpCamera::Stop() {
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::Stop";
  
  stop_.store(true);
  
  // 停止所有流
  std::lock_guard<std::mutex> lock(streams_mutex_);
  for (auto& pair : streams_) {
    StopStream(pair.first);
  }
  
  // 清理 FFmpeg 资源
  CleanupFFmpeg();
  
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera stopped";
}

void StandardIpCamera::StartStream(const std::string& stream_id) {
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
    return;
  }
  
  auto& stream_info = it->second;
  if (stream_info->pull_thread) {
    IPCAMERA_SERVICE_LOG_WARN << "Stream " << stream_id << " already running";
    return;
  }
  
  stream_info->stop.store(false);
  stream_info->pull_thread.reset(new std::thread([this, stream_id]() {
    this->TaskPullRtspStream(stream_id);
  }));
  
  IPCAMERA_SERVICE_LOG_INFO << "Started stream: " << stream_id;
}

void StandardIpCamera::StopStream(const std::string& stream_id) {
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    return;
  }
  
  auto& stream_info = it->second;
  stream_info->stop.store(true);
  stream_info->connected.store(false);
  
  if (stream_info->pull_thread && stream_info->pull_thread->joinable()) {
    stream_info->pull_thread->join();
  }
  
  if (stream_info->format_ctx) {
    avformat_close_input(&stream_info->format_ctx);
    stream_info->format_ctx = nullptr;
  }
  
  // 关闭H.264文件
  if (enable_h264_save_) {
    CloseH264File(stream_id);
  }
  
  IPCAMERA_SERVICE_LOG_INFO << "Stopped stream: " << stream_id;
}

bool StandardIpCamera::InitFFmpeg() {
  IPCAMERA_SERVICE_LOG_INFO << "Initializing FFmpeg";
  
  // 注册所有格式和编解码器
  av_register_all();
  avformat_network_init();
  
  IPCAMERA_SERVICE_LOG_INFO << "FFmpeg initialized successfully";
  return true;
}

void StandardIpCamera::CleanupFFmpeg() {
  IPCAMERA_SERVICE_LOG_INFO << "Cleaning up FFmpeg resources";
  
  std::lock_guard<std::mutex> lock(streams_mutex_);
  for (auto& pair : streams_) {
    auto& stream_info = pair.second;
    if (stream_info->format_ctx) {
      avformat_close_input(&stream_info->format_ctx);
      stream_info->format_ctx = nullptr;
    }
    stream_info->connected.store(false);
  }
  
  IPCAMERA_SERVICE_LOG_INFO << "FFmpeg resources cleaned up";
}



void StandardIpCamera::TaskPullRtspStream(const std::string& stream_id) {
  IPCAMERA_SERVICE_LOG_INFO << "RTSP pulling task started for stream: " << stream_id;
  
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
    return;
  }
  
  auto& stream_info = it->second;
  
  while (!stream_info->stop.load() && !stop_.load()) {
    // 尝试连接 RTSP 流
    if (!ConnectRtspStream(stream_id)) {
      IPCAMERA_SERVICE_LOG_ERROR << "Failed to connect to RTSP stream " << stream_id 
                 << ", retrying in " << reconnect_interval_ << " seconds";
      std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_));
      continue;
    }
    
    IPCAMERA_SERVICE_LOG_INFO << "Successfully connected to RTSP stream: " << stream_id;
    stream_info->connected.store(true);
    
    // 读取数据包
    AVPacket pkt;
    av_init_packet(&pkt);
    
    while (!stream_info->stop.load() && !stop_.load() && stream_info->connected.load()) {
      int ret = av_read_frame(stream_info->format_ctx, &pkt);
      if (ret < 0) {
        if (ret == AVERROR_EOF) {
          IPCAMERA_SERVICE_LOG_WARN << "RTSP stream ended: " << stream_id;
        } else {
          IPCAMERA_SERVICE_LOG_ERROR << "Error reading frame from RTSP stream " << stream_id << ": " << ret;
        }
        stream_info->connected.store(false);
        break;
      }
      
      // 只处理视频流
      if (pkt.stream_index == stream_info->video_stream_index) {  
        // 创建压缩图像数据
        auto compressed_image = std::make_shared<os::v2x::device::ipcamera::CompressedImage>();
        
        // 设置图像信息 - 使用每路流的独立递增帧ID (64位整数)
        uint64_t current_frame_id = ++stream_info->frame_counter;
        compressed_image->set_frame_id(std::to_string(current_frame_id));
        compressed_image->set_format("h264");  // H264 格式
        
        // 检测是否为关键帧
        bool is_key_frame = IsKeyFrame(pkt.data, pkt.size);
        compressed_image->set_frame_type(is_key_frame ? 1 : 0);  // 1表示关键帧，0表示普通帧
        
        // 从SEI数据中解析曝光时间戳
        if (enable_sei_timestamp_) {
          int64_t exposure_timestamp = ExtractExposureTimestamp(pkt.data, pkt.size);
          if (exposure_timestamp > 0) {
            compressed_image->set_exposure_time(exposure_timestamp);
          } else {
            // 使用系统当前时间作为曝光时间戳
            int64_t current_time = GetCurrentTimestamp();
            compressed_image->set_exposure_time(current_time);
          }
        } else {
          // 使用系统当前时间作为曝光时间戳
          int64_t current_time = GetCurrentTimestamp();
          compressed_image->set_exposure_time(current_time);
        }
        // 设置 H264 NALU 数据
        std::string h264_data(reinterpret_cast<const char*>(pkt.data), pkt.size);
        compressed_image->set_data(h264_data);
      
        // 在发送前设置header信息（确保时间戳更准确）
        auto header = compressed_image->mutable_header();
        header->set_timestamp_sec(airos::base::TimeUtil::GetCurrentTime());
        header->set_module_name("ipcamera_service");
        header->set_sequence_num(++stream_info->sequence_num);
        header->set_camera_timestamp(GetCurrentTimestamp());
        compressed_image->set_measurement_time(airos::base::TimeUtil::GetCurrentTime());
        
        // 计算发送间隔
        double current_time = airos::base::TimeUtil::GetCurrentTime();
        if (stream_info->last_send_time > 0.0) {
            double interval = current_time - stream_info->last_send_time;
            
            // 更新统计信息
            stream_info->total_interval_sec += interval;
            stream_info->interval_count++;
            if (interval < stream_info->min_interval_sec) {
                stream_info->min_interval_sec = interval;
            }
            if (interval > stream_info->max_interval_sec) {
                stream_info->max_interval_sec = interval;
            }
        }
        stream_info->last_send_time = current_time;
        
        // 发送数据，使用多路流回调
        if (sender_) {
          sender_(stream_info->output_topic, compressed_image);
          
          // 如果启用H.264保存，则保存到文件
          if (enable_h264_save_) {
            SaveH264ToFile(stream_id, pkt.data, pkt.size);
          }
          
          // 每100帧打印间隔统计信息
          if (stream_info->sequence_num % 100 == 0 && stream_info->interval_count > 0) {
            double avg_interval = stream_info->total_interval_sec / stream_info->interval_count;
            IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " send interval stats: "
                      << "avg=" << std::fixed << std::setprecision(3) << avg_interval << "s, "
                      << "min=" << std::fixed << std::setprecision(3) << stream_info->min_interval_sec << "s, "
                      << "max=" << std::fixed << std::setprecision(3) << stream_info->max_interval_sec << "s, "
                      << "count=" << stream_info->interval_count;
          }
        }
      }
      
      av_packet_unref(&pkt);
    }
    
    // 断开连接，准备重连
    if (stream_info->format_ctx) {
      avformat_close_input(&stream_info->format_ctx);
      stream_info->format_ctx = nullptr;
    }
    
    if (!stream_info->stop.load() && !stop_.load()) {
      IPCAMERA_SERVICE_LOG_INFO << "Reconnecting to RTSP stream " << stream_id 
                << " in " << reconnect_interval_ << " seconds";
      std::this_thread::sleep_for(std::chrono::seconds(reconnect_interval_));
    }
  }
  
  IPCAMERA_SERVICE_LOG_INFO << "RTSP pulling task finished for stream: " << stream_id;
}

bool StandardIpCamera::ConnectRtspStream(const std::string& stream_id) {
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    IPCAMERA_SERVICE_LOG_ERROR << "Stream not found: " << stream_id;
    return false;
  }
  
  auto& stream_info = it->second;
  IPCAMERA_SERVICE_LOG_INFO << "Connecting to RTSP stream: " << stream_info->rtsp_url;
  
  // 打开 RTSP 流
  if (avformat_open_input(&stream_info->format_ctx, stream_info->rtsp_url.c_str(), nullptr, nullptr) < 0) {
    IPCAMERA_SERVICE_LOG_ERROR << "Could not open RTSP stream: " << stream_info->rtsp_url;
    return false;
  }
  
  // 获取流信息
  if (avformat_find_stream_info(stream_info->format_ctx, nullptr) < 0) {
    IPCAMERA_SERVICE_LOG_ERROR << "Could not find stream information";
    avformat_close_input(&stream_info->format_ctx);
    return false;
  }
  
  // 打印流信息
  av_dump_format(stream_info->format_ctx, 0, stream_info->rtsp_url.c_str(), 0);
  
  // 查找视频流索引
  stream_info->video_stream_index = -1;
  for (unsigned i = 0; i < stream_info->format_ctx->nb_streams; ++i) {
    if (stream_info->format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      stream_info->video_stream_index = i;
      break;
    }
  }
  
  if (stream_info->video_stream_index == -1) {
    IPCAMERA_SERVICE_LOG_ERROR << "No video stream found";
    avformat_close_input(&stream_info->format_ctx);
    return false;
  }
  
  IPCAMERA_SERVICE_LOG_INFO << "Successfully connected to RTSP stream " << stream_id 
            << ", video stream index: " << stream_info->video_stream_index;
  return true;
}

uint64_t StandardIpCamera::GetCurrentTimestamp() {
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
  return timestamp.count();
}

int64_t StandardIpCamera::ExtractExposureTimestamp(const uint8_t* data, size_t size) {
  if (!data || size == 0) {
    return -1;
  }
  
  size_t pos = 0;
  
  while (pos < size) {
    // 查找 NALU 起始码 (0x00 0x00 0x01)
    if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
      uint8_t nal_type = data[pos + 3] & 0x1F;
      size_t start_code_len = 3;
      
      if (nal_type == 6) { // SEI NALU
        size_t sei_payload_start = pos + start_code_len;
        size_t sei_payload_size = 0;
        
        // 找到 SEI 数据的结束位置
        while (sei_payload_start + sei_payload_size < size && data[sei_payload_start + sei_payload_size] != 0x80) {
          sei_payload_size++;
        }
        
        if (sei_payload_start + sei_payload_size < size) {
          // 假设时间戳从 SEI 数据的固定偏移开始
          const size_t timestamp_offset = 20; // 根据实际 SEI 数据调整
          const size_t timestamp_size = 14;    // 时间戳长度（字节）
          
          if (sei_payload_start + timestamp_offset + timestamp_size <= size) {
            char timestamp[timestamp_size + 1] = {0};
            memcpy(timestamp, data + sei_payload_start + timestamp_offset, timestamp_size);
            timestamp[timestamp_size] = '\0';
            
            try {
              int64_t timestamp_ms = std::stoll(timestamp);
              if (timestamp_ms > 1000000000000) { // 验证时间戳合理性
                return timestamp_ms;
              }
            } catch (const std::exception& e) {
              IPCAMERA_SERVICE_LOG_WARN << "Failed to parse timestamp: " << e.what();
            }
          } else {
            IPCAMERA_SERVICE_LOG_WARN << "Timestamp field out of SEI data range";
          }
        }
        
        // 跳过整个 SEI 数据块
        pos += start_code_len + sei_payload_size + 1;
        continue;
      }
    }
    
    pos++;
  }
  
  return -1; // 未找到有效时间戳
}

bool StandardIpCamera::IsKeyFrame(const uint8_t* data, size_t size) {
  if (!data || size == 0) {
    return false;
  }
  
  size_t pos = 0;
  
  while (pos < size) {
    // 查找 NALU 起始码 (0x00 0x00 0x01)
    if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
      uint8_t nal_type = data[pos + 3] & 0x1F;
      
      // I帧的NALU类型是5 (IDR帧)
      if (nal_type == 5) {
        return true;
      }
      
      // 跳过当前NALU
      pos += 4;
      while (pos < size) {
        // 查找下一个NALU起始码
        if (pos + 3 < size && data[pos] == 0x00 && data[pos + 1] == 0x00 && data[pos + 2] == 0x01) {
          break;
        }
        pos++;
      }
      continue;
    }
    
    pos++;
  }
  
  return false;
}

void StandardIpCamera::WriteToDevice(
    const std::shared_ptr<const os::v2x::device::ipcamera::IpCameraReceiveData>& receive_data) {
  // 对于标准IP Camera，通常不需要写入数据
  // 这里可以实现一些控制命令，比如调整参数等
  IPCAMERA_SERVICE_LOG_INFO << "StandardIpCamera::WriteToDevice - Control command received";
  
  if (receive_data) {
    IPCAMERA_SERVICE_LOG_INFO << "Received control data with size: " << receive_data->data().size();
    // 这里可以根据需要实现具体的控制逻辑
  }
}

IpCameraDeviceState StandardIpCamera::GetState() {
  if (stop_.load()) {
    return IpCameraDeviceState::STOP;
  }
  
  // 检查多路流状态
  std::lock_guard<std::mutex> lock(streams_mutex_);
  for (const auto& pair : streams_) {
    if (pair.second->connected.load()) {
      return IpCameraDeviceState::RUNNING;
    }
  }
  
  return IpCameraDeviceState::UNKNOWN;
}

void StandardIpCamera::SaveH264ToFile(const std::string& stream_id, const uint8_t* data, size_t size) {
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    return;
  }
  
  auto& stream_info = it->second;
  
  // 检查文件是否已打开，如果没有则创建新文件
  if (!stream_info->h264_file.is_open()) {
    // 生成文件名：stream_id_YYYYMMDD_HHMMSS.h264
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time_t);
    
    // 从stream_id中提取IP地址（格式：/sensor/ipcamera/h264/172_20_65_193）
    std::string ip_address = stream_id;
    size_t pos = ip_address.find_last_of("/");
    if (pos != std::string::npos) {
        ip_address = ip_address.substr(pos + 1);  // 提取IP部分：172_20_65_193
    }

    // 生成包含完整IP的文件名
    std::stringstream filename_ss;
    filename_ss << ip_address << "_"
                << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".h264";
    stream_info->h264_filename = "/home/airos/data/" + filename_ss.str();
    
    // 确保目录存在
    std::string dir_path = "/home/airos/data";
    struct stat st = {0};
    if (stat(dir_path.c_str(), &st) == -1) {
        mkdir(dir_path.c_str(), 0755);
    }
    
    // 打开文件
    stream_info->h264_file.open(stream_info->h264_filename, std::ios::binary | std::ios::app);
    if (!stream_info->h264_file.is_open()) {
      IPCAMERA_SERVICE_LOG_ERROR << "Failed to open H.264 file: " << stream_info->h264_filename;
      return;
    }
    
    IPCAMERA_SERVICE_LOG_INFO << "Created H.264 file: " << stream_info->h264_filename << " for stream: " << stream_id;
  }
  
  // 写入H.264数据
  stream_info->h264_file.write(reinterpret_cast<const char*>(data), size);
  stream_info->h264_file.flush();  // 确保数据立即写入磁盘
  
  // 更新统计信息
  stream_info->saved_frame_count++;
  
  // 每1000帧打印一次统计信息
  if (stream_info->saved_frame_count % 1000 == 0) {
    IPCAMERA_SERVICE_LOG_INFO << "Stream " << stream_id << " saved " 
              << stream_info->saved_frame_count << " frames to H.264 file: " << stream_info->h264_filename;
  }
}

void StandardIpCamera::CloseH264File(const std::string& stream_id) {
  auto it = streams_.find(stream_id);
  if (it == streams_.end()) {
    return;
  }
  
  auto& stream_info = it->second;
  if (stream_info->h264_file.is_open()) {
    stream_info->h264_file.close();
    IPCAMERA_SERVICE_LOG_INFO << "Closed H.264 file for stream: " << stream_id 
              << " (saved " << stream_info->saved_frame_count << " frames): " << stream_info->h264_filename;
  }
}

// 注册设备到工厂
V2XOS_IPCAMERA_REG_FACTORY(StandardIpCamera, "standard_ipcamera");

}  // namespace device
}  // namespace v2x
}  // namespace os
