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

#ifndef AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_H
#define AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"

NAMESPACE_START_OM_COMPONENT_CAMERA
#include <string>
#include <vector>
struct Translation : public afl::base::SerializableData
{
    double x;
    double y;
    double z;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(x,                          "x",                          j, false);
        JsonSerialize(y,                          "y",                          j, false);
        JsonSerialize(z,                          "z",                          j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(x,                          "x",                          j, noUse_isEmptyFlag);
        JsonDeserialize(y,                          "y",                          j, noUse_isEmptyFlag);
        JsonDeserialize(z,                          "z",                          j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "x: "                                << x                                 << std::endl;
        ss << std::left << std::setw(40) << "y: "                                << y                                 << std::endl;
        ss << std::left << std::setw(40) << "z: "                                << z                                 << std::endl;
        return ss.str();
    }
} ;

struct Rotation : public afl::base::SerializableData
{
    double w;
    double x;
    double y;
    double z;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(w,                       "w",                     j, false);
        JsonSerialize(x,                       "x",                     j, false);
        JsonSerialize(y,                       "y",                     j, false);
        JsonSerialize(z,                       "z",                     j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(w,                     "w",                     j, noUse_isEmptyFlag);
        JsonDeserialize(x,                     "x",                     j, noUse_isEmptyFlag);
        JsonDeserialize(y,                     "y",                     j, noUse_isEmptyFlag);
        JsonDeserialize(z,                     "z",                     j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "w : "          << w                             << std::endl;
        ss << std::left << std::setw(40) << "x : "          << x                             << std::endl;
        ss << std::left << std::setw(40) << "y : "          << y                             << std::endl;
        ss << std::left << std::setw(40) << "z : "          << z                             << std::endl;
        return ss.str();
    }
} ;
struct Transform : public afl::base::SerializableData
{
    Translation translation;
    Rotation rotation;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(translation,                       "translation",                     j, false);
        JsonSerialize(rotation,                       "rotation",                     j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(translation,                     "translation",                     j, noUse_isEmptyFlag);
        JsonDeserialize(rotation,                     "rotation",                     j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "translation : "          << translation.to_string()                             << std::endl;
        ss << std::left << std::setw(40) << "rotation : "          << rotation.to_string()                             << std::endl;
        return ss.str();
    }
};

struct Position2D : public afl::base::SerializableData
{
    string x;
    string y;
private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(x,                                "x",                        j, false);
        JsonSerialize(y,                                "y",                        j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(x,                              "x",                        j, noUse_isEmptyFlag);
        JsonDeserialize(y,                              "y",                        j, noUse_isEmptyFlag);
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

struct Position3D : public afl::base::SerializableData
{
    string x;
    string y;
    string z;
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

NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
