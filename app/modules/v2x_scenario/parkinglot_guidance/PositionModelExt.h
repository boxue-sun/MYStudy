/*
 * Position.h
 *
 *  Created on: 2018��3��26��
 *      Author: chewenyao
 */

#ifndef DATAMODEL_POSITIONMODELEXT_H_
#define DATAMODEL_POSITIONMODELEXT_H_


#include <string>
#include <sstream>
#include <bitset>

namespace airos {
namespace app {
class PositionModelExt
{
public:
    PositionModelExt() = default;
    PositionModelExt(std::initializer_list<double> ini);
    std::string to_string();
    std::string to_string() const;

    double getLongitude() const
    {
        return m_Longitude;
    }

    void setLongitude(double longitude)
    {
        m_Valid[0] = 1;
        m_Longitude = longitude;
    }

    double getLatitude() const
    {
        return m_Latitude;
    }

    void setLatitude(double latitude)
    {
        m_Valid[1] = 1;
        m_Latitude = latitude;
    }

    double getElevation() const
    {
        return m_Elevation;
    }

    void setElevation(double elevation)
    {
        m_Valid[2] = 1;
        m_Elevation = elevation;
    }

    double getHeading() const
    {
        return m_Heading;
    }

    void setHeading(double heading)
    {
        m_Valid[3] = 1;
        m_Heading = heading;
    }
    double getLength() const
	{
		return m_VehL;
	}

	void setLength(double len)
	{
		m_Valid[5] = 1;
		m_VehL = len;
	}
    double getWidth() const
	{
		return m_VehW;
	}
	void setWidth(double wid)
	{
		m_Valid[6] = 1;
		m_VehW = wid;
	}
    double getHeight() const
	{
		return m_VehH;
	}
	void setHeight(double hgt)
	{
		m_Valid[7] = 1;
		m_VehL = hgt;
	}
    double getSpeed() const
    {
        return m_Speed;
    }
    double getVehType() const
   {
	   return m_VehType;
    }

    void setSpeed(double speed)
    {
        m_Valid[4] = 1;
        m_Speed = speed;
    }

    bool isLongitudeValid() const
    {
        return m_Valid[0];
    }

    bool isLatitudeValid() const
    {
        return m_Valid[1];
    }

    bool isElevationValid() const
    {
        return m_Valid[2];
    }

    bool isSpeedValid() const
    {
        return m_Valid[3];
    }

    bool isHeadingValid() const
    {
        return m_Valid[4];
    }

    void reset()
    {
        m_Valid.set(0);
    }

    double getLatAcc() const
	{
		return m_LatAcc;
	}
	void setLatAcc(double latAcc)
	{
		m_LatAcc = latAcc;
	}
    double getLongAcc() const
	{
		return m_LongAcc;
	}
	void setLongAcc(double longAcc)
	{
		m_LongAcc = longAcc;
	}
	double getVertAcc() const
	{
		return m_VertAcc;
	}
	void setVertAcc(double vertAcc)
	{
		m_VertAcc = vertAcc;
	}
	double getYawRate() const
	{
		return m_YawRate;
	}
	void setYawRate(double yawRate)
	{
		m_YawRate = yawRate;
	}

private:
	double m_Longitude = 0;
	double m_Latitude = 0;
	double m_Elevation = 0;
	double m_Speed = 0;
	double m_Heading = 0;
	double m_VehType=0;
	double m_VehL = 0;
	double m_VehW = 0;
	double m_VehH = 0;

	double m_LatAcc = 0;	//横向加速度，向右为正,m/s2
	double m_LongAcc = 0;	//纵向加速度，向前加速为正,m/s2
	double m_VertAcc = 0;	//垂直加速度，沿重力方向向下为正,m/s2
	double m_YawRate = 0;	//横摆角速度，顺时针旋转为正,°/s

	std::bitset<8> m_Valid; //Mod by heling 2020/4/27
};

} }

#endif /* DATAMODEL_PositionModelExt_H_ */
