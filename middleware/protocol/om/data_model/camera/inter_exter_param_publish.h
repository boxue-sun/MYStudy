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

#ifndef AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_PUBLISH_H
#define AIROS2_0_CONFIGER_OM_CAMERA_INTER_EXTER_PARAM_PUBLISH_H
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
struct InternalParam : public afl::base::SerializableData {
    std::string serialNumber;
    std::string cameraType;
    std::string reprojectionError;
    std::string calibrationResultFlag;
    int height;
    int width;
    std::string distortionModel;

    std::string  D;                   // Distortion coefficients
    std::string  K;                   // Intrinsic matrix
    std::string  R;                   // Rotation matrix
    std::string  P;                   // Projection matrix
private:
    virtual void serialize(json &j) override {
        JsonSerialize(serialNumber, "serialNumber", j, false);
        JsonSerialize(cameraType, "cameraType", j, false);
        JsonSerialize(reprojectionError, "reprojectionError", j, false);
        JsonSerialize(calibrationResultFlag, "calibrationResultFlag", j, false);
        JsonSerialize(height, "height", j, false);
        JsonSerialize(width, "width", j, false);
        JsonSerialize(distortionModel, "distortionModel", j, false);
        JsonSerialize(D, "D", j, false);
        JsonSerialize(K, "K", j, false);
        JsonSerialize(R, "R", j, false);
        JsonSerialize(P, "P", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(serialNumber,             "serialNumber",               j, noUse_isEmptyFlag);
        JsonDeserialize(cameraType,               "cameraType",                 j, noUse_isEmptyFlag);
        JsonDeserialize(reprojectionError,        "reprojectionError",          j, noUse_isEmptyFlag);
        JsonDeserialize(calibrationResultFlag,   "calibrationResultFlag",     j, noUse_isEmptyFlag);
        JsonDeserialize(height,                    "height",                      j, noUse_isEmptyFlag);
        JsonDeserialize(width,                     "width",                       j, noUse_isEmptyFlag);
        JsonDeserialize(distortionModel,          "distortionModel",            j, noUse_isEmptyFlag);
        JsonDeserialize(D,                         "D",                           j, noUse_isEmptyFlag);
        JsonDeserialize(K,                         "K",                           j, noUse_isEmptyFlag);
        JsonDeserialize(R,                         "R",                           j, noUse_isEmptyFlag);
        JsonDeserialize(P,                         "P",                           j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "serialNumber: "                  << serialNumber                  << std::endl;
        ss << std::left << std::setw(40) << "cameraType: "                    << cameraType                    << std::endl;
        ss << std::left << std::setw(40) << "reprojectionError: "             << reprojectionError             << std::endl;
        ss << std::left << std::setw(40) << "calibrationResultFlag: "        << calibrationResultFlag        << std::endl;
        ss << std::left << std::setw(40) << "height: "                         << height                         << std::endl;
        ss << std::left << std::setw(40) << "width: "                          << width                          << std::endl;
        ss << std::left << std::setw(40) << "distortionModel: "               << distortionModel               << std::endl;
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

        return ss.str();
    }
};

struct MappingData : public afl::base::SerializableData
{
    std::string imageName;
    std::string imageUrl;
    std::string serialNumber;
    Position2D twoPosition;
    Position3D threePosition;

    virtual void serialize(json &j) override
    {
        JsonSerialize(imageName,                    "imageName",                    j, false);
        JsonSerialize(imageUrl,                     "imageUrl",                     j, false);
        JsonSerialize(serialNumber,                 "serialNumber",                 j, false);
        JsonSerialize(twoPosition,                  "twoPosition",                  j, false);
        JsonSerialize(threePosition,                "threePosition",                j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(imageName,                    "imageName",                    j, noUse_isEmptyFlag);
        JsonDeserialize(imageUrl,                     "imageUrl",                     j, noUse_isEmptyFlag);
        JsonDeserialize(serialNumber,                 "serialNumber",                 j, noUse_isEmptyFlag);
        JsonDeserialize(twoPosition,                  "twoPosition",                  j, noUse_isEmptyFlag);
        JsonDeserialize(threePosition,                "threePosition",                j, noUse_isEmptyFlag);
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "imageName: "                     << imageName                     << std::endl;
        ss << std::left << std::setw(40) << "imageUrl: "                      << imageUrl                      << std::endl;
        ss << std::left << std::setw(40) << "serialNumber: "                  << serialNumber                  << std::endl;
        ss << std::left << std::setw(40) << "twoPosition: "                   << std::endl << twoPosition.to_string();
        ss << std::left << std::setw(40) << "threePosition: "                 << std::endl << threePosition.to_string();
        return ss.str();
    }
};
struct TranslationPush : public afl::base::SerializableData
{
    string x;
    string y;
    string z;
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

struct RotationPush : public afl::base::SerializableData
{
    string w;
    string x;
    string y;
    string z;
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

struct TransformPush : public afl::base::SerializableData
{
    TranslationPush translation;
    RotationPush rotation;
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



struct PosePositionAngle : public afl::base::SerializableData
{
    std::string childFrameId;
    std::string frameId;
    std::string serialNumber;
    TransformPush   transform;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(childFrameId,                     "childFrameId",                     j, false);
        JsonSerialize(frameId,                          "frameId",                          j, false);
        JsonSerialize(serialNumber,                     "serialNumber",                     j, false);
        JsonSerialize(transform,                        "transform",                        j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(childFrameId,                   "childFrameId",                     j, noUse_isEmptyFlag);
        JsonDeserialize(frameId,                        "frameId",                          j, noUse_isEmptyFlag);
        JsonDeserialize(serialNumber,                   "serialNumber",                     j, noUse_isEmptyFlag);
        JsonDeserialize(transform,                      "transform",                        j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "childFrameId: "                  << childFrameId                  << std::endl;
        ss << std::left << std::setw(40) << "frameId: "                       << frameId                       << std::endl;
        ss << std::left << std::setw(40) << "serialNumber: "                  << serialNumber                  << std::endl;
        ss << std::left << std::setw(40) << "transform: "                     << transform.to_string();
        return ss.str();
    }
};

struct ExternalParam : public afl::base::SerializableData
{
    std::vector<MappingData> mappingDataList;
    PosePositionAngle posePositionAngle;

private:
    virtual void serialize(json &j) override {
        JsonSerialize(mappingDataList,              "mappingDataList",              j, false);
        JsonSerialize(posePositionAngle,            "posePositionAngle",            j, false);
    }

    virtual void deserialize(const json &j) override {
        JsonDeserialize(mappingDataList,           "mappingDataList",              j, noUse_isEmptyFlag);
        JsonDeserialize(posePositionAngle,         "posePositionAngle",            j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const {
        std::stringstream ss;
        int i = 0;
        for(auto mappingData:mappingDataList)
        {
            i++;
            ss << std::left << std::setw(40) << "mappingDataList[" << i <<   "]:"   << mappingData.to_string() << std::endl;
        }

        ss << std::left << std::setw(40) << "posePositionAngle: "             << posePositionAngle.to_string()  << std::endl;
        return ss.str();
    }
};
struct CameraCalibrationPublishData : public afl::base::SerializableData
{
    uint64_t        timestamp;
    std::string     seqNum;               // e.g., "18.22.57.11"
    std::string deviceID;
    InternalParam internalParam;
    ExternalParam externalParam;
    bool ack;
    bool ackEmpty = true;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(timestamp,                     "timestamp",                     j, false);
        JsonSerialize(seqNum,                        "seqNum",                        j, false);
        JsonSerialize(deviceID,                      "deviceID",                      j, false);
        JsonSerialize(internalParam,                 "internalParam",                 j, false);
        JsonSerialize(externalParam,                 "externalParam",                 j, false);
        JsonSerialize(ack,                           "ack",                           j, ackEmpty);
    }

    virtual void deserialize(const json &j) override
    {

        JsonDeserialize(timestamp,                  "timestamp",                     j, noUse_isEmptyFlag);
        JsonDeserialize(seqNum,                     "seqNum",                        j, noUse_isEmptyFlag);
        JsonDeserialize(deviceID,                   "deviceID",                      j, noUse_isEmptyFlag);
        JsonDeserialize(internalParam,              "internalParam",                 j, noUse_isEmptyFlag);
        JsonDeserialize(externalParam,              "externalParam",                 j, noUse_isEmptyFlag);
        JsonDeserialize(ack,                        "ack",                           j, ackEmpty);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "timestamp: "                     << timestamp                    << std::endl;
        ss << std::left << std::setw(40) << "seqNum: "                        << seqNum                       << std::endl;
        ss << std::left << std::setw(40) << "deviceID: "                      << deviceID                     << std::endl;
        ss << std::left << std::setw(40) << "internalParam: "                 << internalParam.to_string()    << std::endl;
        ss << std::left << std::setw(40) << "externalParam: "                 << externalParam.to_string()    << std::endl;
        ss << std::left << std::setw(40) << "ack: "                           << ack                          << std::endl;

        return ss.str();
    }
};


class InterExterParam {
public:
    InterExterParam() = default;

    virtual ~InterExterParam();

    static bool
    getWorkParamFromFile(std::string workParamFilePath, CameraCalibrationPublishData &cameraCalibrationPublishData) {
//        ERRORPRINT("[work-param-file-path]%s", workParamFilePath.c_str());
        std::ifstream file(workParamFilePath);
        if (!file) {
            ERRORPRINT("[error]Failed to open file!");
            return false;
        }
        afl::base::json workParamJson;
        try {
            file >> workParamJson;
            cameraCalibrationPublishData = workParamJson;
        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
            return false;
        }
        return true;
    }

    static CameraCalibrationPublishData
    getWorkParamFromFile(std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag")
    {

        CameraCalibrationPublishData cameraCalibrationPublishData;
        std::ifstream file(workParamFilePath);
        if (!file){
            ERRORPRINT("[error]Failed to open file!");
        }
        afl::base::json workParamJson;
        try {
            file >> workParamJson;
            cameraCalibrationPublishData = workParamJson;
        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
        }
        return cameraCalibrationPublishData;
    }


    static afl::base::json
    getWorkParamFromFileJson(std::string workParamFilePath = "/home/airos/common_config/work_param_config.flag")
    {

        CameraCalibrationPublishData cameraCalibrationPublishData;
        std::ifstream file(workParamFilePath);
        if (!file){
            ERRORPRINT("[error]Failed to open file!");
        }
        afl::base::json workParamJson;
        try {
            file >> workParamJson;

        }
        catch (afl::base::json::exception &e) {
            ERRORPRINT("[error]%s", e.what());
        }
        return workParamJson;
    }

};

NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif //AIROS2_0_CONFIGER_OM_CAMERA_H
