#include "AdaptiveCellMesh.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_map>
#include <vector>

namespace
{
struct CellAddress
{
    int x;
    int y;
    int resolution;

    bool operator==(const CellAddress& other) const noexcept
    {
        return x == other.x && y == other.y &&
               resolution == other.resolution;
    }
};

struct CellAddressHash
{
    std::size_t operator()(const CellAddress& address) const noexcept
    {
        std::size_t hash = 2166136261u;
        hash ^= static_cast<std::size_t>(static_cast<unsigned int>(address.x));
        hash *= 16777619u;
        hash ^= static_cast<std::size_t>(static_cast<unsigned int>(address.y));
        hash *= 16777619u;
        hash ^= static_cast<std::size_t>(static_cast<unsigned int>(address.resolution));
        hash *= 16777619u;
        return hash;
    }
};

CellAddress addressFor(const AdaptiveSpatialGrid::RenderCell& cell)
{
    const float resolution = std::max(cell.resolution, 0.000001f);
    return {
        static_cast<int>(std::lround(cell.minX / resolution)),
        static_cast<int>(std::lround(cell.minY / resolution)),
        static_cast<int>(std::lround(resolution * 1000000.0f))
    };
}

bool isStructure(const AdaptiveSpatialGrid::RenderCell& cell)
{
    return cell.pointCount >= 2 &&
           cell.elevationSpan >= AdaptiveCellMesh::FLAT_THRESHOLD;
}

bool sameStructure(const AdaptiveSpatialGrid::RenderCell& left,
                   const AdaptiveSpatialGrid::RenderCell& right)
{
    constexpr float epsilon = 0.001f;
    return isStructure(left) && isStructure(right) &&
           std::fabs(left.minimumElevation - right.minimumElevation) <= epsilon &&
           std::fabs(left.maximumElevation - right.maximumElevation) <= epsilon;
}

std::optional<float> localGroundElevation(
    const AdaptiveSpatialGrid::RenderCell& cell,
    const std::unordered_map<CellAddress, std::size_t, CellAddressHash>& lookup,
    const std::vector<AdaptiveSpatialGrid::RenderCell>& cells)
{
    const CellAddress address = addressFor(cell);
    const int resolutionKey = address.resolution;
    std::vector<float> elevations;
    elevations.reserve(4);

    const int offsets[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    for (const auto& offset : offsets)
    {
        const auto it = lookup.find({
            address.x + offset[0],
            address.y + offset[1],
            resolutionKey
        });

        if (it == lookup.end()) continue;

        const auto& neighbor = cells[it->second];
        if (!isStructure(neighbor))
            elevations.push_back(neighbor.elevation);
    }

    if (elevations.empty()) return std::nullopt;

    std::sort(elevations.begin(), elevations.end());
    return elevations[elevations.size() / 2];
}

int structuralNeighborCount(
    const AdaptiveSpatialGrid::RenderCell& cell,
    const std::unordered_map<CellAddress, std::size_t, CellAddressHash>& lookup,
    const std::vector<AdaptiveSpatialGrid::RenderCell>& cells)
{
    const CellAddress address = addressFor(cell);
    const int resolutionKey = address.resolution;
    int count = 0;

    const int offsets[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    for (const auto& offset : offsets)
    {
        const auto it = lookup.find({
            address.x + offset[0],
            address.y + offset[1],
            resolutionKey
        });

        if (it != lookup.end() && isStructure(cells[it->second]))
            ++count;
    }

    return count;
}
}

void AdaptiveCellMesh::emitTopQuad(
    float x0, float x1,
    float y0, float y1,
    float z,
    float intensity,
    float nx,
    float ny,
    float nz)
{
    const auto base = static_cast<unsigned int>(vertices.size());

    vertices.push_back({x0, y0, z, intensity, nx, ny, nz});
    vertices.push_back({x1, y0, z, intensity, nx, ny, nz});
    vertices.push_back({x1, y1, z, intensity, nx, ny, nz});
    vertices.push_back({x0, y1, z, intensity, nx, ny, nz});

    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

void AdaptiveCellMesh::emitWall(
    float ax, float ay,
    float bx, float by,
    float bottom,
    float top,
    float intensity)
{
    if (top - bottom <= 0.001f) return;

    const float dx = bx - ax;
    const float dy = by - ay;
    const float length = std::sqrt(dx * dx + dy * dy);
    if (length <= 0.000001f) return;

    const float nx = dy / length;
    const float ny = -dx / length;
    const auto base = static_cast<unsigned int>(vertices.size());

    vertices.push_back({ax, ay, bottom, intensity, nx, ny, 0.0f});
    vertices.push_back({bx, by, bottom, intensity, nx, ny, 0.0f});
    vertices.push_back({bx, by, top, intensity, nx, ny, 0.0f});
    vertices.push_back({ax, ay, top, intensity, nx, ny, 0.0f});

    // Emit both windings so a boundary remains visible from either side.
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
    indices.push_back(base + 2);
    indices.push_back(base + 1);
    indices.push_back(base + 0);
    indices.push_back(base + 3);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
}

void AdaptiveCellMesh::build(const AdaptiveSpatialGrid& grid)
{
    vertices.clear();
    indices.clear();

    const std::vector<AdaptiveSpatialGrid::RenderCell> cells =
        grid.getRenderCells();

    std::unordered_map<CellAddress, std::size_t, CellAddressHash> lookup;
    lookup.reserve(cells.size());
    for (std::size_t index = 0; index < cells.size(); ++index)
        lookup.emplace(addressFor(cells[index]), index);

    vertices.reserve(cells.size() * 8);
    indices.reserve(cells.size() * 18);

    for (const auto& cell : cells)
    {
        const float span = std::max(cell.elevationSpan, 0.0f);
        const bool structure = isStructure(cell);
        const float top = structure
            ? std::min(cell.maximumElevation,
                       cell.minimumElevation + MAX_SPAN_CAP)
            : cell.elevation;

        emitTopQuad(cell.minX, cell.maxX,
                    cell.minY, cell.maxY,
                    top, cell.intensity,
                    0.0f, 0.0f, 1.0f);

        const CellAddress address = addressFor(cell);
        const int resolutionKey = address.resolution;
        const auto findNeighbor = [&](int x, int y)
            -> const AdaptiveSpatialGrid::RenderCell*
        {
            const auto it = lookup.find({x, y, resolutionKey});
            return it == lookup.end() ? nullptr : &cells[it->second];
        };

        const auto groundElevation = localGroundElevation(cell, lookup, cells);
        const int supportedNeighbors = structuralNeighborCount(cell, lookup, cells);

        // A structure needs either observed nearby ground or continuity with
        // at least two structural cells. Isolated aerial returns remain only
        // as their sampled top surface instead of becoming needles.
        const bool hasSupport = groundElevation.has_value() ||
                                supportedNeighbors >= 2;

        float wallBottom = cell.minimumElevation;
        if (groundElevation.has_value())
        {
            // Snap only near-ground structures to the local terrain. A large
            // gap is treated as an overhang/ canopy and is not filled in.
            constexpr float groundSnapGap = 0.75f;
            if (cell.minimumElevation <= *groundElevation + groundSnapGap)
                wallBottom = *groundElevation;
        }

        const auto emitStructureBoundary =
            [&](const AdaptiveSpatialGrid::RenderCell* neighbor,
                float ax, float ay, float bx, float by)
        {
            if (!structure || !hasSupport ||
                (neighbor != nullptr && sameStructure(cell, *neighbor)))
                return;

            emitWall(ax, ay, bx, by,
                     wallBottom,
                     top,
                     cell.intensity);
        };

        emitStructureBoundary(findNeighbor(address.x, address.y - 1),
                              cell.minX, cell.minY, cell.maxX, cell.minY);
        emitStructureBoundary(findNeighbor(address.x, address.y + 1),
                              cell.maxX, cell.maxY, cell.minX, cell.maxY);
        emitStructureBoundary(findNeighbor(address.x - 1, address.y),
                              cell.minX, cell.maxY, cell.minX, cell.minY);
        emitStructureBoundary(findNeighbor(address.x + 1, address.y),
                              cell.maxX, cell.minY, cell.maxX, cell.maxY);

        const auto emitSurfaceStep =
            [&](const AdaptiveSpatialGrid::RenderCell* neighbor,
                float ax, float ay, float bx, float by)
        {
            if (structure || neighbor == nullptr || isStructure(*neighbor)) return;

            const float difference = neighbor->elevation - cell.elevation;
            if (std::fabs(difference) <= FLAT_THRESHOLD) return;

            emitWall(ax, ay, bx, by,
                     std::min(cell.elevation, neighbor->elevation),
                     std::max(cell.elevation, neighbor->elevation),
                     0.5f * (cell.intensity + neighbor->intensity));
        };

        emitSurfaceStep(findNeighbor(address.x + 1, address.y),
                        cell.maxX, cell.minY, cell.maxX, cell.maxY);
        emitSurfaceStep(findNeighbor(address.x, address.y + 1),
                        cell.minX, cell.maxY, cell.maxX, cell.maxY);
    }
}

const std::vector<AdaptiveCellMesh::Vertex>&
AdaptiveCellMesh::getVertices() const
{
    return vertices;
}

const std::vector<unsigned int>&
AdaptiveCellMesh::getIndices() const
{
    return indices;
}

std::size_t AdaptiveCellMesh::getVertexCount() const
{
    return vertices.size();
}

std::size_t AdaptiveCellMesh::getIndexCount() const
{
    return indices.size();
}