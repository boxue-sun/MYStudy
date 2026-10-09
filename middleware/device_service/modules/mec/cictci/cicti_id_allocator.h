/*
 * @Author: libo
 * @Date: 2024-01-24 10:16:47
 * @LastEditors: zhangenwei
 * @LastEditTime:  2024-01-24 10:16:47
 * @Description:
 * @FilePath: /airos-edge/base/common/net_util.h
 */
#ifndef BASE_COMMON_DEVICE_CONNECT_MEC_CICTCI_OBJIDALLOCATOR_H_
#define BASE_COMMON_DEVICE_CONNECT_MEC_CICTCI_OBJIDALLOCATOR_H_

#include <string>
#include <map>
#include "base/common/network/cs_singleton.h"
namespace os {
namespace v2x {
namespace device {
#define ID_ARRAY_SIZE 1024

struct ObjIdElement
{
    int id;                     //1~ID_ARRAY_SIZE-1
    time_t lastTs;              //上次使用的ts
    std::string sensorReportId; //上次是传感器上报的哪个id映射过来的，格式是 协议_objId，分辨不同传感器过来的数据
};

class SensorObjIdAllocator: public afl::base::Singleton<SensorObjIdAllocator>
{
    DECLARE_SINGLETON_CLASS(SensorObjIdAllocator);
public:
    SensorObjIdAllocator();
    //格式是 协议_objId，分辨不同传感器过来的数据
    int getObjId(const std::string& sensorReportId);

private:
    int nextId;
    ObjIdElement* objIdArray[ID_ARRAY_SIZE];  //index 0不用，使用1~ID_ARRAY_SIZE-1
    std::map<std::string, ObjIdElement*> SensorReportIdStr2ObjIdEleMap;
};

#define SENSOROBJIDALLOCATOR_INSTANCE_REF    afl::base::Singleton<SensorObjIdAllocator>::getInstanceRef()

}  // namespace device
}  // namespace v2x
}  // namespace os
#endif
