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

#include "air_service/modules/perception-visualization/visualization/viewports/dummy_viewport.h"

#include <locale>
#include <memory>
#include <string>
#include <vector>

#include "air_service/modules/perception-visualization/visualization/common/display_options_manager.h"
#include "air_service/modules/perception-visualization/visualization/common/gl_raster_text.h"
#include "air_service/modules/perception-visualization/visualization/viewports/viewport_utils.h"
#include "base/common/singleton.h"

namespace airos {
namespace perception {
namespace visualization {

void DummyViewport::draw() {
  draw_text();
}

void DummyViewport::draw_text() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, get_width(), get_height(), 0, 0.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glClear(GL_DEPTH_BUFFER_BIT);

  // GLRasterTextSingleton* raster_text =
  //     lib::Singleton<GLRasterTextSingleton>::get_instance();
  GLRasterText *raster_text =
      airos::base::Singleton<GLRasterText>::get_instance();

  // draw 'title'
  std::string str_title = "DUMMY";
  int x_title           = 50;
  int y_title           = 50;
  raster_text->DrawString(str_title.c_str(), x_title, y_title, 255, 255, 255);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

REGISTER_GLVIEWPORT(DummyViewport);

}  // namespace visualization
}  // namespace perception
}  // namespace airos
