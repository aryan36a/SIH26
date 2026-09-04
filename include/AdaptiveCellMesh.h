#pragma once

#include <cstddef>
#include <vector>

#include "AdaptiveSpatialGrid.h"

/*
 * Builds indexed triangle geometry for the 2.5D adaptive representation.
 *
 * The mesh is surface-first:
 *   - every occupied cell contributes a surface top;
 *   - meaningful height discontinuities contribute boundary walls;
 *   - multi-return cells with real vertical span expose structural sides.
 *
 * Vertex layout (must match adaptive.vert attribute locations):
 *   location 0: vec3 position  (x, y, z)
 *   location 1: float intensity (already normalized 0..1 by Renderer)
 *   location 2: vec3 normal
 *
 * Bottom faces are omitted — they face away from the camera in all
 * practical viewing angles and halve the bottom-face triangle count.
 */
class AdaptiveCellMesh
{
public:
    struct Vertex
    {
        float x;
        float y;
        float z;
        float intensity;
        float nx;
        float ny;
        float nz;
    };

    // Small returns are treated as surface variation rather than walls.
    static constexpr float FLAT_THRESHOLD = 0.35f;

    // Hard cap on emitted column height to prevent outlier explosions
    static constexpr float MAX_SPAN_CAP   = 30.0f;

    void build(const AdaptiveSpatialGrid& grid);

    const std::vector<Vertex>&       getVertices() const;
    const std::vector<unsigned int>& getIndices()  const;

    std::size_t getVertexCount() const;
    std::size_t getIndexCount()  const;

private:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    void emitTopQuad(
        float x0, float x1,
        float y0, float y1,
        float z,
        float intensity,
        float nx,
        float ny,
        float nz);

    void emitWall(
        float ax, float ay,
        float bx, float by,
        float bottom,
        float top,
        float intensity);
};