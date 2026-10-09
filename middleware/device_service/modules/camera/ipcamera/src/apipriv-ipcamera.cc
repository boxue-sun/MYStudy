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

#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-ipcamera.h"

#include <sys/time.h>
#include <time.h>

#include <ctime>

#include "ipcamera/proto/sensor_image.pb.h"

#include "base/blob/cuda_util.h"
#include "base/common/log.h"
#include "base/common/time_util.h"
#include "middleware/device_service/modules/camera/ipcamera/include/IPCameraDriver.h"
#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-avcodec-mgr.h"
#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-custom-avlog.h"
namespace airos {
namespace base {
namespace device {

const char *const DriverAPI::INPUT_CHANNEL_PREFIX = "/sensor/ipcamera/h264/";
std::mutex DriverAPI::_S_lock_instance;
DriverAPI *DriverAPI::_S_instance = nullptr;

DriverAPI::DriverAPI(uint32_t mode)
    : _M_mode(mode)
    , _M_initflag(false) {
  struct timeval t_val = {0, 0};
  gettimeofday(&t_val, nullptr);
  std::string unique_str =
      std::to_string(t_val.tv_sec * 1000000 + t_val.tv_usec);
  _M_debug_node = std::make_shared<airos::middleware::AirMiddlewareNode>(
      unique_str + "_DriverStreamingInfo");

  AVCodecCtxManager::getInstance();
  if (_M_mode & API_MODE_OFFLINE) {
    // OFFLINE
    struct timeval tv = {0, 0};
    gettimeofday(&tv, nullptr);
    std::string cur_time = std::to_string(tv.tv_sec * 1000000 + tv.tv_usec);

    _M_node_offline = std::make_shared<airos::middleware::AirMiddlewareNode>(
        std::string("IPCameraOfflineDebugger").append(cur_time));

    if (!_M_node_offline) {
      LOG_FATAL << "Failed to create Node for offline!";
      return;
    }
  } else {
    // ONLINE MODE
    if (!IPCameraDriver::getInstance()) {
      LOG_FATAL << "Failed to start Driver!";
      return;
    }
  }
  if (_M_mode & API_MODE_DISABLE_REDIRECT_AVLOG) {
    LOG_WARN << "Ignore FFMPEG av_log.";
  } else {
    LOG_INFO << "Redirect FFMPEG av_log to glog.";
    av_log_set_callback(apipriv_custom_avlog);
  }
  _M_initflag = true;

  LOG_INFO << "DriverAPI Instance Init OK! Modes: "
           << (_M_mode & API_MODE_OFFLINE ? "OFFLINE" : "ONLINE");
}

Vendor get_vendor(const std::string &vendor) {
  if (vendor == "hikvision") {
    return Vendor::HIKVISION;
  } else if (vendor == "dahua") {
    return Vendor::DAHUA;
  }
}

void DriverAPI::__convert_img(
    std::shared_ptr<GPUImage> decoded_img,
    std::shared_ptr<CameraImageData> output_data) {
  output_data->device_id    = decoded_img->dev_id;
  output_data->camera_name  = decoded_img->channel_str;
  output_data->mode         = decoded_img->mode;
  output_data->height       = decoded_img->height;
  output_data->width        = decoded_img->width;
  output_data->timestamp    = decoded_img->meatime_us;
  output_data->sequence_num = decoded_img->sequence_num;

  output_data->image = std::make_shared<airos::base::Image8U>(
      output_data->width, output_data->height, output_data->mode);
  unsigned int img_len = output_data->width * output_data->height * 3;
  airos::base::CudaUtil::CopyDeviceToDevice(
      output_data->image->mutable_device_data(),
      img_len,
      decoded_img->gpu_ptr.get(),
      img_len);
}

bool DriverAPI::__add_handle(
    const std::string &ipport, const std::string &channel_str, int32_t cudadev,
    airos::base::Color imgmode, const std::string &vendor, int stream_num,
    int channel_num, const std::string &username, const std::string &passwd,
    const CameraImageCallBack cb) {
  std::string ipstr   = ipport;
  std::string ip_name = ipport;
  // Judge IP not existed
  auto hasport = ipstr.find(':');
  unsigned int port;
  if (hasport != std::string::npos) {
    port  = strtol(ipstr.substr(hasport + 1).c_str(), nullptr, 10);
    ipstr = ipstr.substr(0, hasport);
  } else if ("hikvision" == vendor) {
    // hikvision default port
    port = 8000;
  } else if ("dahua" == vendor) {
    // dahua default port
    port = 37777;
  } else {
    LOG_ERROR << "unspported vendor: " << vendor;
    return false;
  }

  // Judge IP valid
  LOG_INFO << ipport << " => " << channel_str;
  struct in_addr ip = {};
  if (0 >= inet_pton(AF_INET, ipstr.c_str(), &ip)) {
    LOG_ERROR << "Wrong IP: " << ipstr;
    return false;
  }

  std::replace(ipstr.begin(), ipstr.end(), '.', '_');
  std::string channel_in = std::string(INPUT_CHANNEL_PREFIX) + ipstr;
  {
    std::lock_guard<std::mutex> g(_M_lock3_channelset_input);
    if (_M_channelset_input.find(channel_in) != _M_channelset_input.end()) {
      LOG_ERROR << "Duplicated IP: " << ipport;
      return false;
    }
    _M_channelset_input.insert(channel_in);
  }
  // Judge Output Channel
  {
    std::lock_guard<std::mutex> g(_M_lock4_channelset_output);
    if (_M_channelset_output.find(channel_str) != _M_channelset_output.end()) {
      LOG_ERROR << "Duplicated channel_str: " << channel_str;
      return false;
    }
    _M_channelset_output.insert(channel_str);
  }

  union {
    uint32_t iplong;
    unsigned char ipbytes[8];
  } iptmp      = {};
  iptmp.iplong = ntohl(ip.s_addr);
  int ctx_idx  = AVCodecCtxManager::getInstance()->create_avctx(cudadev);
  if (ctx_idx < 0) {
    LOG_ERROR << "Failed to init avcodec ctx!";
    return false;
  }

  auto cur_has_port = ip_name.find(':');
  if (cur_has_port != std::string::npos) {
    ip_name = ip_name.substr(0, cur_has_port);
  }
  std::replace(ip_name.begin(), ip_name.end(), '.', '_');
  std::string out_ch_name = "/sensor/ipcamera/h264/" + ip_name;
  auto cy_writer =
      _M_debug_node->CreateWriter<::adu::common::sensor::CompressedImage>(
          out_ch_name);

  static const int64_t err_threshold       = 2LL * 1000000LL;   // 单位 us
  static const int64_t err_continuous_time = 10LL * 1000000LL;  // 单位 us

  uint32_t key = iptmp.ipbytes[3];
  key <<= 8;
  key += iptmp.ipbytes[2];
  key <<= 8;
  key += iptmp.ipbytes[1];
  key <<= 8;
  key += iptmp.ipbytes[0];

  if (_M_mode & API_MODE_OFFLINE) {
    // OFFLINE
    return nullptr !=
           _M_node_offline
               ->CreateReader<::adu::common::sensor::CompressedImage>(
                   channel_in,
                   [this, ctx_idx, channel_str, imgmode, key, cb](
                       const std::shared_ptr<
                           const ::adu::common::sensor::CompressedImage> &msg) {
                     if (msg->format() != "h264") {
                       LOG_WARN << "Invalid format: \"" << msg->format()
                                << "\" Ignored.";
                       return;
                     }
                     // Create AVPacket
                     std::shared_ptr<AVPacket> avpkt = nullptr;
                     avpkt.reset(av_packet_alloc(), [](AVPacket *pkt) {
                       av_packet_unref(pkt);
                       av_packet_free(&pkt);
                     });
                     size_t datalen = msg->data().size();
                     auto *data = static_cast<uint8_t *>(av_malloc(datalen));
                     memcpy(data, msg->data().data(), datalen);
                     if (av_packet_from_data(avpkt.get(), data, datalen) < 0) {
                       LOG_ERROR << "Failed to create AVPacket!";
                       av_free(data);
                       return;
                     }
                     avpkt->flags =
                         msg->frame_type() != 0 ? AV_PKT_FLAG_KEY : 0;

                     auto iter = timestamp_cache.find(key);
                     if (timestamp_cache.end() == iter) {
                       timestamp_cache.emplace(
                           key, std::map<int64_t, double>());
                       timestamp_cache[key].emplace(
                           msg->header().camera_timestamp() / 1000,
                           msg->header().timestamp_sec());
                     } else {
                       // 为避免异常情况导致的缓存过大，缓存队列大于100时自动清除
                       if (iter->second.size() > 100) {
                         LOG_ERROR << "cache too much timestamp, need clear!";
                         iter->second.clear();
                       }
                       iter->second.emplace(
                           msg->header().camera_timestamp() / 1000,
                           msg->header().timestamp_sec());
                     }

                     avpkt->pts = msg->header().camera_timestamp() / 1000;
                     //// DECODE
                     std::list<std::shared_ptr<GPUImage>> li_images;
                     AVCodecCtxManager::getInstance()->decode_and_convert(
                         ctx_idx,
                         false,
                         avpkt.get(),
                         imgmode,
                         [msg](const AVFrame *frame) {
                           return frame->pts;
                         },
                         &li_images);

                     if (li_images.empty()) {
                       LOG_ERROR << "<API> \"" << channel_str
                                 << "\" NO decode result!";
                       return;
                     }
                     if (li_images.size() > 1) {
                       LOG_ERROR << "<API> \"" << channel_str
                                 << "\" More than one decode result!";
                     }

                     for (auto &image : li_images) {
                       image->channel_str = channel_str;
                       auto it = timestamp_cache[key].find(image->meatime_us);
                       if (timestamp_cache[key].end() != it) {
                         auto cur_sys_time = it->second * 1000000;
                         timestamp_cache[key].erase(it);
                         if ((abs(cur_sys_time - image->meatime_us) >
                              err_threshold)) {
                           LOG_ERROR << "<API> \"" << channel_str
                                     << "\" Invalid timestamp["
                                     << static_cast<int64_t>(cur_sys_time)
                                     << "," << image->meatime_us << "]!";
                         } else {
                           if (cb) {
                             auto data = std::make_shared<CameraImageData>();
                             __convert_img(image, data);
                             cb(data);
                           }
                         }
                       }
                       break;
                     }
                   });
  } else {
    // ONLINE
    auto last_starttime_us = std::make_shared<int64_t>(0);

    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    auto time_for_show = std::make_shared<int64_t>(0);
    *time_for_show     = ts.tv_sec;

    return IPCameraDriver::getInstance()->newInstanceHandle(
        get_vendor(vendor),
        iptmp.ipbytes,
        [this,
         key,
         ctx_idx,
         channel_str,
         imgmode,
         last_starttime_us,
         cy_writer,
         time_for_show,
         cb,
         vendor](
            int64_t starttime_us,
            int64_t recvtime_us,
            const AVStream *stream,
            const AVPacket *pkt) {
          // 计时器，每分钟输出一次； 标识为 need_print = true
          bool need_print    = false;
          struct timespec ts = {0, 0};
          clock_gettime(CLOCK_MONOTONIC, &ts);
          if (ts.tv_sec - *time_for_show > 60) {
            *time_for_show = ts.tv_sec;
            need_print     = true;
          }
          // 记录接收时间戳
          auto recvtime_blk = airos::base::TimeUtil::GetCurrentTime();
          auto item = IPCameraDriver::getInstance()->_M_map_private[key];
          // double cur_frame_ratio = item->_M_frame_rate;
          std::string cur_ip = item->_M_ip;
          // int64_t reverse_thre =
          //     0.02 * stream->time_base.den / stream->time_base.num;
          bool flag_new_stream = false;
          if (0 == *last_starttime_us) {
            *last_starttime_us = starttime_us;
          } else if (*last_starttime_us != starttime_us) {
            *last_starttime_us = starttime_us;
            flag_new_stream    = true;
          }
          // todo:test replace pts with SEI timestamp
          // int64_t new_starttime_us = 0;
          // DECODE
          std::list<std::shared_ptr<GPUImage>> li_images;
          AVCodecCtxManager::getInstance()->decode_and_convert(
              ctx_idx,
              flag_new_stream,
              pkt,
              imgmode,
              [](const AVFrame *frame) {
                return frame->pkt_dts;
              },
              &li_images);
          struct tm tm;
          time_t log_ts = time(0);
          localtime_r(&log_ts, &tm);
          char buf[128];
          strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
          std::string cur_time(buf);

          for (auto &image : li_images) {
            item->single_camera_sequence++;  // sequence_num
                                             // 进行计数
            image->channel_str = channel_str;
            image->single_camera_hik_sequence =
                item->single_camera_hik_sequence;
            image->single_camera_sequence = item->single_camera_sequence;
            image->sequence_num           = item->single_camera_sequence;
            item->perf.process(
                image,
                item,
                recvtime_us,
                image->meatime_us,
                item->_M_frame_rate);
            // 频率计算
            int frequency = 0;
            if (item->_frequency_counter.trigger(frequency)) {
              LOG_WARN << channel_str << " " << item->_M_ip
                       << " frequency:" << frequency;
            }
            if (image->yuvtime_us == 1) {
              auto msg =
                  std::make_shared<::adu::common::sensor::CompressedImage>();
              msg->Clear();
              auto publish_time = airos::base::TimeUtil::GetCurrentTime();
              msg->mutable_header()->set_timestamp_sec(
                  publish_time);  // 发布通道的时间, 单位:秒
              msg->mutable_header()->set_camera_timestamp(
                  image->meatime_us * 1000);
              msg->mutable_header()->set_sequence_num(
                  item->single_camera_sequence);
              msg->set_measurement_time(publish_time - recvtime_blk);
              msg->set_format("h264");
              msg->set_frame_type(pkt->flags == AV_PKT_FLAG_KEY ? 1 : 0);
              msg->set_data(pkt->data, (unsigned)pkt->size);
              cy_writer->Write(msg);
            }  // else {}

            struct timeval tv = {0, 0};
            gettimeofday(&tv, nullptr);
            int64_t cur_sys_time = tv.tv_sec * 1000 * 1000 + tv.tv_usec;

            static const int64_t err_threshold = 2LL * 1000000LL;  // 单位 us
            static const int64_t err_continuous_time =
                10LL * 1000000LL;  // 单位 us

            //-----------------------------------------------------
            /* 判断 当前时间戳 与 相机绝对时间戳， 如果相差
             * err_threshold， 则 触发 记录*/
            if ((abs(cur_sys_time - image->meatime_us) > err_threshold)) {
              LOG_ERROR << "<API> \"" << channel_str << "\" Invalid timestamp["
                        << cur_sys_time << "," << image->meatime_us << "]!";
              // 判断 是否已经处于 trick 错误状态
              if (item->_M_trickerror) {
                // 判断 trick错误状态已经持续的时间，超过
                // err_continuous_time, 则触发 相机重启,并重置
                // _M_trickerror
                if (abs(cur_sys_time - item->_trickererr_starttime) >
                    err_continuous_time) {
                  item->reset_stream();
                  LOG_WARN << channel_str << " " << item->_M_ip
                           << " trick time error continues :"
                           << err_continuous_time
                           << " us, start time:" << item->_trickererr_starttime
                           << ", current time:" << cur_sys_time
                           << " us, now restart stream ";
                  // 重置 _M_trickerror，避免重复触发
                  item->_M_trickerror = false;
                }  // else {}
              } else {
                // 非 trick错误状态，则置位 _M_trickerror,
                // 并记录 _trickererr_starttime
                item->_M_trickerror         = true;
                item->_trickererr_starttime = cur_sys_time;
              }
            } else {
              item->_M_trickerror = false;
              if (cb) {
                auto data = std::make_shared<CameraImageData>();
                __convert_img(image, data);
                cb(data);
              }
              // else {
              //     _M_outputer->push(channel_str, image);
              // }
            }
            break;
          }
        },
        stream_num,
        channel_num,
        port,
        username,
        passwd);
  }
}

bool DriverAPI::addHandle(
    uint32_t drv_modes, const std::string &ipport,
    const std::string &channel_str, int32_t cudadev, airos::base::Color imgmode,
    const std::string &vendor, int stream_num, int channel_num,
    const std::string &username, const std::string &passwd,
    const CameraImageCallBack cb) {
  if (!_S_instance) {
    std::lock_guard<std::mutex> g(_S_lock_instance);
    if (!_S_instance) {
      _S_instance = new DriverAPI(drv_modes);
    }
  }
  if (!_S_instance->_M_initflag) {
    return false;
  }
  return _S_instance->__add_handle(
      ipport,
      channel_str,
      cudadev,
      imgmode,
      vendor,
      stream_num,
      channel_num,
      username,
      passwd,
      cb);
}

}  // END namespace device
}  // END namespace base
}  // namespace airos
