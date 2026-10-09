/*********************************************************************************
 * @file		protobuf_to_json.h
 * @brief		protobuf_to_json belongs to CICTCI
 * @details
 * @author		cs
 * @date		2024-02-04
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2024-02-04 cs       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/
#ifndef BASE_COMMON_BASE_PROTOBUF_TO_JSON_H_
#define BASE_COMMON_BASE_PROTOBUF_TO_JSON_H_
#include "define.h"
#include "time_stamp.h"
#include "glog/logging.h"
#include "glog/raw_logging.h"
#include "google/protobuf/util/json_util.h"
#include "serializable_data.h"
class Pb2Json
{
public:
    template<typename T>
    static std::string transPb2Json(T& pb)
    {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = false;  // 不添加额外的空格和换行
        options.always_print_primitive_fields = true;  // 即使是默认值也打印基本类型字段
        options.always_print_enums_as_ints = false;  // 打印枚举的名称而不是值
        std::string json_string;

        auto status = google::protobuf::util::MessageToJsonString(pb, &json_string, options);

        if (status.ok()) {
          return json_string;  // 打印单行的JSON字符串
        } else {
            std::cerr << "Failed to convert protobuf message to JSON string." << std::endl;
            return "";
        }
    }
};

#endif //AIROS_EDGE_PRINT_H
