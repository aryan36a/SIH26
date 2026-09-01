#include "SpatialGrid.h"

#include <cmath>
#include <stdexcept>

SpatialGrid::SpatialGrid(
    float minX,
    float maxX,
    float minY,
    float maxY,
    float cellSize)
    : minX(minX),
      maxX(maxX),
      minY(minY),
      maxY(maxY),
      cellSize(cellSize),
      width(0),
      height(0) {

    if (cellSize <= 0.0f) {
        throw std::invalid_argument(
            "Cell size must be greater than zero"
        );
    }

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

    width =
        static_cast<std::size_t>(
            std::ceil(
                (maxX - minX) / cellSize
            )
        );

    height =
        static_cast<std::size_t>(
            std::ceil(
                (maxY - minY) / cellSize
            )
        );

    cells.resize(
        width * height
    );
}


std::size_t SpatialGrid::getIndex(
    std::size_t x,
    std::size_t y) const {

    return y * width + x;
}


std::size_t SpatialGrid::getWidth() const {
    return width;
}


std::size_t SpatialGrid::getHeight() const {
    return height;
}


float SpatialGrid::getMinX() const {
    return minX;
}


float SpatialGrid::getMinY() const {
    return minY;
}


float SpatialGrid::getCellSize() const {
    return cellSize;
}


const MapCell &SpatialGrid::getCell(
    std::size_t x,
    std::size_t y) const {

    if (x >= width || y >= height) {
        throw std::out_of_range(
            "SpatialGrid cell coordinates out of range"
        );
    }

    return cells[
        getIndex(x, y)
    ];
}


void SpatialGrid::build(
    const PointCloud &cloud) {

    /*
     * --------------------------------------------------
     * Reset cells
     * --------------------------------------------------
     */

    for (MapCell &cell : cells) {

        cell.elevation = 0.0f;

        cell.intensity = 0.0f;

        cell.pointCount = 0;

        cell.elevationSum = 0.0f;

        cell.minimumElevation = 0.0f;

        cell.maximumElevation = 0.0f;

        cell.semanticEvidence.fill(0);

        cell.semanticClass =
            SemanticClass::Unknown;

        cell.semanticConfidence = 0.0f;
    }


    /*
     * --------------------------------------------------
     * Insert points
     * --------------------------------------------------
     */

    for (std::size_t i = 0;
         i < cloud.size();
         ++i) {

        const Point &point =
            cloud.getPoint(i);


        /*
         * Ignore points outside
         * the configured grid.
         */
        if (point.x < minX ||
            point.x > maxX ||
            point.y < minY ||
            point.y > maxY) {

            continue;
        }


        std::size_t cellX =
            static_cast<std::size_t>(
                std::floor(
                    (point.x - minX) /
                    cellSize
                )
            );

        std::size_t cellY =
            static_cast<std::size_t>(
                std::floor(
                    (point.y - minY) /
                    cellSize
                )
            );


        /*
         * A point exactly at maxX/maxY
         * can produce width/height.
         *
         * Clamp it to the final cell.
         */
        if (cellX >= width) {
            cellX = width - 1;
        }

        if (cellY >= height) {
            cellY = height - 1;
        }


        if (cellX >= width ||
            cellY >= height) {

            continue;
        }


        MapCell &cell =
            cells[
                getIndex(
                    cellX,
                    cellY
                )
            ];


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


        cell.pointCount++;
    }


    /*
     * --------------------------------------------------
     * Finalize averages
     * --------------------------------------------------
     */

    for (MapCell &cell : cells) {

        if (cell.pointCount == 0) {
            continue;
        }


        cell.elevation =
            cell.elevationSum /
            static_cast<float>(
                cell.pointCount
            );


        cell.intensity /=
            static_cast<float>(
                cell.pointCount
            );
    }
}


bool SpatialGrid::hasData(
    std::size_t x,
    std::size_t y) const {

    return getCell(
        x,
        y
    ).pointCount > 0;
}


float SpatialGrid::getElevation(
    std::size_t x,
    std::size_t y) const {

    return getCell(
        x,
        y
    ).elevation;
}


float SpatialGrid::getIntensity(
    std::size_t x,
    std::size_t y) const {

    return getCell(
        x,
        y
    ).intensity;
}


float SpatialGrid::getHeightVariation(
    std::size_t x,
    std::size_t y) const {

    const MapCell &cell =
        getCell(
            x,
            y
        );

    if (cell.pointCount == 0) {
        return 0.0f;
    }

    return cell.maximumElevation -
           cell.minimumElevation;
}