/*
 * PositionModelExt.cpp
 *
 *  Created on: 2018年6月28日
 *      Author: catt
 */

#include "PositionModelExt.h"

namespace airos {
namespace app {
PositionModelExt::PositionModelExt(std::initializer_list<double> ini)
{
    switch(ini.size())
    {
    case 5: setHeading(*(ini.begin() + 4));
    case 4: setSpeed(*(ini.begin() + 3));
    case 3: setElevation(*(ini.begin() + 2));
    case 2: setLatitude(*(ini.begin() + 1));
    case 1: setLongitude(*ini.begin());
    default:break;
    }
}

std::string PositionModelExt::to_string() const
{
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(7);
    ss << "longitude:" << m_Longitude << " latitude:" << m_Latitude;

    ss.precision(3);
    if(isElevationValid())
        ss << " altitude:" << m_Elevation;

    if(isSpeedValid())
        ss << " speed:" << m_Speed;

    if(isHeadingValid())
        ss << " heading:" << m_Heading;

    return ss.str();
}

std::string PositionModelExt::to_string()
{
    std::stringstream ss;
    ss.setf(std::ios::fixed);
    ss.precision(7);
    ss << "longitude:" << m_Longitude << " latitude:" << m_Latitude;

    ss.precision(3);
    if(isElevationValid())
        ss << " altitude:" << m_Elevation;

    if(isSpeedValid())
        ss << " speed:" << m_Speed;

    if(isHeadingValid())
        ss << " heading:" << m_Heading;

    return ss.str();
}


} }
