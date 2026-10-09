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

#include "base/cv_image/image.h"

namespace airos {
namespace base {

// class ImageProcessor {
//  public:
//   ~ImageProcessor() = default;
//   ImageProcessor(const ImageProcessor&) = delete;
//   ImageProcessor& operator=(const ImageProcessor&) = delete;
//   // bool Init();
//   static bool resize(const Image8U& src_img, Image8U& dst_img, int height,
//                      int width);
//   static bool resize(const Image8U& src_img, base::Blob<float> > dst_img,
//                      int height, int width);
//   // bool resize_mean(const Image8U& input, Image8U& output, int height, int
//   // width);//TODO
//   static bool undistortion_handle(const Image8U& src_img, Image8U& dst_img);
//   static bool convert_color_space(const Image8U& src_img, Image8U& dst_img,
//                                   Color color_type);

//  private:
//   ImageProcessor() = default;
// }

}  // namespace base
}  // namespace airos
