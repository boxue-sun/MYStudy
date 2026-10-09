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
#include <list>

#include "base/common/log.h"
#include "middleware/device_service/modules/camera/ipcamera/include/IPCameraDriver.h"

namespace airos {
namespace base {
namespace device {

int IPCameraDriver::__ffmpeg_callback(
    void *opaque, uint8_t *buf, int buf_size) {
  std::shared_ptr<PrivateInfoT> info = nullptr;
  {
    auto key = reinterpret_cast<int64_t>(opaque);
    std::lock_guard<std::mutex> g(_S_instance->_M_lock_map);
    info = _S_instance->_M_map_private[key];
  }
  std::unique_lock<std::mutex> g(info->_M_lock);
  info->_M_cond.wait(g, [info]() {
    return info->_M_flag2_ffmpeg_need_rebuild ||
           (0 != info->_M_buffer->length());
  });
  if (info->_M_flag2_ffmpeg_need_rebuild) {
    return AVERROR_EOF;
  }
  return static_cast<int>(
      info->_M_buffer->getdata(buf, static_cast<unsigned int>(buf_size)));
}

static bool get_timestamp(
    const void *buf, int64_t size, int64_t *timestamp_us) {
  const unsigned char SEI_header[5] = {0x00, 0x00, 0x00, 0x01, 0x06};
  typedef struct SEI_info {
    unsigned char SEI_payload_type;
    char SEI_payload_size;
    char SEI_payload[0];
  } SEI_info;

  const char *header = reinterpret_cast<const char *>(buf);
  int64_t offset     = 0;
  while (offset < size - (int64_t)sizeof(SEI_header)) {
    if (0 == memcmp(SEI_header, header + offset, sizeof(SEI_header))) {
      const SEI_info *sei_info = reinterpret_cast<const SEI_info *>(
          header + offset + sizeof(SEI_header));
      if (sei_info->SEI_payload_type == 0x05) {
        // dahua的type为0x05增加了自定义uuid，长度固定16
        const int UUID_LEN = 16;
        *timestamp_us =
            1000 *
            atoll(std::string(
                      sei_info->SEI_payload + UUID_LEN,
                      static_cast<int>(sei_info->SEI_payload_size) - UUID_LEN)
                      .c_str());
        return true;
      } else if (sei_info->SEI_payload_type == 0xf0) {
        // hikvision的type为0xf0
        *timestamp_us =
            1000 * atoll(std::string(
                             sei_info->SEI_payload,
                             static_cast<int>(sei_info->SEI_payload_size))
                             .c_str());
        return true;
      } else {
        // unknown type,do nothing
      }
    }
    offset++;
  }

  return false;
}

void IPCameraDriver::__ffmpeg_thread_func() {
  std::list<std::shared_ptr<PrivateInfoT>> li;
  {
    std::lock_guard<std::mutex> g(_S_instance->_M_lock_map);
    for (auto &item : _S_instance->_M_map_private) {
      auto info = item.second;
      std::lock_guard<std::mutex> g2(info->_M_lock);
      if (info->_M_flag0_hik && !info->_M_flag1_ffmpeg) {
        li.emplace_back(info);
      }
    }
  }
  std::list<std::thread> li_thread;
  for (auto &item : li) {
    li_thread.emplace_back([item]() {
      {
        std::lock_guard<std::mutex> g(item->_M_lock);
        item->_M_flag2_ffmpeg_need_rebuild = false;
      }
      //// AVFMT
      AVFormatContext *avfmt = avformat_alloc_context();
      if (!avfmt) {  // UNLIKELY!
        LOG_ERROR << "Failed to create format context!";
        return;
      }
      void *buf = av_malloc(65536);
      if (!buf) {  // UNLIKELY!
        LOG_ERROR << "Failed to create temp buffer!";
        return;
      }
      // AVIO
      AVIOContext *avio = avio_alloc_context(
          reinterpret_cast<unsigned char *>(buf),  // buffer
          65536,                                   // buffer_size
          0,                                       // write_flag
          reinterpret_cast<void *>(item->_M_key),  // opaque
          IPCameraDriver::__ffmpeg_callback,       // read_function
          nullptr,                                 // write_function
          nullptr);                                // seek_function
      if (!avio) {                                 // UNLIKELY!
        LOG_ERROR << "Failed to create avio context!";
        av_freep(&buf);
        return;
      }
      avfmt->pb = avio;
      // OPEN
      if (0 != avformat_open_input(&avfmt, nullptr, nullptr, nullptr)) {
        LOG_ERROR << "<AVFMT>: Can not open stream.";
        avformat_free_context(avfmt);
        return;
      }
      std::shared_ptr<AVFormatContext> avfmt_ptr = nullptr;
      avfmt_ptr.reset(avfmt, [](AVFormatContext *avfmt) {
        avio_context_free(&avfmt->pb);
        avformat_close_input(&avfmt);
      });
      if (0 > avformat_find_stream_info(avfmt_ptr.get(), nullptr)) {
        LOG_ERROR << "<AVFMT>: Can not find stream info.";
        return;
      }
      int video_index = -1;
      for (unsigned int ii = 0; ii < avfmt_ptr->nb_streams; ii++) {
        if (avfmt_ptr->streams[ii]->codecpar->codec_type ==
            AVMEDIA_TYPE_VIDEO) {
          video_index = ii;
          break;
        }
      }
      if (video_index < 0) {
        LOG_ERROR << "<AVFMT>: Can not find video from stream!";
        return;
      }
      // OK!
      item->_M_flag1_ffmpeg = true;
      LOG_INFO << "<AVFMT>: " << item->_M_ip << " starting thread...";
      std::thread([avfmt_ptr, item, video_index]() {
        while ([item]() {
          std::lock_guard<std::mutex> g(item->_M_lock);
          return !item->_M_flag2_ffmpeg_need_rebuild;
        }()) {
          //// Create AVPKT
          std::shared_ptr<AVPacket> avpkt = nullptr;
          avpkt.reset(av_packet_alloc(), [](AVPacket *pkt) {
            av_packet_unref(pkt);
            av_packet_free(&pkt);
          });
          if (0 > av_read_frame(avfmt_ptr.get(), avpkt.get())) {
            LOG_ERROR << "<AVFMT>: Failed to read Packet!";
            continue;
          }
          // Check flags trice
          {
            std::lock_guard<std::mutex> g(item->_M_lock);
            if (item->_M_flag2_ffmpeg_need_rebuild) {
              continue;
            }
          }
          // GET RECVTIME
          struct timespec recvtime = {0, 0};
          clock_gettime(CLOCK_MONOTONIC, &recvtime);
          if (avpkt->stream_index != video_index) {
            continue;
          }
          // CALLBACK
          int64_t starttime_us = item->_M_start_timespec.tv_sec * 1000 * 1000;
          starttime_us += item->_M_start_timespec.tv_nsec / 1000;
          int64_t recvtime_us = recvtime.tv_sec * 1000 * 1000;
          recvtime_us += recvtime.tv_nsec / 1000;

          LOG_INFO << item->_M_ip << " frame sequence "
                   << (++item->single_camera_hik_sequence) << " timestamp "
                   << recvtime.tv_sec << "s " << recvtime.tv_nsec / 1000000.0
                   << "ms";

          struct timeval t = {0, 0};
          gettimeofday(&t, nullptr);
          recvtime_us = t.tv_sec * 1000 * 1000 + t.tv_usec;
          // 获取绝对时间戳，若绝对时间戳不存在就接收时间
          int64_t dts;
          if (get_timestamp(avpkt->buf->data, avpkt->buf->size, &dts)) {
            avpkt->dts = dts;
          } else {
            avpkt->dts = recvtime_us;
            LOG_ERROR << "camera get SEI failed, use "
                         "recvtime instead!";
          }
          item->_M_callback(
              starttime_us,
              recvtime_us,
              avfmt_ptr->streams[video_index],
              avpkt.get());
        }
        std::lock_guard<std::mutex> g(item->_M_lock);
        LOG_INFO << "<AVFMT>: " << item->_M_ip << " rebuilding...";
        item->_M_flag1_ffmpeg = false;
      }).detach();
      LOG_INFO << "<AVFMT>: " << item->_M_ip << " is Ready.";
    });
  }
  for (auto &thread : li_thread) {
    thread.join();
  }
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
