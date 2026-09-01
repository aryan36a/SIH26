cmake_minimum_required(VERSION 3.20)

project(SIH26 LANGUAGES C CXX)

# ============================================================
# Project configuration
# ============================================================

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSYS2 UCRT64
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

# ============================================================
# GLAD
# ============================================================

add_library(glad
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

# ============================================================
# Main executable
# ============================================================

add_executable(SIH26

    # --------------------------------------------------------
    # Application
    # --------------------------------------------------------

    src/main.cpp

    # --------------------------------------------------------
    # Point cloud
    # --------------------------------------------------------

    src/PointCloud.cpp

    # --------------------------------------------------------
    # Map representation
    # --------------------------------------------------------

    src/MapCell.cpp
    src/SpatialGrid.cpp

    # --------------------------------------------------------
    # Adaptive mapping
    # --------------------------------------------------------

    src/ResolutionProfile.cpp
    src/AdaptiveSpatialGrid.cpp
    src/AdaptiveCellMesh.cpp

    # --------------------------------------------------------
    # Camera / rendering
    # --------------------------------------------------------

    src/Camera.cpp
    src/Renderer.cpp
)

target_include_directories(SIH26 PRIVATE
    include
)

# ============================================================
# GLFW
# ============================================================

find_package(glfw3 CONFIG REQUIRED)

# ============================================================
# OpenGL
# ============================================================

find_package(OpenGL REQUIRED)

# ============================================================
# Main executable libraries
# ============================================================

target_link_libraries(SIH26 PRIVATE
    glad
    glfw
    OpenGL::GL
)

# ============================================================
# Test data generator
# ============================================================

add_executable(CreateTestBin

    src/create_test_bin.cpp
)

target_include_directories(CreateTestBin PRIVATE
    include
)

# ============================================================
# SpatialGrid tests
# ============================================================

add_executable(SpatialGridTest

    test/SpatialGridTest.cpp

    src/PointCloud.cpp
    src/SpatialGrid.cpp
    src/MapCell.cpp
)

target_include_directories(SpatialGridTest PRIVATE
    include
)

# ============================================================
# ResolutionProfile tests
# ============================================================

add_executable(ResolutionProfileTest

    test/ResolutionProfileTest.cpp

    src/ResolutionProfile.cpp
)

target_include_directories(ResolutionProfileTest PRIVATE
    include
)

# ============================================================
# AdaptiveSpatialGrid tests
# ============================================================

add_executable(AdaptiveSpatialGridTest

    test/AdaptiveSpatialGridTest.cpp

    src/AdaptiveSpatialGrid.cpp
    src/ResolutionProfile.cpp
    src/MapCell.cpp
    src/PointCloud.cpp
)

target_include_directories(AdaptiveSpatialGridTest PRIVATE
    include
)

# ============================================================
# CTest integration
# ============================================================

enable_testing()

add_test(
    NAME SpatialGridTest
    COMMAND SpatialGridTest
)

add_test(
    NAME ResolutionProfileTest
    COMMAND ResolutionProfileTest
)

add_test(
    NAME AdaptiveSpatialGridTest
    COMMAND AdaptiveSpatialGridTest
)