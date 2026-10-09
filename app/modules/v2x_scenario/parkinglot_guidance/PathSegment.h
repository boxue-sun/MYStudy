/*
 * PathSegment.h
 *
 *  Created on: 2018年5月21日
 *      Author: catt
 */

#ifndef DATAMODEL_PATHSEGMENT_H_
#define DATAMODEL_PATHSEGMENT_H_

#include <vector>
#include <memory>
#include "PositionModelExt.h"

namespace airos {
namespace app {
class CoarseScale;

class PathSegment
{
    static const double k_OnPathDistThresHold;
    static const double k_OnPathDegThresHold;
    static const double k_PathMatchDegCoff;
    static const double k_PathMatchDistCoff;

public:
    struct MatchResult
    {
        MatchResult() { m_Estimate = 0; m_VertDist = 0;  m_OverDest = false; }

        double m_Estimate;
        double m_VertDist;  /* < 0 left , > 0 right*/

        bool   m_OverDest;
    };

public:
    PathSegment(const PositionModelExt& src, const PositionModelExt& dst, double width = -1);
    PathSegment(const PathSegment& ps);
    virtual ~PathSegment();

    MatchResult PathMatch(const PositionModelExt& pos);

    double getRemainDist(const PositionModelExt& pos);

    const PositionModelExt& getDstPosition() const
    {
        return m_Dst;
    }

    const PositionModelExt& getSrcPosition() const
    {
        return m_Src;
    }

    double getPathLength() const
    {
        return m_PathLength;
    }

    double getAccPathLength() const
    {
        return m_AccPathLength;
    }

    void setAccPathLength(double accPathLength)
    {
        m_AccPathLength = accPathLength;
    }

    double getHeading() const
    {
        return m_Heading;
    }

    static void getNextPosition(PositionModelExt& node, double dist);
    static bool getMinDistance(const PositionModelExt& p1, const PositionModelExt& p2, PositionModelExt& node, double& vd, double& md);

    std::unique_ptr<CoarseScale> m_CoarseScale;
    std::string to_string();

private:
    MatchResult PathMatchWithParam(const PositionModelExt& pos
            , double onPathDegThresHold
            , double pathMatchDegCoff
            , double onPathDistThresHold
            , double pathMatchDistCoff);

private:
    PositionModelExt m_Src;
    PositionModelExt m_Dst;

    double m_PathLength;
    double m_AccPathLength;

    double m_Width;
    double m_Heading;
};

bool operator < (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2);
bool operator == (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2);
bool operator > (const PathSegment::MatchResult& mr1, const PathSegment::MatchResult& mr2);

using PathSegmentPtr = std::shared_ptr<PathSegment>;

} }

#endif /* DATAMODEL_PATHSEGMENT_H_ */
