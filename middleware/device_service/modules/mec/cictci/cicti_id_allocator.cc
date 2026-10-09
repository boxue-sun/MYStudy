/*
 * @Author: libo
 * @Date: 2024-01-24 10:16:47
 * @LastEditors: zhangenwei
 * @LastEditTime:  2024-01-24 10:16:47
 * @Description:
 * @FilePath: /airos-edge/base/common/net_util.h
 */

#include "cicti_id_allocator.h"
namespace os {
namespace v2x {
namespace device {
//单例模式创建时自动调用
SensorObjIdAllocator::SensorObjIdAllocator() : nextId(1)
{
    time_t now = time(NULL);

    //index=0不使用，只使用1~ID_ARRAY_SIZE-1
    for(uint32_t i = 0; i < ID_ARRAY_SIZE; i++)
    {
        auto ele = new ObjIdElement;
        ele->id = i;
        ele->lastTs = now;
        ele->sensorReportId = "";
        objIdArray[i] = ele;
    }
}
int SensorObjIdAllocator::getObjId(const std::string& sensorReportId)
{
    time_t now = time(NULL);
    int id = nextId;

    if(SensorReportIdStr2ObjIdEleMap.find(sensorReportId) != SensorReportIdStr2ObjIdEleMap.end())
    {   //相同的传感器objId映射到相同的ptcId
        auto ele = SensorReportIdStr2ObjIdEleMap[sensorReportId];
        //assert(ele->sensorReportId == sensorReportId);
        ele->lastTs = now;
        id = ele->id;
    }
    else
    {   //新的传感器objId使用新的ptcId
        id = nextId;
        auto ele = objIdArray[id];

        if(ele->sensorReportId != "")
        {
            auto it = SensorReportIdStr2ObjIdEleMap.find(ele->sensorReportId);
            if(it != SensorReportIdStr2ObjIdEleMap.end())
            {
                SensorReportIdStr2ObjIdEleMap.erase(it);
            }
        }

        ele->sensorReportId = sensorReportId;
        ele->lastTs = now;
        SensorReportIdStr2ObjIdEleMap[sensorReportId] = ele;

        //产生下一个可用id
        if(nextId >= (ID_ARRAY_SIZE - 1))
        {
            nextId = 1;
        }
        else
        {
            nextId++;
        }
    }

    return id;
}

}  // namespace device
}  // namespace v2x
}  // namespace os