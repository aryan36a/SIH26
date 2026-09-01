#include "Renderer.h"

#include <glad/gl.h>

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "AdaptiveCellMesh.h"

// ============================================================
// Constructor
// ============================================================

Renderer::Renderer()
    : pointVBO(0),
      pointVAO(0),
      gridVBO(0),
      gridVAO(0),
      adaptiveVBO(0),
      adaptiveVAO(0),
      adaptiveEBO(0),
      adaptiveIndexCount(0),
      shaderProgram(0),
      adaptiveShaderProgram(0),
      minIntensity(0.0f),
      maxIntensity(1.0f),
      minElevation(0.0f),
      maxElevation(1.0f)
{
    // --------------------------------------------------------
    // Raw point cloud buffers
    // --------------------------------------------------------

    glGenVertexArrays(
        1,
        &pointVAO
    );

    glGenBuffers(
        1,
        &pointVBO
    );

    // --------------------------------------------------------
    // Uniform grid buffers
    // --------------------------------------------------------

    glGenVertexArrays(
        1,
        &gridVAO
    );

    glGenBuffers(
        1,
        &gridVBO
    );

    // --------------------------------------------------------
    // Adaptive mesh buffers
    // --------------------------------------------------------

    glGenVertexArrays(
        1,
        &adaptiveVAO
    );

    glGenBuffers(
        1,
        &adaptiveVBO
    );

    glGenBuffers(
        1,
        &adaptiveEBO
    );

    // --------------------------------------------------------
    // Create shaders
    // --------------------------------------------------------

    if (!createShaderProgram())
    {
        std::cerr
            << "Failed to create point/grid shader program\n";
    }

    if (!createAdaptiveShaderProgram())
    {
        std::cerr
            << "Failed to create adaptive shader program\n";
    }
}


// ============================================================
// Destructor
// ============================================================

Renderer::~Renderer()
{
    // --------------------------------------------------------
    // Shader programs
    // --------------------------------------------------------

    if (shaderProgram != 0)
    {
        glDeleteProgram(
            shaderProgram
        );

        shaderProgram = 0;
    }

    if (adaptiveShaderProgram != 0)
    {
        glDeleteProgram(
            adaptiveShaderProgram
        );

        adaptiveShaderProgram = 0;
    }

    // --------------------------------------------------------
    // Point cloud buffers
    // --------------------------------------------------------

    if (pointVBO != 0)
    {
        glDeleteBuffers(
            1,
            &pointVBO
        );

        pointVBO = 0;
    }

    if (pointVAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &pointVAO
        );

        pointVAO = 0;
    }

    // --------------------------------------------------------
    // Uniform grid buffers
    // --------------------------------------------------------

    if (gridVBO != 0)
    {
        glDeleteBuffers(
            1,
            &gridVBO
        );

        gridVBO = 0;
    }

    if (gridVAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &gridVAO
        );

        gridVAO = 0;
    }

    // --------------------------------------------------------
    // Adaptive mesh buffers
    // --------------------------------------------------------

    if (adaptiveVBO != 0)
    {
        glDeleteBuffers(
            1,
            &adaptiveVBO
        );

        adaptiveVBO = 0;
    }

    if (adaptiveEBO != 0)
    {
        glDeleteBuffers(
            1,
            &adaptiveEBO
        );

        adaptiveEBO = 0;
    }

    if (adaptiveVAO != 0)
    {
        glDeleteVertexArrays(
            1,
            &adaptiveVAO
        );

        adaptiveVAO = 0;
    }
}


// ============================================================
// Shader compilation
// ============================================================

unsigned int Renderer::compileShader(
    unsigned int type,
    const char* source)
{
    unsigned int shader =
        glCreateShader(type);

    if (shader == 0)
    {
        std::cerr
            << "Failed to create shader object\n";

        return 0;
    }

    glShaderSource(
        shader,
        1,
        &source,
        nullptr
    );

    glCompileShader(
        shader
    );

    int success = 0;

    glGetShaderiv(
        shader,
        GL_COMPILE_STATUS,
        &success
    );

    if (!success)
    {
        char infoLog[1024];

        glGetShaderInfoLog(
            shader,
            sizeof(infoLog),
            nullptr,
            infoLog
        );

        std::cerr
            << "Shader compilation failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(
            shader
        );

        return 0;
    }

    return shader;
}


// ============================================================
// Normal point/grid shader
// ============================================================

bool Renderer::createShaderProgram()
{
    std::ifstream vertexFile(
        "shaders/point.vert"
    );

    std::ifstream fragmentFile(
        "shaders/point.frag"
    );

    if (!vertexFile.is_open())
    {
        std::cerr
            << "Failed to open shaders/point.vert\n";

        return false;
    }

    if (!fragmentFile.is_open())
    {
        std::cerr
            << "Failed to open shaders/point.frag\n";

        return false;
    }

    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexStream
        << vertexFile.rdbuf();

    fragmentStream
        << fragmentFile.rdbuf();

    const std::string vertexSource =
        vertexStream.str();

    const std::string fragmentSource =
        fragmentStream.str();

    unsigned int vertexShader =
        compileShader(
            GL_VERTEX_SHADER,
            vertexSource.c_str()
        );

    if (vertexShader == 0)
    {
        return false;
    }

    unsigned int fragmentShader =
        compileShader(
            GL_FRAGMENT_SHADER,
            fragmentSource.c_str()
        );

    if (fragmentShader == 0)
    {
        glDeleteShader(
            vertexShader
        );

        return false;
    }

    shaderProgram =
        glCreateProgram();

    if (shaderProgram == 0)
    {
        std::cerr
            << "Failed to create shader program\n";

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return false;
    }

    glAttachShader(
        shaderProgram,
        vertexShader
    );

    glAttachShader(
        shaderProgram,
        fragmentShader
    );

    glLinkProgram(
        shaderProgram
    );

    int success = 0;

    glGetProgramiv(
        shaderProgram,
        GL_LINK_STATUS,
        &success
    );

    if (!success)
    {
        char infoLog[1024];

        glGetProgramInfoLog(
            shaderProgram,
            sizeof(infoLog),
            nullptr,
            infoLog
        );

        std::cerr
            << "Shader linking failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glDeleteProgram(shaderProgram);

        shaderProgram = 0;

        return false;
    }

    glDeleteShader(
        vertexShader
    );

    glDeleteShader(
        fragmentShader
    );

    return true;
}


// ============================================================
// Adaptive shader
// ============================================================

bool Renderer::createAdaptiveShaderProgram()
{
    std::ifstream vertexFile(
        "shaders/adaptive.vert"
    );

    std::ifstream fragmentFile(
        "shaders/adaptive.frag"
    );

    if (!vertexFile.is_open())
    {
        std::cerr
            << "Failed to open shaders/adaptive.vert\n";

        return false;
    }

    if (!fragmentFile.is_open())
    {
        std::cerr
            << "Failed to open shaders/adaptive.frag\n";

        return false;
    }

    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexStream
        << vertexFile.rdbuf();

    fragmentStream
        << fragmentFile.rdbuf();

    const std::string vertexSource =
        vertexStream.str();

    const std::string fragmentSource =
        fragmentStream.str();

    unsigned int vertexShader =
        compileShader(
            GL_VERTEX_SHADER,
            vertexSource.c_str()
        );

    if (vertexShader == 0)
    {
        return false;
    }

    unsigned int fragmentShader =
        compileShader(
            GL_FRAGMENT_SHADER,
            fragmentSource.c_str()
        );

    if (fragmentShader == 0)
    {
        glDeleteShader(
            vertexShader
        );

        return false;
    }

    adaptiveShaderProgram =
        glCreateProgram();

    if (adaptiveShaderProgram == 0)
    {
        std::cerr
            << "Failed to create adaptive shader program\n";

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return false;
    }

    glAttachShader(
        adaptiveShaderProgram,
        vertexShader
    );

    glAttachShader(
        adaptiveShaderProgram,
        fragmentShader
    );

    glLinkProgram(
        adaptiveShaderProgram
    );

    int success = 0;

    glGetProgramiv(
        adaptiveShaderProgram,
        GL_LINK_STATUS,
        &success
    );

    if (!success)
    {
        char infoLog[1024];

        glGetProgramInfoLog(
            adaptiveShaderProgram,
            sizeof(infoLog),
            nullptr,
            infoLog
        );

        std::cerr
            << "Adaptive shader linking failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glDeleteProgram(
            adaptiveShaderProgram
        );

        adaptiveShaderProgram = 0;

        return false;
    }

    glDeleteShader(
        vertexShader
    );

    glDeleteShader(
        fragmentShader
    );

    return true;
}


// ============================================================
// Common matrices
// ============================================================

void Renderer::setCommonUniforms(
    unsigned int program,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (program == 0)
    {
        return;
    }

    const int viewLocation =
        glGetUniformLocation(
            program,
            "uView"
        );

    const int projectionLocation =
        glGetUniformLocation(
            program,
            "uProjection"
        );

    if (viewLocation >= 0)
    {
        glUniformMatrix4fv(
            viewLocation,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );
    }

    if (projectionLocation >= 0)
    {
        glUniformMatrix4fv(
            projectionLocation,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );
    }
}


// ============================================================
// Upload raw point cloud
// ============================================================

bool Renderer::uploadPointCloud(
    const PointCloud& cloud)
{
    if (cloud.size() == 0)
    {
        return false;
    }

    minIntensity =
        cloud.getMinIntensity();

    maxIntensity =
        cloud.getMaxIntensity();

    glBindVertexArray(
        pointVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        pointVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            cloud.size() * sizeof(Point)
        ),
        nullptr,
        GL_STATIC_DRAW
    );

    for (std::size_t i = 0;
         i < cloud.size();
         ++i)
    {
        const Point& point =
            cloud.getPoint(i);

        glBufferSubData(
            GL_ARRAY_BUFFER,
            static_cast<GLintptr>(
                i * sizeof(Point)
            ),
            sizeof(Point),
            &point
        );
    }

    // Position: x, y, z
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(
        0
    );

    // Intensity
    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        reinterpret_cast<void*>(
            sizeof(float) * 3
        )
    );

    glEnableVertexAttribArray(
        1
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(
        0
    );

    return true;
}


// ============================================================
// Upload uniform grid
// ============================================================

bool Renderer::uploadGrid(
    const SpatialGrid& grid)
{
    if (grid.getWidth() == 0 ||
        grid.getHeight() == 0)
    {
        return false;
    }

    std::vector<Point> gridPoints;

    gridPoints.reserve(
        grid.getWidth() *
        grid.getHeight()
    );

    for (std::size_t y = 0;
         y < grid.getHeight();
         ++y)
    {
        for (std::size_t x = 0;
             x < grid.getWidth();
             ++x)
        {
            if (!grid.hasData(x, y))
            {
                continue;
            }

            const float worldX =
                grid.getMinX() +
                (
                    static_cast<float>(x) +
                    0.5f
                ) *
                grid.getCellSize();

            const float worldY =
                grid.getMinY() +
                (
                    static_cast<float>(y) +
                    0.5f
                ) *
                grid.getCellSize();

            const float elevation =
                grid.getElevation(
                    x,
                    y
                );

            const float intensity =
                grid.getIntensity(
                    x,
                    y
                );

            gridPoints.push_back(
                Point{
                    worldX,
                    worldY,
                    elevation,
                    intensity
                }
            );
        }
    }

    if (gridPoints.empty())
    {
        return false;
    }

    glBindVertexArray(
        gridVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        gridVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            gridPoints.size() * sizeof(Point)
        ),
        gridPoints.data(),
        GL_STATIC_DRAW
    );

    // Position
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(
        0
    );

    // Intensity
    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        reinterpret_cast<void*>(
            sizeof(float) * 3
        )
    );

    glEnableVertexAttribArray(
        1
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(
        0
    );

    return true;
}


// ============================================================
// Upload adaptive 2.5D grid
// ============================================================

bool Renderer::uploadAdaptiveGrid(
    const AdaptiveSpatialGrid& grid)
{
    AdaptiveCellMesh mesh;

    mesh.build(
        grid
    );

    if (mesh.getVertexCount() == 0 ||
        mesh.getIndexCount() == 0)
    {
        adaptiveIndexCount = 0;

        return false;
    }

    const auto& vertices =
        mesh.getVertices();

    const auto& indices =
        mesh.getIndices();

    // --------------------------------------------------------
    // Calculate elevation range
    // --------------------------------------------------------

    minElevation =
        vertices[0].z;

    maxElevation =
        vertices[0].z;

    for (const auto& vertex : vertices)
    {
        if (vertex.z < minElevation)
        {
            minElevation =
                vertex.z;
        }

        if (vertex.z > maxElevation)
        {
            maxElevation =
                vertex.z;
        }
    }

    // --------------------------------------------------------
    // Upload vertex data
    // --------------------------------------------------------

    glBindVertexArray(
        adaptiveVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        adaptiveVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            vertices.size() *
            sizeof(AdaptiveCellMesh::Vertex)
        ),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // --------------------------------------------------------
    // Upload index data
    // --------------------------------------------------------

    glBindBuffer(
        GL_ELEMENT_ARRAY_BUFFER,
        adaptiveEBO
    );

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(
            indices.size() *
            sizeof(unsigned int)
        ),
        indices.data(),
        GL_STATIC_DRAW
    );

    // --------------------------------------------------------
    // Position
    // --------------------------------------------------------

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AdaptiveCellMesh::Vertex),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(
        0
    );

    // --------------------------------------------------------
    // Intensity
    // --------------------------------------------------------

    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(AdaptiveCellMesh::Vertex),
        reinterpret_cast<void*>(
            sizeof(float) * 3
        )
    );

    glEnableVertexAttribArray(
        1
    );

    adaptiveIndexCount =
        static_cast<unsigned int>(
            indices.size()
        );

    // --------------------------------------------------------
    // Unbind
    //
    // IMPORTANT:
    // Do NOT unbind the EBO while the VAO
    // is still active.
    // --------------------------------------------------------

    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(
        0
    );

    return true;
}


// ============================================================
// Render raw point cloud
// ============================================================

void Renderer::render(
    const PointCloud& cloud,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (cloud.size() == 0 ||
        shaderProgram == 0)
    {
        return;
    }

    glUseProgram(
        shaderProgram
    );

    setCommonUniforms(
        shaderProgram,
        view,
        projection
    );

    const int minIntensityLocation =
        glGetUniformLocation(
            shaderProgram,
            "uMinIntensity"
        );

    const int maxIntensityLocation =
        glGetUniformLocation(
            shaderProgram,
            "uMaxIntensity"
        );

    if (minIntensityLocation >= 0)
    {
        glUniform1f(
            minIntensityLocation,
            minIntensity
        );
    }

    if (maxIntensityLocation >= 0)
    {
        glUniform1f(
            maxIntensityLocation,
            maxIntensity
        );
    }

    glBindVertexArray(
        pointVAO
    );

    glDrawArrays(
        GL_POINTS,
        0,
        static_cast<GLsizei>(
            cloud.size()
        )
    );

    glBindVertexArray(
        0
    );
}


// ============================================================
// Render uniform grid
// ============================================================

void Renderer::render(
    const SpatialGrid& grid,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (grid.getWidth() == 0 ||
        grid.getHeight() == 0 ||
        shaderProgram == 0)
    {
        return;
    }

    glUseProgram(
        shaderProgram
    );

    setCommonUniforms(
        shaderProgram,
        view,
        projection
    );

    const int minIntensityLocation =
        glGetUniformLocation(
            shaderProgram,
            "uMinIntensity"
        );

    const int maxIntensityLocation =
        glGetUniformLocation(
            shaderProgram,
            "uMaxIntensity"
        );

    if (minIntensityLocation >= 0)
    {
        glUniform1f(
            minIntensityLocation,
            minIntensity
        );
    }

    if (maxIntensityLocation >= 0)
    {
        glUniform1f(
            maxIntensityLocation,
            maxIntensity
        );
    }

    std::size_t populatedCells = 0;

    for (std::size_t y = 0;
         y < grid.getHeight();
         ++y)
    {
        for (std::size_t x = 0;
             x < grid.getWidth();
             ++x)
        {
            if (grid.hasData(x, y))
            {
                ++populatedCells;
            }
        }
    }

    if (populatedCells == 0)
    {
        return;
    }

    glBindVertexArray(
        gridVAO
    );

    glDrawArrays(
        GL_POINTS,
        0,
        static_cast<GLsizei>(
            populatedCells
        )
    );

    glBindVertexArray(
        0
    );
}


// ============================================================
// Render adaptive 2.5D mesh
// ============================================================

void Renderer::renderAdaptive(
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (adaptiveIndexCount == 0 ||
        adaptiveShaderProgram == 0)
    {
        return;
    }

    glUseProgram(
        adaptiveShaderProgram
    );

    setCommonUniforms(
        adaptiveShaderProgram,
        view,
        projection
    );

    const int minElevationLocation =
        glGetUniformLocation(
            adaptiveShaderProgram,
            "uMinElevation"
        );

    const int maxElevationLocation =
        glGetUniformLocation(
            adaptiveShaderProgram,
            "uMaxElevation"
        );

    if (minElevationLocation >= 0)
    {
        glUniform1f(
            minElevationLocation,
            minElevation
        );
    }

    if (maxElevationLocation >= 0)
    {
        glUniform1f(
            maxElevationLocation,
            maxElevation
        );
    }

    glBindVertexArray(
        adaptiveVAO
    );

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(
            adaptiveIndexCount
        ),
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(
        0
    );
}