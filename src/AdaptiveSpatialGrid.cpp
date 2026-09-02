#include "AdaptiveSpatialGrid.h"

#include <cmath>
#include <stdexcept>


namespace
{

/*
 * ============================================================
 * Distance
 * ============================================================
 *
 * Horizontal distance from the LiDAR sensor origin.
 *
 * Z is intentionally ignored because the adaptive resolution
 * profile is based on XY distance.
 */

float calculateDistance(
    float x,
    float y)
{
    return std::sqrt(
        x * x +
        y * y
    );
}


/*
 * ============================================================
 * Reset adaptive cell
 * ============================================================
 */

void resetCell(
    AdaptiveCell& cell,
    float resolution,
    int band)
{
    cell.elevation = 0.0f;
    cell.intensity = 0.0f;

    cell.pointCount = 0;

    cell.elevationSum = 0.0f;

    cell.minimumElevation = 0.0f;
    cell.maximumElevation = 0.0f;

    cell.resolution = resolution;
    cell.band = band;

    cell.semanticEvidence.fill(0);

    cell.semanticClass =
        SemanticClass::Unknown;

    cell.semanticConfidence = 0.0f;
}


/*
 * ============================================================
 * Add point to adaptive cell
 * ============================================================
 */

void addPointToCell(
    AdaptiveCell& cell,
    const Point& point)
{
    /*
     * --------------------------------------------------------
     * Elevation accumulation
     * --------------------------------------------------------
     */

    cell.elevationSum +=
        point.z;


    /*
     * --------------------------------------------------------
     * Intensity accumulation
     * --------------------------------------------------------
     */

    cell.intensity +=
        point.intensity;


    /*
     * --------------------------------------------------------
     * Minimum / maximum elevation
     * --------------------------------------------------------
     */

    if (cell.pointCount == 0)
    {
        cell.minimumElevation =
            point.z;

        cell.maximumElevation =
            point.z;
    }
    else
    {
        if (point.z <
            cell.minimumElevation)
        {
            cell.minimumElevation =
                point.z;
        }

        if (point.z >
            cell.maximumElevation)
        {
            cell.maximumElevation =
                point.z;
        }
    }


    /*
     * --------------------------------------------------------
     * Observation count
     * --------------------------------------------------------
     */

    ++cell.pointCount;
}


/*
 * ============================================================
 * Construct adaptive cell key
 * ============================================================
 */

AdaptiveSpatialGrid::CellKey makeCellKey(
    float x,
    float y,
    float resolution,
    int band)
{
    /*
     * floor() is important here because it gives correct
     * spatial indexing for both positive and negative
     * coordinates.
     */

    const int cellX =
        static_cast<int>(
            std::floor(
                x / resolution
            )
        );

    const int cellY =
        static_cast<int>(
            std::floor(
                y / resolution
            )
        );


    /*
     * CellKey declaration order is:
     *
     *     x
     *     y
     *     band
     */

    return AdaptiveSpatialGrid::CellKey{
        cellX,
        cellY,
        band
    };
}

}


/*
 * ============================================================
 * Constructor
 * ============================================================
 */

AdaptiveSpatialGrid::AdaptiveSpatialGrid(
    float minX,
    float maxX,
    float minY,
    float maxY,
    const ResolutionProfile& profile)
    :
    minX(minX),
    maxX(maxX),
    minY(minY),
    maxY(maxY),
    profile(profile)
{
    if (maxX <= minX)
    {
        throw std::invalid_argument(
            "maxX must be greater than minX"
        );
    }

    if (maxY <= minY)
    {
        throw std::invalid_argument(
            "maxY must be greater than minY"
        );
    }
}


/*
 * ============================================================
 * Build adaptive grid
 * ============================================================
 */

void AdaptiveSpatialGrid::build(
    const PointCloud& cloud)
{
    /*
     * Every build represents a fresh map.
     *
     * This also guarantees that rebuilding the grid does not
     * retain cells from a previous point cloud.
     */

    cells.clear();


    /*
     * --------------------------------------------------------
     * Process every point
     * --------------------------------------------------------
     */

    for (std::size_t i = 0;
         i < cloud.size();
         ++i)
    {
        const Point& point =
            cloud.getPoint(i);


        /*
         * ----------------------------------------------------
         * Mapping bounds
         * ----------------------------------------------------
         */

        if (point.x < minX ||
            point.x > maxX ||
            point.y < minY ||
            point.y > maxY)
        {
            continue;
        }


        /*
         * ----------------------------------------------------
         * Determine adaptive resolution
         * ----------------------------------------------------
         */

        const float distance =
            calculateDistance(
                point.x,
                point.y
            );

        const int band =
            profile.getBand(
                distance
            );

        const float resolution =
            profile.getResolution(
                distance
            );


        /*
         * ----------------------------------------------------
         * Determine spatial cell
         * ----------------------------------------------------
         */

        const CellKey key =
            makeCellKey(
                point.x,
                point.y,
                resolution,
                band
            );


        /*
         * ----------------------------------------------------
         * Create cell if necessary
         * ----------------------------------------------------
         *
         * IMPORTANT:
         *
         * cells stores:
         *
         *     CellKey -> AdaptiveCell
         *
         * Never insert MapCell here.
         */

        auto [iterator, inserted] =
            cells.try_emplace(
                key,
                AdaptiveCell{}
            );


        AdaptiveCell& cell =
            iterator->second;


        /*
         * Newly-created cells receive their adaptive
         * metadata once.
         */

        if (inserted)
        {
            resetCell(
                cell,
                resolution,
                band
            );
        }


        /*
         * ----------------------------------------------------
         * Accumulate LiDAR observation
         * ----------------------------------------------------
         */

        addPointToCell(
            cell,
            point
        );
    }


    /*
     * --------------------------------------------------------
     * Finalize cell averages
     * --------------------------------------------------------
     */

    for (auto& entry : cells)
    {
        AdaptiveCell& cell =
            entry.second;

        if (cell.pointCount == 0)
        {
            continue;
        }


        const float count =
            static_cast<float>(
                cell.pointCount
            );


        /*
         * Average elevation.
         */

        cell.elevation =
            cell.elevationSum /
            count;


        /*
         * Average intensity.
         */

        cell.intensity /=
            count;
    }
}


/*
 * ============================================================
 * Cell count
 * ============================================================
 */

std::size_t AdaptiveSpatialGrid::getCellCount() const
{
    return cells.size();
}


/*
 * ============================================================
 * Render cells
 * ============================================================
 */

std::vector<AdaptiveSpatialGrid::RenderCell>
AdaptiveSpatialGrid::getRenderCells() const
{
    std::vector<RenderCell> renderCells;

    renderCells.reserve(
        cells.size()
    );


    /*
     * --------------------------------------------------------
     * Convert adaptive cells into renderer-facing cells
     * --------------------------------------------------------
     */

    for (const auto& entry : cells)
    {
        const CellKey& key =
            entry.first;

        const AdaptiveCell& cell =
            entry.second;


        /*
         * The cell already stores the resolution that was
         * selected during build().
         */

        const float resolution =
            cell.resolution;


        /*
         * ----------------------------------------------------
         * World-space cell bounds
         * ----------------------------------------------------
         *
         * Integer coordinates identify the lower-left cell
         * corner.
         */

        const float minWorldX =
            static_cast<float>(key.x) *
            resolution;

        const float maxWorldX =
            minWorldX +
            resolution;

        const float minWorldY =
            static_cast<float>(key.y) *
            resolution;

        const float maxWorldY =
            minWorldY +
            resolution;


        /*
         * ----------------------------------------------------
         * World-space center
         * ----------------------------------------------------
         */

        const float worldX =
            minWorldX +
            resolution * 0.5f;

        const float worldY =
            minWorldY +
            resolution * 0.5f;


        /*
         * ----------------------------------------------------
         * Build render cell
         * ----------------------------------------------------
         */

        RenderCell renderCell;

        renderCell.x =
            worldX;

        renderCell.y =
            worldY;

        renderCell.minX =
            minWorldX;

        renderCell.maxX =
            maxWorldX;

        renderCell.minY =
            minWorldY;

        renderCell.maxY =
            maxWorldY;

        renderCell.elevation =
            cell.elevation;

        renderCell.intensity =
            cell.intensity;

        renderCell.resolution =
            resolution;

        renderCell.band =
            cell.band;


        renderCells.push_back(
            renderCell
        );
    }


    return renderCells;
}