cmake_minimum_required(VERSION 3.20)

project(SIH26 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSYS2 UCRT64
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

# GLAD
add_library(glad
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

# Main executable
add_executable(SIH26
    src/main.cpp
    src/PointCloud.cpp
    src/Renderer.cpp
    src/Camera.cpp
    src/MapCell.cpp
    src/SpatialGrid.cpp
)

# Project headers
target_include_directories(SIH26 PRIVATE
    include
)

# GLFW
find_package(glfw3 CONFIG REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

target_link_libraries(SIH26 PRIVATE
    glad
    glfw
    OpenGL::GL
)

# Testing executable
add_executable(CreateTestBin
    src/create_test_bin.cpp
)

target_include_directories(CreateTestBin PRIVATE
    include
)