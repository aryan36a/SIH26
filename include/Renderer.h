#pragma once

#include "PointCloud.h"
#include "SpatialGrid.h"

#include <glm/glm.hpp>

class Renderer
{
private:

    unsigned int pointVBO;
    unsigned int pointVAO;

    unsigned int gridVBO;
    unsigned int gridVAO;

    unsigned int shaderProgram;

    float minIntensity;
    float maxIntensity;

    unsigned int compileShader(
        unsigned int type,
        const char* source
    );

    bool createShaderProgram();

public:

    Renderer();

    ~Renderer();

    bool uploadPointCloud(
        const PointCloud& cloud
    );

    bool uploadGrid(
        const SpatialGrid& grid
    );

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
};