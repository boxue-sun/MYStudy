/*
 * cooperation_rampin.cc
 *
 *  Created on: 2024年4月10日
 *      Author: hezhiyong
 */
#include "cooperation_rampin.h"
#include "base/common/singleton.h"
#include "base/common/math_util.h"
#include "app/modules/v2x_scenario/common_module/remote_vehicles/remote_vehicles.h"
#include "yaml-cpp/yaml.h"
#include <sys/time.h>
//#include "app/modules/v2x_application/usecase/base/config_manager.h"

namespace airos {
namespace app {

using namespace v2xpb::asn;
CooperationRampIn::CooperationRampIn()
{


}

CooperationRampIn::~CooperationRampIn()
{

}

bool CooperationRampIn::Init(const airos::app::ApplicationCallBack& send_cb, const std::string& conf_path)
{
	std::cout << "rampIn init" << std::endl;
	send_ = send_cb;
	std::string lanechange_configs = conf_path + "/rampin.yaml";

	YAML::Node root_node = YAML::LoadFile(lanechange_configs);
	if (!root_node.IsMap()) {
		std::cout << "cooperation_rampin Init ConfigManager Failed!" << std::endl;
		return false;
	}

	rsu_id = root_node["rsu_id"].as<std::string>();
	max_service_dist = root_node["max_service_dist"].as<double>();
	max_ttc = root_node["max_ttc"].as<double>();
	safty_area_length = root_node["safty_area_length"].as<double>();
	lat = root_node["lat"].as<double>();
	lon = root_node["lon"].as<double>();
	startPointLat = root_node["startPointLat"].as<double>();
	startPointLon = root_node["startPointLon"].as<double>();
	endPointLat = root_node["endPointLat"].as<double>();
	endPointLon = root_node["endPointLon"].as<double>();
	mainRoadLaneId = root_node["mainRoadLaneId"].as<int>();
	sideRoadLaneId = root_node["sideRoadLaneId"].as<int>();


	return true;
}

void CooperationRampIn::ProcV2xMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{
	if(!frame->has_extframe() || frame->extframe().messagevalue().typresent() != 8)//8：vir
	{
		return;
	}
	else
	{
		std::cout << "cooperation_rampIn recv VIR!" << std::endl;
	}

	auto vir = frame->extframe().messagevalue().vehintentionandrequest();

	if(vir.intandreq().resps_size() > 0)//协作方rv应答
	{
		processRvVIR(frame);
	}
	else if(vir.intandreq().reqs_size() > 0)//非协作方即请求方请求
	{
		processHvVIR(frame);
	}
	else
	{
		std::cout << "vir has no req and resp! " << std::endl;
	}

}

void CooperationRampIn::processRvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{

}

void CooperationRampIn::processHvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{
	auto hvir = frame->extframe().messagevalue().vehintentionandrequest();

	if(!hvir.intandreq().reqs(0).has_targetrsu() || hvir.intandreq().reqs(0).targetrsu() != rsu_id)
	{
		std::cout << "target rsu id error" <<std::endl;
		return;
	}

	if(!hvir.intandreq().reqs(0).has_info() || !hvir.intandreq().reqs(0).info().has_vehmerge())
	{
		std::cout << "req info error" << std::endl;
		return;
	}

	std::string hvId = hvir.id();
	auto status = hvir.intandreq().reqs(0).status();
	auto reqId = hvir.intandreq().reqs(0).reqid();
	double dist = max_service_dist + 1;//默认赋值超过最大值
	if(hvir.refpos().has_llh())
	{
		dist = base::MathUtil::getDistance(hvir.refpos().llh().latitude(), hvir.refpos().llh().longitude(), lat, lon);
	}

	//hv汇入请求不在rsu服务范围内
	if(guide_list.find(hvId) == guide_list.end() && dist > max_service_dist && hvir.intandreq().reqs(0).has_info() && hvir.intandreq().reqs(0).info().has_vehmerge())
	{
		std::cout <<"reject, dist more than max_dist" << std::endl;
		transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_notsafe, DriveBehavior::DriveBehavior_goStraightForward);
		return;
	}

	//hv驶离服务范围
	if(guide_list.find(hvId) != guide_list.end() && dist > max_service_dist)
	{
		transmitRSC(hvId, reqId, ReqStatus::ReqStatus_cancel, Reason::Reason_unknown, DriveBehavior::DriveBehavior_goStraightForward);
		guide_list.erase(hvId);
		return;
	}

	if(status == 1)//处于请求状态
	{
		RampInInfo rii;
		rii.status = status;

		auto &remote_vehs = RemoteVehicles::GetInstance();
		remote_vehs.LockRemoteVehiclesMap();

		auto &vehs = remote_vehs.GetRemoteVehiclesMap();
		if(vehs.find(hvId) != vehs.end())
		{

			double hvLat = vehs.at(hvId).GetPosition().GetPositionLLH().latitude;
			double hvLon = vehs.at(hvId).GetPosition().GetPositionLLH().longitude;
			double hvHeading = vehs.at(hvId).GetPosition().GetHeading();
			double hvSpeed = vehs.at(hvId).GetPosition().GetSpeed();
			int hvlaneid = vehs.at(hvId).GetTracker()->GetRefLaneId();

			if(hvlaneid != sideRoadLaneId)//hv不在辅路
			{
				std::cout << "hv not on side road" << std::endl;
				return;
			}

			double invalidValue;
			bool startonHvFront = onHvFront(hvLat, hvLon, hvHeading, startPointLat, startPointLon, invalidValue);
			bool endonHvFront = onHvFront(hvLat, hvLon, hvHeading, endPointLat, endPointLon, invalidValue);

			if(startonHvFront)//hv位于合流区起点前
			{
				bool firstVehBehindStartPoint = true;
				for(auto &iter : vehs)
				{
					if(iter.second.GetTracker()->GetRefLaneId() == sideRoadLaneId)
					{
						bool startonRvFront = onHvFront(iter.second.GetPosition().GetPositionLLH().latitude
														, iter.second.GetPosition().GetPositionLLH().longitude
														, iter.second.GetPosition().GetHeading()
														, startPointLat
														, startPointLon
														, invalidValue);

						bool rvonHvFront = onHvFront(hvLat, hvLon, hvHeading
								, iter.second.GetPosition().GetPositionLLH().latitude
								, iter.second.GetPosition().GetPositionLLH().longitude
								, invalidValue);

						if(startonRvFront && rvonHvFront)
						{
							firstVehBehindStartPoint = false;
							break;
						}
					}
				}
				//hv为合流区起点前第一辆车
				if(firstVehBehindStartPoint)
				{
					std::cout << "rampin req accept" << std::endl;
					transmitRSC(hvId, reqId, ReqStatus::ReqStatus_accept, Reason::Reason_unknown, DriveBehavior::DriveBehavior_rampIn);
					return;
				}
				else//hv为合流区起点前非第一辆车
				{
					std::cout << "rampin req reject" << std::endl;
					transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_engaged, DriveBehavior::DriveBehavior_rampIn);
					return;
				}

			}
			else if(!endonHvFront)//hv位于合流区终点后
			{
				return;
			}
			else//hv位于合流区
			{
				double dist2end = base::MathUtil::getDistance(hvLat, hvLon, endPointLat, endPointLon);
				std::string firstVehIdfrontHvInTargetLane, firstVehIdbehindHvInTargetLane;//目标车道在hv前方/后方第一辆车的ID
				bool firstVehbehindHvInTargetLaneisV2XType;//目标车道在hv后方第一辆车是否为网联车
				double firstVehSpeedfrontHvInTargetLane, firstVehSpeedbehindHvInTargetLane;//目标车道在hv前方/后方第一辆车的速度
				double minfrontDist = 200;//目标车道在hv前方第一辆车与hv的距离
				double minbehindDist = 200;//目标车道在hv后方第一辆车与hv的距离
				double frontLateralDist, behindLateralDist;//目标车道在hv前方/后方第一辆车与hv的横向距离
				double minfrontLongitudinalDist, minbehindLongitudinalDist;//目标车道在hv前方/后方第一辆车与hv的纵向距离
				double frontInangle, behindInangle;//hv与目标车道在hv前方/后方第一辆车的连线方向跟hv航向角的夹角

				for(auto &iter : vehs)
				{
					if(!isEmergencyVeh(vehs.at(hvId).GetVehicleType()))
					{
						if(isEmergencyVeh(iter.second.GetVehicleType()))
						{
							transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_higherPriority, DriveBehavior::DriveBehavior_goStraightForward);
							return;
						}
					}

					if(iter.second.GetTracker()->GetRefNodeId().id() == hvir.intandreq().reqs(0).info().vehmerge().downstreamnode().id()
							&& iter.second.GetTracker()->GetRefLinkUpStreamNodeId().id() == hvir.intandreq().reqs(0).info().vehmerge().upstreamnode().id()
							&& iter.second.GetTracker()->GetRefLaneId() == hvir.intandreq().reqs(0).info().vehmerge().targetlane())
					{
						double tempDist = base::MathUtil::getDistance(hvLat, hvLon
								, iter.second.GetPosition().GetPositionLLH().latitude
								, iter.second.GetPosition().GetPositionLLH().longitude);
						double tempInangle;

						if(onHvFront(hvLat, hvLon, hvHeading
								, iter.second.GetPosition().GetPositionLLH().latitude
								, iter.second.GetPosition().GetPositionLLH().longitude, tempInangle))
						{
							if(tempDist < minfrontDist)
							{
								minfrontDist = tempDist;
								firstVehIdfrontHvInTargetLane = iter.first;
								frontInangle = tempInangle;
								firstVehSpeedfrontHvInTargetLane = iter.second.GetPosition().GetSpeed();
							}
						}
						else
						{
							if(tempDist < minbehindDist)
							{
								minbehindDist = tempDist;
								firstVehIdbehindHvInTargetLane = iter.first;
								behindInangle = tempInangle;
								firstVehSpeedbehindHvInTargetLane = iter.second.GetPosition().GetSpeed();
								firstVehbehindHvInTargetLaneisV2XType = iter.second.GetIsV2XVehicleFlag();

							}
						}
					}
				}

				base::MathUtil::getSinineDist(minfrontDist, frontInangle, frontLateralDist, 90 - frontInangle, minfrontLongitudinalDist);
				base::MathUtil::getSinineDist(minbehindDist, 180 - behindInangle, behindLateralDist, behindInangle - 90, minbehindLongitudinalDist);

				bool collisionToFirstFront = false;
				bool collisionToFirstBehind = false;


				if(firstVehSpeedfrontHvInTargetLane < hvSpeed)
				{
					int temp_ttc = minfrontLongitudinalDist/(hvSpeed - firstVehSpeedfrontHvInTargetLane);
					if(temp_ttc < max_ttc)//追尾风险
					{
						collisionToFirstFront = true;
					}

				}

				if(firstVehSpeedbehindHvInTargetLane > hvSpeed)
				{
					int temp_ttc = minbehindLongitudinalDist/(firstVehSpeedbehindHvInTargetLane - hvSpeed);
					if(temp_ttc < max_ttc)//追尾风险
					{
						collisionToFirstBehind = true;
					}

				}


				if(dist2end <= 5)//与合流区终点距离小于5m
				{
					if(collisionToFirstFront || collisionToFirstBehind)//目标车道有前向/后向碰撞风险
					{
						transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_notsafe, DriveBehavior::DriveBehavior_rampIn);
					}
					else
					{
						transmitRSC(hvId, reqId, ReqStatus::ReqStatus_accept, Reason::Reason_unknown, DriveBehavior::DriveBehavior_rampIn);
					}
				}
				else
				{
					if(minfrontLongitudinalDist < safty_area_length || minfrontLongitudinalDist < safty_area_length)//危险区域有车,向rv发送协作,同时拒绝hv请求
					{
						transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_notsafe, DriveBehavior::DriveBehavior_rampIn);
//						if(collisionToFirstFront)//目标车道fcw，向目标车道前方第一辆rv发送加速请求
//						{
//							transmitRSC(firstVehIdfrontHvInTargetLane, reqId, 1, 0, 12);
//						}

						if(collisionToFirstBehind)//目标车道bcw，向目标车道后方第一辆rv发送减速请求
						{
							rii.rv_list[firstVehIdbehindHvInTargetLane] = 0;
							transmitRSC(firstVehIdbehindHvInTargetLane, reqId, ReqStatus::ReqStatus_request, Reason::Reason_unknown, DriveBehavior::DriveBehavior_slow_down);
						}
					}
					else//危险区域无车，同意hv请求
					{
						transmitRSC(hvId, reqId, ReqStatus::ReqStatus_accept, Reason::Reason_unknown, DriveBehavior::DriveBehavior_rampIn);
					}

				}
			}

		}
		guide_list[hvId] = rii;
		remote_vehs.UnlockRemoteVehiclesMap();
	}
	else if(status == 5)//处于执行状态
	{
		if(guide_list.find(hvId) == guide_list.end())
		{
			std::cout << "vehicle id not found in guide_list" <<std::endl;
			return;
		}

		guide_list[hvId].status = 5;
	}
	else if(status == 3)//请求取消
	{
		guide_list.erase(hvId);
	}
	else if(status == 4)//请求完成
	{
		guide_list.erase(hvId);
	}





}


void CooperationRampIn::transmitRSC(std::string vehId, int reqID, ReqStatus status, Reason reason, DriveBehavior suggestBehavior)
{
	auto asn_pb = std::make_shared<v2xpb::asn::MessageFrame>();
	auto msgext = asn_pb->mutable_extframe();
	auto msgvalue = msgext->mutable_messagevalue();

	msgvalue->set_typresent(MessageFrameExt__value::MessageFrameExt__value_PR_RoadsideCoordination);
	auto rsc = msgvalue->mutable_roadsidecoordination();

	rsc_count_ = (rsc_count_ >= 127) ? 1 : (rsc_count_ + 1);
	rsc->set_msgcnt(rsc_count_);
	rsc->set_id(rsu_id);
	rsc->set_secmark(get_mill_second_minute());
	auto ref_llh = rsc->mutable_refpos()->mutable_llh();
	ref_llh->set_latitude(lat);
	ref_llh->set_longitude(lon);

    auto veh_coordination = rsc->add_coordinates();
    veh_coordination->set_vehid(vehId);
    veh_coordination->set_reqid(reqID);
    veh_coordination->set_status(status);

    auto suggestion = veh_coordination->mutable_drivesuggestion();
    suggestion->set_suggestion(suggestBehavior);
    veh_coordination->set_info(CoordinationInfo::CoordinationInfo_cooperativeVehMerging);

    if(status == 7)
    {
    	veh_coordination->set_rejectreason(reason);
    }

    auto message_pb = std::make_shared<airos::app::ApplicationData>();
    message_pb->mutable_road_side_frame()->operator=(*asn_pb);
    send_(message_pb);

}

int64_t CooperationRampIn::get_mill_second_minute() {
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0) {
		return -1;
	}
	struct tm* t     = nullptr;
	time_t startTime = time(0);

	struct tm buf = {};
	localtime_r(&startTime, &buf);
	t = &buf;

	if (t == nullptr) {
		return -1;
	}
	return (t->tm_sec * 1000 + tv.tv_usec / 1000);
}

bool CooperationRampIn::onHvFront(double hlat, double hlon, double hheading, double rlat, double rlon, double &inangle)
{
	double angle = base::MathUtil::getAzimuth(hlat, hlon, rlat, rlon);
	inangle = base::MathUtil::getInAngle(hheading, angle);

    if(inangle > 90)
    {
    	return false;
    }

    return true;

}

bool CooperationRampIn::isEmergencyVeh(int type)
{
	bool isEmerge = false;
	switch(type)
	{
	case 60:
		isEmerge = true;
		break;
	default:
		break;
	}

	return isEmerge;
}




}
}
