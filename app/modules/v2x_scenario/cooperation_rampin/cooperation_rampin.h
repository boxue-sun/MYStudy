/*
 * cooperation_rampin.h
 *
 *  Created on: 2024年4月10日
 *      Author: hezhiyong
 */

#ifndef APP_MODULES_V2X_SCENARIO_COOPERATION_RAMPIN_COOPERATION_RAMPIN_H_
#define APP_MODULES_V2X_SCENARIO_COOPERATION_RAMPIN_COOPERATION_RAMPIN_H_

#include "app/framework/proto/v2xpb-asn-message-frame.pb.h"
#include "app/framework/proto/v2xpb-asn-vir.pb.h"
#include "app/framework/proto/v2xpb-asn-rsc.pb.h"
#include "app/framework/interface/app_base.h"
#include "app/modules/v2x_scenario/common_module/data_model/position_model.h"

namespace airos {
namespace app {
using namespace v2xpb::asn;

struct RampInInfo
{
	int status;//请求车状态
	std::map<std::string, int> rv_list;//协作方id与协作应答<id, rsp> rsp:0未收到应答  1 同意  2拒绝
};

class CooperationRampIn
{
public:
	CooperationRampIn();
	~CooperationRampIn();

	bool Init(const airos::app::ApplicationCallBack& send_cb, const std::string& conf_path);
	void ProcV2xMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

private:
	void processHvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);
	void processRvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame);

	void transmitRSC(std::string vehId, int reqID, ReqStatus status, Reason reason, DriveBehavior suggestBehavior);

	bool onHvFront(double hlat, double hlon, double hheading, double rlat, double rlon, double &inangle);
	int64_t get_mill_second_minute();
	bool isEmergencyVeh(int type);

private:
	airos::app::ApplicationCallBack send_;
	std::string rsu_id;
	double max_service_dist = 100.0;//单位m，最大服务范围，请求方超过此距离则拒绝引导
	double max_ttc = 5;
	double safty_area_length = 5;//m

	double lat = 36.0;//RSU/MEC设备纬度
	double lon = 129.0;//RSU/MEC设备经度
	std::map<std::string, RampInInfo> guide_list;

	double startPointLat = 36.0;//合流区起点纬度
	double startPointLon = 129.0;//合流区起点经度
	double endPointLat = 36.0;//合流区终点纬度
	double endPointLon = 129.0;//合流区终点经度
	int mainRoadLaneId = 1;
	int sideRoadLaneId = 2;
	int rsc_count_ = 0;
};

}
}

#endif /* APP_MODULES_V2X_SCENARIO_COOPERATION_RAMPIN_COOPERATION_RAMPIN_H_ */
