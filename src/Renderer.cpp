#include "Renderer.h"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "AdaptiveCellMesh.h"


// ============================================================
// Helpers
// ============================================================

static std::string readFile(const char* path)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        std::cerr << "Renderer: cannot open " << path << '\n';
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}


void Renderer::setUniform1f(unsigned int prog, const char* name, float val)
{
    int loc = glGetUniformLocation(prog, name);
    if (loc >= 0) glUniform1f(loc, val);
}


void Renderer::setCommonUniforms(unsigned int prog,
                                  const glm::mat4& view,
                                  const glm::mat4& proj)
{
    int vLoc = glGetUniformLocation(prog, "uView");
    int pLoc = glGetUniformLocation(prog, "uProjection");
    if (vLoc >= 0) glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (pLoc >= 0) glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(proj));
}


// ============================================================
// Constructor
// ============================================================

Renderer::Renderer()
{
    // Raw point cloud buffers
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO);

    // Uniform grid buffers
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);

    // Adaptive mesh buffers
    glGenVertexArrays(1, &adaptiveVAO);
    glGenBuffers(1, &adaptiveVBO);
    glGenBuffers(1, &adaptiveEBO);

    if (!createShaderProgram())
        std::cerr << "Renderer: failed to create point/grid shader\n";

    if (!createAdaptiveShaderProgram())
        std::cerr << "Renderer: failed to create adaptive shader\n";
}


// ============================================================
// Destructor
// ============================================================

Renderer::~Renderer()
{
    if (shaderProgram)         { glDeleteProgram(shaderProgram);         shaderProgram = 0; }
    if (adaptiveShaderProgram) { glDeleteProgram(adaptiveShaderProgram); adaptiveShaderProgram = 0; }

    if (pointVBO) { glDeleteBuffers(1, &pointVBO);  pointVBO = 0; }
    if (pointVAO) { glDeleteVertexArrays(1, &pointVAO); pointVAO = 0; }

    if (gridVBO) { glDeleteBuffers(1, &gridVBO);  gridVBO = 0; }
    if (gridVAO) { glDeleteVertexArrays(1, &gridVAO); gridVAO = 0; }

    if (adaptiveVBO) { glDeleteBuffers(1, &adaptiveVBO); adaptiveVBO = 0; }
    if (adaptiveEBO) { glDeleteBuffers(1, &adaptiveEBO); adaptiveEBO = 0; }
    if (adaptiveVAO) { glDeleteVertexArrays(1, &adaptiveVAO); adaptiveVAO = 0; }
}


// ============================================================
// Shader compilation
// ============================================================

unsigned int Renderer::compileShader(unsigned int type, const char* src)
{
    unsigned int shader = glCreateShader(type);
    if (!shader) return 0;

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int linkProgram(unsigned int vs, unsigned int fs)
{
    unsigned int prog = glCreateProgram();
    if (!prog) return 0;

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    int ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[2048];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "Shader link error:\n" << log << '\n';
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

bool Renderer::createShaderProgram()
{
    const std::string vsrc = readFile("shaders/point.vert");
    const std::string fsrc = readFile("shaders/point.frag");
    if (vsrc.empty() || fsrc.empty()) return false;

    unsigned int vs = compileShader(GL_VERTEX_SHADER,   vsrc.c_str());
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
    if (!vs || !fs) { glDeleteShader(vs); glDeleteShader(fs); return false; }

    shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return shaderProgram != 0;
}

bool Renderer::createAdaptiveShaderProgram()
{
    const std::string vsrc = readFile("shaders/adaptive.vert");
    const std::string fsrc = readFile("shaders/adaptive.frag");
    if (vsrc.empty() || fsrc.empty()) return false;

    unsigned int vs = compileShader(GL_VERTEX_SHADER,   vsrc.c_str());
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsrc.c_str());
    if (!vs || !fs) { glDeleteShader(vs); glDeleteShader(fs); return false; }

    adaptiveShaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return adaptiveShaderProgram != 0;
}


// ============================================================
// Upload: raw point cloud
// Single contiguous glBufferData (not per-point glBufferSubData)
// ============================================================

bool Renderer::uploadPointCloud(const PointCloud& cloud)
{
    // Pack into flat float buffer [x,y,z,intensity, ...]
    const std::size_t n = cloud.size();
    std::vector<float> buf;
    buf.reserve(n * 4);

    // Keep the true raw range for diagnostics, but use a robust percentile
    // range for display. A few outlier returns (e.g. vegetation/isolated
    // reflections) should not compress the useful terrain into the bottom
    // of the thermal colormap.
    std::vector<float> elevationSamples;
    elevationSamples.reserve(std::min<std::size_t>(n, 200000));

    bool haveFinitePoint = false;
    float rawMinElevation = 0.0f;
    float rawMaxElevation = 1.0f;
    float firstElevation = 0.0f;
    float middleElevation = 0.0f;
    float lastElevation = 0.0f;
    std::size_t finitePointIndex = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Point& p = cloud.getPoint(i);
        if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
            !std::isfinite(p.z) || !std::isfinite(p.intensity)) continue;

        if (!haveFinitePoint)
        {
            rawMinElevation = p.z;
            rawMaxElevation = p.z;
            firstElevation = p.z;
            haveFinitePoint = true;
        }
        else
        {
            rawMinElevation = std::min(rawMinElevation, p.z);
            rawMaxElevation = std::max(rawMaxElevation, p.z);
        }

        if (finitePointIndex == n / 2) middleElevation = p.z;
        lastElevation = p.z;

        // Deterministic bounded sample for percentile normalization.
        const std::size_t sampleStride = std::max<std::size_t>(n / 200000, 1);
        if ((finitePointIndex % sampleStride) == 0)
            elevationSamples.push_back(p.z);

        ++finitePointIndex;

        buf.push_back(p.x);
        buf.push_back(p.y);
        buf.push_back(p.z);
        buf.push_back(p.intensity);
    }

    if (!haveFinitePoint) return false;

    // 1st/99th percentile gives the terrain most of the color gamut while
    // retaining rare high/low returns in the geometry itself.
    std::sort(elevationSamples.begin(), elevationSamples.end());
    const std::size_t sampleCount = elevationSamples.size();
    const std::size_t lowIndex = static_cast<std::size_t>((sampleCount - 1) * 0.01f);
    const std::size_t highIndex = static_cast<std::size_t>((sampleCount - 1) * 0.99f);

    minPointElevation = elevationSamples[lowIndex];
    maxPointElevation = elevationSamples[highIndex];

    if (maxPointElevation - minPointElevation < 1e-4f)
    {
        minPointElevation = rawMinElevation;
        maxPointElevation = rawMaxElevation;
    }

    std::cout << "Raw LiDAR elevation Z range: "
              << rawMinElevation << " -> " << rawMaxElevation << '\n'
              << "Raw LiDAR display Z range (P01-P99): "
              << minPointElevation << " -> " << maxPointElevation << '\n'
              << "Raw LiDAR representative Z values: "
              << firstElevation << ", " << middleElevation << ", "
              << lastElevation << '\n'
              << "Raw LiDAR normalized display range: 0 -> 1" << '\n';

    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(buf.size() * sizeof(float)),
                 buf.data(), GL_STATIC_DRAW);

    // location 0: position (x,y,z), stride=16, offset=0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // location 1: intensity, stride=16, offset=12
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    pointCount = static_cast<GLsizei>(buf.size() / 4);
    return true;
}


// ============================================================
// Upload: uniform spatial grid
// ============================================================

bool Renderer::uploadGrid(const SpatialGrid& grid)
{
    if (grid.getWidth() == 0 || grid.getHeight() == 0) return false;

    std::vector<float> buf;
    buf.reserve(grid.getWidth() * grid.getHeight() * 4);

    for (std::size_t y = 0; y < grid.getHeight(); ++y)
    for (std::size_t x = 0; x < grid.getWidth();  ++x)
    {
        if (!grid.hasData(x, y)) continue;
        float wx = grid.getMinX() + (static_cast<float>(x) + 0.5f) * grid.getCellSize();
        float wy = grid.getMinY() + (static_cast<float>(y) + 0.5f) * grid.getCellSize();
        buf.push_back(wx);
        buf.push_back(wy);
        buf.push_back(grid.getElevation(x, y));
        buf.push_back(grid.getIntensity(x, y));
    }

    if (buf.empty()) return false;

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(buf.size() * sizeof(float)),
                 buf.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    gridVertCount = static_cast<GLsizei>(buf.size() / 4);
    return true;
}


// ============================================================
// Upload: adaptive 2.5D mesh
// ============================================================

bool Renderer::uploadAdaptiveGrid(const AdaptiveSpatialGrid& grid)
{
    AdaptiveCellMesh mesh;
    mesh.build(grid);

    if (mesh.getVertexCount() == 0 || mesh.getIndexCount() == 0)
    {
        adaptiveIndexCount = 0;
        return false;
    }

    const auto& rawVerts = mesh.getVertices();
    const auto& idxData  = mesh.getIndices();
    const auto renderCells = grid.getRenderCells();

    // Derive the color range from observed cell statistics, not synthetic
    // wall vertices. Skirt geometry must never stretch the elevation map.
    {
        std::vector<float> zs;
        zs.reserve(renderCells.size() * 3);
        for (const auto& cell : renderCells)
        {
            zs.push_back(cell.minimumElevation);
            zs.push_back(cell.elevation);
            zs.push_back(cell.maximumElevation);
        }

        if (zs.empty())
        {
            adaptiveIndexCount = 0;
            return false;
        }

        std::sort(zs.begin(), zs.end());

        const std::size_t n   = zs.size();
        const std::size_t lo  = static_cast<std::size_t>(n * 0.02f);
        const std::size_t hi  = static_cast<std::size_t>(n * 0.98f);
        minElevation = zs[lo];
        maxElevation = zs[std::min(hi, n - 1)];
        if (maxElevation - minElevation < 1e-4f) maxElevation = minElevation + 1.0f;
    }

    // ---- Normalize intensity to [0,1] before GPU upload ----
    float iMin = rawVerts[0].intensity;
    float iMax = rawVerts[0].intensity;
    for (const auto& v : rawVerts) {
        if (v.intensity < iMin) iMin = v.intensity;
        if (v.intensity > iMax) iMax = v.intensity;
    }
    const float iRange = (iMax - iMin > 1e-6f) ? (iMax - iMin) : 1.0f;

    std::vector<AdaptiveCellMesh::Vertex> normVerts = rawVerts;
    for (auto& v : normVerts)
        v.intensity = (v.intensity - iMin) / iRange;

    // ---- GPU upload ----
    glBindVertexArray(adaptiveVAO);

    glBindBuffer(GL_ARRAY_BUFFER, adaptiveVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(normVerts.size() * sizeof(AdaptiveCellMesh::Vertex)),
                 normVerts.data(), GL_STATIC_DRAW);

    // IMPORTANT: bind EBO WHILE VAO is active so it is stored in VAO state
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, adaptiveEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(idxData.size() * sizeof(unsigned int)),
                 idxData.data(), GL_STATIC_DRAW);

    // location 0: position (x,y,z), stride = sizeof(Vertex) = 16
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(AdaptiveCellMesh::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // location 1: intensity (1 float at offset 12)
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                          sizeof(AdaptiveCellMesh::Vertex),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                          sizeof(AdaptiveCellMesh::Vertex),
                          reinterpret_cast<void*>(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    adaptiveIndexCount = static_cast<unsigned int>(idxData.size());

    // Unbind VBO ONLY — do NOT unbind EBO while VAO is active
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}


// ============================================================
// Render: raw point cloud
// ============================================================

void Renderer::render(const PointCloud& cloud,
                       const glm::mat4& view,
                       const glm::mat4& projection)
{
    (void)cloud;  // pointCount is cached from upload
    if (pointCount == 0 || shaderProgram == 0) return;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glUseProgram(shaderProgram);
    setCommonUniforms(shaderProgram, view, projection);
    setUniform1f(shaderProgram, "uMinElevation", minPointElevation);
    setUniform1f(shaderProgram, "uMaxElevation", maxPointElevation);
    setUniform1f(shaderProgram, "uPointBaseSize", 0.12f);

    glBindVertexArray(pointVAO);
    glDrawArrays(GL_POINTS, 0, pointCount);
    glBindVertexArray(0);
}


// ============================================================
// Render: uniform spatial grid (as points)
// ============================================================

void Renderer::render(const SpatialGrid& grid,
                       const glm::mat4& view,
                       const glm::mat4& projection)
{
    (void)grid;
    if (gridVertCount == 0 || shaderProgram == 0) return;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glUseProgram(shaderProgram);
    setCommonUniforms(shaderProgram, view, projection);
    setUniform1f(shaderProgram, "uMinElevation", minPointElevation);
    setUniform1f(shaderProgram, "uMaxElevation", maxPointElevation);
    setUniform1f(shaderProgram, "uPointBaseSize", 0.28f);

    glBindVertexArray(gridVAO);
    glDrawArrays(GL_POINTS, 0, gridVertCount);
    glBindVertexArray(0);
}


// ============================================================
// Render: adaptive 2.5D mesh
// ============================================================

void Renderer::renderAdaptive(const glm::mat4& view,
                               const glm::mat4& projection)
{
    if (adaptiveIndexCount == 0 || adaptiveShaderProgram == 0) return;

    glUseProgram(adaptiveShaderProgram);
    setCommonUniforms(adaptiveShaderProgram, view, projection);
    setUniform1f(adaptiveShaderProgram, "uMinElevation", minElevation);
    setUniform1f(adaptiveShaderProgram, "uMaxElevation", maxElevation);

    // Mesh winding is defined for the Z-up basis, so culling removes
    // hidden internal back faces while depth testing handles overlap.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindVertexArray(adaptiveVAO);
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(adaptiveIndexCount),
                   GL_UNSIGNED_INT,
                   nullptr);
    glBindVertexArray(0);
}