#pragma once

#include "MapCell.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

#include <array>
#include <cstddef>
#include <unordered_map>
#include <vector>

struct AdaptiveCell
{
    float elevation = 0.0f;
    float intensity = 0.0f;

    std::size_t pointCount = 0;

    float elevationSum = 0.0f;

    float minimumElevation = 0.0f;
    float maximumElevation = 0.0f;

    float resolution = 0.0f;

    int band = 0;

    std::array<int, 6> semanticEvidence{};

    SemanticClass semanticClass =
        SemanticClass::Unknown;

    float semanticConfidence = 0.0f;
};


class AdaptiveSpatialGrid
{
public:

    struct CellKey
    {
        int x;
        int y;
        int band;

        bool operator==(const CellKey& other) const
        {
            return x == other.x &&
                   y == other.y &&
                   band == other.band;
        }
    };


    struct CellKeyHash
    {
        std::size_t operator()(const CellKey& key) const
        {
            std::size_t h1 =
                std::hash<int>{}(key.x);

            std::size_t h2 =
                std::hash<int>{}(key.y);

            std::size_t h3 =
                std::hash<int>{}(key.band);

            return h1 ^
                   (h2 << 1) ^
                   (h3 << 2);
        }
    };


    /*
     * ========================================================
     * RenderCell
     * ========================================================
     *
     * World-space representation of an adaptive cell.
     *
     * x/y:
     *     World-space center.
     *
     * minX/maxX:
     *     Exact world-space X bounds.
     *
     * minY/maxY:
     *     Exact world-space Y bounds.
     *
     * resolution:
     *     Cell width and height.
     */

    struct RenderCell
    {
        float x = 0.0f;
        float y = 0.0f;

        float minX = 0.0f;
        float maxX = 0.0f;

        float minY = 0.0f;
        float maxY = 0.0f;

        float elevation = 0.0f;
        float intensity = 0.0f;

        float resolution = 0.0f;

        int band = 0;
    };


    AdaptiveSpatialGrid(
        float minX,
        float maxX,
        float minY,
        float maxY,
        const ResolutionProfile& profile
    );


    void build(
        const PointCloud& cloud
    );


    std::size_t getCellCount() const;


    std::vector<RenderCell>
    getRenderCells() const;


private:

    float minX;
    float maxX;

    float minY;
    float maxY;

    ResolutionProfile profile;


    std::unordered_map<
        CellKey,
        AdaptiveCell,
        CellKeyHash
    > cells;
};