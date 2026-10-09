/*********************************************************************************
* @file		inter_exter_param
* @brief	inter_exter_param belongs to CICTCI
* @details
* @author		alfred
* @email       zhangenwei64@gmail.com
* @date		24-6-9
* @copyright	Copyright (c) 2024 Mec-Airos Division.
* @verbatim
*
*  Change History:
*  Date      Author    Version  ChangeId           Description
*  ------------------------------------------------------------------------------
*  25-3-4 alfred       1.0       ————             Create this file
*
* @endverbatim
********************************************************************************/

#ifndef AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_FILE_H
#define AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_FILE_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"
#include "inter_exter_param.h"
NAMESPACE_START_OM_COMPONENT_CAMERA
#include <string>
#include <vector>
using namespace os::v2x::protocol::om::camera;

struct Position2DFile : public afl::base::SerializableData
{
    int x;
    int y;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(x,                                "A_Position_X",                        j, false);
        JsonSerialize(y,                                "B_Position_Y",                        j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(x,                              "A_Position_X",                        j, noUse_isEmptyFlag);
        JsonDeserialize(y,                              "B_Position_Y",                        j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "Position_X: "                                    << x           << std::endl;
        ss << std::left << std::setw(40) << "Position_Y: "                                    << y           << std::endl;
        return ss.str();
    }
};

struct Position3DFile : public afl::base::SerializableData
{
    double x;
    double y;
    double z;
private:
    // 序列化函数
    virtual void serialize(json &j) override
    {
        JsonSerialize(x,                             "x",           j, false);
        JsonSerialize(y,                             "y",           j, false);
        JsonSerialize(z,                             "z",           j, false);
    }

    // 反序列化函数
    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(x,                           "x",           j, noUse_isEmptyFlag);
        JsonDeserialize(y,                           "y",           j, noUse_isEmptyFlag);
        JsonDeserialize(z,                           "z",           j, noUse_isEmptyFlag);
    }

public:
    // 将信息转为字符串
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "x: "                   << x                   << std::endl;
        ss << std::left << std::setw(40) << "y: "                   << y                   << std::endl;
        ss << std::left << std::setw(40) << "z: "                   << z                   << std::endl;
        return ss.str();
    }
};



struct CameraCalibrationData : public afl::base::SerializableData
{
    std::string     serial_number;               // e.g., "18.22.57.11"
    std::string     camera_type;                 // e.g., "camera"
    double          reprojection_error;               // e.g., 14.6441
    std::string     calibration_result_flag;     // e.g., "Success!"
    int             height;                              // e.g., 1080
    int             width;                               // e.g., 1920
    std::string     distortion_model;            // e.g., "plumb_bob"
    std::vector<double> D;                   // Distortion coefficients
    std::vector<double> K;                   // Intrinsic matrix
    std::vector<double> R;                   // Rotation matrix
    std::vector<double> P;                   // Projection matrix
    Transform transform;                     // Transformation (translation and rotation)
    Position2DFile position_2D;                  // 2D position
    Position3DFile position_3D;                  // 3D position

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(serial_number,               "serial_number",               j, false);
        JsonSerialize(camera_type,                 "camera_type",                 j, false);
        JsonSerialize(reprojection_error,          "reprojection_error",          j, false);
        JsonSerialize(calibration_result_flag,     "calibration_result_flag",     j, false);
        JsonSerialize(height,                      "height",                      j, false);
        JsonSerialize(width,                       "width",                       j, false);
        JsonSerialize(distortion_model,            "distortion_model",            j, false);
        JsonSerialize(D,                           "D",                           j, false);
        JsonSerialize(K,                           "K",                           j, false);
        JsonSerialize(R,                           "R",                           j, false);
        JsonSerialize(P,                           "P",                           j, false);
        JsonSerialize(transform,                  "transform",                   j, false);
        JsonSerialize(position_2D,                "2D_position",                 j, false);
        JsonSerialize(position_3D,                "3D_position",                 j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(serial_number,             "serial_number",               j, noUse_isEmptyFlag);
        JsonDeserialize(camera_type,               "camera_type",                 j, noUse_isEmptyFlag);
        JsonDeserialize(reprojection_error,        "reprojection_error",          j, noUse_isEmptyFlag);
        JsonDeserialize(calibration_result_flag,   "calibration_result_flag",     j, noUse_isEmptyFlag);
        JsonDeserialize(height,                    "height",                      j, noUse_isEmptyFlag);
        JsonDeserialize(width,                     "width",                       j, noUse_isEmptyFlag);
        JsonDeserialize(distortion_model,          "distortion_model",            j, noUse_isEmptyFlag);
        JsonDeserialize(D,                         "D",                           j, noUse_isEmptyFlag);
        JsonDeserialize(K,                         "K",                           j, noUse_isEmptyFlag);
        JsonDeserialize(R,                         "R",                           j, noUse_isEmptyFlag);
        JsonDeserialize(P,                         "P",                           j, noUse_isEmptyFlag);
        JsonDeserialize(transform,                "transform",                   j, noUse_isEmptyFlag);
        JsonDeserialize(position_2D,              "2D_position",                 j, noUse_isEmptyFlag);
        JsonDeserialize(position_3D,              "3D_position",                 j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "serial_number: "                  << serial_number                  << std::endl;
        ss << std::left << std::setw(40) << "camera_type: "                    << camera_type                    << std::endl;
        ss << std::left << std::setw(40) << "reprojection_error: "             << reprojection_error             << std::endl;
        ss << std::left << std::setw(40) << "calibration_result_flag: "        << calibration_result_flag        << std::endl;
        ss << std::left << std::setw(40) << "height: "                         << height                         << std::endl;
        ss << std::left << std::setw(40) << "width: "                          << width                          << std::endl;
        ss << std::left << std::setw(40) << "distortion_model: "               << distortion_model               << std::endl;
        for(auto data : D)
        {
            ss << std::left << std::setw(40) << "data: "    << data             << std::endl;
        }
        for(auto data : K)
        {
            ss << std::left << std::setw(40) << "data: "    << data             << std::endl;
        }
        for(auto data : R)
        {
            ss << std::left << std::setw(40) << "data: "    << data             << std::endl;
        }
        for(auto data : P)
        {
            ss << std::left << std::setw(40) << "data: "    << data             << std::endl;
        }

        ss << std::left << std::setw(40) << "transform: "                      << transform.to_string()          << std::endl;
        ss << std::left << std::setw(40) << "position_2D: "                    << position_2D.to_string()        << std::endl;
        ss << std::left << std::setw(40) << "position_3D: "                    << position_3D.to_string()        << std::endl;
        return ss.str();
    }
};



NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
