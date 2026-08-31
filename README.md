# SIH 26' --- LiDAR Semantic Mapping Engine

A hand-coded C++ prototype for processing 3D LiDAR point clouds and
building the foundations of a variable-resolution 2.5D
elevation/semantic mapping system.

The project is being developed for the **SIH 26'** problem statement,
with the long-term pipeline:

``` text
Raw 3D LiDAR
     ↓
Point Cloud Ingestion
     ↓
Spatial Representation
     ↓
AI / Semantic Understanding
     ↓
Variable-Resolution 2.5D Map
     ↓
Rendering / Visualization
```

The current implementation focuses on building the core C++ mapping and
rendering infrastructure before integrating the AI/semantic pipeline.

------------------------------------------------------------------------

## Current Status

### Implemented

-   C++17 project structure
-   CMake build system
-   GLFW + OpenGL 3.3 window
-   GLAD OpenGL loader
-   GLM mathematics
-   LiDAR `.bin` point-cloud loading
-   XYZ point-cloud loading
-   Point-cloud statistics:
    -   point count
    -   X/Y/Z ranges
    -   intensity range
-   OpenGL point-cloud renderer
-   Camera movement and mouse look
-   Spatial grid representation
-   Grid cell indexing
-   Per-cell:
    -   point count
    -   average elevation
    -   average intensity
    -   minimum elevation
-   Grid generation from raw point-cloud data
-   Test `.bin` generator for development
-   Basic grid visualization infrastructure

### In Progress

The next stages are:

1.  Improve the 2.5D map representation.
2.  Add terrain/elevation processing.
3.  Introduce variable-resolution grid logic.
4.  Add semantic classes to map cells.
5.  Build the AI/semantic understanding pipeline.
6.  Connect AI output to the mapping engine.
7.  Improve visualization and interaction.
8.  Test against larger and more realistic LiDAR datasets.

> **Important:** AI pipelining is part of the planned architecture. It
> is intentionally introduced after the low-level point-cloud and
> spatial-mapping foundations are stable.

------------------------------------------------------------------------

## Project Structure

``` text
SIH/
├── CMakeLists.txt
├── README.md
├── .gitignore
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
│   ├── MapCell.cpp
│   ├── PointCloud.cpp
│   ├── Renderer.cpp
│   ├── SpatialGrid.cpp
│   ├── create_test_bin.cpp
│   └── main.cpp
│
├── shaders/
│   ├── point.vert
│   ├── point.frag
│   ├── grid.vert
│   └── grid.frag
│
├── data/
│   └── test.bin
│
├── external/
│   └── glad/
│
└── build/
```

------------------------------------------------------------------------

# Prerequisites

## Required

### C++ compiler

The project uses **C++17**.

The current development environment uses:

-   Windows
-   MSYS2
-   UCRT64
-   MinGW-w64 GCC

### CMake

CMake **3.20 or newer**.

### GLFW

GLFW is required for:

-   window creation
-   OpenGL context creation
-   keyboard input
-   mouse input

### OpenGL

An OpenGL 3.3 compatible graphics driver is required.

### GLM

GLM is used for:

-   vectors
-   matrices
-   camera transformations
-   projection matrices

### GLAD

The required GLAD source is included in:

``` text
external/glad/
```

------------------------------------------------------------------------

# Building

Open a terminal at the project root:

``` powershell
cd D:\SIH
```

Create the build directory:

``` powershell
cmake -S . -B build
```

Build the project:

``` powershell
cmake --build build
```

A successful build should produce:

``` text
build/SIH26.exe
build/CreateTestBin.exe
```

------------------------------------------------------------------------

# Running

## 1. Generate test LiDAR data

The project includes a test-data generator so test cases do not need to
be hardcoded into the main application.

Run:

``` powershell
.\build\CreateTestBin.exe
```

This creates:

``` text
data/test.bin
```

The generated file contains synthetic LiDAR points in the expected
binary format.

------------------------------------------------------------------------

## 2. Run the LiDAR viewer

From the project root:

``` powershell
.\build\SIH26.exe
```

You should see console output similar to:

``` text
OpenGL initialized successfully
OpenGL version: 3.3.0
Point cloud loaded successfully
Points: 10000
X range: ...
Y range: ...
Z range: ...
Intensity range: ...
Grid width: ...
Grid height: ...
```

The application then opens the OpenGL visualization window.

------------------------------------------------------------------------

# Controls

  Key     Action
  ------- ----------------------
  `W`     Move forward
  `S`     Move backward
  `A`     Move left
  `D`     Move right
  `Q`     Move down
  `E`     Move up
  Mouse   Look around
  `ESC`   Toggle mouse capture

------------------------------------------------------------------------

# Point Cloud Format

The binary LiDAR format currently used by the loader is:

``` text
float x
float y
float z
float intensity
```

Therefore every point occupies:

``` text
4 × sizeof(float)
```

bytes.

Conceptually:

``` text
Point {
    x
    y
    z
    intensity
}
```

The loader reads the binary file sequentially without converting the
entire file into text.

------------------------------------------------------------------------

# Spatial Grid

The current mapping foundation uses a 2D spatial grid over the X-Y
plane.

Each grid cell stores information derived from the LiDAR points that
fall inside it.

Conceptually:

``` text
                 X
        ───────────────────→

        ┌─────┬─────┬─────┐
        │     │     │     │
        ├─────┼─────┼─────┤
        │     │     │     │
        ├─────┼─────┼─────┤
        │     │     │     │
        └─────┴─────┴─────┘
        ↑
        Y
```

Current cell information includes:

``` text
pointCount
elevation
elevationSum
minimumElevation
intensity
```

The current elevation value is calculated as the mean Z value of the
points in a cell.

------------------------------------------------------------------------

# Rendering Pipeline

The current renderer has two primary concepts:

### Point cloud

``` text
PointCloud
    ↓
OpenGL VBO
    ↓
VAO
    ↓
Vertex Shader
    ↓
Fragment Shader
    ↓
Rendered LiDAR points
```

### Grid

``` text
PointCloud
    ↓
SpatialGrid
    ↓
MapCell[]
    ↓
Grid representation
    ↓
OpenGL
```

The renderer is being kept separate from the mapping/data-processing
code so that the mapping engine can evolve independently from
visualization.

------------------------------------------------------------------------

# Planned AI Pipeline

The AI component is a major part of the final architecture.

The intended flow is:

``` text
             Raw LiDAR
                 │
                 ▼
          Point Cloud Loader
                 │
                 ▼
          Spatial Processing
                 │
                 ▼
       ┌─────────────────────┐
       │ AI / Semantic Model │
       └─────────────────────┘
                 │
                 ▼
        Semantic Information
                 │
                 ▼
       2.5D Semantic Map
                 │
                 ▼
       Variable Resolution
                 │
                 ▼
             Renderer
```

The AI pipeline is expected to provide semantic information such as
terrain/object classes that can later be associated with map cells.

The low-level C++ engine remains responsible for:

-   point-cloud ingestion
-   spatial indexing
-   map representation
-   map updates
-   resolution management
-   rendering

This separation allows the AI system to act as a semantic-information
provider rather than coupling the entire application to a single model.

------------------------------------------------------------------------

# Variable-Resolution Mapping

The final mapping system is intended to use different spatial
resolutions depending on distance and importance.

For example:

``` text
Sensor
  ●
  │
  ├── High resolution
  │      5 cm cells
  │
  ├──────── Medium resolution
  │
  └──────────────── Low resolution
```

The exact resolution strategy will be implemented as the mapping engine
develops.

The goal is to retain high-detail information near the sensor while
reducing memory and computation requirements farther away.

------------------------------------------------------------------------

# Development Philosophy

This project is intentionally being developed from the fundamentals
upward.

The core engine is being hand-coded in C++ rather than relying on a
large pre-built mapping framework.

The development order is approximately:

``` text
1. Project / Build System
        ↓
2. Point Cloud Representation
        ↓
3. Point Cloud Loading
        ↓
4. OpenGL Rendering
        ↓
5. Camera / Interaction
        ↓
6. Spatial Grid
        ↓
7. Elevation / MapCell Processing
        ↓
8. Variable Resolution
        ↓
9. Semantic Representation
        ↓
10. AI Pipeline
        ↓
11. AI → Map Integration
        ↓
12. Final Visualization
```

------------------------------------------------------------------------

# Testing

A synthetic point-cloud generator is included:

``` text
src/create_test_bin.cpp
```

Build it with:

``` powershell
cmake --build build
```

Then run:

``` powershell
.\build\CreateTestBin.exe
```

This makes it possible to test the loader and mapping engine with
generated data instead of embedding test cases inside the production
code.

------------------------------------------------------------------------

# Clean Build

If the build directory becomes inconsistent:

``` powershell
Remove-Item -Recurse -Force build
```

Then configure again:

``` powershell
cmake -S . -B build
```

Build:

``` powershell
cmake --build build
```

------------------------------------------------------------------------

# Git Workflow

Initialize the repository:

``` powershell
git init
```

Add files:

``` powershell
git add .
```

Create the first commit:

``` powershell
git commit -m "Initial LiDAR mapping engine"
```

Check repository status:

``` powershell
git status
```

------------------------------------------------------------------------

# Roadmap

-   [x] CMake project
-   [x] OpenGL initialization
-   [x] Point structure
-   [x] Point-cloud loader
-   [x] Point-cloud statistics
-   [x] Point rendering
-   [x] Camera controls
-   [x] Spatial grid
-   [x] Map cell aggregation
-   [x] Synthetic LiDAR generator
-   [ ] Robust map-cell representation
-   [ ] Variable-resolution spatial map
-   [ ] Terrain/elevation processing
-   [ ] Semantic classes
-   [ ] AI semantic pipeline
-   [ ] AI → map integration
-   [ ] Dynamic map updates
-   [ ] Large-scale LiDAR testing
-   [ ] Performance optimization
-   [ ] Final visualization
-   [ ] Final SIH demonstration build

------------------------------------------------------------------------

# License

This project is currently being developed as part of the SIH 26'
project.

License details can be added when the repository is finalized.
