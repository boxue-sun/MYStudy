/*
 * cooperation_lanechange.cc
 *
 *  Created on: 2024年4月10日
 *      Author: hezhiyong
 */

#include "cooperation_lanechange.h"
#include "base/common/singleton.h"
#include "base/common/math_util.h"
#include "yaml-cpp/yaml.h"
#include "app/modules/v2x_scenario/common_module/remote_vehicles/remote_vehicles.h"
#include <sys/time.h>

//#include "app/modules/v2x_application/usecase/base/config_manager.h"

namespace airos {
namespace app {

using namespace v2xpb::asn;
CooperationLaneChange::CooperationLaneChange(){


}

CooperationLaneChange::~CooperationLaneChange()

{

}

bool CooperationLaneChange::Init(const airos::app::ApplicationCallBack& send_cb, const std::string& conf_path)
{
	std::cout << "lanechange init" << std::endl;
	send_ = send_cb;
	std::string lanechange_configs = conf_path + "/lanechange.yaml";

	YAML::Node root_node = YAML::LoadFile(lanechange_configs);
	if (!root_node.IsMap()) {
		std::cout << "cooperation_lanechange Init ConfigManager Failed!" << std::endl;
		return false;
	}

	rsu_id = root_node["rsu_id"].as<std::string>();
	max_service_dist = root_node["max_service_dist"].as<double>();
	max_ttc = root_node["max_ttc"].as<double>();
	safty_area_length = root_node["safty_area_length"].as<double>();
	lat = root_node["lat"].as<double>();
	lon = root_node["lon"].as<double>();

	return true;
}

void CooperationLaneChange::ProcV2xMsgRecv(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{
	if(!frame->has_extframe() || frame->extframe().messagevalue().typresent() != 8)//8：vir
	{
		return;
	}
	else
	{
		std::cout << "cooperation_lanechange recv VIR!" << std::endl;
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

void CooperationLaneChange::processHvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{
	auto hvir = frame->extframe().messagevalue().vehintentionandrequest();

	if(!hvir.intandreq().reqs(0).has_targetrsu() || hvir.intandreq().reqs(0).targetrsu() != rsu_id)
	{
		std::cout << "target rsu id error" <<std::endl;
		return;
	}

	if(!hvir.intandreq().reqs(0).has_info() || !hvir.intandreq().reqs(0).info().has_lanechange())
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

	//hv变道请求不在rsu服务范围内
	if(guide_list.find(hvId) == guide_list.end() && dist > max_service_dist && hvir.intandreq().reqs(0).has_info() && hvir.intandreq().reqs(0).info().has_lanechange())
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
		LaneChangeInfo lci;
		lci.status = status;
		lci.downstreamNode = hvir.intandreq().reqs(0).info().lanechange().downstreamnode().id();
		lci.upstreamNode = hvir.intandreq().reqs(0).info().lanechange().upstreamnode().id();
		lci.targetLane = hvir.intandreq().reqs(0).info().lanechange().targetlane();

		bool hastargetlane = hasTargetLane(lci.targetLane, lci.upstreamNode);
		if(!hastargetlane)
		{
			std::cout << "target lane not found" << std::endl;
			return;
		}

//			bool isBeside = isTargetLaneBeside(lci.targetLane, lci.upstreamNode);//判断目标车道是否为最侧边车道

		auto &remote_vehs = RemoteVehicles::GetInstance();
		remote_vehs.LockRemoteVehiclesMap();

		auto &vehs = remote_vehs.GetRemoteVehiclesMap();
		if(vehs.find(hvId) != vehs.end())
		{
			int current_laneid = vehs.at(hvId).GetTracker()->GetRefLaneId();
			if(abs(lci.targetLane - current_laneid) != 1 )
			{
				std::cout << "target lane too far" << std::endl;
				return;
			}

			DriveBehavior possible_turn = (lci.targetLane > current_laneid) ? DriveBehavior::DriveBehavior_laneChangingToRight : DriveBehavior::DriveBehavior_laneChangingToLeft;
			double hvLat = vehs.at(hvId).GetPosition().GetPositionLLH().latitude;
			double hvLon = vehs.at(hvId).GetPosition().GetPositionLLH().longitude;
			double hvHeading = vehs.at(hvId).GetPosition().GetHeading();
			double hvSpeed = vehs.at(hvId).GetPosition().GetSpeed();

			std::string firstVehIdfrontHvInTargetLane, firstVehIdbehindHvInTargetLane;//目标车道在hv前方/后方第一辆车的ID
			std::string firstVehIdbehindHvInThirdLane;//目标车道为非最侧车道时，目标车道的旁边车道hv后方第一辆车ID
			bool firstVehbehindHvInTargetLaneisV2XType;//目标车道在hv后方第一辆车是否为网联车
			bool firstVehbehindHvInThirdLaneisV2XType;//目标车道在hv后方第一辆车是否为网联车
			double firstVehSpeedfrontHvInTargetLane, firstVehSpeedbehindHvInTargetLane;//目标车道在hv前方/后方第一辆车的速度
			double minfrontDist = 200;//目标车道在hv前方第一辆车与hv的距离
			double minbehindDist = 200;//目标车道在hv后方第一辆车与hv的距离
			double minbehindDistThirdLaneDist = 200;//目标车道为非最侧车道时，hv与目标车道的旁边车道hv后方第一辆车的距离
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

				if(iter.second.GetTracker()->GetRefNodeId().id() == lci.downstreamNode
						&& iter.second.GetTracker()->GetRefLinkUpStreamNodeId().id() == lci.upstreamNode)
				{
					if(iter.second.GetTracker()->GetRefLaneId() == lci.targetLane)
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
					else if((possible_turn == DriveBehavior::DriveBehavior_laneChangingToRight && iter.second.GetTracker()->GetRefLaneId() == lci.targetLane + 1)
							|| (possible_turn == DriveBehavior::DriveBehavior_laneChangingToLeft && iter.second.GetTracker()->GetRefLaneId() == lci.targetLane - 1))
					{

						double tempDist = base::MathUtil::getDistance(hvLat, hvLon
								, iter.second.GetPosition().GetPositionLLH().latitude
								, iter.second.GetPosition().GetPositionLLH().longitude);

						double tempInangle;


						if(!onHvFront(hvLat, hvLon, hvHeading
								, iter.second.GetPosition().GetPositionLLH().latitude
								, iter.second.GetPosition().GetPositionLLH().longitude, tempInangle))
						{
							if(tempDist < minbehindDistThirdLaneDist)
							{
								firstVehIdbehindHvInThirdLane = iter.first;
								firstVehbehindHvInThirdLaneisV2XType = iter.second.GetIsV2XVehicleFlag();

							}
						}
					}
					else
					{

					}
				}
			}

			base::MathUtil::getSinineDist(minfrontDist, frontInangle, frontLateralDist, 90 - frontInangle, minfrontLongitudinalDist);
			base::MathUtil::getSinineDist(minbehindDist, 180 - behindInangle, behindLateralDist, frontInangle - 90, minfrontLongitudinalDist);

			//是否有车在危险区域内，暂时认为无车时无需发送协作，有车时拒绝请求，业务逻辑待完善
			if(minfrontLongitudinalDist < safty_area_length || minfrontLongitudinalDist < safty_area_length)
			{
				if(firstVehbehindHvInTargetLaneisV2XType)
				{
					lci.rv_list[firstVehIdbehindHvInTargetLane] = 0;
					if(firstVehSpeedbehindHvInTargetLane > hvSpeed)
					{
						int temp_ttc = minbehindLongitudinalDist/(firstVehSpeedbehindHvInTargetLane - hvSpeed);
						if(temp_ttc < max_ttc)//追尾风险
						{
							transmitRSC(firstVehIdbehindHvInTargetLane, reqId, ReqStatus::ReqStatus_request, Reason::Reason_unknown, DriveBehavior::DriveBehavior_slow_down);
						}

					}
					else
					{
						transmitRSC(firstVehIdbehindHvInTargetLane, reqId, ReqStatus::ReqStatus_request, Reason::Reason_unknown, DriveBehavior::DriveBehavior_goStraightForward);
					}
				}
				transmitRSC(hvId, reqId, ReqStatus::ReqStatus_reject, Reason::Reason_notsafe, possible_turn);
			}
			else
			{
				transmitRSC(hvId, reqId, ReqStatus::ReqStatus_accept, Reason::Reason_unknown, possible_turn);
			}

			if(firstVehbehindHvInThirdLaneisV2XType && firstVehIdbehindHvInThirdLane != "")//第三车道hv后方第一辆车建议直行
			{
				lci.rv_list[firstVehIdbehindHvInThirdLane] = 0;
				transmitRSC(firstVehIdbehindHvInThirdLane, reqId, ReqStatus::ReqStatus_request, Reason::Reason_unknown, DriveBehavior::DriveBehavior_goStraightForward);
			}


		}
		guide_list[hvId] = lci;
		remote_vehs.UnlockRemoteVehiclesMap();


	}
	else if(status == 5)//处于执行状态
	{
		if(guide_list.find(hvId) == guide_list.end())
		{
			std::cout << "vehicle id not found in guide_list" <<std::endl;
			return;
		}

		if(hvir.intandreq().reqs(0).info().lanechange().downstreamnode().id() != guide_list[hvId].downstreamNode
				|| hvir.intandreq().reqs(0).info().lanechange().upstreamnode().id() != guide_list[hvId].upstreamNode
				|| hvir.intandreq().reqs(0).info().lanechange().targetlane() != guide_list[hvId].targetLane)
		{
			std::cout << "veh reqinfo conflict" << std::endl;
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

void CooperationLaneChange::processRvVIR(const std::shared_ptr<const v2xpb::asn::MessageFrame>& frame)
{

}

void CooperationLaneChange::transmitRSC(std::string vehId, int reqID, ReqStatus status, Reason reason, DriveBehavior suggestBehavior)
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
    veh_coordination->set_info(CoordinationInfo::CoordinationInfo_cooperativeLaneChanging);

    if(status == 7)
    {
    	veh_coordination->set_rejectreason(reason);
    }

    auto message_pb = std::make_shared<airos::app::ApplicationData>();
    message_pb->mutable_road_side_frame()->operator=(*asn_pb);
    send_(message_pb);

}

int64_t CooperationLaneChange::get_mill_second_minute() {
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

bool CooperationLaneChange::hasTargetLane(int targetLane, int upstreamNode)
{
	auto local_maps = LocalMaps::GetInstance().GetLocalMap();
	for(int i = 0 ; i < local_maps.nodes(0).links_size(); i++)
	{
		if(local_maps.nodes(0).links(i).upstream_node_id().id() == upstreamNode)
		{
			if(local_maps.nodes(0).links(i).lanes(0).id() <= targetLane
					&& local_maps.nodes(0).links(i).lanes(local_maps.nodes(0).links(i).lanes_size() - 1).id() >= targetLane)
			{
				return true;
			}
		}
	}
	return false;
}

//bool CooperationLaneChange::isTargetLaneBeside(int targetLane, int upstreamNode)
//{
//	auto local_maps = LocalMaps::GetInstance().GetLocalMap();
//	for(int i = 0 ; i < local_maps.nodes(0).links_size() ; i++)
//	{
//		if(local_maps.nodes(0).links[i].upstream_node_id == upstreamNode)
//		{
//			if(local_maps.nodes(0).links[i].lanes[0].id() == targetLane
//					|| local_maps.nodes(0).links[i].lanes[local_maps.nodes(0).links[i].lanes_size() - 1].id() == targetLane)
//			{
//				return true;
//			}
//		}
//	}
//	return false;
//}

bool CooperationLaneChange::onHvFront(double hlat, double hlon, double hheading, double rlat, double rlon, double &inangle)
{
	double angle = base::MathUtil::getAzimuth(hlat, hlon, rlat, rlon);
	inangle = base::MathUtil::getInAngle(hheading, angle);

    if(inangle > 90)
    {
    	return false;
    }

    return true;

}

void CooperationLaneChange::checkGuideList()
{


}

bool CooperationLaneChange::isEmergencyVeh(int type)
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
