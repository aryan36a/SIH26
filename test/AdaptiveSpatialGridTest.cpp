#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>


namespace
{

int testsPassed = 0;
int testsFailed = 0;


/*
 * ============================================================
 * Test helpers
 * ============================================================
 */

void check(
    bool condition,
    const std::string& testName)
{
    if (condition)
    {
        std::cout
            << "[PASS] "
            << testName
            << '\n';

        ++testsPassed;
    }
    else
    {
        std::cerr
            << "[FAIL] "
            << testName
            << '\n';

        ++testsFailed;
    }
}


void checkNear(
    float actual,
    float expected,
    float tolerance,
    const std::string& testName)
{
    check(
        std::fabs(actual - expected) <= tolerance,
        testName
    );
}


/*
 * ============================================================
 * Resolution bands
 * ============================================================
 *
 * Resolution-band ownership belongs to ResolutionProfile.
 *
 * AdaptiveSpatialGrid consumes this profile when building
 * adaptive cells.
 *
 * Therefore these boundary tests intentionally call:
 *
 *     ResolutionProfile::getBand()
 *
 * instead of testing an internal AdaptiveSpatialGrid helper.
 */

void testResolutionBands()
{
    ResolutionProfile profile;

    check(
        profile.getBand(0.0f) == 0,
        "distance 0 belongs to band 0"
    );

    check(
        profile.getBand(9.999f) == 0,
        "distance 9.999 belongs to band 0"
    );

    check(
        profile.getBand(10.0f) == 1,
        "distance 10 belongs to band 1"
    );

    check(
        profile.getBand(10.001f) == 1,
        "distance 10.001 belongs to band 1"
    );

    check(
        profile.getBand(24.999f) == 1,
        "distance 24.999 belongs to band 1"
    );

    check(
        profile.getBand(25.0f) == 2,
        "distance 25 belongs to band 2"
    );

    check(
        profile.getBand(25.001f) == 2,
        "distance 25.001 belongs to band 2"
    );

    check(
        profile.getBand(100.0f) == 2,
        "distance 100 belongs to band 2"
    );


    checkNear(
        profile.getResolutionForBand(0),
        0.05f,
        0.000001f,
        "band 0 resolution is 0.05m"
    );

    checkNear(
        profile.getResolutionForBand(1),
        0.10f,
        0.000001f,
        "band 1 resolution is 0.10m"
    );

    checkNear(
        profile.getResolutionForBand(2),
        0.25f,
        0.000001f,
        "band 2 resolution is 0.25m"
    );


    checkNear(
        profile.getResolution(5.0f),
        0.05f,
        0.000001f,
        "5m uses 0.05m resolution"
    );

    checkNear(
        profile.getResolution(15.0f),
        0.10f,
        0.000001f,
        "15m uses 0.10m resolution"
    );

    checkNear(
        profile.getResolution(30.0f),
        0.25f,
        0.000001f,
        "30m uses 0.25m resolution"
    );
}


/*
 * ============================================================
 * Adaptive cell creation
 * ============================================================
 */

void testAdaptiveCellCreation()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -40.0f,
        40.0f,
        -40.0f,
        40.0f,
        profile
    );

    PointCloud cloud;


    /*
     * Three points deliberately placed in different
     * resolution bands and different adaptive cells.
     */

    cloud.addPoint(
        Point{
            1.0f,
            1.0f,
            10.0f,
            50.0f
        }
    );

    cloud.addPoint(
        Point{
            15.0f,
            0.0f,
            20.0f,
            60.0f
        }
    );

    cloud.addPoint(
        Point{
            30.0f,
            0.0f,
            30.0f,
            70.0f
        }
    );

    grid.build(cloud);

    check(
        grid.getCellCount() == 3,
        "three isolated points produce three adaptive cells"
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 3,
        "three adaptive cells produce three render cells"
    );


    bool foundNear = false;
    bool foundMid = false;
    bool foundFar = false;


    for (const auto& cell : renderCells)
    {
        if (std::fabs(cell.resolution - 0.05f)
            < 0.000001f)
        {
            foundNear = true;
        }

        if (std::fabs(cell.resolution - 0.10f)
            < 0.000001f)
        {
            foundMid = true;
        }

        if (std::fabs(cell.resolution - 0.25f)
            < 0.000001f)
        {
            foundFar = true;
        }
    }


    check(
        foundNear,
        "adaptive grid contains a 0.05m cell"
    );

    check(
        foundMid,
        "adaptive grid contains a 0.10m cell"
    );

    check(
        foundFar,
        "adaptive grid contains a 0.25m cell"
    );
}


/*
 * ============================================================
 * Cell aggregation
 * ============================================================
 */

void testCellAggregation()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -10.0f,
        10.0f,
        -10.0f,
        10.0f,
        profile
    );

    PointCloud cloud;


    /*
     * Both points lie inside the same 0.05m cell.
     *
     * x:
     *   1.000
     *   1.020
     *
     * y:
     *   1.000
     *
     * Therefore they must aggregate into one cell.
     */

    cloud.addPoint(
        Point{
            1.000f,
            1.000f,
            10.0f,
            20.0f
        }
    );

    cloud.addPoint(
        Point{
            1.020f,
            1.000f,
            20.0f,
            40.0f
        }
    );

    grid.build(cloud);

    check(
        grid.getCellCount() == 1,
        "two points in one adaptive cell produce one cell"
    );


    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "one aggregated cell produces one render cell"
    );


    if (renderCells.empty())
    {
        return;
    }


    checkNear(
        renderCells[0].elevation,
        15.0f,
        0.000001f,
        "cell elevation is averaged correctly"
    );

    checkNear(
        renderCells[0].intensity,
        30.0f,
        0.000001f,
        "cell intensity is averaged correctly"
    );

    checkNear(
        renderCells[0].resolution,
        0.05f,
        0.000001f,
        "aggregated cell retains near resolution"
    );
}


/*
 * ============================================================
 * Spatial separation
 * ============================================================
 */

void testSpatialSeparation()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -10.0f,
        10.0f,
        -10.0f,
        10.0f,
        profile
    );

    PointCloud cloud;


    cloud.addPoint(
        Point{
            1.00f,
            1.00f,
            1.0f,
            10.0f
        }
    );

    cloud.addPoint(
        Point{
            1.10f,
            1.00f,
            2.0f,
            20.0f
        }
    );

    cloud.addPoint(
        Point{
            1.20f,
            1.00f,
            3.0f,
            30.0f
        }
    );

    grid.build(cloud);

    check(
        grid.getCellCount() == 3,
        "spatially separated points produce separate cells"
    );
}


/*
 * ============================================================
 * Negative coordinates
 * ============================================================
 */

void testNegativeCoordinates()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -10.0f,
        10.0f,
        -10.0f,
        10.0f,
        profile
    );

    PointCloud cloud;

    cloud.addPoint(
        Point{
            -1.0f,
            -1.0f,
            5.0f,
            25.0f
        }
    );

    grid.build(cloud);

    check(
        grid.getCellCount() == 1,
        "negative coordinates are accepted"
    );


    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "negative-coordinate cell is rendered"
    );


    if (renderCells.empty())
    {
        return;
    }


    check(
        renderCells[0].x < 0.0f,
        "negative cell center remains negative"
    );

    check(
        renderCells[0].y < 0.0f,
        "negative cell Y center remains negative"
    );
}


/*
 * ============================================================
 * Rebuild behavior
 * ============================================================
 */

/*
 * ============================================================
 * Rebuild behavior
 * ============================================================
 */

void testRebuild()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -20.0f,
        20.0f,
        -20.0f,
        20.0f,
        profile
    );

    /*
     * --------------------------------------------------------
     * First build
     * --------------------------------------------------------
     *
     * The grid starts with one adaptive cell.
     */

    PointCloud firstCloud;

    firstCloud.addPoint(
        Point{
            1.0f,
            1.0f,
            5.0f,
            25.0f
        }
    );

    grid.build(
        firstCloud
    );

    check(
        grid.getCellCount() == 1,
        "first build creates one cell"
    );


    /*
     * --------------------------------------------------------
     * Second build
     * --------------------------------------------------------
     *
     * These points belong to two different adaptive
     * resolution bands:
     *
     *   5m  -> 0.05m
     *   15m -> 0.10m
     *
     * Both are inside the configured grid.
     *
     * The important property being tested is that
     * the old cell from firstCloud is removed.
     */

    PointCloud secondCloud;

    secondCloud.addPoint(
        Point{
            5.0f,
            0.0f,
            8.0f,
            40.0f
        }
    );

    secondCloud.addPoint(
        Point{
            15.0f,
            0.0f,
            9.0f,
            50.0f
        }
    );


    grid.build(
        secondCloud
    );


    /*
     * The second build must contain exactly the
     * two cells generated from secondCloud.
     *
     * The original cell from firstCloud must no
     * longer exist.
     */

    check(
        grid.getCellCount() == 2,
        "rebuild clears cells from previous build"
    );
}

/*
 * ============================================================
 * Render-cell resolution propagation
 * ============================================================
 */

void testRenderCellResolution()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -40.0f,
        40.0f,
        -40.0f,
        40.0f,
        profile
    );

    PointCloud cloud;


    /*
     * Near band:
     * distance = 5m
     */

    cloud.addPoint(
        Point{
            5.0f,
            0.0f,
            1.0f,
            10.0f
        }
    );


    /*
     * Mid band:
     * distance = 15m
     */

    cloud.addPoint(
        Point{
            15.0f,
            0.0f,
            2.0f,
            20.0f
        }
    );


    /*
     * Far band:
     * distance = 30m
     */

    cloud.addPoint(
        Point{
            30.0f,
            0.0f,
            3.0f,
            30.0f
        }
    );


    grid.build(cloud);

    const auto renderCells =
        grid.getRenderCells();


    bool foundNear = false;
    bool foundMid = false;
    bool foundFar = false;


    for (const auto& cell : renderCells)
    {
        if (std::fabs(cell.resolution - 0.05f)
            < 0.000001f)
        {
            foundNear = true;
        }

        if (std::fabs(cell.resolution - 0.10f)
            < 0.000001f)
        {
            foundMid = true;
        }

        if (std::fabs(cell.resolution - 0.25f)
            < 0.000001f)
        {
            foundFar = true;
        }
    }


    check(
        foundNear,
        "render cells expose near resolution"
    );

    check(
        foundMid,
        "render cells expose mid resolution"
    );

    check(
        foundFar,
        "render cells expose far resolution"
    );
}

}


/*
 * ============================================================
 * Main
 * ============================================================
 */

int main()
{
    std::cout
        << "========================================\n"
        << "SIH 26 Adaptive Spatial Grid Tests\n"
        << "Phase 5.1\n"
        << "========================================\n";


    testResolutionBands();

    testAdaptiveCellCreation();

    testCellAggregation();

    testSpatialSeparation();

    testNegativeCoordinates();

    testRebuild();

    testRenderCellResolution();


    std::cout
        << '\n'
        << "========================================\n"
        << "Tests passed: "
        << testsPassed
        << '\n'
        << "Tests failed: "
        << testsFailed
        << '\n'
        << "========================================\n";


    return testsFailed == 0
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}