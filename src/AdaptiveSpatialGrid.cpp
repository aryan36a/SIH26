#include "AdaptiveSpatialGrid.h"

#include <cmath>
#include <stdexcept>

namespace
{

/*
 * Horizontal distance from sensor origin.
 * Z is intentionally excluded — resolution is based on XY range only.
 */
inline float horizontalDist(float x, float y)
{
    return std::sqrt(x * x + y * y);
}

void resetCell(AdaptiveCell& c, float resolution, int band)
{
    c.elevation          = 0.0f;
    c.intensity          = 0.0f;
    c.pointCount         = 0;
    c.elevationSum       = 0.0f;
    c.intensitySum       = 0.0f;
    c.minimumElevation   = 0.0f;
    c.maximumElevation   = 0.0f;
    c.resolution         = resolution;
    c.band               = band;
    c.semanticEvidence.fill(0);
    c.semanticClass      = SemanticClass::Unknown;
    c.semanticConfidence = 0.0f;
}

void accumulatePoint(AdaptiveCell& c, const Point& pt)
{
    c.elevationSum += pt.z;
    c.intensitySum += pt.intensity;

    if (c.pointCount == 0)
    {
        c.minimumElevation = pt.z;
        c.maximumElevation = pt.z;
    }
    else
    {
        if (pt.z < c.minimumElevation) c.minimumElevation = pt.z;
        if (pt.z > c.maximumElevation) c.maximumElevation = pt.z;
    }

    ++c.pointCount;
}

AdaptiveSpatialGrid::CellKey makeCellKey(float x, float y, float res, int band)
{
    // std::floor handles negative coordinates correctly
    return {
        static_cast<int>(std::floor(x / res)),
        static_cast<int>(std::floor(y / res)),
        band
    };
}

} // namespace


AdaptiveSpatialGrid::AdaptiveSpatialGrid(
    float minX_, float maxX_,
    float minY_, float maxY_,
    const ResolutionProfile& profile_)
    : minX(minX_), maxX(maxX_), minY(minY_), maxY(maxY_), profile(profile_)
{
    if (maxX <= minX)
        throw std::invalid_argument("maxX must be greater than minX");
    if (maxY <= minY)
        throw std::invalid_argument("maxY must be greater than minY");
}


void AdaptiveSpatialGrid::build(const PointCloud& cloud)
{
    cells.clear();

    for (std::size_t i = 0; i < cloud.size(); ++i)
    {
        const Point& pt = cloud.getPoint(i);

        // Reject out-of-bounds or non-finite points
        if (pt.x < minX || pt.x > maxX || pt.y < minY || pt.y > maxY) continue;
        if (!std::isfinite(pt.x) || !std::isfinite(pt.y) ||
            !std::isfinite(pt.z) || !std::isfinite(pt.intensity)) continue;

        const float dist = horizontalDist(pt.x, pt.y);
        const int   band = profile.getBand(dist);
        const float res  = profile.getResolutionForBand(band);

        const CellKey key = makeCellKey(pt.x, pt.y, res, band);

        auto [it, inserted] = cells.try_emplace(key, AdaptiveCell{});
        if (inserted) resetCell(it->second, res, band);

        accumulatePoint(it->second, pt);
    }

    // Finalize averages
    for (auto& [key, c] : cells)
    {
        if (c.pointCount == 0) continue;
        const float n = static_cast<float>(c.pointCount);
        c.elevation = c.elevationSum / n;
        c.intensity = c.intensitySum / n;
    }
}


std::size_t AdaptiveSpatialGrid::getCellCount() const
{
    return cells.size();
}


std::vector<AdaptiveSpatialGrid::RenderCell>
AdaptiveSpatialGrid::getRenderCells() const
{
    std::vector<RenderCell> out;
    out.reserve(cells.size());

    for (const auto& [key, c] : cells)
    {
        if (c.pointCount == 0) continue;

        const float res = c.resolution;

        RenderCell rc;
        rc.minX = static_cast<float>(key.x) * res;
        rc.maxX = rc.minX + res;
        rc.minY = static_cast<float>(key.y) * res;
        rc.maxY = rc.minY + res;
        rc.x    = rc.minX + res * 0.5f;
        rc.y    = rc.minY + res * 0.5f;

        rc.minimumElevation = c.minimumElevation;
        rc.maximumElevation = c.maximumElevation;
        rc.elevation        = c.elevation;
        rc.elevationSpan    = c.maximumElevation - c.minimumElevation;
        rc.intensity        = c.intensity;
        rc.pointCount       = c.pointCount;
        rc.resolution       = res;
        rc.band             = c.band;

        out.push_back(rc);
    }

    return out;
}