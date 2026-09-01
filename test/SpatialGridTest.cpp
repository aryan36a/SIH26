#include <cassert>
#include <cmath>
#include <iostream>

#include "PointCloud.h"
#include "SpatialGrid.h"

bool nearlyEqual(float a, float b, float epsilon = 0.0001f) {
  return std::fabs(a - b) < epsilon;
}

int main() {

  // ==================================================
  // TEST 1: Basic cell aggregation
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{0.1f, 0.1f, 10.0f, 20.0f});
    cloud.addPoint(Point{0.2f, 0.2f, 12.0f, 30.0f});
    cloud.addPoint(Point{0.8f, 0.7f, 8.0f, 40.0f});

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    assert(grid.getWidth() == 2);
    assert(grid.getHeight() == 1);

    const MapCell &cell = grid.getCell(0, 0);

    assert(cell.pointCount == 3);

    assert(nearlyEqual(cell.minimumElevation, 8.0f));

    assert(nearlyEqual(cell.maximumElevation, 12.0f));

    assert(nearlyEqual(cell.elevation, 10.0f));

    assert(nearlyEqual(cell.intensity, 30.0f));

    assert(nearlyEqual(
        grid.getHeightVariation(0, 0),
        4.0f
    ));
  }

  // ==================================================
  // TEST 2: Points in separate cells
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{0.2f, 0.5f, 10.0f, 20.0f});
    cloud.addPoint(Point{1.2f, 0.5f, 20.0f, 40.0f});

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    assert(grid.getCell(0, 0).pointCount == 1);
    assert(grid.getCell(1, 0).pointCount == 1);

    assert(nearlyEqual(
        grid.getElevation(0, 0),
        10.0f
    ));

    assert(nearlyEqual(
        grid.getElevation(1, 0),
        20.0f
    ));
  }

  // ==================================================
  // TEST 3: Exact cell boundary
  // ==================================================

  {
    PointCloud cloud;

    // x = 1.0 should belong to cell 1
    cloud.addPoint(Point{1.0f, 0.5f, 50.0f, 10.0f});

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    assert(grid.getCell(0, 0).pointCount == 0);

    assert(grid.getCell(1, 0).pointCount == 1);
  }

  // ==================================================
  // TEST 4: Point just before boundary
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{
        0.9999f,
        0.5f,
        25.0f,
        10.0f
    });

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    assert(grid.getCell(0, 0).pointCount == 1);

    assert(grid.getCell(1, 0).pointCount == 0);
  }

  // ==================================================
  // TEST 5: Maximum boundary
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{
        2.0f,
        1.0f,
        30.0f,
        15.0f
    });

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    // maxX/maxY are mapped into the final cell.

    assert(grid.getCell(1, 0).pointCount == 1);
  }

  // ==================================================
  // TEST 6: Empty cell
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{
        0.2f,
        0.2f,
        10.0f,
        20.0f
    });

    SpatialGrid grid(
        0.0f,
        2.0f,
        0.0f,
        2.0f,
        1.0f
    );

    grid.build(cloud);

    assert(grid.getWidth() == 2);
    assert(grid.getHeight() == 2);

    assert(grid.hasData(0, 0));

    assert(!grid.hasData(1, 0));

    assert(!grid.hasData(0, 1));

    assert(!grid.hasData(1, 1));

    assert(grid.getCell(1, 1).pointCount == 0);
  }

  // ==================================================
  // TEST 7: Height variation
  // ==================================================

  {
    PointCloud cloud;

    cloud.addPoint(Point{0.1f, 0.1f, 2.0f, 10.0f});
    cloud.addPoint(Point{0.2f, 0.2f, 5.0f, 20.0f});
    cloud.addPoint(Point{0.3f, 0.3f, 8.0f, 30.0f});

    SpatialGrid grid(
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f
    );

    grid.build(cloud);

    assert(nearlyEqual(
        grid.getHeightVariation(0, 0),
        6.0f
    ));
  }
    // ==================================================
  // TEST 8: Rebuilding the same grid
  // ==================================================

  {
    PointCloud cloudA;

    cloudA.addPoint(Point{
        0.2f,
        0.2f,
        10.0f,
        20.0f
    });

    PointCloud cloudB;

    cloudB.addPoint(Point{
        0.2f,
        0.2f,
        50.0f,
        80.0f
    });

    SpatialGrid grid(
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f
    );

    // First build
    grid.build(cloudA);

    assert(grid.getCell(0, 0).pointCount == 1);

    assert(nearlyEqual(
        grid.getElevation(0, 0),
        10.0f
    ));

    assert(nearlyEqual(
        grid.getIntensity(0, 0),
        20.0f
    ));

    // Second build using completely different data
    grid.build(cloudB);

    // Old data must be gone
    assert(grid.getCell(0, 0).pointCount == 1);

    assert(nearlyEqual(
        grid.getElevation(0, 0),
        50.0f
    ));

    assert(nearlyEqual(
        grid.getIntensity(0, 0),
        80.0f
    ));

    assert(nearlyEqual(
        grid.getCell(0, 0).minimumElevation,
        50.0f
    ));

    assert(nearlyEqual(
        grid.getCell(0, 0).maximumElevation,
        50.0f
    ));}

  std::cout << "====================================\n";
  std::cout << "All SpatialGrid tests passed!\n";
  std::cout << "====================================\n";

  return 0;
}