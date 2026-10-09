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

#include "util.h"

#include <cstdlib>

#include "base/common/log.h"

namespace airos {
namespace perception {
namespace algorithm {

bool LoadAnchors(const std::string &path, std::vector<float> *anchors) {
  int num_anchors = 0;
  std::ifstream ifs(path, std::ifstream::in);
  ifs >> num_anchors;
  if (!ifs.good()) {
    LOG_ERROR << "Failed to get number of anchors!";
    return false;
  }
  (*anchors).resize(num_anchors * 2);
  for (int i = 0; i < num_anchors; ++i) {
    ifs >> (*anchors)[i * 2] >> (*anchors)[i * 2 + 1];
    if (!ifs.good()) {
      LOG_ERROR << "Failed to load the " << i << "-th anchor!";
      return false;
    }
  }
  ifs.close();
  return true;
}

bool LoadTypes(
    const std::string &path, std::vector<track::ObjectSubType> *types) {
  std::ifstream ifs(path, std::ifstream::in);
  if (!ifs.good()) {
    LOG_ERROR << "Type_list not found: " << path;
    return false;
  }
  std::string type;
  LOG_INFO << "Supported types: ";
  while (ifs >> type) {
    if (track::kName2SubTypeMap.find(type) == track::kName2SubTypeMap.end()) {
      LOG_ERROR << "Invalid type: " << type;
      return false;
    }
    (*types).push_back(track::kName2SubTypeMap.at(type));
    LOG_INFO << "\t\t" << type;
  }
  LOG_INFO << "\t\t" << (*types).size() << " in total.";
  ifs.close();
  return true;
}
bool LoadExpand(const std::string &path, std::vector<float> *expands) {
  std::ifstream ifs(path, std::ifstream::in);
  if (!ifs.good()) {
    LOG_ERROR << "expand_list not found: " << path;
    return false;
  }
  float expand;
  LOG_INFO << "Expand nums: ";
  while (ifs >> expand) {
    expands->push_back(expand);
    LOG_INFO << "\t\t" << expand;
  }
  ifs.close();
  return true;
}
bool LoadConfidenceMap(
    const std::string &path, std::vector<float> *confidence) {
  std::ifstream ifs(path, std::ifstream::in);
  if (!ifs.good()) {
    LOG_ERROR << "confidence map not found: " << path;
    return false;
  }
  confidence->clear();
  float conf;
  while (ifs >> conf) {
    confidence->push_back(conf);
  }
  ifs.close();
  return true;
}
// bool ResizeCPU(const airos::perception::Blob<uint8_t> &src_blob,
//                std::shared_ptr<airos::perception::Blob<float>> dst_blob,
//                int stepwidth,
//                int start_axis) {
//   int width = dst_blob->shape(2);
//   int height = dst_blob->shape(1);
//   int channel = dst_blob->shape(3);
//   int origin_channel = src_blob.shape(3);
//   int origin_height = src_blob.shape(1);
//   int origin_width = src_blob.shape(2);
//   if (origin_channel != dst_blob->shape(3)) {
//     LOG_ERROR << "channel should be the same after resize.";
//     return false;
//   }
//   float fx = static_cast<float>(origin_width) / static_cast<float>(width);
//   float fy = static_cast<float>(origin_height) / static_cast<float>(height);
//   auto src = src_blob.cpu_data();
//   auto dst = dst_blob->mutable_cpu_data();
//   for (int dst_y = 0; dst_y < height; dst_y++) {
//     for (int dst_x = 0; dst_x < width; dst_x++) {
//       float src_x = (dst_x + 0.5) * fx - 0.5;
//       float src_y = (dst_y + 0.5) * fy - 0.5;
//       const int x1 = round(src_x);
//       const int y1 = round(src_y);
//       const int x1_read = std::max(x1, 0);
//       const int y1_read = std::max(y1, 0);
//       const int x2 = x1 + 1;
//       const int y2 = y1 + 1;
//       const int x2_read = std::min(x2, width - 1);
//       const int y2_read = std::min(y2, height - 1);
//       int src_reg = 0;
//       for (int c = 0; c < channel; c++) {
//         float out = 0;

//         int idx11 = (y1_read * stepwidth + x1_read) * channel;
//         src_reg = src[idx11 + c];
//         out = out + (x2 - src_x) * (y2 - src_y) * src_reg;
//         int idx12 = (y1_read * stepwidth + x2_read) * channel;
//         src_reg = src[idx12 + c];
//         out = out + src_reg * (src_x - x1) * (y2 - src_y);

//         int idx21 = (y2_read * stepwidth + x1_read) * channel;
//         src_reg = src[idx21 + c];
//         out = out + src_reg * (x2 - src_x) * (src_y - y1);

//         int idx22 = (y2_read * stepwidth + x2_read) * channel;
//         src_reg = src[idx22 + c];
//         out = out + src_reg * (src_x - x1) * (src_y - y1);
//         if (out < 0) {
//           out = 0;
//         }
//         if (out > 255) {
//           out = 255;
//         }
//         int dst_idx = (dst_y * width + dst_x) * channel + c;
//         //   printf("%f %d %d %d %d\n",out,x1,y1,x2,y2);
//         dst[dst_idx] = out;
//       }
//     }
//   }
//   return true;
// }

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
