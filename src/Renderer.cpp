#include "Renderer.h"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Renderer::Renderer()
    : pointVBO(0), pointVAO(0), gridVBO(0), gridVAO(0), shaderProgram(0),
      minIntensity(0.0f), maxIntensity(1.0f) {
  glGenVertexArrays(1, &pointVAO);
  glGenBuffers(1, &pointVBO);

  glGenVertexArrays(1, &gridVAO);
  glGenBuffers(1, &gridVBO);

  createShaderProgram();
}

Renderer::~Renderer() {
  if (shaderProgram != 0) {
    glDeleteProgram(shaderProgram);
  }

  glDeleteBuffers(1, &pointVBO);
  glDeleteVertexArrays(1, &pointVAO);

  glDeleteBuffers(1, &gridVBO);
  glDeleteVertexArrays(1, &gridVAO);
}

unsigned int Renderer::compileShader(unsigned int type, const char *source) {
  unsigned int shader = glCreateShader(type);

  glShaderSource(shader, 1, &source, nullptr);

  glCompileShader(shader);

  int success = 0;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    char infoLog[512];

    glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);

    std::cerr << "Shader compilation failed:\n" << infoLog << '\n';

    glDeleteShader(shader);

    return 0;
  }

  return shader;
}

bool Renderer::createShaderProgram() {
  std::ifstream vertexFile("shaders/point.vert");

  std::ifstream fragmentFile("shaders/point.frag");

  if (!vertexFile.is_open()) {
    std::cerr << "Failed to open vertex shader\n";

    return false;
  }

  if (!fragmentFile.is_open()) {
    std::cerr << "Failed to open fragment shader\n";

    return false;
  }

  std::stringstream vertexStream;
  std::stringstream fragmentStream;

  vertexStream << vertexFile.rdbuf();
  fragmentStream << fragmentFile.rdbuf();

  std::string vertexSource = vertexStream.str();

  std::string fragmentSource = fragmentStream.str();

  unsigned int vertexShader =
      compileShader(GL_VERTEX_SHADER, vertexSource.c_str());

  if (vertexShader == 0) {
    return false;
  }

  unsigned int fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

  if (fragmentShader == 0) {
    glDeleteShader(vertexShader);

    return false;
  }

  shaderProgram = glCreateProgram();

  glAttachShader(shaderProgram, vertexShader);

  glAttachShader(shaderProgram, fragmentShader);

  glLinkProgram(shaderProgram);

  int success = 0;

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

  if (!success) {
    char infoLog[512];

    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);

    std::cerr << "Shader linking failed:\n" << infoLog << '\n';

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glDeleteProgram(shaderProgram);

    shaderProgram = 0;

    return false;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return true;
}

bool Renderer::uploadPointCloud(const PointCloud &cloud) {
  if (cloud.size() == 0) {
    return false;
  }

  minIntensity = cloud.getMinIntensity();

  maxIntensity = cloud.getMaxIntensity();

  glBindVertexArray(pointVAO);

  glBindBuffer(GL_ARRAY_BUFFER, pointVBO);

  glBufferData(GL_ARRAY_BUFFER, cloud.size() * sizeof(Point), nullptr,
               GL_STATIC_DRAW);

  for (std::size_t i = 0; i < cloud.size(); ++i) {
    const Point &point = cloud.getPoint(i);

    glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(Point), sizeof(Point), &point);
  }

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                        reinterpret_cast<void *>(0));

  glEnableVertexAttribArray(0);

  // Intensity
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Point),
                        reinterpret_cast<void *>(sizeof(float) * 3));

  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  return true;
}

bool Renderer::uploadGrid(const SpatialGrid &grid) {
  if (grid.getWidth() == 0 || grid.getHeight() == 0) {
    return false;
  }

  std::vector<Point> gridPoints;

  for (std::size_t y = 0; y < grid.getHeight(); ++y) {
    for (std::size_t x = 0; x < grid.getWidth(); ++x) {
      if (!grid.hasData(x, y)) {
        continue;
      }

      float worldX =
          grid.getMinX() + (static_cast<float>(x) + 0.5f) * grid.getCellSize();

      float worldY =
          grid.getMinY() + (static_cast<float>(y) + 0.5f) * grid.getCellSize();

      float elevation = grid.getElevation(x, y);

      float intensity = grid.getIntensity(x, y);

      Point point{worldX, worldY, elevation, intensity};

      gridPoints.push_back(point);
    }
  }

  if (gridPoints.empty()) {
    return false;
  }

  glBindVertexArray(gridVAO);

  glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

  glBufferData(GL_ARRAY_BUFFER, gridPoints.size() * sizeof(Point),
               gridPoints.data(), GL_STATIC_DRAW);

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                        reinterpret_cast<void *>(0));

  glEnableVertexAttribArray(0);

  // Intensity
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Point),
                        reinterpret_cast<void *>(sizeof(float) * 3));

  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);

  return true;
}

void Renderer::render(const PointCloud &cloud, const glm::mat4 &view,
                      const glm::mat4 &projection) {
  if (cloud.size() == 0 || shaderProgram == 0) {
    return;
  }

  glUseProgram(shaderProgram);

  int viewLocation = glGetUniformLocation(shaderProgram, "uView");

  int projectionLocation = glGetUniformLocation(shaderProgram, "uProjection");

  int minIntensityLocation =
      glGetUniformLocation(shaderProgram, "uMinIntensity");

  int maxIntensityLocation =
      glGetUniformLocation(shaderProgram, "uMaxIntensity");

  glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                     glm::value_ptr(projection));

  glUniform1f(minIntensityLocation, minIntensity);

  glUniform1f(maxIntensityLocation, maxIntensity);

  glBindVertexArray(pointVAO);

  glDrawArrays(GL_POINTS, 0, static_cast<int>(cloud.size()));

  glBindVertexArray(0);
}

void Renderer::render(const SpatialGrid &grid, const glm::mat4 &view,
                      const glm::mat4 &projection) {
  if (grid.getWidth() == 0 || grid.getHeight() == 0 || shaderProgram == 0) {
    return;
  }

  glUseProgram(shaderProgram);

  int viewLocation = glGetUniformLocation(shaderProgram, "uView");

  int projectionLocation = glGetUniformLocation(shaderProgram, "uProjection");

  int minIntensityLocation =
      glGetUniformLocation(shaderProgram, "uMinIntensity");

  int maxIntensityLocation =
      glGetUniformLocation(shaderProgram, "uMaxIntensity");

  glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE,
                     glm::value_ptr(projection));

  glUniform1f(minIntensityLocation, minIntensity);

  glUniform1f(maxIntensityLocation, maxIntensity);

  std::size_t populatedCells = 0;

  for (std::size_t y = 0; y < grid.getHeight(); ++y) {
    for (std::size_t x = 0; x < grid.getWidth(); ++x) {
      if (grid.hasData(x, y)) {
        ++populatedCells;
      }
    }
  }

  if (populatedCells == 0) {
    glBindVertexArray(0);
    return;
  }

  glBindVertexArray(gridVAO);

  glDrawArrays(GL_POINTS, 0, static_cast<int>(populatedCells));

  glBindVertexArray(0);
}