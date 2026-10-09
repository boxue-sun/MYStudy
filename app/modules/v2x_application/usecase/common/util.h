/******************************************************************************
 * Copyright 2022 The Airos Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#pragma once

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

namespace airos {
namespace perception {
namespace usecase {

class Utils {
 public:
  static bool IsFileExists(const std::string& strPath);
  void utm_to_latlon(
      const int& zone, double* utm_x, double* utm_y, double* lat, double* lon);
  void LatLonToUTMXY(
      const double& lat, const double& lon, double* utm_x, double* utm_y);

  double Str2Double(const std::string& str);
  int Str2Int(const std::string& str);
  std::string TransTimestamp(const int64_t& time_s);

 private:
  double DegToRad(const double deg);
  double RadToDeg(const double rad);
  double FootpointLatitude(const double y);
  void MathXYToLanLon(
      const double& x, const double& y, const double& central_meridian_radian,
      double* lat_radian, double* lon_radian);

  double UTMCentralMeridian(const double lon);
  double ArcLengthOfMeridian(const double lat_radian);
  void MathLanLonToXY(
      const double& lat_radian, const double& lon_radian,
      const double& central_meridian_radian, double* x, double* y);

 private:
  const double kUTMScaleFactor_   = 0.9996;
  const double kUTMXCompensation_ = 500000.0;
  const double kUTMYCompensation_ = 10000000.0;

  const double kEmA_ = 6378137.0;
  const double kEmB_ = 6356752.3142451;

 public:
  static const double kPI_;
};

}  // namespace usecase
}  // namespace perception
}  // namespace airos
