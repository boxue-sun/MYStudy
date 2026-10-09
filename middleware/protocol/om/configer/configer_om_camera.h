/*********************************************************************************
 * @file		configer.h
 * @brief		configer belongs to CICTCI
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
 *  24-6-9 alfred       1.0       ————             Create this file
 *
 * @endverbatim
 ********************************************************************************/

#ifndef AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_CAMERA_H
#define AIROS_MIDDLEWARE_PROTOCOL_OM_CONFIGER_CONFIGER_OM_CAMERA_H
#include "middleware/protocol/om_common/namespace.h"
#include "middleware/protocol/om_common/configer_common.h"
#include "middleware/protocol/om_common/configer_topic_om_camera.h"
#include "base/work_param/configer_om_work_param.h"
#include "middleware/protocol/om_common/configer_work_param.h"

NAMESPACE_START_OM_COMPONENT_CAMERA

using namespace os::v2x::protocol::om::common;
using namespace airos::base::workparam;

//rsu客户端配置
struct ConfigerCamera : public afl::base::SerializableData
{
    MqttTopicConfigerCameraRcId         configerTopicRcId;
    MqttEnableConfigerCamera            configerEnable;
    ////////////////////////////////////////////////////////
    MqttTopicConfigerCamera             configerTopic;
    std::unordered_map<uint32_t, MqttTopicConfigerCamera>  topicUnMap;
    ////////////////////////////////////////////////////////


private:
    virtual void serialize(afl::base::json &j) override
    {
        JsonSerialize(configerTopicRcId, "A_configerTopicRcId", j, false);
        JsonSerialize(configerEnable, "B_configerEnable", j, false);
        JsonSerialize(configerTopic, "C_configerTopic", j, false);

    }

    virtual void deserialize(const afl::base::json &j) override
    {
        JsonDeserialize(configerTopicRcId, "A_configerTopicRcId", j, noUse_isEmptyFlag);
        JsonDeserialize(configerEnable, "B_configerEnable", j, noUse_isEmptyFlag);
        JsonDeserialize(configerTopic, "C_configerTopic", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "A_configerTopicRcId:" << std::endl << configerTopicRcId.to_string() << std::endl;
        ss << std::left << std::setw(40) << "B_configerEnable:" << std::endl << configerEnable.to_string() << std::endl;
        ss << std::left << std::setw(40) << "C_configerTopic:" << std::endl << configerTopic.to_string() << std::endl;
        ss << "---------------------------------------------------" << std::endl;
        ss << std::left << std::setw(40) << "topicUnMap-Size: " << topicUnMap.size() << std::endl;
        for (const auto& pair : topicUnMap)
        {
            ss << std::left << "Camera-topic[" << pair.first << "]" << std::endl ;
            ss << pair.second.to_string() << std::endl;
            ss << "--------------------------------" << std::endl;
        }

        return ss.str();
    }
};

struct CameraParamMapFromFileConfiger : public afl::base::SerializableData
{
public:

    std::string         deviceEsn;
    bool                isPubOnce = false;
    afl::base::json     cameraParamJsonContent;
    std::string         cameraParamFilePath;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(deviceEsn, "A_deviceEsn", j, false);
        JsonSerialize(isPubOnce, "B_isPubOnce", j, false);
        JsonSerialize(cameraParamJsonContent, "C_cameraParamJsonContent", j, false);
        JsonSerialize(cameraParamFilePath, "D_cameraParamFilePath", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(deviceEsn, "A_deviceEsn", j, noUse_isEmptyFlag);
        JsonDeserialize(isPubOnce, "B_isPubOnce", j, noUse_isEmptyFlag);
        JsonDeserialize(cameraParamJsonContent, "C_cameraParamJsonContent", j, noUse_isEmptyFlag);
        JsonDeserialize(cameraParamFilePath, "D_cameraParamFilePath", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::endl;
        ss << std::left << std::setw(40) << "deviceEsn: " << deviceEsn <<  std::endl;
        ss << std::left << std::setw(40) << "isPubOnce: " << (isPubOnce ? "True" : "False") <<  std::endl;
        ss << std::left << std::setw(40) << "cameraParamJsonContent: " << cameraParamJsonContent.dump() <<  std::endl;
        ss << std::left << std::setw(40) << "cameraParamFilePath: " << cameraParamFilePath <<  std::endl;
        return ss.str();
    }
};


struct CameraRegisterMapConfiger : public afl::base::SerializableData
{
public:
    bool                isRegistered = false;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(isRegistered, "A_isRegistered", j, false);
    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(isRegistered, "A_isRegistered", j, noUse_isEmptyFlag);
    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "isRegistered: " << isRegistered <<  std::endl;
        return ss.str();
    }
};

struct ConfigerCameraEvent : public afl::base::SerializableData
{
public:
    std::string imageDir = "/camera/";
    std::string httpPath = "/";
    int httpPort = 40415;

private:
    virtual void serialize(json &j) override
    {
        JsonSerialize(imageDir, "A_imageDir", j, false);
        JsonSerialize(httpPath, "B_httpPath", j, false);
        JsonSerialize(httpPort, "C_httpPort", j, false);

    }

    virtual void deserialize(const json &j) override
    {
        JsonDeserialize(imageDir, "A_imageDir", j, noUse_isEmptyFlag);
        JsonDeserialize(httpPath, "B_httpPath", j, noUse_isEmptyFlag);
        JsonDeserialize(httpPort, "C_httpPort", j, noUse_isEmptyFlag);

    }

public:
    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::left << std::setw(40) << "imageDir: " << imageDir <<  std::endl;
        ss << std::left << std::setw(40) << "httpPath: " << httpPath <<  std::endl;
        ss << std::left << std::setw(40) << "httpPort: " << httpPort <<  std::endl;

        return ss.str();
    }


};


NAMESPACE_ENDED_OM_COMPONENT_CAMERA
#endif
