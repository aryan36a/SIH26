#pragma once

#include "MapCell.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

#include <array>
#include <cstddef>
#include <unordered_map>
#include <vector>

/*
 * Internal adaptive cell. One entry per occupied (x, y, band) key.
 */
struct AdaptiveCell
{
    float elevation  = 0.0f;
    float intensity  = 0.0f;

    std::size_t pointCount  = 0;
    float       elevationSum = 0.0f;
    float       intensitySum = 0.0f;

    float minimumElevation = 0.0f;
    float maximumElevation = 0.0f;

    float resolution = 0.0f;
    int   band       = 0;

    std::array<std::size_t, SEMANTIC_CLASS_COUNT> semanticEvidence{};
    SemanticClass semanticClass      = SemanticClass::Unknown;
    float         semanticConfidence = 0.0f;
};


class AdaptiveSpatialGrid
{
public:

    struct CellKey
    {
        int x;
        int y;
        int band;

        bool operator==(const CellKey& o) const noexcept
        {
            return x == o.x && y == o.y && band == o.band;
        }
    };

    struct CellKeyHash
    {
        std::size_t operator()(const CellKey& k) const noexcept
        {
            // FNV-1a inspired mix — avoids trivial collision on axis-aligned data
            std::size_t h = 2166136261u;
            h ^= static_cast<std::size_t>(static_cast<unsigned int>(k.x));
            h *= 16777619u;
            h ^= static_cast<std::size_t>(static_cast<unsigned int>(k.y));
            h *= 16777619u;
            h ^= static_cast<std::size_t>(k.band);
            h *= 16777619u;
            return h;
        }
    };

    /*
     * RenderCell — authoritative world-space description of one adaptive cell.
     * All geometry should be derived exclusively from these fields.
     *
     * minX/maxX/minY/maxY : XY footprint in world space
     * minimumElevation     : lowest Z observed in this cell
     * maximumElevation     : highest Z observed in this cell
     * elevation            : mean Z (representative)
     * elevationSpan        : maximumElevation - minimumElevation
     * intensity            : mean intensity (normalized 0..1 by Renderer)
     */
    struct RenderCell
    {
        float x = 0.0f;
        float y = 0.0f;

        float minX = 0.0f;
        float maxX = 0.0f;
        float minY = 0.0f;
        float maxY = 0.0f;

        float minimumElevation = 0.0f;
        float maximumElevation = 0.0f;
        float elevation        = 0.0f;
        float elevationSpan    = 0.0f;

        float intensity  = 0.0f;
        std::size_t pointCount = 0;
        float resolution = 0.0f;
        int   band       = 0;
    };

    AdaptiveSpatialGrid(
        float minX, float maxX,
        float minY, float maxY,
        const ResolutionProfile& profile);

    void build(const PointCloud& cloud);

    std::size_t getCellCount() const;

    std::vector<RenderCell> getRenderCells() const;

private:
    float minX, maxX, minY, maxY;
    ResolutionProfile profile;

    std::unordered_map<CellKey, AdaptiveCell, CellKeyHash> cells;
};