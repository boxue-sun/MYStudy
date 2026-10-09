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

#include "air_object_detecter.h"

#include <math.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include "base/blob/cuda_util.h"
#include "latency.h"

namespace airos {
namespace perception {
namespace algorithm {

using airos::base::CudaUtil;
using Bbox = Box2f;

bool AirObjectDetecter::Init(const DetectInitParam &op) {
  _model_options.model_dir         = op.model_dir;
  _model_options.gpu_id            = op.gpu_id;
  _model_options.mask_img_filename = op.mask_img_filename;

  _proc.reset(new ModelProcess(ModelProcessOption(op.model_dir)));
  _proc->SetGpuID(op.gpu_id);
  bool init = _proc->Init();
  if (!init) {
    LOG_ERROR << "model init fail";
    return init;
  }
  ModelParam paras = _proc->Params();
  LOG_INFO << "-----" << paras.Descript();
  _threshold = paras.ConfidenceThreshold();

  _nms_thresh = paras.NmsThreshold();
  LOG_INFO << "NmsThreshold " << _nms_thresh;

  _debug_level = paras.DebugLevel();

  _height_filter_ratio = paras.HeightFilterRadio();

  _width_filter_ratio = paras.WidthFilterRadio();

  _minY_filter_ratio = paras.MinYFilterRadio();

  _height_filter_ratio_cyc = paras.HeightFilterRadioCYC();

  _width_filter_ratio_cyc = paras.WidthFilterRadioCYC();

  if (paras.MaxGpuMallocSize() > 0) {
    _max_img_size = paras.MaxGpuMallocSize();
  }
  LOG_INFO << "input img max size " << _max_img_size;

  int data_len = (paras.Channel() + 1) * paras.Size().area();
  LOG_INFO << "gpu len (has extra channel)" << data_len;
  CudaUtil::set_device_id(_proc->UseGpuID());
  _gpu_pre_process_data =
      static_cast<float *>(CudaUtil::malloc(data_len * sizeof(float)));
  LOG_INFO << "gpuID " << _proc->UseGpuID() << " _gpu_pre_process_data "
           << reinterpret_cast<int64_t>(_gpu_pre_process_data)
           << ", malloc size " << data_len;
  return true;
}

int AirObjectDetecter::Process(
    const unsigned char *gpu_data, int channel, int height, int width,
    std::vector<ObjectDetectInfoPtr> &output) {
  return process(
      GpuImg(gpu_data, channel, height, width, channel * width), output);
}

int AirObjectDetecter::process(
    const GpuImg &gpu_img, std::vector<ObjectDetectInfoPtr> &output) {
  Latency lat;
  ModelParam para = _proc->Params();
  if (!gpu_img.Valid()) {
    LOG_ERROR << "input data valid, gpu_img data "
              << reinterpret_cast<int64_t>(gpu_img.Data())
              << ", gpu_img channel " << gpu_img.Channel() << ", src_width "
              << gpu_img.SrcWidth() << ", src_height " << gpu_img.SrcHeight()
              << ", Step " << gpu_img.Step() << ", width " << gpu_img.Width()
              << ", height " << gpu_img.Height();
    return 2;
  }

  std::vector<int> data_shape = {
      1, para.Channel(), para.Size().height, para.Size().width};

  int height_ori = gpu_img.Height();
  int width_ori  = gpu_img.Width();
  bool ok        = pre_process(gpu_img, data_shape, 0);

  if (!ok) {
    LOG_INFO << "Set Input 0 error";
    return 3;
  }

  if (_debug_level == 1) {
    LOG_INFO << "pre_process  " << lat.duration();
  }
  lat.Start();

  std::vector<int> im_size(2, 0);
  im_size[0] = para.Size().height;
  im_size[1] = para.Size().width;
  ok         = _proc->SetInputData<int>(1, im_size.data(), im_size.size());
  if (!ok) {
    LOG_INFO << "Set Input error";
    return 4;
  }
  if (_debug_level == 1) {
    LOG_INFO << "process_set_data  " << lat.duration();
  }
  lat.Start();
  ok = _proc->Process();
  if (!ok) {
    LOG_ERROR << "Process fail";
    return 3;
  }
  if (_debug_level == 1) {
    LOG_INFO << "infer " << lat.duration();
  }

  int status = 0;
  lat.Start();
  status = post_process(output, height_ori, width_ori);
  if (status != 0) {
    LOG_ERROR << "Post fail";
    return 4;
  }
  if (_debug_level == 1) {
    LOG_INFO << "post_process " << lat.duration();
  }

  return 0;
}

int AirObjectDetecter::pre_process_gpu(
    const uint8_t *gpu_img, int channel, int width, int height, int step) {
  ModelParam paras    = _proc->Params();
  const float mean_b  = 0.406 * 255;
  const float mean_g  = 0.456 * 255;
  const float mean_r  = 0.485 * 255;
  const float scale_b = 1.0 / 255.0 / 0.225;
  const float scale_g = 1.0 / 255.0 / 0.224;
  const float scale_r = 1.0 / 255.0 / 0.229;
  LOG_INFO << "mean bgr " << mean_b << " " << mean_g << " " << mean_r;
  LOG_INFO << "scale  bgr " << scale_b << " " << scale_g << " " << scale_r;
  int len = GPUResizeReshape(
      gpu_img, channel, height, width, step, _gpu_pre_process_data,
      paras.Size(), paras.Size().width, cv::INTER_NEAREST, mean_b, mean_g,
      mean_r, scale_b, scale_g, scale_r);
  Latency lat;
  CudaUtil::CopyVecDeviceToDevice(
      _gpu_pre_process_data + paras.Size().area() * 3, paras.Size().area(),
      _gpu_pre_process_data, paras.Size().area());
  CudaUtil::CopyVecDeviceToDevice(
      _gpu_pre_process_data, paras.Size().area(),
      _gpu_pre_process_data + paras.Size().area() * 2, paras.Size().area());
  CudaUtil::CopyVecDeviceToDevice(
      _gpu_pre_process_data + paras.Size().area() * 2, paras.Size().area(),
      _gpu_pre_process_data + paras.Size().area() * 3, paras.Size().area());
  if (_debug_level == 1) {
    LOG_INFO << "swap color B and R, runtime " << lat.duration();
  }
  return len;
}

bool AirObjectDetecter::pre_process(
    const GpuImg &img, const std::vector<int> &shape, int input_level) {
  ModelParam paras = _proc->Params();
  int len          = pre_process_gpu(
      img.Data(), img.Channel(), img.Width(), img.Height(), img.Step());
  UNUSED_VARIABLE(len);
  return _proc->SetInputGpuData<float>(
      input_level, _gpu_pre_process_data,
      paras.Channel() * paras.Size().area());
}
int AirObjectDetecter::post_process(
    std::vector<ObjectDetectInfoPtr> &output_data, int height, int width) {
  Latency lat;
  ModelResult<float> out_box       = _proc->GetOutputData<float>(0);
  std::vector<float> &out_box_data = out_box._data;

  ModelResult<float> out_length       = _proc->GetOutputData<float>(1);
  std::vector<float> &out_length_data = out_length._data;

  // widths
  ModelResult<float> out_width       = _proc->GetOutputData<float>(2);
  std::vector<float> &out_width_data = out_width._data;

  // heights
  ModelResult<float> out_height       = _proc->GetOutputData<float>(3);
  std::vector<float> &out_height_data = out_height._data;

  // angle_bin
  ModelResult<int> out_angle_bin       = _proc->GetOutputData<int>(4);
  std::vector<int> &out_angle_bin_data = out_angle_bin._data;

  ModelResult<float> out_sin       = _proc->GetOutputData<float>(5);
  std::vector<float> &out_sin_data = out_sin._data;

  ModelResult<float> out_cos       = _proc->GetOutputData<float>(6);
  std::vector<float> &out_cos_data = out_cos._data;

  // occluded
  ModelResult<int> out_occluded       = _proc->GetOutputData<int>(7);
  std::vector<int> &out_occluded_data = out_occluded._data;

  // truncated
  ModelResult<int> out_truncated       = _proc->GetOutputData<int>(8);
  std::vector<int> &out_truncated_data = out_truncated._data;

  // bottom_center
  ModelResult<float> out_box3d_bottom_uv       = _proc->GetOutputData<float>(9);
  std::vector<float> &out_box3d_bottom_uv_data = out_box3d_bottom_uv._data;

  if (_debug_level == 1) {
    LOG_INFO << "process get_data " << lat.duration();
  }

  std::vector<ObjectDetectInfoPtr> frame_all_objs;
  // std::vector<float> ratio_tmp1_case1;
  // std::vector<float> ratio_tmp1_case2;
  // std::vector<float> ratio_tmp2;
  // process
  for (size_t i = 0; i < out_length_data.size(); ++i) {
    int label = static_cast<int>(out_box_data[i * 21 + 0]);
    if (out_box_data[i * 21 + 1] < _threshold[label] || label == 14 ||
        label == 13) {
      continue;  // output contains 2d box and the confidence of each class
    }

    auto joint_out_obj_ptr =
        std::make_shared<airos::perception::algorithm::ObjectDetectInfo>();

    // length, width, height
    joint_out_obj_ptr->size.length = out_length_data[i];
    joint_out_obj_ptr->size.width  = out_width_data[i];
    joint_out_obj_ptr->size.height = out_height_data[i];

    // angle
    int cur_bin              = out_angle_bin_data[i];
    float angle              = bin_center_[cur_bin];
    float tmp_cos            = out_cos_data[i * 3 + cur_bin];
    float tmp_sin            = out_sin_data[i * 3 + cur_bin];
    float diff_alpha         = atan2(tmp_sin, tmp_cos + 1e-8);
    float alpha              = angle + diff_alpha * 0.5;
    joint_out_obj_ptr->alpha = alpha;

    Bbox &box    = joint_out_obj_ptr->box;
    float xScale = static_cast<float>(width) / _proc->Params().Size().width;
    float yScale = static_cast<float>(height) / _proc->Params().Size().height;

    box.left_top.x     = out_box_data[i * 21 + 2] * xScale;
    box.left_top.y     = out_box_data[i * 21 + 3] * yScale;
    box.right_bottom.x = out_box_data[i * 21 + 4] * xScale;
    box.right_bottom.y = out_box_data[i * 21 + 5] * yScale;

    // // ObjectSubType
    joint_out_obj_ptr->type_id = static_cast<int>(out_box_data[i * 21]);
    joint_out_obj_ptr->type    = DetectObjectType(joint_out_obj_ptr->type_id);

    joint_out_obj_ptr->type_id_confidence = out_box_data[i * 21 + 1];
    joint_out_obj_ptr->sub_type_probs.assign(
        out_box_data.begin() + i * 21 + 6, out_box_data.begin() + i * 21 + 21);

    // bottom_uv
    joint_out_obj_ptr->bottom_uv.x = out_box3d_bottom_uv_data[i * 2];
    joint_out_obj_ptr->bottom_uv.y = out_box3d_bottom_uv_data[i * 2 + 1];

    float bbox_center_u = (joint_out_obj_ptr->box.left_top.x +
                           joint_out_obj_ptr->box.right_bottom.x) /
                          2.0;
    float bbox_center_v = (joint_out_obj_ptr->box.left_top.y +
                           joint_out_obj_ptr->box.right_bottom.y) /
                          2.0;
    joint_out_obj_ptr->bottom_uv.x =
        joint_out_obj_ptr->bottom_uv.x + bbox_center_u;
    joint_out_obj_ptr->bottom_uv.y =
        joint_out_obj_ptr->bottom_uv.y + bbox_center_v;

    // occluded
    if (out_occluded_data[i] == 2) {
      joint_out_obj_ptr->is_occluded =
          airos::perception::algorithm::TriStatus::TRUE;
    } else if (out_occluded_data[i] == 1) {
      joint_out_obj_ptr->is_occluded =
          airos::perception::algorithm::TriStatus::FALSE;
    } else {
      joint_out_obj_ptr->is_occluded =
          airos::perception::algorithm::TriStatus::UNKNOWN;
    }
    // truncated
    if (out_truncated_data[i] == 2) {
      joint_out_obj_ptr->is_truncated =
          airos::perception::algorithm::TriStatus::TRUE;
    } else if (out_truncated_data[i] == 1) {
      joint_out_obj_ptr->is_truncated =
          airos::perception::algorithm::TriStatus::FALSE;
    } else {
      joint_out_obj_ptr->is_truncated =
          airos::perception::algorithm::TriStatus::UNKNOWN;
    }

    frame_all_objs.push_back(joint_out_obj_ptr);
  }
  for (auto obj_ptr : frame_all_objs) {
    // all
    if (obj_ptr->box.left_top.y / height < _minY_filter_ratio) {
      continue;
    }
    float w = obj_ptr->box.right_bottom.x - obj_ptr->box.left_top.x + 1;
    float h = obj_ptr->box.right_bottom.y - obj_ptr->box.left_top.y + 1;
    if ((h / height < _height_filter_ratio) &&
        (w / width < _width_filter_ratio)) {
      continue;
    }

    if (obj_ptr->type_id >= 6 && obj_ptr->type_id <= 8) {
      if (h / height < _height_filter_ratio_cyc &&
          w / width < _width_filter_ratio_cyc) {
        continue;
      }
    }
    output_data.push_back(obj_ptr);
  }
  inter_class_nms(output_data, _nms_thresh);

  return 0;
}

void Qsort(
    const std::vector<ObjectDetectInfoPtr> &d, std::vector<int> &a, int low,
    int high) {
  if (low >= high) {
    return;
  }

  int first = low;
  int last  = high;

  int key_idx = a[first];

  while (first < last) {
    while (first < last &&
           d[a[last]]->type_id_confidence >= d[key_idx]->type_id_confidence) {
      --last;
    }

    a[first] = a[last];

    while (first < last &&
           d[a[first]]->type_id_confidence <= d[key_idx]->type_id_confidence) {
      ++first;
    }

    a[last] = a[first];
  }

  a[first] = key_idx;
  Qsort(d, a, low, first - 1);
  Qsort(d, a, first + 1, high);
}
void AirObjectDetecter::inter_class_nms(
    std::vector<ObjectDetectInfoPtr> &detected_objects, float nms_thresh) {
  size_t size = detected_objects.size();
  if (size == 0) {
    return;
  }

  // generate idx:
  std::vector<int> idxs;
  idxs.reserve(size);

  std::vector<bool> idx_status;
  idx_status.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    idxs.push_back(static_cast<int>(i));
    idx_status.push_back(false);
  }

  // get areas:
  std::vector<double> areas;
  areas.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    Bbox &bbox = detected_objects[i]->box;
    int bbox_w = static_cast<int>(bbox.right_bottom.x - bbox.left_top.x);
    int bbox_h = static_cast<int>(bbox.right_bottom.y - bbox.left_top.y);
    double tmp = (bbox_w + 1) * (bbox_h + 1);
    areas.push_back(tmp);
  }

  // sort idxs by scores in ascending order ==>quick sort:
  Qsort(detected_objects, idxs, 0, size - 1);

  // get delete detections:
  std::vector<int> delIdxs;

  while (true) {  // get compare idx;
    int i = -1;

    for (int j = size - 1; j > 0; --j) {
      if (idx_status[j] == false) {
        i             = j;
        idx_status[i] = true;
        break;
      }
    }

    if (i == -1) {
      break;  // end circle
    }

    int idx_i = idxs[i];

    int x1 = static_cast<int>(detected_objects[idx_i]->box.left_top.x);
    int y1 = static_cast<int>(detected_objects[idx_i]->box.left_top.y);
    int x2 = static_cast<int>(detected_objects[idx_i]->box.right_bottom.x);
    int y2 = static_cast<int>(detected_objects[idx_i]->box.right_bottom.y);

    for (int j = 0; j < i; j++) {
      if (idx_status[j] == true) {
        continue;
      }

      int idx_j = idxs[j];

      int x1_j = static_cast<int>(detected_objects[idx_j]->box.left_top.x);
      int y1_j = static_cast<int>(detected_objects[idx_j]->box.left_top.y);
      int x2_j = static_cast<int>(detected_objects[idx_j]->box.right_bottom.x);
      int y2_j = static_cast<int>(detected_objects[idx_j]->box.right_bottom.y);

      int xx1 = static_cast<int>(x1_j > x1 ? x1_j : x1);
      int yy1 = static_cast<int>(y1_j > y1 ? y1_j : y1);
      int xx2 = (x2_j < x2 ? x2_j : x2);
      int yy2 = (y2_j < y2 ? y2_j : y2);
      // standard area = w*h;
      int w = xx2 - xx1 + 1;
      w     = (w > 0 ? w : 0);
      int h = yy2 - yy1 + 1;
      h     = (h > 0 ? h : 0);
      // get delIdx;
      double inter_area  = static_cast<double>(w * h);
      double union_area  = areas[idx_i] + areas[idx_j] - inter_area;
      double tmp_overlap = inter_area / (union_area + 1e-8);

      if (tmp_overlap > nms_thresh) {
        delIdxs.push_back(idxs[j]);
        idx_status[j] = true;
      }
    }
  }

  // delete from detections:
  if (delIdxs.size() == 0) return;
  sort(delIdxs.rbegin(), delIdxs.rend());
  for (size_t i = 0; i < delIdxs.size(); ++i) {
    std::vector<ObjectDetectInfoPtr>::iterator it =
        detected_objects.begin() + delIdxs[i];
    detected_objects.erase(it);
  }
}

}  // namespace algorithm
}  // namespace perception
}  // namespace airos
