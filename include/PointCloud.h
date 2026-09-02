#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Point.h"

class PointCloud {
private:
  std::vector<Point> points;

public:
  void clear();
  void addPoint(const Point &point);

  std::size_t size() const;

  const Point &getPoint(std::size_t index) const;

  bool loadXYZ(const std::string &filename);

  bool loadBIN(const std::string &filename);

  float getMinIntensity() const;

  float getMaxIntensity() const;
  float getMinX() const;
  float getMaxX() const;

  float getMinY() const;
  float getMaxY() const;

  float getMinZ() const;
  float getMaxZ() const;
};