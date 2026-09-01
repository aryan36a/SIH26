#pragma once

#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "SpatialGrid.h"

#include <glm/glm.hpp>

class Renderer {
private:
    // --------------------------------------------------
    // Raw point cloud
    // --------------------------------------------------

    unsigned int pointVBO;
    unsigned int pointVAO;

    // --------------------------------------------------
    // Uniform grid
    // --------------------------------------------------

    unsigned int gridVBO;
    unsigned int gridVAO;

    // --------------------------------------------------
    // Adaptive 2.5D mesh
    // --------------------------------------------------

    unsigned int adaptiveVBO;
    unsigned int adaptiveVAO;
    unsigned int adaptiveEBO;

    unsigned int adaptiveIndexCount;

    // --------------------------------------------------
    // Shader programs
    // --------------------------------------------------

    unsigned int shaderProgram;
    unsigned int adaptiveShaderProgram;

    // --------------------------------------------------
    // Point/grid intensity range
    // --------------------------------------------------

    float minIntensity;
    float maxIntensity;

    // --------------------------------------------------
    // Adaptive elevation range
    // --------------------------------------------------

    float minElevation;
    float maxElevation;

    // --------------------------------------------------
    // Shader utilities
    // --------------------------------------------------

    unsigned int compileShader(
        unsigned int type,
        const char* source
    );

    bool createShaderProgram();

    bool createAdaptiveShaderProgram();

    void setCommonUniforms(
        unsigned int program,
        const glm::mat4& view,
        const glm::mat4& projection
    );

public:
    Renderer();

    ~Renderer();

    // --------------------------------------------------
    // GPU upload
    // --------------------------------------------------

    bool uploadPointCloud(
        const PointCloud& cloud
    );

    bool uploadGrid(
        const SpatialGrid& grid
    );

    bool uploadAdaptiveGrid(
        const AdaptiveSpatialGrid& grid
    );

    // --------------------------------------------------
    // Rendering
    // --------------------------------------------------

    void render(
        const PointCloud& cloud,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void render(
        const SpatialGrid& grid,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    void renderAdaptive(
        const glm::mat4& view,
        const glm::mat4& projection
    );
};