#pragma once

#include <cstddef>
#include <vector>

#include "AdaptiveSpatialGrid.h"

class AdaptiveCellMesh {
public:
    struct Vertex {
        float x;
        float y;
        float z;
        float intensity;
    };

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

public:
    void build(const AdaptiveSpatialGrid& grid);

    const std::vector<Vertex>& getVertices() const;
    const std::vector<unsigned int>& getIndices() const;

    std::size_t getVertexCount() const;
    std::size_t getIndexCount() const;
};