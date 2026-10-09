/*
 * CoarseScale.h
 *
 *  Created on: 2018年5月22日
 *      Author: catt
 */

#ifndef DATAMODEL_MAPMODELS_COARSESCALE_H_
#define DATAMODEL_MAPMODELS_COARSESCALE_H_

#include "PositionModelExt.h"

#include <vector>
#include <memory>

namespace airos {
namespace app {

class PathSegment;
using PathSegmentPtr = std::shared_ptr<PathSegment>;

class CoarseScale
{
public:
    CoarseScale(const PositionModelExt& refPos);
    CoarseScale(std::vector<PathSegmentPtr>& m_PathSegment, double laneWidth);
    CoarseScale(const PositionModelExt& pos, double width, double length);
    CoarseScale(double lt_lon, double lt_lat, double rb_lon, double rb_lat);
    CoarseScale(const CoarseScale& cs);

    bool AtThisScale(const PositionModelExt& pos);
    CoarseScale& operator += (const CoarseScale& cs);
    bool OverLap(const CoarseScale& cs) const;

    std::string to_string();

private:
    PositionModelExt m_LeftTop;
    PositionModelExt m_RightBottom;
};

bool OverLap(const CoarseScale& cs1, const CoarseScale& cs2);


} }

#endif /* DATAMODEL_MAPMODELS_COARSESCALE_H_ */
