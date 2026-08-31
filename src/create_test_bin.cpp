#include "PointCloud.h"

#include <cmath>
#include <fstream>
#include <iostream>

int main() {
  std::ofstream file("data/test.bin", std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Failed to create data/test.bin\n";

    return 1;
  }

  const int width = 100;
  const int height = 100;

  const float spacing = 0.1f;

  std::size_t pointCount = 0;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float worldX = static_cast<float>(x) * spacing;

      float worldY = static_cast<float>(y) * spacing;

      /*
       * Procedural terrain.
       *
       * Base terrain + two smooth hills.
       */
      float base = 2.0f;

      float hill1 = 3.0f * std::exp(-((worldX - 4.0f) * (worldX - 4.0f) +
                                      (worldY - 4.0f) * (worldY - 4.0f)) /
                                    4.0f);

      float hill2 = 1.5f * std::exp(-((worldX - 7.0f) * (worldX - 7.0f) +
                                      (worldY - 2.5f) * (worldY - 2.5f)) /
                                    2.0f);

      float wave = 0.3f * std::sin(worldX) * std::cos(worldY);

      float z = base + hill1 + hill2 + wave;

      /*
       * Procedural intensity.
       */
      float intensity = 10.0f + 50.0f * (static_cast<float>(x) /
                                         static_cast<float>(width - 1));

      Point point{worldX, worldY, z, intensity};

      file.write(reinterpret_cast<const char *>(&point), sizeof(Point));

      ++pointCount;
    }
  }

  file.close();

  std::cout << "Created data/test.bin\n";

  std::cout << "Points: " << pointCount << '\n';

  return 0;
}