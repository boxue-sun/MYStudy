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

#include <cuda_runtime_api.h>
#include <sys/time.h>

#include <fstream>

#include "middleware/device_service/modules/camera/ipcamera/proto/sensor_image.pb.h"

#include "base/common/log.h"
#include "gflags/gflags.h"
#include "middleware/device_service/modules/camera/ipcamera/include/api-ipcamera.h"
#include "middleware/device_service/modules/camera/ipcamera/include/apipriv-custom-avlog.h"
#include "middleware/runtime/src/air_middleware_node.h"

// #include <opencv2/core.hpp>
// #include <opencv/cv.hpp>
using airos::base::device::api_get_image;
using airos::base::device::api_init_instance;
using airos::base::device::apipriv_custom_avlog;

static double GetCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  const double timestamp = tv.tv_sec * 1000000 + tv.tv_usec;
  return timestamp / 1000000;
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_log_dir       = "./";
  std::string binname = basename(argv[0]);
  bool offline        = false;
  if (binname == "demo_online") {
    offline = false;
  } else if (binname == "demo_offline") {
    offline = true;
  } else {
    std::cerr << "Unknown bin-name: " << binname << std::endl;
    return 1;
  }
  std::cout << "Offline Mode: " << (offline ? "ON" : "OFF") << std::endl;
  if (argc % 4 != 1) {
    std::cerr << argv[0] << " ip channel_str stream_num channel_num ...\n";
    /*
    在线demo执行: ./demo_online 192.168.1.64  /sensor/ipcamera/h264/192.168.1.64
    0 1 string_num选择 主or次 码流，fish_camera时 0:1280x1280 1:720x480
    */
    return 0;
  }

  std::list<std::string> li_args;
  for (int i = 1; i < argc; i++) {
    li_args.emplace_back(argv[i]);
  }
  std::map<std::string, std::string> __map({});
  std::map<std::string, int> stream_num_map({});
  std::map<std::string, int> channel_num_map({});
  while (li_args.size() > 3) {
    std::string ipstr = li_args.front();
    li_args.pop_front();
    std::string channel_str = li_args.front();
    li_args.pop_front();
    __map[ipstr] = channel_str;

    int stream_num = atoi(li_args.front().c_str());
    li_args.pop_front();
    stream_num_map[ipstr] = stream_num;

    int channel_num = atoi(li_args.front().c_str());
    li_args.pop_front();
    channel_num_map[ipstr] = channel_num;
  }
  if (!li_args.empty()) {
    std::cerr << "Invalid args!" << std::endl;
    return 1;
  }
  if (__map.empty()) {
    std::cerr << "Need args!" << std::endl;
    return 1;
  }
  av_log_set_callback(apipriv_custom_avlog);
  airos::middleware::AirRuntimeInit("apidemos");
  auto __node = std::make_shared<airos::middleware::AirMiddlewareNode>(
      "IPCameraDemo_H264Decompress" + __map.begin()->second);
  for (const auto &item : __map) {
    auto ipport      = item.first;
    auto channel_str = item.second;
    std::cout << ipport << " => " << channel_str << std::endl;
    // api_init_instance(ipport, channel_str, 0, GPU_IMAGE_MODE_BGR24, offline,
    // 50000, 20, "hikvision",
    // stream_num_map[item.first],channel_num_map[item.first], 3, "admin",
    // "baidu123");

    api_init_instance(
        offline ? API_MODE_OFFLINE : 0,
        ipport,
        channel_str,
        0,
        airos::base::Color::RGB,
        "hikvision",
        stream_num_map[item.first],
        channel_num_map[item.first],
        "admin",
        "baidu123",
        nullptr);

    auto writer = __node->CreateWriter<adu::common::sensor::Image>("/test");
    if (!writer) {
      LOG_FATAL << "Failed to create writter!";
      exit(1);
    }
    std::shared_ptr<int> max_counter = std::make_shared<int>(INT32_MAX);
    std::string bad_channel_prefix   = "/bad/";
    if (channel_str.substr(0, bad_channel_prefix.size()) ==
        bad_channel_prefix) {
      *max_counter = 1;
    }
    void *cpubuf = nullptr;
    cudaMallocHost(&cpubuf, 3LL * 4096 * 4096);

    std::thread([writer, channel_str, max_counter, cpubuf]() {
      int counter = 0;
      while (1) {
        if (counter >= *max_counter) {
          return;
        }
        usleep(50 * 1000);
        continue;
        counter++;
        auto img = api_get_image(channel_str);
        if (nullptr == img) {
          std::cout << channel_str << " img is nullptr " << std::endl;
          continue;
        }
        auto image_msg = std::make_shared<::adu::common::sensor::Image>();
        if (!image_msg) {
          LOG_ERROR << "Failed to create image message!";
          continue;
        }
        std::cout << channel_str << ": " << img->sequence_num << ": "
                  << img->width << "x" << img->height << std::endl;
        image_msg->Clear();
        image_msg->set_encoding("rgb8");
        image_msg->mutable_header()->set_sequence_num(img->sequence_num);
        image_msg->set_width(img->width);
        image_msg->set_height(img->height);
        image_msg->set_step(img->width * 3);
        image_msg->set_measurement_time(img->meatime_us / 1000000.0);
        image_msg->mutable_header()->set_camera_timestamp(
            img->meatime_us * 1000);
        if (img->gpu_ptr) {
          size_t bufsize = 3LL * img->width * img->height;
          cudaMemcpy(
              cpubuf, img->gpu_ptr.get(), bufsize, cudaMemcpyDeviceToHost);
          image_msg->set_data(cpubuf, bufsize);

          // of.write((unsigned char*)cpubuf, bufsize);
        }
        image_msg->mutable_header()->set_timestamp_sec(GetCurrentTime());
        writer->Write(image_msg);
      }
      // of.close();
    }).detach();
  }

  airos::middleware::WaitForShutdown();
  return 0;
}
