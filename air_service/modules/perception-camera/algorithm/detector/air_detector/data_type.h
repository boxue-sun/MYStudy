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

class GpuImg {
 public:
  GpuImg() {}
  GpuImg(
      const unsigned char* data, int channel, int src_height, int src_width,
      int step)
      : _data(data)
      , _channel(channel)
      , _src_height(src_height)
      , _src_width(src_width)
      , _step(step) {
    _roi_left_x = 0;
    _roi_left_y = 0;
    _height     = _src_height;
    _width      = _src_width;
    _valid      = valid();
  }

  GpuImg(const GpuImg& img)
      : _data(img._data)
      , _channel(img._channel)
      , _src_height(img._src_height)
      , _src_width(img._src_width)
      , _step(img._step) {
    _roi_left_x = 0;
    _roi_left_y = 0;
    _height     = _src_height;
    _width      = _src_width;
    _valid      = valid();
  }
  GpuImg(const GpuImg& img, int roi_x, int roi_y, int roi_w, int roi_h)
      : _data(img._data)
      , _channel(img._channel)
      , _src_height(img._src_height)
      , _src_width(img._src_width)
      , _step(img._step) {
    _roi_left_x = roi_x;
    _roi_left_y = roi_y;
    _height     = roi_h;
    _width      = roi_w;
    _valid      = valid();
  }
  bool Valid() const {
    return _valid;
  }
  const unsigned char* Data() const {
    return _data + _roi_left_y * _step + _roi_left_x * _channel;
  }
  int SrcHeight() const {
    return _src_height;
  }
  int SrcWidth() const {
    return _src_width;
  }
  int Step() const {
    return _step;
  }
  int Height() const {
    return _height;
  }
  int Width() const {
    return _width;
  }
  int Channel() const {
    return _channel;
  }

 private:
  bool valid() const {
    return (
        _data != nullptr && _roi_left_x >= 0 &&
        _roi_left_x + _width <= _src_width && _roi_left_y >= 0 &&
        _roi_left_y + _height <= _src_height && _step >= _src_width &&
        _channel != 0);
  }
  bool _valid                = false;
  const unsigned char* _data = nullptr;
  int _channel               = 0;
  int _src_height            = 0;
  int _src_width             = 0;
  int _step                  = 0;
  int _roi_left_x            = 0;
  int _roi_left_y            = 0;
  int _height                = 0;
  int _width                 = 0;
};
