#include "AdaptiveCellMesh.h"
#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "ResolutionProfile.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>


namespace
{

int testsPassed = 0;
int testsFailed = 0;


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
 * Empty grid
 * ============================================================
 */

void testEmptyGrid()
{
    ResolutionProfile profile;

    AdaptiveSpatialGrid grid(
        -10.0f,
        10.0f,
        -10.0f,
        10.0f,
        profile
    );

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    check(
        mesh.getVertexCount() == 0,
        "empty grid produces zero vertices"
    );

    check(
        mesh.getIndexCount() == 0,
        "empty grid produces zero indices"
    );

    check(
        mesh.getVertices().empty(),
        "empty grid has no vertex data"
    );

    check(
        mesh.getIndices().empty(),
        "empty grid has no index data"
    );
}


/*
 * ============================================================
 * Single cell
 * ============================================================
 */

void testSingleCell()
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

    grid.build(cloud);

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    check(
        grid.getCellCount() == 1,
        "single point produces one adaptive cell"
    );

    check(
        mesh.getVertexCount() == 4,
        "one cell produces four vertices"
    );

    check(
        mesh.getIndexCount() == 6,
        "one cell produces six indices"
    );
}


/*
 * ============================================================
 * Vertex geometry
 * ============================================================
 */

void testVertexGeometry()
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
     * Distance from origin:
     *
     * sqrt(1^2 + 1^2) ~= 1.414m
     *
     * Therefore:
     *
     * resolution = 0.05m
     */

    cloud.addPoint(
        Point{
            1.0f,
            1.0f,
            5.0f,
            25.0f
        }
    );

    grid.build(cloud);

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    const auto& vertices =
        mesh.getVertices();

    check(
        vertices.size() == 4,
        "geometry test contains four vertices"
    );

    if (vertices.size() != 4)
    {
        return;
    }


    constexpr float resolution = 0.05f;
    constexpr float halfSize = resolution * 0.5f;

    /*
     * AdaptiveSpatialGrid converts the world coordinate
     * into integer cell coordinates:
     *
     * cellX = floor(1.0 / 0.05) = 20
     * cellY = floor(1.0 / 0.05) = 20
     *
     * It then reconstructs the cell center:
     *
     * centerX = (20 + 0.5) * 0.05
     *         = 1.025
     *
     * centerY = (20 + 0.5) * 0.05
     *         = 1.025
     */

    constexpr int cellX = 20;
    constexpr int cellY = 20;

    const float centerX =
        (static_cast<float>(cellX) + 0.5f) *
        resolution;

    const float centerY =
        (static_cast<float>(cellY) + 0.5f) *
        resolution;


    checkNear(
        vertices[0].x,
        centerX - halfSize,
        0.000001f,
        "bottom-left X uses cell resolution"
    );

    checkNear(
        vertices[0].y,
        centerY - halfSize,
        0.000001f,
        "bottom-left Y uses cell resolution"
    );


    checkNear(
        vertices[1].x,
        centerX + halfSize,
        0.000001f,
        "bottom-right X uses cell resolution"
    );

    checkNear(
        vertices[1].y,
        centerY - halfSize,
        0.000001f,
        "bottom-right Y uses cell resolution"
    );


    checkNear(
        vertices[2].x,
        centerX + halfSize,
        0.000001f,
        "top-right X uses cell resolution"
    );

    checkNear(
        vertices[2].y,
        centerY + halfSize,
        0.000001f,
        "top-right Y uses cell resolution"
    );


    checkNear(
        vertices[3].x,
        centerX - halfSize,
        0.000001f,
        "top-left X uses cell resolution"
    );

    checkNear(
        vertices[3].y,
        centerY + halfSize,
        0.000001f,
        "top-left Y uses cell resolution"
    );
}


/*
 * ============================================================
 * Vertex attributes
 * ============================================================
 */

void testVertexAttributes()
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
            7.5f,
            42.0f
        }
    );

    grid.build(cloud);

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    const auto& vertices =
        mesh.getVertices();

    check(
        vertices.size() == 4,
        "attribute test contains four vertices"
    );

    if (vertices.empty())
    {
        return;
    }

    for (std::size_t i = 0;
         i < vertices.size();
         ++i)
    {
        checkNear(
            vertices[i].z,
            7.5f,
            0.000001f,
            "elevation propagated to mesh vertex"
        );

        checkNear(
            vertices[i].intensity,
            42.0f,
            0.000001f,
            "intensity propagated to mesh vertex"
        );
    }
}


/*
 * ============================================================
 * Index topology
 * ============================================================
 */

void testIndexTopology()
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
            2.0f,
            10.0f
        }
    );

    grid.build(cloud);

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    const auto& indices =
        mesh.getIndices();

    check(
        indices.size() == 6,
        "single-cell topology contains six indices"
    );

    if (indices.size() != 6)
    {
        return;
    }

    check(
        indices[0] == 0 &&
        indices[1] == 1 &&
        indices[2] == 2 &&
        indices[3] == 0 &&
        indices[4] == 2 &&
        indices[5] == 3,
        "indices form the expected two triangles"
    );
}


/*
 * ============================================================
 * Multiple cells
 * ============================================================
 */

void testMultipleCells()
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

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    const std::size_t cellCount =
        grid.getCellCount();

    check(
        cellCount == 3,
        "three separated points produce three cells"
    );

    check(
        mesh.getVertexCount() == cellCount * 4,
        "mesh creates four vertices per cell"
    );

    check(
        mesh.getIndexCount() == cellCount * 6,
        "mesh creates six indices per cell"
    );
}


/*
 * ============================================================
 * Adaptive resolutions reach the mesh
 * ============================================================
 */

void testAdaptiveResolutions()
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
     * Near band.
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
     * Mid band.
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
     * Far band.
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

    AdaptiveCellMesh mesh;

    mesh.build(grid);

    const auto& vertices =
        mesh.getVertices();

    check(
        vertices.size() == 12,
        "three resolution bands produce twelve vertices"
    );

    check(
        mesh.getIndexCount() == 18,
        "three resolution bands produce eighteen indices"
    );


    bool foundNear = false;
    bool foundMid = false;
    bool foundFar = false;


    /*
     * Each cell has four consecutive vertices.
     *
     * Therefore the distance between vertex 0
     * and vertex 1 is the cell width.
     */

    for (std::size_t i = 0;
         i < vertices.size();
         i += 4)
    {
        const float width =
            std::fabs(
                vertices[i + 1].x -
                vertices[i].x
            );

        if (std::fabs(width - 0.05f) < 0.000001f)
        {
            foundNear = true;
        }

        if (std::fabs(width - 0.10f) < 0.000001f)
        {
            foundMid = true;
        }

        if (std::fabs(width - 0.25f) < 0.000001f)
        {
            foundFar = true;
        }
    }


    check(
        foundNear,
        "mesh contains a 0.05m cell"
    );


    check(
        foundMid,
        "mesh contains a 0.10m cell"
    );

    check(
        foundFar,
        "mesh contains a 0.25m cell"
    );
}

/*
 * A high local return must never acquire a synthetic global floor.
 */
void testLocalStructureExtent()
{
    ResolutionProfile profile;
    AdaptiveSpatialGrid grid(
        -10.0f, 10.0f, -10.0f, 10.0f, profile);

    PointCloud cloud;
    cloud.addPoint({1.000f, 1.000f, 2.8f, 10.0f});
    cloud.addPoint({1.010f, 1.010f, 3.0f, 10.0f});
    cloud.addPoint({1.020f, 1.020f, 3.3f, 10.0f});
    grid.build(cloud);

    const auto cells = grid.getRenderCells();
    check(cells.size() == 1, "local structure points share one cell");
    if (cells.size() != 1) return;

    checkNear(cells[0].minimumElevation, 2.8f, 0.000001f,
              "local structure minimum is observed minimum");
    checkNear(cells[0].maximumElevation, 3.3f, 0.000001f,
              "local structure maximum is observed maximum");

    AdaptiveCellMesh mesh;
    mesh.build(grid);

    float minimumVertexZ = mesh.getVertices().front().z;
    float maximumVertexZ = minimumVertexZ;
    for (const auto& vertex : mesh.getVertices())
    {
        minimumVertexZ = std::min(minimumVertexZ, vertex.z);
        maximumVertexZ = std::max(maximumVertexZ, vertex.z);
    }

    check(minimumVertexZ >= 2.8f - 0.000001f,
          "structural mesh has no global floor extrusion");
    check(maximumVertexZ <= 3.3f + 0.000001f,
          "structural mesh does not exceed local maximum");
}

void testLocalWallSupport()
{
    ResolutionProfile profile;
    AdaptiveSpatialGrid grid(
        -10.0f, 10.0f, -10.0f, 10.0f, profile);

    PointCloud cloud;
    cloud.addPoint({0.95f, 1.00f, 0.10f, 1.0f});
    cloud.addPoint({1.05f, 1.00f, 0.12f, 1.0f});
    cloud.addPoint({1.00f, 0.95f, 0.11f, 1.0f});
    cloud.addPoint({1.00f, 1.05f, 0.13f, 1.0f});
    cloud.addPoint({1.00f, 1.00f, 2.00f, 1.0f});
    cloud.addPoint({1.01f, 1.01f, 2.50f, 1.0f});
    grid.build(cloud);

    AdaptiveCellMesh mesh;
    mesh.build(grid);

    bool hasLocalBase = false;
    for (const auto& vertex : mesh.getVertices())
    {
        if (std::fabs(vertex.z - 0.11f) < 0.000001f)
        {
            hasLocalBase = true;
            break;
        }
    }

    check(hasLocalBase, "supported wall uses local neighboring ground");
}

void testUnsupportedAerialCell()
{
    ResolutionProfile profile;
    AdaptiveSpatialGrid grid(
        -10.0f, 10.0f, -10.0f, 10.0f, profile);

    PointCloud cloud;
    cloud.addPoint({5.00f, 5.00f, 3.0f, 1.0f});
    cloud.addPoint({5.01f, 5.01f, 3.5f, 1.0f});
    grid.build(cloud);

    AdaptiveCellMesh mesh;
    mesh.build(grid);

    check(mesh.getVertexCount() == 4,
          "unsupported aerial cell has no vertical needle");
    check(mesh.getIndexCount() == 6,
          "unsupported aerial cell remains a sampled surface");
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
        << "SIH 26 Adaptive Cell Mesh Tests\n"
        << "Phase 4.6\n"
        << "========================================\n";


    testEmptyGrid();

    testSingleCell();

    testVertexGeometry();

    testVertexAttributes();

    testIndexTopology();

    testMultipleCells();

    testAdaptiveResolutions();

    testLocalWallSupport();

    testUnsupportedAerialCell();

    testLocalStructureExtent();


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