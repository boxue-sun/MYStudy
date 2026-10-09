/*
 * PathSegment.cpp
 *
 *  Created on: 2018年5月21日
 *      Author: catt
 */

#include "PathSegment.h"
#include "CoarseScale.h"
#include "base/common/math_util.h"

namespace airos {
namespace app {
const double PathSegment::k_OnPathDegThresHold = 90;
const double PathSegment::k_PathMatchDegCoff  = 0.4;

const double PathSegment::k_OnPathDistThresHold = 50;
const double PathSegment::k_PathMatchDistCoff = 0.6;

PathSegment::PathSegment(const PositionModelExt& src, const PositionModelExt& dst, double width)
    : m_Src(src)
    , m_Dst(dst)
    , m_Width(width)
{
    // TODO Auto-generated constructor stub
    m_AccPathLength = 0;

    if(m_Width < 0 || m_Width > 3.75)
        m_Width = 3.75;

    m_PathLength = base::MathUtil::getDistance(src.getLatitude(), src.getLongitude(), dst.getLatitude(), dst.getLongitude());
    m_Heading = base::MathUtil::getAzimuth(src.getLatitude(), src.getLongitude(), dst.getLatitude(), dst.getLongitude());

    //
    PositionModelExt pmSrcLeft = src;
    pmSrcLeft.setHeading(base::MathUtil::getValidDegAngle(m_Heading - 90));
    getNextPosition(pmSrcLeft, width/2);

    PositionModelExt pmSrcRight = src;
    pmSrcRight.setHeading(base::MathUtil::getValidDegAngle(m_Heading + 90));
    getNextPosition(pmSrcRight, width/2);

    PositionModelExt pmDstLeft = dst;
    pmDstLeft.setHeading(base::MathUtil::getValidDegAngle(m_Heading-90));
    getNextPosition(pmDstLeft, width/2);

    PositionModelExt pmDstRight = dst;
    pmDstRight.setHeading(base::MathUtil::getValidDegAngle(m_Heading + 90));
    getNextPosition(pmDstRight, width/2);


/*
m_CoarseScale.reset(
        new CoarseScale(
            std::min(m_Src.getLongitude(), m_Dst.getLongitude())
            ,std::max(m_Src.getLatitude(), m_Dst.getLatitude())
            ,std::max(m_Src.getLongitude(), m_Dst.getLongitude())
            ,std::min(m_Src.getLatitude(), m_Dst.getLatitude())
        )
    );
*/

    m_CoarseScale.reset(
        new CoarseScale(
            std::min(std::min(pmSrcLeft.getLongitude(), pmSrcRight.getLongitude()), std::min(pmDstLeft.getLongitude(), pmDstRight.getLongitude()))
            ,std::max(std::max(pmSrcLeft.getLatitude(), pmSrcRight.getLatitude()), std::max(pmDstLeft.getLatitude(), pmDstRight.getLatitude()))
            ,std::max(std::max(pmSrcLeft.getLongitude(), pmSrcRight.getLongitude()), std::max(pmDstLeft.getLongitude(), pmDstRight.getLongitude()))
            ,std::min(std::min(pmSrcLeft.getLatitude(), pmSrcRight.getLatitude()), std::min(pmDstLeft.getLatitude(), pmDstRight.getLatitude()))
        )
    );
}

PathSegment::PathSegment(const PathSegment& ps)
{
    this->m_Src = ps.m_Src;
    this->m_Dst = ps.m_Dst;

    this->m_PathLength = ps.m_PathLength;
    this->m_AccPathLength = ps.m_AccPathLength;

    this->m_Width = ps.m_Width;
    this->m_Heading = ps.m_Heading;

    this->m_CoarseScale.reset(ps.m_CoarseScale.get());
}

PathSegment::~PathSegment()
{
    // TODO Auto-generated destructor stub
}

PathSegment::MatchResult PathSegment::PathMatchWithParam(const PositionModelExt& pos
        , double onPathDegThresHold
        , double pathMatchDegCoff
        , double onPathDistThresHold
        , double pathMatchDistCoff)
{
    MatchResult retval;

    double vd, md;
    PositionModelExt footPoint = pos;
    if(!getMinDistance(m_Src, m_Dst, footPoint, vd, md))
    {
        retval.m_OverDest = (base::MathUtil::getDistance(m_Dst.getLatitude(), m_Dst.getLongitude(), footPoint.getLatitude(), footPoint.getLongitude()) < 0.00001);
    }
    retval.m_VertDist = vd;
    double deg = base::MathUtil::getInAngle(pos.getHeading(), m_Heading);
    if(pos.getSpeed() <= 1.3888)
    {
        deg = 0;
    }
    /* too far away */
    if(md < onPathDistThresHold && deg < onPathDegThresHold)
    {
        double esti_dist = (onPathDistThresHold - md) / onPathDistThresHold;
        double esti_deg = (onPathDegThresHold - deg) / onPathDegThresHold;
        retval.m_Estimate = esti_dist*pathMatchDistCoff + esti_deg*pathMatchDegCoff;

        /* wrong algorithm*/
/*        if( NCSMath::getValidDegAngle( NCSMath::getAzimuth(m_Src, pos) - NCSMath::getAzimuth(m_Src, m_Dst) ) > 180 )
        {
            retval.m_VertDist = -retval.m_VertDist;
        }
*/
    }

    return retval;
}

PathSegment::MatchResult PathSegment::PathMatch(const PositionModelExt& pos)
{
    if(pos.getHeading() < 0 || pos.getHeading() > 360)
        return PathMatchWithParam(pos, 360, 0, k_OnPathDistThresHold, 1);

    return PathMatchWithParam(pos, k_OnPathDegThresHold, k_PathMatchDegCoff,
            k_OnPathDistThresHold, k_PathMatchDistCoff);
}

double PathSegment::getRemainDist(const PositionModelExt& pos)
{
    return base::MathUtil::getDistance(pos.getLatitude(), pos.getLongitude(), m_Dst.getLatitude(), m_Dst.getLongitude());
}

std::string PathSegment::to_string()
{
    std::stringstream ss;
    ss << " Length:" << m_PathLength << "m AccLength:" << m_AccPathLength <<"m width:" << m_Width << "m heading:" << m_Heading;

    return std::string("src_pos:").append(m_Src.to_string()).append(" dst_pos:").append(m_Dst.to_string()).append(ss.str());
}

bool operator < (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2)
{
    return mr1.m_Estimate < mr2.m_Estimate;
}

bool operator == (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2)
{
    return std::fabs(mr1.m_Estimate - mr2.m_Estimate) < 0.0000001;
}

bool operator > (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2)
{
    return mr1.m_Estimate > mr2.m_Estimate;
}

void PathSegment::getNextPosition(PositionModelExt& node, double dist)
{
    double lat = base::MathUtil::convertDeg2Rad(node.getLatitude());
    double hea = base::MathUtil::convertDeg2Rad(node.getHeading());
    double rad = dist/base::MathUtil::getEarthRadius(node.getLatitude());
    double ret = sin(lat)*cos(rad)+cos(lat)*sin(rad)*cos(hea);
    ret = acos(ret);

    node.setLatitude(90 - base::MathUtil::convertDeg2Rad(ret));
    node.setLongitude(node.getLongitude() + (base::MathUtil::convertDeg2Rad(asin(sin(rad)*sin(hea)/sin(ret)))));
}

bool PathSegment::getMinDistance(const PositionModelExt& p1, const PositionModelExt& p2, PositionModelExt& node, double& vd, double& md)
{
    bool ret = false;
    const PositionModelExt backup = node;

    double o2n_a = base::MathUtil::getAzimuth(p1.getLatitude(), p1.getLongitude(), p2.getLatitude(), p2.getLongitude());
    double n2o_a = base::MathUtil::getValidDegAngle(o2n_a + 180.0);

    double o2h_a = base::MathUtil::getAzimuth(p1.getLatitude(), p1.getLongitude(), node.getLatitude(), node.getLongitude());
    double n2h_a = base::MathUtil::getAzimuth(p2.getLatitude(), p2.getLongitude(), node.getLatitude(), node.getLongitude());

    double noh_a = base::MathUtil::getInAngle(o2n_a, o2h_a);
    double onh_a = base::MathUtil::getInAngle(n2o_a, n2h_a);

    double o2h_l = base::MathUtil::getDistance(p1.getLatitude(), p1.getLongitude(), node.getLatitude(), node.getLongitude());
//	vd = o2h_l*sin(convertDeg2Rad(noh_a));
    vd = o2h_l*sin(base::MathUtil::convertDeg2Rad(base::MathUtil::getValidDegAngle(o2h_a)-base::MathUtil::getValidDegAngle(o2n_a)));

    double anothdist = o2h_l*cos(base::MathUtil::convertDeg2Rad(noh_a)); 
    node = p1;
    node.setHeading(o2n_a);
    getNextPosition(node, anothdist);
    
    if(noh_a > 90)
    {
        md = base::MathUtil::getDistance(backup.getLatitude(), backup.getLongitude(), p1.getLatitude(), p1.getLongitude());
        node = p1;
    }
    else if(onh_a > 90)
    {
        md = base::MathUtil::getDistance(backup.getLatitude(), backup.getLongitude(), p2.getLatitude(), p2.getLongitude());
        node = p2;
    }
    else
    {
        md = fabs(vd);
        ret = true;
    }
    
    /* restore heading value*/
    node.setHeading(backup.getHeading());
    return ret;
}

} }
