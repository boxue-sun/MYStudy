/*
 * @Author: lht
 * @Date: 2024-01-23 16:38:21
 * @LastEditors: lht
 * @LastEditTime: 2024-01-24 14:11:13
 * @Description: math util for common convert
 * @FilePath: /airos-edge/base/common/math_util.cc
 */

#include <time.h>
#include <algorithm>
#include "math_util.h"

namespace airos
{
namespace base
{
bool MathUtil::outOfChina(double lat, double lon)
{
    if (lon < 72.004 || lon > 137.8347)
        return true;
    if (lat < 0.8293 || lat > 55.8271)
        return true;
    return false;
}

double MathUtil::transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * PI) + 40.0 * sin(y / 3.0 * PI)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * PI) + 320 * sin(y * PI / 30.0)) * 2.0 / 3.0;
    return ret;
}

double MathUtil::transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * PI) + 20.0 * sin(2.0 * x * PI)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * PI) + 40.0 * sin(x / 3.0 * PI)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * PI) + 300.0 * sin(x / 30.0 * PI)) * 2.0 / 3.0;
    return ret;
}

void MathUtil::gps84ToGcj02(double lat, double lon, double *mgLon, double *mgLat)
{
    if (outOfChina(lat, lon))
    {
        *mgLon = lon;
        *mgLat = lat;
        return;
    }
    double dLat = transformLat(lon - 105.0, lat - 35.0);
    double dLon = transformLon(lon - 105.0, lat - 35.0);

    double radLat = lat / 180.0 * PI;
    double magic = sin(radLat);
    magic = 1 - EE * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((A * (1 - EE)) / (magic * sqrtMagic) * PI);
    dLon = (dLon * 180.0) / (A / sqrtMagic * cos(radLat) * PI);
    *mgLat = lat + dLat;
    *mgLon = lon + dLon;
}

double MathUtil::getValidDegAngle(double Deg)
{
    double retval = Deg;
    while (retval < 0)
    {
        retval += AIROS_FULL_DEG;
    }
    while (retval >= AIROS_FULL_DEG)
    {
        retval -= AIROS_FULL_DEG;
    }

    return retval;
}

double MathUtil::getInAngle(double hea1, double hea2)
{
    double ret;

    getValidDegAngle(hea1);
    getValidDegAngle(hea2);

    ret = fabs(hea2 - hea1);
    ret = (ret > AIROS_SEMI_DEG) ? (AIROS_FULL_DEG - ret) : ret;

    return ret;
}

double MathUtil::convertDeg2Rad(double Deg)
{
    return (Deg / AIROS_SEMI_DEG * PI);
}

double MathUtil::convertRad2Deg(double Rad)
{
    return (Rad * AIROS_SEMI_DEG / PI);
}

bool MathUtil::getSinineDist(double dist_c, double angle_a, double &dist_a, double angle_b, double &dist_b)
{
    double angle_c = AIROS_SEMI_DEG - angle_a - angle_b;

    if (angle_c <= 0 || angle_c >= AIROS_SEMI_DEG)
    {
        return false;
    }
    angle_a = convertDeg2Rad(angle_a);
    angle_b = convertDeg2Rad(angle_b);
    angle_c = convertDeg2Rad(angle_c);

    dist_a = sin(angle_a) * dist_c / sin(angle_c);
    dist_b = sin(angle_b) * dist_c / sin(angle_c);

    return true;
}

double MathUtil::getEarthRadius(double deglat)
{
    return AIROS_EARTH_POLAR_RADIUS + (AIROS_EARTH_EQUAT_RADIUS - AIROS_EARTH_POLAR_RADIUS) * (90 - deglat) / 90;
}

double MathUtil::convertM2CM(double m)
{
    return m * 100.0;
}

double MathUtil::convertCM2M(double cm)
{
    return cm / 100.0;
}

double MathUtil::convertDegLatLonI2F(int32_t latlon)
{
    return latlon * AIROS_LATLON_UNIT;
}

int32_t MathUtil::convertDegLatLonF2I(double latlon)
{
    return latlon / AIROS_LATLON_UNIT;
}

double MathUtil::convertRadLatLonI2F(int32_t latlon)
{
    return convertDeg2Rad(latlon * AIROS_LATLON_UNIT);
}

int32_t MathUtil::convertRadLatLonF2I(double latlon)
{
    return convertRad2Deg(latlon) / AIROS_LATLON_UNIT;
}

double MathUtil::convertEleI2F(int32_t ele)
{
    return ele * AIROS_ELE_UNIT;
}

int32_t MathUtil::convertEleF2I(double ele)
{
    return ele / AIROS_ELE_UNIT;
}

double MathUtil::convertSpeedI2F(int32_t speed)
{
    return speed * AIROS_SPEED_UNIT;
}

int32_t MathUtil::convertSpeedF2I(double speed)
{
    return speed / AIROS_SPEED_UNIT;
}

double MathUtil::convertRadHeadingI2F(int32_t heading)
{
    return convertDeg2Rad(heading * AIROS_HEADING_UNIT);
}

int32_t MathUtil::convertRadHeadingF2I(double heading)
{
    return convertRad2Deg(heading) / AIROS_HEADING_UNIT;
}

double MathUtil::convertWidthI2F(int32_t width)
{
    return width * AIROS_WIDTH_UNIT;
}

int32_t MathUtil::convertWidthF2I(double width)
{
    int32_t w = round(width / AIROS_WIDTH_UNIT);
    if (w < 0)
    {
        w = 0; // 无效值
    } else if (w > 1023)
    {
        w = 1023;
    }
    return w;
}

double MathUtil::convertLengthI2F(int32_t length)
{
    return length * AIROS_LENGTH_UNIT;
}

int32_t MathUtil::convertLengthF2I(double length)
{
    int32_t l = round(length / AIROS_LENGTH_UNIT);

    if (l < 0)
    {
        l = 0; // 无效值
    } else if (l > 4095)
    {
        l = 4095;
    }

    return l;
}

double MathUtil::convertHeightI2F(int32_t height)
{
    return height * AIROS_HEIGTH_UNIT;
}

int32_t MathUtil::convertHeightF2I(double height)
{
    int32_t h = round(height / AIROS_HEIGTH_UNIT);

    if (h < 0)
    {
        h = 0; // 无效值
    } else if (h > 127)
    {
        h = 127;
    }

    return h;
}

double MathUtil::convertAccXI2F(int32_t accx)
{
    if (accx == AIROS_INVALID_ACCELERATION)
    {
        return INT_MAX;
    }
    return accx * AIROS_YAWRATE_UNIT;
}

int32_t MathUtil::convertAccXF2I(double accx)
{
    if (accx == INT_MAX)
    {
        return AIROS_INVALID_ACCELERATION;
    }
    if (accx > 20)
    {
        return 2000;
    } else if (accx < -20)
    {
        return -2000;
    }
    return accx / AIROS_YAWRATE_UNIT;
}

double MathUtil::convertAccYI2F(int32_t accy)
{
    if (accy == AIROS_INVALID_ACCELERATION)
    {
        return INT_MAX;
    }
    return accy * AIROS_YAWRATE_UNIT;
}

int32_t MathUtil::convertAccYF2I(double accy)
{
    if (accy == INT_MAX)
    {
        return AIROS_INVALID_ACCELERATION;
    }
    if (accy > 20)
    {
        return 2000;
    } else if (accy < -20)
    {
        return -2000;
    }
    return accy / AIROS_YAWRATE_UNIT;
}

double MathUtil::convertAccZI2F(int32_t accz)
{
    if (accz == AIROS_INVALID_VETICALACCELERATION)
    {
        return INT_MAX;
    }
    return accz * AIROS_ACCELERATION_UNIT;
}

int32_t MathUtil::convertAccZF2I(double accz)
{
    if (accz == INT_MAX)
    {
        return AIROS_INVALID_VETICALACCELERATION;
    }
    if (accz >= 25.4)
    {
        return 127;
    } else if (accz <= -25.2)
    {
        return -126;
    }
    return accz / AIROS_ACCELERATION_UNIT;
}

double MathUtil::convertYawRateI2F(int32_t yawrate)
{
    return yawrate * AIROS_YAWRATE_UNIT;
}

int32_t MathUtil::convertYawRateF2I(double yawrate)
{
    if (yawrate == INT_MAX)
    {
        return 0;
    }
    return yawrate / AIROS_YAWRATE_UNIT;
}

double MathUtil::convertSemiMajorI2F(int32_t value)
{
    return value * AIROS_AXISACCURACY_UNIT;
}

int32_t MathUtil::convertSemiMajorF2I(double value)
{
    if (value == INT_MAX)
    {
        return 255;
    }
    if (value >= 12.7)
    {
        return 254;
    }
    return value / AIROS_AXISACCURACY_UNIT;
}

double MathUtil::convertSemiMinorI2F(int32_t value)
{
    return value * AIROS_AXISACCURACY_UNIT;
}

int32_t MathUtil::convertSemiMinorF2I(double value)
{
    if (value == INT_MAX)
    {
        return 255;
    }
    if (value >= 12.7)
    {
        return 254;
    }
    return value / AIROS_AXISACCURACY_UNIT;
}

double MathUtil::convertSemiMajorOrientationI2F(int32_t value)
{
    return value * AIROS_AXISORIENTATION_UNIT;
}

int32_t MathUtil::convertSemiMajorOrientationF2I(double value)
{
    if (value == INT_MAX)
    {
        return 65535;
    }
    return value / AIROS_AXISACCURACY_UNIT;
}

int MathUtil::getRandomValue(int min, int max, const int &exclude)
{
    if (min > max)
        std::swap(min, max);

    unsigned int seed = 0;
    srand((unsigned) time(NULL) + seed++);

    int result = rand() % (max - min + 1) + min;
    if (result == exclude)
    {
        result = getRandomValue(min, max, exclude);
    }

    return result;
}

bool MathUtil::headingInArea(double host, double heading_a, double heading_b)
{
    if (heading_a > heading_b)
    {
        return (host >= heading_a || host <= heading_b);
    }
    return (host >= heading_a && host <= heading_b);
}

int32_t MathUtil::getMaxNum(int32_t m, int32_t n)
{
    return (n > m) ? n : m;
}

int32_t MathUtil::getMinNum(int32_t m, int32_t n)
{
    return (n < m) ? n : m;
}

double MathUtil::convertDegHeadingI2F(int32_t heading)
{
    return heading * AIROS_HEADING_UNIT;
}

int32_t MathUtil::convertDegHeadingF2I(double heading)
{
    return heading / AIROS_HEADING_UNIT;
}

double MathUtil::convertSpeedKMH2MS(double speed)
{
    return speed / AIROS_SPEED_KMH_MS_UNIT;
}

double MathUtil::convertSpeedMS2KMH(double speed)
{
    return speed * AIROS_SPEED_KMH_MS_UNIT;
}

double MathUtil::getDistance(double lat_x, double lon_x, double lat_y, double lon_y)
{
	if(lat_x == lat_y && lon_x == lon_y)
	{
		return 0;
	}

	double ret = (sin(lat_y)*sin(lat_x)) + (cos(lat_y)*cos(lat_x)*cos(lon_y-lon_x));
	// 保证 ret 在-1.0 ~ 1.0 之间
	ret = ret > 1.0 ? 1.0 : (ret < -1.0 ? -1.0 : ret);
	ret = acos(ret);
	double radius = getEarthRadius(lat_x);
	return (ret*radius);
}

double MathUtil::getAzimuth(double lat_x, double lon_x, double lat_y, double lon_y)
{
	double lat1 = convertDeg2Rad(lat_x);
	double lon1 = convertDeg2Rad(lon_x);
	double lat2 = convertDeg2Rad(lat_y);
	double lon2 = convertDeg2Rad(lon_y);

	double ret = (sin(lat2)*sin(lat1)) + (cos(lat2)*cos(lat1)*cos(lon2-lon1));

	ret = sqrt(1-(ret*ret));

	if(ret > 0.0000001 || ret < -0.0000001)
	{
		double tmpa = sin(lon2-lon1)*cos(lat2)/ret;

		tmpa = (tmpa > 1 )?  1 : tmpa;
		tmpa = (tmpa < -1)? -1 : tmpa;

		ret = asin(tmpa);
	}

	ret = convertRad2Deg(ret);

	if(lat2 >= lat1)
	{
		ret = (ret < 0) ? (ret + AIROS_FULL_DEG) : ret;
	}
	else
	{
		ret = AIROS_SEMI_DEG - ret;
	}

	return getValidDegAngle(ret);
}

} // namespace base
} // namespace airos
