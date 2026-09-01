#include "PointCloud.h"

#include <cmath>
#include <fstream>
#include <iostream>

int main() {

    std::ofstream file(
        "data/test.bin",
        std::ios::binary
    );

    if (!file.is_open()) {

        std::cerr
            << "Failed to create data/test.bin\n";

        return 1;
    }

    /*
     * --------------------------------------------------
     * Synthetic LiDAR scene
     * --------------------------------------------------
     *
     * We intentionally generate points at a finer
     * spacing than our adaptive cells.
     *
     * Adaptive resolution:
     *
     *   0 - 10 m  -> 0.05 m
     *   10 - 30 m -> 0.10 m
     *   30+ m     -> 0.25 m
     *
     * The point spacing is 0.025 m in the dense
     * region, allowing multiple points to fall
     * inside the same adaptive cell.
     */

    const float spacing = 0.025f;

    const int width =
        static_cast<int>(40.0f / spacing);

    const int height =
        static_cast<int>(40.0f / spacing);

    std::size_t pointCount = 0;

    /*
     * --------------------------------------------------
     * Generate terrain
     * --------------------------------------------------
     */

    for (int y = 0; y < height; ++y) {

        for (int x = 0; x < width; ++x) {

            const float worldX =
                static_cast<float>(x) * spacing;

            const float worldY =
                static_cast<float>(y) * spacing;

            /*
             * Base terrain.
             */
            float base =
                1.5f;

            /*
             * Large smooth hill.
             */
            float hill1 =
                3.0f *
                std::exp(
                    -(
                        (worldX - 8.0f) *
                        (worldX - 8.0f)
                        +
                        (worldY - 8.0f) *
                        (worldY - 8.0f)
                    ) / 35.0f
                );

            /*
             * Second terrain feature.
             */
            float hill2 =
                1.5f *
                std::exp(
                    -(
                        (worldX - 20.0f) *
                        (worldX - 20.0f)
                        +
                        (worldY - 12.0f) *
                        (worldY - 12.0f)
                    ) / 50.0f
                );

            /*
             * Long wave-like terrain.
             */
            float wave =
                0.4f *
                std::sin(worldX * 0.35f) *
                std::cos(worldY * 0.30f);

            /*
             * Small local feature near the sensor.
             *
             * This makes the high-resolution region
             * visually interesting.
             */
            float localFeature =
                0.8f *
                std::exp(
                    -(
                        (worldX - 4.0f) *
                        (worldX - 4.0f)
                        +
                        (worldY - 4.0f) *
                        (worldY - 4.0f)
                    ) / 4.0f
                );

            float z =
                base +
                hill1 +
                hill2 +
                wave +
                localFeature;

            /*
             * Intensity varies with distance.
             *
             * This is synthetic, but gives the
             * renderer something useful to visualize.
             */
            float distance =
                std::sqrt(
                    worldX * worldX +
                    worldY * worldY
                );

            float intensity;

            if (distance < 10.0f) {

                intensity = 60.0f;

            } else if (distance < 30.0f) {

                intensity = 40.0f;

            } else {

                intensity = 20.0f;
            }

            /*
             * Add a small spatial variation to
             * intensity.
             */
            intensity +=
                5.0f *
                std::sin(worldX * 0.2f);

            Point point{
                worldX,
                worldY,
                z,
                intensity
            };

            file.write(
                reinterpret_cast<const char *>(&point),
                sizeof(Point)
            );

            ++pointCount;
        }
    }

    file.close();

    std::cout
        << "Created data/test.bin\n";

    std::cout
        << "Points: "
        << pointCount
        << '\n';

    std::cout
        << "Scene size: 40m x 40m\n";

    std::cout
        << "Point spacing: "
        << spacing
        << "m\n";

    return 0;
}