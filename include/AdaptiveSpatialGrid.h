#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "MapCell.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

class AdaptiveSpatialGrid {
public:

    /*
     * Unique identity of an adaptive cell.
     *
     * band:
     *     resolution region
     *
     * x/y:
     *     integer cell coordinates within that
     *     resolution system
     */
    struct CellKey {

        int band;
        int x;
        int y;

        bool operator==(const CellKey &other) const {
            return band == other.band &&
                   x == other.x &&
                   y == other.y;
        }
    };

    /*
     * Data required by the renderer.
     */
    struct RenderCell {

        float x;
        float y;

        float elevation;
        float intensity;

        float resolution;

        int band;
    };

private:

    struct CellKeyHash {

        std::size_t operator()(
            const CellKey &key
        ) const;
    };

    float minX;
    float maxX;

    float minY;
    float maxY;

    ResolutionProfile resolutionProfile;

    std::unordered_map<
        CellKey,
        MapCell,
        CellKeyHash
    > cells;

    float calculateDistance(
        float x,
        float y
    ) const;

    int getResolutionBand(
        float distance
    ) const;

    CellKey getCellKey(
        float x,
        float y,
        float distance
    ) const;

    void resetCell(
        MapCell &cell
    );

    void addPointToCell(
        MapCell &cell,
        const Point &point
    );

public:

    AdaptiveSpatialGrid(
        float minX,
        float maxX,
        float minY,
        float maxY,
        const ResolutionProfile &resolutionProfile
    );

    void build(
        const PointCloud &cloud
    );

    std::size_t getCellCount() const;

    bool hasCell(
        const CellKey &key
    ) const;

    const MapCell &getCell(
        const CellKey &key
    ) const;

    float getCellResolution(
        const CellKey &key
    ) const;

    std::vector<RenderCell> getRenderCells() const;
};