#include <cassert>
#include <cmath>
#include <iostream>

#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

bool nearlyEqual(
    float a,
    float b,
    float epsilon = 0.0001f) {

    return std::fabs(a - b) < epsilon;
}

int main() {

    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -50.0f,
        50.0f,
        -50.0f,
        50.0f,
        profile
    );

    // ==============================================
    // TEST 1: Near point uses fine resolution
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            2.0f,
            0.0f,
            10.0f,
            20.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 1);
    }

    // ==============================================
    // TEST 2: Mid-range point
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            15.0f,
            0.0f,
            20.0f,
            30.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 1);
    }

    // ==============================================
    // TEST 3: Far point
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            40.0f,
            0.0f,
            30.0f,
            40.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 1);
    }

    // ==============================================
    // TEST 4: Near points share a cell
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            1.01f,
            1.01f,
            10.0f,
            20.0f
        });

        cloud.addPoint(Point{
            1.04f,
            1.04f,
            12.0f,
            40.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 1);
    }

    // ==============================================
    // TEST 5: Different resolutions create
    // different adaptive cells
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            2.0f,
            0.0f,
            10.0f,
            20.0f
        });

        cloud.addPoint(Point{
            20.0f,
            0.0f,
            20.0f,
            30.0f
        });

        cloud.addPoint(Point{
            40.0f,
            0.0f,
            30.0f,
            40.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 3);
    }

    // ==============================================
    // TEST 6: Aggregation
    // ==============================================

    {
        PointCloud cloud;

        cloud.addPoint(Point{
            2.01f,
            2.01f,
            10.0f,
            20.0f
        });

        cloud.addPoint(Point{
            2.02f,
            2.02f,
            14.0f,
            40.0f
        });

        grid.build(cloud);

        assert(grid.getCellCount() == 1);

        /*
         * These two points must occupy the
         * same 5 cm cell.
         */
        AdaptiveSpatialGrid::CellKey key{
            0,
            40,
            40
        };

        assert(grid.hasCell(key));

        const MapCell &cell =
            grid.getCell(key);

        assert(cell.pointCount == 2);

        assert(nearlyEqual(
            cell.elevation,
            12.0f
        ));

        assert(nearlyEqual(
            cell.intensity,
            30.0f
        ));

        assert(nearlyEqual(
            cell.minimumElevation,
            10.0f
        ));

        assert(nearlyEqual(
            cell.maximumElevation,
            14.0f
        ));
    }

    // ==============================================
    // TEST 7: Rebuilding clears old data
    // ==============================================

    {
        PointCloud cloudA;

        cloudA.addPoint(Point{
            2.0f,
            0.0f,
            10.0f,
            20.0f
        });

        grid.build(cloudA);

        assert(grid.getCellCount() == 1);

        PointCloud cloudB;

        cloudB.addPoint(Point{
            40.0f,
            0.0f,
            50.0f,
            60.0f
        });

        grid.build(cloudB);

        assert(grid.getCellCount() == 1);
    }

    std::cout
        << "====================================\n";

    std::cout
        << "All AdaptiveSpatialGrid tests passed!\n";

    std::cout
        << "====================================\n";

    return 0;
}