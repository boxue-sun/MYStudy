/*
 * CoarseScale.cpp
 *
 *  Created on: 2018年5月22日
 *      Author: catt
 */

#include "CoarseScale.h"
#include "PathSegment.h"
#include "base/common/math_util.h"

namespace airos {
namespace app {

CoarseScale::CoarseScale(const PositionModelExt& refPos)
{
    m_LeftTop = m_RightBottom = refPos;
}

CoarseScale::CoarseScale(std::vector<PathSegmentPtr>& m_PathSegment, double laneWidth)
{
    if(!m_PathSegment.empty())
    {
        *this = *(m_PathSegment[0]->m_CoarseScale);
        for(decltype(m_PathSegment.size()) i = 1; i < m_PathSegment.size(); i++)
        {
            *this  += *(m_PathSegment[i]->m_CoarseScale);
        }
    }
}

CoarseScale::CoarseScale(const PositionModelExt& pos, double width, double length)
{
    m_LeftTop = pos;
    m_LeftTop.setHeading(0);
    PathSegment::getNextPosition(m_LeftTop, length / 2);
    m_LeftTop.setHeading(270);
    PathSegment::getNextPosition(m_LeftTop, width / 2);


    m_RightBottom = pos;
    m_RightBottom.setHeading(180);
    PathSegment::getNextPosition(m_RightBottom, width / 2);
    m_RightBottom.setHeading(90);
    PathSegment::getNextPosition(m_RightBottom, length / 2);
}

CoarseScale::CoarseScale(double lt_lon, double lt_lat, double rb_lon, double rb_lat)
{
    m_LeftTop.setLongitude(lt_lon);
    m_LeftTop.setLatitude(lt_lat);

    m_RightBottom.setLongitude(rb_lon);
    m_RightBottom.setLatitude(rb_lat);
}

CoarseScale::CoarseScale(const CoarseScale& cs)
{
    m_LeftTop.setLongitude(cs.m_LeftTop.getLongitude());
    m_LeftTop.setLatitude(cs.m_LeftTop.getLatitude());
    m_RightBottom.setLongitude(cs.m_RightBottom.getLongitude());
    m_RightBottom.setLatitude(cs.m_RightBottom.getLatitude());
}

bool CoarseScale::AtThisScale(const PositionModelExt& pos)
{
    return  (  pos.getLongitude() >= m_LeftTop.getLongitude()
            && pos.getLongitude() <= m_RightBottom.getLongitude()
            && pos.getLatitude() <= m_LeftTop.getLatitude()
            && pos.getLatitude() >= m_RightBottom.getLatitude() );
}

CoarseScale& CoarseScale::operator += (const CoarseScale& cs)
{
    m_LeftTop.setLongitude( std::min(this->m_LeftTop.getLongitude(), cs.m_LeftTop.getLongitude()) );
    m_LeftTop.setLatitude( std::max(this->m_LeftTop.getLatitude(), cs.m_LeftTop.getLatitude()) );

    m_RightBottom.setLongitude( std::max(this->m_RightBottom.getLongitude(), cs.m_RightBottom.getLongitude()) );
    m_RightBottom.setLatitude( std::min(this->m_RightBottom.getLatitude(), cs.m_RightBottom.getLatitude()) );

    return *this;
}

bool CoarseScale::OverLap(const CoarseScale& cs) const
{
    return ( this->m_RightBottom.getLatitude() < cs.m_LeftTop.getLatitude()
            && this->m_LeftTop.getLatitude() > cs.m_RightBottom.getLatitude()
            && this->m_RightBottom.getLongitude() > cs.m_LeftTop.getLongitude()
            && this->m_LeftTop.getLongitude() < cs.m_RightBottom.getLongitude() );
}

std::string CoarseScale::to_string()
{
    return std::string("LeftTop:").append(m_LeftTop.to_string()).append(" RightBottom:").append(m_RightBottom.to_string());
}

bool OverLap(const CoarseScale& cs1, const CoarseScale& cs2)
{
    return cs1.OverLap(cs2);
}


} }
