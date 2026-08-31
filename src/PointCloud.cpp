#include "PointCloud.h"

#include <fstream>

void PointCloud::addPoint(const Point& point)
{
    points.push_back(point);
}

std::size_t PointCloud::size() const
{
    return points.size();
}

const Point& PointCloud::getPoint(std::size_t index) const
{
    return points[index];
}

bool PointCloud::loadXYZ(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    points.clear();

    float x;
    float y;
    float z;

    while (file >> x >> y >> z)
    {
        Point point{x, y, z, 0.0f};
        points.push_back(point);
    }

    return true;
}

bool PointCloud::loadBIN(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    file.seekg(0, std::ios::end);

    const std::streamsize fileSize = file.tellg();

    file.seekg(0, std::ios::beg);

    constexpr std::streamsize pointSize = sizeof(float) * 4;

    if (fileSize <= 0 || fileSize % pointSize != 0)
    {
        return false;
    }

    const std::size_t pointCount =
        static_cast<std::size_t>(fileSize / pointSize);

    points.clear();
    points.reserve(pointCount);

    float values[4];

    for (std::size_t i = 0; i < pointCount; ++i)
    {
        if (!file.read(
                reinterpret_cast<char*>(values),
                sizeof(values)))
        {
            points.clear();
            return false;
        }

        Point point{
            values[0],
            values[1],
            values[2],
            values[3]
        };

        points.push_back(point);
    }

    return true;
}
float PointCloud::getMinIntensity() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float minimum = points[0].intensity;

    for (const Point& point : points)
    {
        if (point.intensity < minimum)
        {
            minimum = point.intensity;
        }
    }

    return minimum;
}

float PointCloud::getMaxIntensity() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float maximum = points[0].intensity;

    for (const Point& point : points)
    {
        if (point.intensity > maximum)
        {
            maximum = point.intensity;
        }
    }

    return maximum;
}
float PointCloud::getMinX() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float minimum = points[0].x;

    for (const Point& point : points)
    {
        if (point.x < minimum)
        {
            minimum = point.x;
        }
    }

    return minimum;
}
float PointCloud::getMaxX() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float maximum = points[0].x;

    for (const Point& point : points)
    {
        if (point.x > maximum)
        {
            maximum = point.x;
        }
    }

    return maximum;
}

float PointCloud::getMinY() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float minimum = points[0].y;

    for (const Point& point : points)
    {
        if (point.y < minimum)
        {
            minimum = point.y;
        }
    }

    return minimum;
}

float PointCloud::getMaxY() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float maximum = points[0].y;

    for (const Point& point : points)
    {
        if (point.y > maximum)
        {
            maximum = point.y;
        }
    }

    return maximum;
}

float PointCloud::getMinZ() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float minimum = points[0].z;

    for (const Point& point : points)
    {
        if (point.z < minimum)
        {
            minimum = point.z;
        }
    }

    return minimum;
}

float PointCloud::getMaxZ() const
{
    if (points.empty())
    {
        return 0.0f;
    }

    float maximum = points[0].z;

    for (const Point& point : points)
    {
        if (point.z > maximum)
        {
            maximum = point.z;
        }
    }

    return maximum;
}