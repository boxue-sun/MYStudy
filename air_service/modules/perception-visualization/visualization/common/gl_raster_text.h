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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// #include "lib/singleton/singleton.h"

namespace airos {
namespace perception {
namespace visualization {

class GLRasterText {
 public:
  GLRasterText() {
    Init();
  }
  ~GLRasterText() {}

  void Init();

  void DrawString(const char* s, int x, int y, int r, int g, int b);

  // Deprecated
  void PrintString(const char* s);

 private:
  void MakeRasterFont();
  static GLubyte s_asccii_bitmaps_[][13];
  static GLuint s_font_offset_;
};

// class GLRasterTextSingleton {
//  public:
//   void DrawString(const char* s, int x, int y, int r, int g, int b) {
//     raster_text_.DrawString(s, x, y, r, g, b);
//   }

//  private:
//   GLRasterTextSingleton() {
//     raster_text_.Init();
//   }

//   ~GLRasterTextSingleton() {}

//   friend class ::idg::perception::lib::Singleton<GLRasterTextSingleton>;

//   GLRasterText raster_text_;
// };

}  // namespace visualization
}  // namespace perception
}  // namespace airos
