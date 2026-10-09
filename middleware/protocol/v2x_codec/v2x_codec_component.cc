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

#include "v2x_codec_component.h"

#include <sys/time.h>

#include <gflags/gflags.h>

#include "base/common/log.h"
#include "base/work_param/configer_om_work_param.h"
#include "base/common/auth/Authenticator.h"

namespace os {
namespace v2x {
namespace protocol {

DEFINE_string(asn_version, "new_4layer", "v2x message Asn.1 file version");

bool CODEC_COMPONENT::Init() {

#if ENABLE_ENCRYPTION
  auto omWorkParamConfiger = airos::base::workparam::WorkParam::getWorkParamFromFile();
  std::string license = omWorkParamConfiger.mecDeviceWorkParam.license;
  V2X_CODEC_LOG_INFO << "License is: " << license << std::endl;
  int result = FusionService::Authenticator::GetInstance().Authorize(license);
  if (result != 0)
  {
    V2X_CODEC_LOG_ERROR << "License generated fail, error code:  " << result;
    exit(1);
  }
#endif

  std::string asn_version(FLAGS_asn_version);
  if (asn_version.compare("4layer") == 0) {
    asn_type_ = EnAsnType::CASE_53_2020;
  } else if (asn_version.compare("new_4layer") == 0) {
    asn_type_ = EnAsnType::YDT_3709_2020;
  } else if (asn_version.compare("new_4layer_ext") == 0) {
    asn_type_ = EnAsnType::YDT_3709_2020_EXT;
  } else {
    // pass
  }
  // parser
  auto reader = node_->CreateReader<os::v2x::device::RSUData>(
      "/airos/device/rsu_out",
      std::bind(&CODEC_COMPONENT::RSUMessageProc, this, std::placeholders::_1));
  if (reader == nullptr) {
    return false;
  }
  return true;
}

bool CODEC_COMPONENT::RsuPb2MessageFrame(
    const std::shared_ptr<const os::v2x::device::RSUData>& rsu_pb_data,
    std::shared_ptr<v2xpb::asn::MessageFrame>& frame_pb) {
  if (!rsu_pb_data || !frame_pb || !rsu_pb_data->has_data()) {
    V2X_CODEC_LOG_INFO << "input data error";
    return false;
  }

  if (rsu_pb_data->has_type()) {
    V2X_CODEC_LOG_WARN << "recv rsu data type: " << RSUDataType_Name(rsu_pb_data->type());
  }

  // if (rsu_pb_data->has_version()) {
  //   switch (rsu_pb_data->version()) {
  //     case os::v2x::device::CSAE_53_2020:
  //       asn_type_ = EnAsnType::CASE_53_2020;
  //       break;
  //     case os::v2x::device::YDT_3709_2020:
  //       asn_type_ = EnAsnType::YDT_3709_2020;
  //       break;
  //     default:
  //       V2X_CODEC_LOG_WARN << "rsu data version:false";
  //       return false;
  //   }
  // } else {
  //   V2X_CODEC_LOG_WARN << "rsu data version null";
  // }

  std::string str_asn(
      rsu_pb_data->data().c_str(), rsu_pb_data->data().length());
  std::string str_pb("");
  if (0 > message_frame_uper2pbstr_adapter(str_asn, &str_pb, asn_type_)) {
    V2X_CODEC_LOG_WARN << "asn to pb false";
    return false;
  }
  V2X_CODEC_LOG_INFO << "asn to pb:true";

  if (!frame_pb->ParsePartialFromString(str_pb)) {
    V2X_CODEC_LOG_WARN << "pb parse:false";
    return false;
  }

  return true;
}

void CODEC_COMPONENT::RSUMessageProc(
    const std::shared_ptr<const os::v2x::device::RSUData>& rsu_pb_data) {
  if (!rsu_pb_data) {
    V2X_CODEC_LOG_WARN << "ptr null";
    return;
  }
  std::shared_ptr<v2xpb::asn::MessageFrame> frame_pb =
      std::make_shared<v2xpb::asn::MessageFrame>();
  if (!RsuPb2MessageFrame(rsu_pb_data, frame_pb)) {
    V2X_CODEC_LOG_WARN << " recvpb to sendpb false";
    return;
  }

  V2X_CODEC_LOG_INFO << "write pb:" << frame_pb->DebugString();
  Send("/airos/message/received", frame_pb);
  return;
}

bool CODEC_COMPONENT::Proc(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) {
  if (!frame) {
    V2X_CODEC_LOG_WARN << "frame is nullptr";
    return false;
  }
  auto encode_pb = std::make_shared<os::v2x::device::RSUData>();
  if (!MessageFrame2RsuPb(frame, encode_pb)) {
    AWARN << "message frame to rsupb failed, total fail num "
          << ++message_frame_to_rsupb_failed_num;
    return false;
  }

  // AERROR << "[summary] message frame to rsupb failed, total fail num "
  //        << message_frame_to_rsupb_failed_num;

  // 统计丢包率，延迟
  if (v2xpb::asn::MessageFrame::PayloadCase::kRsmFrame ==
      frame->payload_case()) {
    const v2xpb::asn::Rsm& rsmframe = frame->rsmframe();
    encode_pb->set_obj_num(rsmframe.participants_size());

    recv_total_obj_num += rsmframe.participants_size();
    if (rsmframe.has_end_flag() && true == rsmframe.end_flag()) {
        encode_pb->set_end_flag(true);
        encode_pb->set_total_obj_num(rsmframe.total_obj_num());
        encode_pb->set_message_timestamp(rsmframe.message_timestamp());
        V2X_CODEC_LOG_INFO << "sumarry: Expected recv total obj num: " << rsmframe.total_obj_num()
                           << ", Actual recv obj num: " << recv_total_obj_num
                           << ", Obj loss rate: " << std::fixed
                           << (rsmframe.total_obj_num() - recv_total_obj_num) * 1.0 /
                                  rsmframe.total_obj_num();
    }
  }
  Send("/airos/device/rsu_in", encode_pb);
  return true;
}

bool CODEC_COMPONENT::MessageFrame2RsuPb(
    const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame,
    std::shared_ptr<os::v2x::device::RSUData> encode_pb) {
  if (!frame || !encode_pb) {
    V2X_CODEC_LOG_WARN << "input is nullptr";
    return false;
  }

  os::v2x::device::RSUDataType data_type;
  switch (frame->payload_case()) {
    case v2xpb::asn::MessageFrame::PayloadCase::kBsmFrame:
      data_type = os::v2x::device::RSU_BSM;
      V2X_CODEC_LOG_INFO << "recv msg pb bsm";
      break;
    case v2xpb::asn::MessageFrame::PayloadCase::kMapFrame:
      data_type = os::v2x::device::RSU_MAP;
      V2X_CODEC_LOG_INFO << "recv msg pb map";
      break;
    case v2xpb::asn::MessageFrame::PayloadCase::kRsmFrame:
      data_type = os::v2x::device::RSU_RSM;
      V2X_CODEC_LOG_INFO << "recv msg pb rsm";
      break;
    case v2xpb::asn::MessageFrame::PayloadCase::kSpatFrame:
      data_type = os::v2x::device::RSU_SPAT;
      V2X_CODEC_LOG_INFO << "recv msg pb spat";
      break;
    case v2xpb::asn::MessageFrame::PayloadCase::kRsiFrame:
      data_type = os::v2x::device::RSU_RSI;
      V2X_CODEC_LOG_INFO << "recv msg pb rsi";
      break;
    case v2xpb::asn::MessageFrame::PayloadCase::kExtFrame:
      data_type = os::v2x::device::RSU_PERCEPTION;
      V2X_CODEC_LOG_INFO << "recv msg pb ext";
      break;
    default:
      V2X_CODEC_LOG_WARN << "msg type not support ";
      return false;
  }

  std::string str_pb;
  if (!frame->SerializePartialToString(&str_pb)) {
    V2X_CODEC_LOG_WARN << "pb serialize false";
    return false;
  }

  std::string str_asn;
  if (0 > message_frame_pbstr2uper_adapter(str_pb, &str_asn, asn_type_)) {
    V2X_CODEC_LOG_WARN << "pb to asn false";
    return false;
  }

  if (asn_type_ == EnAsnType::YDT_3709_2020) {
    encode_pb->set_version(os::v2x::device::YDT_3709_2020);
  } else {
    encode_pb->set_version(os::v2x::device::CSAE_53_2020);
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  encode_pb->set_time_stamp(
      static_cast<uint64_t>(tv.tv_sec * 1000 + tv.tv_usec / 1000));
  encode_pb->set_data(
      (const void*)(str_asn.data()), static_cast<size_t>(str_asn.size()));
  encode_pb->set_type(data_type);

  encode_pb->set_sequence_num(++sequence_num % 128);

  return true;
}

}  // namespace protocol
}  // namespace v2x
}  // namespace os
