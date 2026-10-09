/*
 * @Author: lht
 * @Date: 2024-01-23 16:36:47
 * @LastEditors: lht
 * @LastEditTime: 2024-01-24 14:11:50
 * @Description:
 * @FilePath: /airos-edge/base/common/math_util.h
 */

#pragma once

#include <cinttypes>
#include <cmath>
#include <math.h>
#include <limits.h>

namespace airos
{
    namespace base
    {
        class MathUtil
        {
        public:
            /**
             * @description: 84 to 火星坐标系 (GCJ-02) World Geodetic System ==> Mars Geodetic System
             * @param {double} lat
             * @param {double} lon
             * @param {double} *mgLon
             * @param {double} *mgLat
             * @return {*}
             */
            static void gps84ToGcj02(double lat, double lon, double *mgLon, double *mgLat);
            static bool outOfChina(double lat, double lon);
            static double transformLat(double x, double y);
            static double transformLon(double x, double y);

            /* 计算两个方向的夹角 */
            static double getInAngle(double hea1, double hea2);

            /**
             *
             * @param dist_c
             * @param angle_a
             * @param dist_a
             * @param angle_b
             * @param dist_b
             * @return
             */
            static bool getSinineDist(double dist_c, double angle_a, double &dist_a, double angle_b, double &dist_b);

            /**
             * 计算当前经纬度下的地球半径
             * @param deglat
             * @return
             */
            static double getEarthRadius(double deglat);

            /**
             *米转厘米
             * @param m
             * @return
             */
            static double convertM2CM(double m);
            static double convertCM2M(double cm);
            /**
             *
             * @param Deg
             * @return
             */
            static double convertDeg2Rad(double Deg);
            static double convertRad2Deg(double Rad);

            /**
             * 角度合法化到 0-360
             * @param Deg
             */
            static double getValidDegAngle(double Deg);

            /**
             * 经纬度转为整数 乘 10000000
             * @param latlon
             * @return
             */
            static double convertDegLatLonI2F(int32_t latlon);
            static int32_t convertDegLatLonF2I(double latlon);

            /**
             * 弧度制经纬度转为整数 乘 10000000
             * @param latlon
             * @return
             */
            static double convertRadLatLonI2F(int32_t latlon);
            static int32_t convertRadLatLonF2I(double latlon);

            /**
             * 海拔转换
             * @param ele
             * @return
             */
            static double convertEleI2F(int32_t ele);
            static int32_t convertEleF2I(double ele);

            /**
             * 速度转换
             * @param speed
             * @return
             */
            static double convertSpeedI2F(int32_t speed);
            static int32_t convertSpeedF2I(double speed);

            /**
             * 车身尺寸宽度转换(add by ChangXuhui)
             * @param width
             * @return
             */
            static double convertWidthI2F(int32_t width);
            static int32_t convertWidthF2I(double width);

            /**
             * 车身尺寸长度转换(add by ChangXuhui)
             * @param length
             * @return
             */
            static double convertLengthI2F(int32_t length);
            static int32_t convertLengthF2I(double length);

            /**
             * 车身尺寸高度转换(add by ChangXuhui)
             * @param height
             * @return
             */
            static double convertHeightI2F(int32_t height);
            static int32_t convertHeightF2I(double height);

            /**
             * 车头朝向
             * @param heading
             * @return
             */
            static double convertDegHeadingI2F(int32_t heading);
            static int32_t convertDegHeadingF2I(double heading);

            /**
             * 弧度制车头朝向
             * @param heading
             * @return
             */
            static double convertRadHeadingI2F(int32_t heading);
            static int32_t convertRadHeadingF2I(double heading);

            /**
             * @description: 四轴加速度转换
             * @param {int32_t} accx
             * @return {*}
             */
            static double convertAccXI2F(int32_t accx);
            static int32_t convertAccXF2I(double accx);

            static double convertAccYI2F(int32_t accy);
            static int32_t convertAccYF2I(double accy);

            static double convertAccZI2F(int32_t accz);
            static int32_t convertAccZF2I(double accz);

            static double convertYawRateI2F(int32_t yawrate);
            static int32_t convertYawRateF2I(double yawrate);

            static double convertSemiMajorI2F(int32_t value);
            static int32_t convertSemiMajorF2I(double value);

            static double convertSemiMinorI2F(int32_t value);
            static int32_t convertSemiMinorF2I(double value);

            static double convertSemiMajorOrientationI2F(int32_t value);
            static int32_t convertSemiMajorOrientationF2I(double value);

            /**
             * 获取一个随机数
             * @param min 随机数最小值
             * @param max 随机数最大值
             * @param exclude 产生的随机数不应为该值
             */
            static int getRandomValue(int min, int max, const int &exclude);

            /**
             * 判断本车方向角是否在给定方向角之间
             * */
            static bool headingInArea(double host, double heading_a, double heading_b);

            /**
             * 控制浮点数小数点后的位数
             * @param src 原始的浮点数
             * @param prec 保留小数点后面的位数
             * @return double 返回指定小数点位数的浮点数
             */
            static double controlDoublePrecision(double src, int prec = 2);

            static int32_t getMaxNum(int32_t m, int32_t n);
            static int32_t getMinNum(int32_t m, int32_t n);
			static double convertSpeedKMH2MS(double speed);
			static double convertSpeedMS2KMH(double speed);
			static double getDistance(double lat_x, double lon_x, double lat_y, double lon_y);
			static double getAzimuth(double lat_x, double lon_x, double lat_y, double lon_y);
        private:
            MathUtil(const MathUtil &) = delete;
            MathUtil &operator=(const MathUtil &) = delete;

        private:
            static constexpr  double AIROS_SPEED_KMH_MS_UNIT = 3.6;
            static constexpr  double AIROS_LATLON_UNIT = 0.0000001;
            static constexpr  double AIROS_ELE_UNIT = 0.1;
            static constexpr  double AIROS_SPEED_UNIT = 0.02;
            static constexpr  double AIROS_HEADING_UNIT = 0.0125;
            static constexpr  double AIROS_YAWRATE_UNIT = 0.01;
            static constexpr  double AIROS_ACCELERATION_UNIT = 0.2;
            static constexpr  double AIROS_AXISACCURACY_UNIT = 0.05;
            static constexpr  double AIROS_AXISORIENTATION_UNIT = 0.0054932479;

            static constexpr  double AIROS_WIDTH_UNIT = 0.01;
            static constexpr  double AIROS_LENGTH_UNIT = 0.01;
            static constexpr  double AIROS_HEIGTH_UNIT = 0.05;

            static constexpr  double AIROS_FULL_DEG = 360.0;
            static constexpr  double AIROS_SEMI_DEG = 180.0;
            static constexpr  double AIROS_QUAR_DEG = 90.0;
            static constexpr  double PI = 3.1415926535897932384626;
            static const int32_t AIROS_INVALID_ACCELERATION = 2001;
            static const int32_t AIROS_INVALID_VETICALACCELERATION = -127;

            static constexpr  double AIROS_EARTH_RADIUS = 6366000.0;

            static constexpr  double AIROS_EARTH_POLAR_RADIUS = 6356752.0;
            static constexpr  double AIROS_EARTH_EQUAT_RADIUS = 6378137.0;

            static constexpr  double EE = 0.00669342162296594323;
            static constexpr  double A = 6378245.0;
        };
    } // namespace base
} // namespace airos
