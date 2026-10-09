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

#include <memory>
#include <string>

#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "base/device_connect/proto/rsu_data.pb.h"

#include "middleware/runtime/src/air_middleware_component.h"
#include "v2xpb-asn/v2x-asn-msgs-adapter.hpp"

namespace os {
namespace v2x {
namespace protocol {

#define CODEC_COMPONENT AIROS_COMPONENT_CLASS_NAME(V2xCodecComponent)

class CODEC_COMPONENT
    : public airos::middleware::ComponentAdapter<v2xpb::asn::MessageFrame> {
 public:
  CODEC_COMPONENT() = default;

  bool Init() override;
  bool Proc(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame) override;

 private:
  bool MessageFrame2RsuPb(
      const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame,
      std::shared_ptr<os::v2x::device::RSUData> encode_pb);
  void RSUMessageProc(
      const std::shared_ptr<const os::v2x::device::RSUData>& rsu_pb_data);
  bool RsuPb2MessageFrame(
      const std::shared_ptr<const os::v2x::device::RSUData>& rsu_pb_data,
      std::shared_ptr<v2xpb::asn::MessageFrame>& frame_pb);

  EnAsnType asn_type_{EnAsnType::YDT_3709_2020};

  uint32_t message_frame_to_rsupb_failed_num = 0;
  uint32_t sequence_num = 0;
  // 收到目标的数量
  uint64_t recv_total_obj_num = 0;
};

REGISTER_AIROS_COMPONENT_CLASS(V2xCodecComponent, v2xpb::asn::MessageFrame);

}  // namespace protocol
}  // namespace v2x
}  // namespace os
