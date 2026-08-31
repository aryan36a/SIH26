# SIH 26 - LiDAR Mapping Engine

A hand-coded C++ LiDAR processing and visualization engine developed for
the SIH 26 project.

The project is being built incrementally, starting from raw LiDAR
point-cloud loading and visualization and progressing toward semantic
understanding and a variable-resolution 2.5D elevation/semantic map.

------------------------------------------------------------------------

## Project Status

Current development includes:

-   LiDAR point-cloud loading from `.bin` files
-   Point-cloud validation and statistics
-   OpenGL-based point-cloud visualization
-   3D camera controls
-   Spatial grid generation
-   Per-cell elevation calculation
-   Per-cell intensity calculation
-   Per-cell point counting
-   Minimum elevation tracking
-   Grid-based point visualization
-   Test LiDAR dataset generation

### Planned Processing Pipeline

``` text
Raw LiDAR Point Cloud
        ↓
Point Cloud Processing
        ↓
Spatial Representation
        ↓
AI / Semantic Understanding
        ↓
Semantic Classification
        ↓
Variable-Resolution 2.5D Map
        ↓
Visualization / Rendering
```

The AI semantic-understanding and semantic-mapping stages are planned
phases of development.

------------------------------------------------------------------------

## Repository Structure

``` text
SIH/
│
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── data/
│   ├── test.bin
│   └── test.xyz
│
├── external/
│   └── glad/
│
├── include/
│   ├── Camera.h
│   ├── MapCell.h
│   ├── Point.h
│   ├── PointCloud.h
│   ├── Renderer.h
│   └── SpatialGrid.h
│
├── src/
│   ├── Camera.cpp
│   ├── create_test_bin.cpp
│   ├── main.cpp
│   ├── MapCell.cpp
│   ├── PointCloud.cpp
│   ├── Renderer.cpp
│   └── SpatialGrid.cpp
│
├── shaders/
│   ├── grid.frag
│   ├── grid.vert
│   ├── point.frag
│   └── point.vert
│
└── vid/
    └── tutorial video
```

------------------------------------------------------------------------

## Tutorial Video

The `vid/` directory contains the tutorial video used as a development
reference.

Open the `vid/` directory in the repository to access the video.

The tutorial video is kept separate from the source code but is included
as part of the project's development documentation.

------------------------------------------------------------------------

## Prerequisites

### Operating System

The current development environment uses:

-   Windows
-   MSYS2 UCRT64
-   MinGW
-   CMake
-   Git

### Required Software

A C++17-compatible compiler is required.

The current setup uses the MinGW compiler provided by MSYS2 UCRT64.

Check CMake:

``` powershell
cmake --version
```

Check Git:

``` powershell
git --version
```

### OpenGL

The project currently targets:

``` text
OpenGL 3.3
```

### GLFW

GLFW is used for:

-   Window creation
-   OpenGL context creation
-   Keyboard input
-   Mouse input

### GLAD

GLAD is included in the repository:

``` text
external/glad/
```

------------------------------------------------------------------------

## Building the Project

Clone the repository:

``` powershell
git clone <repository-url>
cd SIH
```

Create the build directory:

``` powershell
cmake -S . -B build
```

Build the project:

``` powershell
cmake --build build
```

The main executable is generated as:

``` text
build/SIH26.exe
```

------------------------------------------------------------------------

## Running the LiDAR Viewer

Run from the project root:

``` powershell
.\build\SIH26.exe
```

The application loads:

``` text
data/test.bin
```

and displays the LiDAR point cloud using OpenGL.

------------------------------------------------------------------------

## Generating Test LiDAR Data

The project contains a test-data generator:

``` text
src/create_test_bin.cpp
```

Build the project:

``` powershell
cmake --build build
```

Then run:

``` powershell
.\build\CreateTestBin.exe
```

This generates:

``` text
data/test.bin
```

The generated synthetic point cloud is used to test the point-cloud
loader, spatial grid, and renderer.

------------------------------------------------------------------------

## Current LiDAR Data Flow

``` text
data/test.bin
      ↓
PointCloud
      ↓
Point validation / statistics
      ↓
SpatialGrid
      ↓
MapCell
      ↓
Renderer
      ↓
OpenGL
```

Each `MapCell` currently stores information such as:

``` text
elevation
intensity
pointCount
elevationSum
minimumElevation
```

The spatial grid converts continuous XY point-cloud space into discrete
cells.

------------------------------------------------------------------------

## Spatial Grid

The current implementation uses a configurable cell size.

Example:

``` cpp
SpatialGrid grid(
    cloud.getMinX(),
    cloud.getMaxX(),
    cloud.getMinY(),
    cloud.getMaxY(),
    1.0f
);
```

Each LiDAR point is mapped into a grid cell using its X/Y coordinates.

For cells containing points, the system calculates:

``` text
Average Elevation
Average Intensity
Point Count
Minimum Elevation
```

This forms the initial foundation for the project's 2.5D mapping system.

------------------------------------------------------------------------

## Rendering

The renderer currently supports:

-   LiDAR point rendering
-   Grid-derived point rendering
-   OpenGL shaders
-   Depth testing
-   3D camera movement
-   Mouse-based camera control
-   Keyboard navigation

### Controls

  Key     Action
  ------- -------------------------
  W       Move forward
  S       Move backward
  A       Move left
  D       Move right
  Q       Move down
  E       Move up
  Mouse   Look around
  ESC     Capture / release mouse

------------------------------------------------------------------------

## Development Philosophy

The core processing and mapping engine is being implemented manually in
C++.

The goal is to understand and implement the underlying pipeline rather
than relying on a black-box mapping solution.

Major components will be developed incrementally:

``` text
Point Cloud
     ↓
Spatial Grid
     ↓
Elevation Mapping
     ↓
Semantic Processing
     ↓
Adaptive Resolution
     ↓
2.5D Semantic Map
     ↓
Rendering
```

------------------------------------------------------------------------

## Planned Features

### Point Cloud Processing

-   [x] Binary point-cloud loading
-   [x] Point statistics
-   [x] Synthetic test-data generation

### Spatial Mapping

-   [x] Spatial grid
-   [x] Point-to-cell mapping
-   [x] Average elevation
-   [x] Average intensity
-   [x] Point count
-   [x] Minimum elevation

### Visualization

-   [x] OpenGL initialization
-   [x] Point rendering
-   [x] Camera controls
-   [x] Grid-derived visualization

### AI / Semantic Pipeline

-   [ ] Point-cloud preprocessing
-   [ ] Ground / non-ground separation
-   [ ] Semantic feature extraction
-   [ ] Object / terrain classification
-   [ ] Semantic labels
-   [ ] Semantic map integration

### Variable-Resolution 2.5D Mapping

-   [ ] Multi-resolution spatial representation
-   [ ] High-resolution local mapping
-   [ ] Progressive resolution reduction with distance
-   [ ] Elevation representation
-   [ ] Semantic representation
-   [ ] Map update strategy

### Final Visualization

-   [ ] Variable-resolution map rendering
-   [ ] Semantic visualization
-   [ ] Terrain/object visualization
-   [ ] Performance optimization

------------------------------------------------------------------------

## License

This project is currently developed as part of the SIH 26 project.

License information will be added as the project is finalized.
