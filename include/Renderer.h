#pragma once

#include "AdaptiveSpatialGrid.h"
#include "PointCloud.h"
#include "SpatialGrid.h"

#include <glad/gl.h>
#include <glm/glm.hpp>

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // Upload data to GPU. Returns false on failure or empty data.
    bool uploadPointCloud(const PointCloud& cloud);
    bool uploadGrid(const SpatialGrid& grid);
    bool uploadAdaptiveGrid(const AdaptiveSpatialGrid& grid);

    // Render calls
    void render(const PointCloud& cloud,
                const glm::mat4& view,
                const glm::mat4& projection);

    void render(const SpatialGrid& grid,
                const glm::mat4& view,
                const glm::mat4& projection);

    void renderAdaptive(const glm::mat4& view,
                        const glm::mat4& projection);

private:
    // ---- Raw point cloud ----
    unsigned int pointVAO    = 0;
    unsigned int pointVBO    = 0;
    GLsizei      pointCount  = 0;

    // ---- Uniform spatial grid ----
    unsigned int gridVAO      = 0;
    unsigned int gridVBO      = 0;
    GLsizei      gridVertCount = 0;

    // ---- Adaptive 2.5D mesh ----
    unsigned int adaptiveVAO        = 0;
    unsigned int adaptiveVBO        = 0;
    unsigned int adaptiveEBO        = 0;
    unsigned int adaptiveIndexCount = 0;

    // ---- Shader programs ----
    unsigned int shaderProgram         = 0;
    unsigned int adaptiveShaderProgram = 0;

    // ---- Cached intensity range (raw cloud) ----
    float minIntensity = 0.0f;
    float maxIntensity = 1.0f;

    // ---- Cached elevation range (adaptive, percentile-clamped) ----
    float minElevation = 0.0f;
    float maxElevation = 1.0f;

    // ---- Private helpers ----
    unsigned int compileShader(unsigned int type, const char* src);
    bool createShaderProgram();
    bool createAdaptiveShaderProgram();
    void setCommonUniforms(unsigned int prog,
                           const glm::mat4& view,
                           const glm::mat4& proj);
    void setUniform1f(unsigned int prog, const char* name, float val);
};