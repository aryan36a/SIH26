#pragma once

#include <cstddef>
#include <vector>

#include "MapCell.h"
#include "PointCloud.h"

class SpatialGrid {
public:
  SpatialGrid(float minX, float maxX, float minY, float maxY, float cellSize);

  void build(const PointCloud &cloud);

  std::size_t getWidth() const;

  std::size_t getHeight() const;

  float getMinX() const;

  float getMinY() const;

  float getCellSize() const;

  const MapCell &getCell(std::size_t x, std::size_t y) const;

  bool hasData(std::size_t x, std::size_t y) const;

  float getElevation(std::size_t x, std::size_t y) const;

  float getIntensity(std::size_t x, std::size_t y) const;

  float getHeightVariation(std::size_t x, std::size_t y) const;

private:
  float minX;
  float maxX;

  float minY;
  float maxY;

  float cellSize;

  std::size_t width;
  std::size_t height;

  std::vector<MapCell> cells;

  std::size_t getIndex(std::size_t x, std::size_t y) const;
};