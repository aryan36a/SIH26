# SIH 26 --- LiDAR Mapping & Semantic 2.5D Engine

A hand-coded C++ LiDAR processing, spatial mapping, and OpenGL
visualization project being developed for **SIH 26**.

The project is being built incrementally from the low-level point-cloud
layer toward an AI-assisted semantic understanding pipeline and a
**variable-resolution 2.5D elevation/semantic map**.

------------------------------------------------------------------------

## 🎥 Project Tutorial / Development Video

### Watch the tutorial

> **GitHub does not reliably render a repository-local `.mp4` as an
> inline video player inside `README.md`.**
>
> The project therefore keeps the tutorial video in the `vid/`
> directory. If GitHub has been given a hosted video URL, replace the
> placeholder below with that URL.

**Tutorial video:** [`vid/`](./vid/)

If you want the video to appear as a playable player on the GitHub
README page, upload the video to a GitHub-supported hosted location and
replace the link above with that hosted video.

------------------------------------------------------------------------

## Project Overview

The intended processing architecture is:

``` text
                 RAW LiDAR
                    │
                    ▼
          Point Cloud Acquisition
                    │
                    ▼
          Point Cloud Preprocessing
                    │
                    ▼
             Spatial Mapping
                    │
                    ▼
        ┌─────────────────────────┐
        │ AI / Semantic Pipeline   │
        │                         │
        │ • Feature extraction    │
        │ • Classification        │
        │ • Semantic labeling     │
        └────────────┬────────────┘
                     │
                     ▼
       Variable-Resolution 2.5D Map
                     │
          ┌──────────┴──────────┐
          ▼                     ▼
     Elevation Data       Semantic Data
          │                     │
          └──────────┬──────────┘
                     ▼
                OpenGL Renderer
```

The current implementation is focused on establishing the **C++
point-cloud and spatial-mapping foundation** before implementing the
AI/semantic pipeline.

------------------------------------------------------------------------

# Current Status

### Implemented

-   [x] C++17 project setup
-   [x] CMake build system
-   [x] GLFW window creation
-   [x] GLAD OpenGL loading
-   [x] OpenGL 3.3 initialization
-   [x] LiDAR `.bin` loading
-   [x] XYZ point-cloud loading
-   [x] Point-cloud statistics
-   [x] Synthetic LiDAR test-data generator
-   [x] 3D camera
-   [x] Keyboard camera movement
-   [x] Mouse camera control
-   [x] OpenGL point-cloud rendering
-   [x] Spatial grid generation
-   [x] Point-to-cell mapping
-   [x] Per-cell point count
-   [x] Per-cell average elevation
-   [x] Per-cell average intensity
-   [x] Per-cell minimum elevation
-   [x] Grid-derived rendering

### Next Major Phase

The next major development phase is the **AI / semantic pipelining
layer**.

``` text
Raw Point Cloud
      ↓
Preprocessing
      ↓
Feature Extraction
      ↓
AI / Semantic Classification
      ↓
Semantic Labels
      ↓
Semantic + Elevation Map
```

------------------------------------------------------------------------

# Repository Structure

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
│       ├── include/
│       └── src/
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
├── vid/
│   └── tutorial video
│
└── build/
    └── generated locally; ignored by Git
```

------------------------------------------------------------------------

# Prerequisites

## Operating System

The current development environment is:

-   Windows
-   MSYS2 UCRT64
-   MinGW-w64
-   CMake
-   Git

The project can be adapted to other platforms later.

## Required Tools

### C++ Compiler

A compiler supporting **C++17** is required.

The current project uses the MinGW-w64 compiler supplied by MSYS2
UCRT64.

Check the compiler:

``` powershell
g++ --version
```

### CMake

Required version:

``` text
CMake 3.20+
```

Check:

``` powershell
cmake --version
```

### Git

Check:

``` powershell
git --version
```

### GLFW

GLFW is required for:

-   OpenGL context creation
-   Window creation
-   Keyboard input
-   Mouse input

The current CMake configuration expects GLFW to be available through the
MSYS2 UCRT64 environment.

### OpenGL

The renderer currently targets:

``` text
OpenGL 3.3 Core Profile
```

### GLAD

GLAD is included in the repository:

``` text
external/glad/
```

------------------------------------------------------------------------

# Building

## 1. Clone the repository

``` powershell
git clone <repository-url>
cd SIH
```

## 2. Configure CMake

``` powershell
cmake -S . -B build
```

## 3. Build

``` powershell
cmake --build build
```

A successful build produces:

``` text
build/SIH26.exe
build/CreateTestBin.exe
```

------------------------------------------------------------------------

# Running the Application

From the project root:

``` powershell
.\build\SIH26.exe
```

The application currently loads:

``` text
data/test.bin
```

and initializes the OpenGL LiDAR viewer.

A successful startup currently reports information such as:

``` text
OpenGL initialized successfully
OpenGL version: 3.3.0
Point cloud loaded successfully
Points: ...
X range: ...
Y range: ...
Z range: ...
Intensity range: ...
Grid width: ...
Grid height: ...
```

------------------------------------------------------------------------

# Generating Test LiDAR Data

The repository contains a synthetic LiDAR data generator:

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

This creates:

``` text
data/test.bin
```

The generator is used to create test data without hardcoding individual
test cases into the main application.

After generating the dataset:

``` powershell
.\build\SIH26.exe
```

------------------------------------------------------------------------

# LiDAR Data Format

The binary point-cloud loader currently expects four `float` values per
point:

``` text
X
Y
Z
Intensity
```

Conceptually:

``` cpp
struct Point
{
    float x;
    float y;
    float z;
    float intensity;
};
```

The binary layout is therefore:

``` text
[x][y][z][intensity]
[x][y][z][intensity]
[x][y][z][intensity]
...
```

The loader determines the number of points from the binary file size.

------------------------------------------------------------------------

# Point Cloud Processing

The `PointCloud` class currently handles:

-   Loading `.bin` point clouds
-   Loading XYZ data
-   Storing points
-   Accessing individual points
-   Counting points
-   Calculating X/Y/Z ranges
-   Calculating intensity ranges

Current data flow:

``` text
data/test.bin
      │
      ▼
 PointCloud
      │
      ├── X range
      ├── Y range
      ├── Z range
      └── Intensity range
```

------------------------------------------------------------------------

# Spatial Grid

The `SpatialGrid` converts continuous LiDAR XY coordinates into discrete
map cells.

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

The final argument controls the cell size.

For example:

``` text
cellSize = 1.0 m
```

produces approximately one grid cell per square metre.

------------------------------------------------------------------------

# Map Cells

Each spatial cell currently contains:

``` text
elevation
intensity
pointCount
elevationSum
minimumElevation
```

When a LiDAR point enters a cell:

``` text
point.z
    ↓
elevationSum

point.intensity
    ↓
intensity

point count
    ↓
pointCount
```

After all points have been processed:

``` text
average elevation =
elevationSum / pointCount

average intensity =
intensity / pointCount
```

This provides the initial 2.5D representation.

------------------------------------------------------------------------

# Current Spatial Mapping Pipeline

``` text
             PointCloud
                 │
                 ▼
        Extract X / Y position
                 │
                 ▼
       Calculate grid coordinates
                 │
                 ▼
            MapCell
                 │
       ┌─────────┼─────────┐
       ▼         ▼         ▼
   Elevation  Intensity  Count
       │         │         │
       └─────────┼─────────┘
                 ▼
          SpatialGrid
```

------------------------------------------------------------------------

# Rendering

The renderer uses OpenGL 3.3.

Current rendering components include:

-   Vertex Array Objects
-   Vertex Buffer Objects
-   GLSL shaders
-   Point rendering
-   Grid-derived rendering
-   Depth testing
-   Perspective projection

Shaders are stored in:

``` text
shaders/
```

Point shaders:

``` text
shaders/point.vert
shaders/point.frag
```

Grid shaders:

``` text
shaders/grid.vert
shaders/grid.frag
```

------------------------------------------------------------------------

# Camera Controls

  Input   Action
  ------- -----------------------
  `W`     Move forward
  `S`     Move backward
  `A`     Move left
  `D`     Move right
  `Q`     Move down
  `E`     Move up
  Mouse   Look around
  `ESC`   Capture/release mouse

------------------------------------------------------------------------

# Development Roadmap

The project is being developed in stages.

## Phase 1 --- Project Foundation

-   [x] CMake project
-   [x] C++17
-   [x] GLFW
-   [x] GLAD
-   [x] OpenGL initialization
-   [x] Basic application loop

## Phase 2 --- LiDAR Point Cloud

-   [x] Point representation
-   [x] Binary loader
-   [x] XYZ loader
-   [x] Point statistics
-   [x] Synthetic dataset generator

## Phase 3 --- Spatial Representation

-   [x] Spatial grid
-   [x] Point-to-cell mapping
-   [x] Cell statistics
-   [x] Elevation calculation
-   [x] Intensity calculation
-   [x] Minimum elevation

## Phase 4 --- 2.5D Elevation Mapping

-   [ ] Robust terrain representation
-   [ ] Ground/non-ground separation
-   [ ] Improved elevation model
-   [ ] Map update strategy
-   [ ] Variable-resolution cells

## Phase 5 --- AI / Semantic Pipeline

This is the next major pipeline stage.

Planned flow:

``` text
LiDAR
  ↓
Preprocessing
  ↓
Feature Extraction
  ↓
AI Model
  ↓
Semantic Classification
  ↓
Semantic Labels
```

Potential semantic categories will be defined during implementation.

The AI layer will then feed semantic information into the spatial map
rather than existing as an isolated classifier.

## Phase 6 --- Variable-Resolution Mapping

The target architecture uses higher spatial detail near the sensor and
progressively coarser representation farther away.

Conceptually:

``` text
                 SENSOR
                   ●
              ┌─────────┐
              │  HIGH   │
              │ DETAIL  │
              └─────────┘
           ┌───────────────┐
           │    MEDIUM     │
           │    DETAIL     │
           └───────────────┘
       ┌───────────────────────┐
       │        COARSE         │
       │        DETAIL         │
       └───────────────────────┘
```

The exact resolution strategy will be implemented after the base mapping
and semantic pipeline are established.

## Phase 7 --- Integrated Semantic 2.5D Map

Target output:

``` text
             LiDAR
               │
               ▼
        Point Processing
               │
               ▼
        Spatial Mapping
               │
               ▼
       AI Semantic Layer
               │
               ▼
     Semantic + Elevation
               │
               ▼
    Variable-Resolution Map
               │
               ▼
          Visualization
```

------------------------------------------------------------------------

# Development Philosophy

The core engine is intentionally being implemented manually in C++.

The objective is to build and understand each processing stage:

``` text
Data
 ↓
Geometry
 ↓
Spatial Representation
 ↓
Features
 ↓
Semantics
 ↓
Mapping
 ↓
Rendering
```

Each stage is developed and tested independently before being connected
to the next stage.

This makes it easier to:

-   isolate bugs
-   validate intermediate data
-   benchmark individual stages
-   replace individual algorithms
-   understand the complete pipeline

------------------------------------------------------------------------

# Testing Strategy

The project uses generated synthetic LiDAR data during development.

The test generator allows different point distributions and values to be
generated without modifying the main application.

Basic validation currently checks:

``` text
Point count
X range
Y range
Z range
Intensity range
Grid width
Grid height
Cell occupancy
Cell elevation
Cell intensity
```

Example:

``` text
Points: 10000
X range: 0 -> 9.9
Y range: 0 -> 9.9
Z range: ...
Intensity range: ...
Grid width: 10
Grid height: 10
```

------------------------------------------------------------------------

# Git / Generated Files

Build output is intentionally excluded from Git.

The repository should contain source files and required project assets,
while generated files such as:

``` text
build/
*.exe
*.obj
*.o
```

are ignored through `.gitignore`.

The synthetic test dataset may be regenerated using:

``` powershell
.\build\CreateTestBin.exe
```

------------------------------------------------------------------------

# Contributing / Development

The project is currently being developed incrementally.

When adding a new subsystem:

1.  Define its data structures.
2.  Implement the core logic.
3.  Add a deterministic test.
4.  Validate the output.
5.  Integrate it with the next subsystem.
6.  Update the renderer only after the underlying data is validated.

This keeps the mapping pipeline modular.

------------------------------------------------------------------------

# License

This project is currently being developed as part of the **SIH 26**
project.

License information will be added when the project is finalized.
