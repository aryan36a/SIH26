#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>


namespace
{

int testsPassed = 0;
int testsFailed = 0;


void check(
    bool condition,
    const std::string& testName
)
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
    const std::string& testName
)
{
    check(
        std::fabs(actual - expected) <= tolerance,
        testName
    );
}


/*
 * ============================================================
 * Single cell geometry
 * ============================================================
 */

void testSingleCellGeometry()
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
            1.0f,
            1.0f,
            5.0f,
            25.0f
        }
    );

    grid.build(
        cloud
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "single point produces one render cell"
    );

    if (renderCells.size() != 1)
    {
        return;
    }

    const auto& cell =
        renderCells[0];

    /*
     * Point (1,1) is in the near band.
     *
     * Near resolution = 0.05m.
     *
     * floor(1 / 0.05) = 20
     *
     * Therefore:
     *
     * minX = 1.00
     * maxX = 1.05
     *
     * minY = 1.00
     * maxY = 1.05
     */

    checkNear(
        cell.minX,
        1.0f,
        0.000001f,
        "cell minX is correct"
    );

    checkNear(
        cell.maxX,
        1.05f,
        0.000001f,
        "cell maxX is correct"
    );

    checkNear(
        cell.minY,
        1.0f,
        0.000001f,
        "cell minY is correct"
    );

    checkNear(
        cell.maxY,
        1.05f,
        0.000001f,
        "cell maxY is correct"
    );

    checkNear(
        cell.x,
        1.025f,
        0.000001f,
        "cell center X is derived from bounds"
    );

    checkNear(
        cell.y,
        1.025f,
        0.000001f,
        "cell center Y is derived from bounds"
    );

    checkNear(
        cell.maxX - cell.minX,
        0.05f,
        0.000001f,
        "cell width matches resolution"
    );

    checkNear(
        cell.maxY - cell.minY,
        0.05f,
        0.000001f,
        "cell height matches resolution"
    );
}


/*
 * ============================================================
 * Negative coordinates
 * ============================================================
 */

void testNegativeCoordinateGeometry()
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
            -1.01f,
            -2.01f,
            3.0f,
            20.0f
        }
    );

    grid.build(
        cloud
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "negative point produces one render cell"
    );

    if (renderCells.size() != 1)
    {
        return;
    }

    const auto& cell =
        renderCells[0];

    /*
     * floor(-1.01 / 0.05) = -21
     *
     * Therefore:
     *
     * minX = -1.05
     * maxX = -1.00
     *
     * floor(-2.01 / 0.05) = -41
     *
     * minY = -2.05
     * maxY = -2.00
     */

    checkNear(
        cell.minX,
        -1.05f,
        0.000001f,
        "negative cell minX is correct"
    );

    checkNear(
        cell.maxX,
        -1.00f,
        0.000001f,
        "negative cell maxX is correct"
    );

    checkNear(
        cell.minY,
        -2.05f,
        0.000001f,
        "negative cell minY is correct"
    );

    checkNear(
        cell.maxY,
        -2.00f,
        0.000001f,
        "negative cell maxY is correct"
    );

    check(
        cell.maxX < 0.0f &&
        cell.maxY < 0.0f,
        "negative cell remains entirely negative"
    );
}


/*
 * ============================================================
 * Resolution geometry
 * ============================================================
 */

void testResolutionGeometry()
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

    cloud.addPoint(
        Point{
            5.0f,
            0.0f,
            1.0f,
            10.0f
        }
    );

    cloud.addPoint(
        Point{
            15.0f,
            0.0f,
            2.0f,
            20.0f
        }
    );

    cloud.addPoint(
        Point{
            30.0f,
            0.0f,
            3.0f,
            30.0f
        }
    );

    grid.build(
        cloud
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 3,
        "three points produce three render cells"
    );

    if (renderCells.size() != 3)
    {
        return;
    }

    bool foundNear = false;
    bool foundMid = false;
    bool foundFar = false;

    for (const auto& cell : renderCells)
    {
        const float width =
            cell.maxX - cell.minX;

        const float height =
            cell.maxY - cell.minY;

        checkNear(
            width,
            cell.resolution,
            0.000001f,
            "render cell width matches resolution"
        );

        checkNear(
            height,
            cell.resolution,
            0.000001f,
            "render cell height matches resolution"
        );

        if (std::fabs(
                cell.resolution - 0.05f
            ) < 0.000001f)
        {
            foundNear = true;
        }

        if (std::fabs(
                cell.resolution - 0.10f
            ) < 0.000001f)
        {
            foundMid = true;
        }

        if (std::fabs(
                cell.resolution - 0.25f
            ) < 0.000001f)
        {
            foundFar = true;
        }
    }

    check(
        foundNear,
        "near geometry uses 0.05m"
    );

    check(
        foundMid,
        "mid geometry uses 0.10m"
    );

    check(
        foundFar,
        "far geometry uses 0.25m"
    );
}


/*
 * ============================================================
 * Mapping bounds
 * ============================================================
 */

void testMappingBounds()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        profile
    );

    PointCloud cloud;

    cloud.addPoint(
        Point{
            0.999f,
            0.999f,
            5.0f,
            20.0f
        }
    );

    grid.build(
        cloud
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "boundary point produces one render cell"
    );

    if (renderCells.size() != 1)
    {
        return;
    }

    const auto& cell =
        renderCells[0];

    check(
        cell.minX >= 0.0f &&
        cell.maxX <= 1.0f,
        "cell X bounds stay inside mapping region"
    );

    check(
        cell.minY >= 0.0f &&
        cell.maxY <= 1.0f,
        "cell Y bounds stay inside mapping region"
    );
}


/*
 * ============================================================
 * Rebuild
 * ============================================================
 */

void testRebuildGeometry()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -10.0f,
        10.0f,
        -10.0f,
        10.0f,
        profile
    );

    PointCloud firstCloud;

    firstCloud.addPoint(
        Point{
            1.0f,
            1.0f,
            1.0f,
            10.0f
        }
    );

    grid.build(
        firstCloud
    );

    check(
        grid.getCellCount() == 1,
        "first build creates one cell"
    );

    PointCloud secondCloud;

    secondCloud.addPoint(
        Point{
            5.0f,
            5.0f,
            2.0f,
            20.0f
        }
    );

    grid.build(
        secondCloud
    );

    check(
        grid.getCellCount() == 1,
        "rebuild removes previous cells"
    );

    const auto renderCells =
        grid.getRenderCells();

    check(
        renderCells.size() == 1,
        "rebuild creates one new render cell"
    );

    if (renderCells.size() != 1)
    {
        return;
    }

    checkNear(
        renderCells[0].minX,
        5.0f,
        0.000001f,
        "rebuild geometry uses new X position"
    );

    checkNear(
        renderCells[0].minY,
        5.0f,
        0.000001f,
        "rebuild geometry uses new Y position"
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
        << "SIH 26 Adaptive Geometry Tests\n"
        << "Phase 5.2\n"
        << "========================================\n";

    testSingleCellGeometry();

    testNegativeCoordinateGeometry();

    testResolutionGeometry();

    testMappingBounds();

    testRebuildGeometry();

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