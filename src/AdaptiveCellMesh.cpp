#include "AdaptiveCellMesh.h"

void AdaptiveCellMesh::build(
    const AdaptiveSpatialGrid& grid)
{
    vertices.clear();
    indices.clear();

    const std::vector<AdaptiveSpatialGrid::RenderCell>
        renderCells = grid.getRenderCells();

    /*
     * Every adaptive cell becomes one quad.
     *
     * Four vertices:
     *
     * (x0,y0) -------- (x1,y0)
     *    |                |
     *    |      CELL      |
     *    |                |
     * (x0,y1) -------- (x1,y1)
     *
     * The elevation is constant across
     * the cell for this first 2.5D mesh.
     */

    vertices.reserve(renderCells.size() * 4);
    indices.reserve(renderCells.size() * 6);

    for (const auto& cell : renderCells)
    {
        const float halfSize =
            cell.resolution * 0.5f;

        const float x0 =
            cell.x - halfSize;

        const float x1 =
            cell.x + halfSize;

        const float y0 =
            cell.y - halfSize;

        const float y1 =
            cell.y + halfSize;

        const unsigned int baseIndex =
            static_cast<unsigned int>(
                vertices.size()
            );

        /*
         * Bottom-left
         */
        vertices.push_back(
            Vertex{
                x0,
                y0,
                cell.elevation,
                cell.intensity
            }
        );

        /*
         * Bottom-right
         */
        vertices.push_back(
            Vertex{
                x1,
                y0,
                cell.elevation,
                cell.intensity
            }
        );

        /*
         * Top-right
         */
        vertices.push_back(
            Vertex{
                x1,
                y1,
                cell.elevation,
                cell.intensity
            }
        );

        /*
         * Top-left
         */
        vertices.push_back(
            Vertex{
                x0,
                y1,
                cell.elevation,
                cell.intensity
            }
        );

        /*
         * Triangle 1
         */
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);

        /*
         * Triangle 2
         */
        indices.push_back(baseIndex + 0);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
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