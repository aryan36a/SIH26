#include "AdaptiveSpatialGrid.h"

#include <cmath>
#include <functional>
#include <stdexcept>


/*
 * ============================================================
 * CellKey hashing
 * ============================================================
 */

std::size_t AdaptiveSpatialGrid::CellKeyHash::operator()(
    const CellKey &key
) const {

    const std::size_t h1 =
        std::hash<int>{}(key.band);

    const std::size_t h2 =
        std::hash<int>{}(key.x);

    const std::size_t h3 =
        std::hash<int>{}(key.y);

    /*
     * Combine the three integer hashes.
     */
    std::size_t result = h1;

    result ^= h2 +
              static_cast<std::size_t>(0x9e3779b9) +
              (result << 6) +
              (result >> 2);

    result ^= h3 +
              static_cast<std::size_t>(0x9e3779b9) +
              (result << 6) +
              (result >> 2);

    return result;
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
    const ResolutionProfile &resolutionProfile
)
    : minX(minX),
      maxX(maxX),
      minY(minY),
      maxY(maxY),
      resolutionProfile(resolutionProfile) {

    if (maxX <= minX) {
        throw std::invalid_argument(
            "maxX must be greater than minX"
        );
    }

    if (maxY <= minY) {
        throw std::invalid_argument(
            "maxY must be greater than minY"
        );
    }
}


/*
 * ============================================================
 * Distance
 * ============================================================
 *
 * Distance is measured in the horizontal XY plane.
 *
 * Z is deliberately ignored because the resolution
 * profile describes spatial sampling around the sensor.
 */

float AdaptiveSpatialGrid::calculateDistance(
    float x,
    float y
) const {

    return std::sqrt(
        x * x +
        y * y
    );
}


/*
 * ============================================================
 * Resolution band
 * ============================================================
 *
 * IMPORTANT:
 *
 * AdaptiveSpatialGrid no longer knows:
 *
 *     0.05
 *     0.10
 *     0.25
 *
 * Those values belong exclusively to ResolutionProfile.
 */

int AdaptiveSpatialGrid::getResolutionBand(
    float distance
) const {

    return resolutionProfile.getBand(
        distance
    );
}


/*
 * ============================================================
 * Cell key
 * ============================================================
 */

AdaptiveSpatialGrid::CellKey
AdaptiveSpatialGrid::getCellKey(
    float x,
    float y,
    float distance
) const {

    /*
     * Ask the profile which resolution applies.
     */
    const float resolution =
        resolutionProfile.getResolution(
            distance
        );

    /*
     * Ask the profile which band applies.
     */
    const int band =
        resolutionProfile.getBand(
            distance
        );

    /*
     * Convert world position into integer
     * coordinates for this resolution system.
     *
     * NOTE:
     *
     * This is still the Phase 4 prototype indexing
     * model. We will address cross-band cell geometry
     * and projection correctness in the next step.
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

    return CellKey{
        band,
        cellX,
        cellY
    };
}


/*
 * ============================================================
 * Reset cell
 * ============================================================
 */

void AdaptiveSpatialGrid::resetCell(
    MapCell &cell) {

    cell.elevation = 0.0f;

    cell.intensity = 0.0f;

    cell.pointCount = 0;

    cell.elevationSum = 0.0f;

    cell.minimumElevation = 0.0f;

    cell.maximumElevation = 0.0f;

    cell.semanticEvidence.fill(0);

    cell.semanticClass =
        SemanticClass::Unknown;

    cell.semanticConfidence =
        0.0f;
}
/*
 * ============================================================
 * Add point
 * ============================================================
 */

void AdaptiveSpatialGrid::addPointToCell(
    MapCell &cell,
    const Point &point) {

    /*
     * --------------------------------------------------
     * Elevation
     * --------------------------------------------------
     */

    cell.elevationSum +=
        point.z;


    /*
     * --------------------------------------------------
     * Intensity
     * --------------------------------------------------
     */

    cell.intensity +=
        point.intensity;


    /*
     * --------------------------------------------------
     * Min/max elevation
     * --------------------------------------------------
     */

    if (cell.pointCount == 0) {

        cell.minimumElevation =
            point.z;

        cell.maximumElevation =
            point.z;

    } else {

        if (point.z <
            cell.minimumElevation) {

            cell.minimumElevation =
                point.z;
        }

        if (point.z >
            cell.maximumElevation) {

            cell.maximumElevation =
                point.z;
        }
    }


    /*
     * --------------------------------------------------
     * Observation count
     * --------------------------------------------------
     */

    cell.pointCount++;
}


/*
 * ============================================================
 * Build
 * ============================================================
 */

void AdaptiveSpatialGrid::build(
    const PointCloud &cloud
) {

    /*
     * A build represents a fresh map.
     */
    cells.clear();

    /*
     * Process every LiDAR point.
     */
    for (std::size_t i = 0;
         i < cloud.size();
         ++i) {

        const Point &point =
            cloud.getPoint(i);

        /*
         * Ignore points outside the configured
         * mapping region.
         */
        if (point.x < minX ||
            point.x > maxX ||
            point.y < minY ||
            point.y > maxY) {

            continue;
        }

        /*
         * Calculate horizontal distance
         * from the sensor origin.
         */
        const float distance =
            calculateDistance(
                point.x,
                point.y
            );

        /*
         * Convert the point into a unique
         * adaptive cell identity.
         */
        const CellKey key =
            getCellKey(
                point.x,
                point.y,
                distance
            );

        /*
         * Create the cell if it does not
         * already exist.
         */
        auto [iterator, inserted] =
            cells.try_emplace(
                key,
                MapCell{}
            );

        MapCell &cell =
            iterator->second;

        /*
         * Newly-created MapCell objects need
         * deterministic initialization.
         */
        if (inserted) {
            resetCell(cell);
        }

        /*
         * Accumulate this LiDAR observation.
         */
        addPointToCell(
            cell,
            point
        );
    }

    /*
     * Convert accumulators into representative
     * averages.
     */
    for (auto &entry : cells) {

        MapCell &cell =
            entry.second;

        if (cell.pointCount == 0) {
            continue;
        }

        const float count =
            static_cast<float>(
                cell.pointCount
            );

        cell.elevation =
            cell.elevationSum / count;

        cell.intensity /=
            count;
    }
}


/*
 * ============================================================
 * Cell count
 * ============================================================
 */

std::size_t AdaptiveSpatialGrid::getCellCount() const {

    return cells.size();
}


/*
 * ============================================================
 * Cell existence
 * ============================================================
 */

bool AdaptiveSpatialGrid::hasCell(
    const CellKey &key
) const {

    return cells.find(key) != cells.end();
}


/*
 * ============================================================
 * Get cell
 * ============================================================
 */

const MapCell &AdaptiveSpatialGrid::getCell(
    const CellKey &key
) const {

    const auto iterator =
        cells.find(key);

    if (iterator == cells.end()) {

        throw std::out_of_range(
            "AdaptiveSpatialGrid cell does not exist"
        );
    }

    return iterator->second;
}


/*
 * ============================================================
 * Get cell resolution
 * ============================================================
 *
 * IMPORTANT:
 *
 * No resolution values are hard-coded here.
 *
 * The ResolutionProfile is the single owner
 * of the relationship:
 *
 *     band -> resolution
 */

float AdaptiveSpatialGrid::getCellResolution(
    const CellKey &key
) const {

    return resolutionProfile.getResolutionForBand(
        key.band
    );
}


/*
 * ============================================================
 * Render cells
 * ============================================================
 */

std::vector<AdaptiveSpatialGrid::RenderCell>
AdaptiveSpatialGrid::getRenderCells() const {

    std::vector<RenderCell> renderCells;

    renderCells.reserve(
        cells.size()
    );

    for (const auto &entry : cells) {

        const CellKey &key =
            entry.first;

        const MapCell &cell =
            entry.second;

        const float resolution =
            getCellResolution(
                key
            );

        /*
         * Cell center in world coordinates.
         */
        const float worldX =
            (
                static_cast<float>(key.x) +
                0.5f
            ) * resolution;

        const float worldY =
            (
                static_cast<float>(key.y) +
                0.5f
            ) * resolution;

        RenderCell renderCell;

        renderCell.x =
            worldX;

        renderCell.y =
            worldY;

        renderCell.elevation =
            cell.elevation;

        renderCell.intensity =
            cell.intensity;

        renderCell.resolution =
            resolution;

        renderCell.band =
            key.band;

        renderCells.push_back(
            renderCell
        );
    }

    return renderCells;
}