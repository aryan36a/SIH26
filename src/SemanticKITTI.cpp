#include "SemanticKITTI.h"

#include <cstdint>
#include <fstream>
#include <utility>
#include <vector>


namespace
{

bool readBinaryFile(
    const std::string& filename,
    std::vector<std::uint8_t>& data)
{
    std::ifstream file(
        filename,
        std::ios::binary
    );

    if (!file.is_open())
    {
        return false;
    }

    file.seekg(
        0,
        std::ios::end
    );

    const std::streamsize fileSize =
        file.tellg();

    if (fileSize < 0)
    {
        return false;
    }

    file.seekg(
        0,
        std::ios::beg
    );

    data.resize(
        static_cast<std::size_t>(fileSize)
    );

    if (fileSize == 0)
    {
        return true;
    }

    return static_cast<bool>(
        file.read(
            reinterpret_cast<char*>(data.data()),
            fileSize
        )
    );
}

}


// ============================================================
// SemanticKITTIFrame
// ============================================================

void SemanticKITTIFrame::clear()
{
    /*
     * PointCloud does not expose a clear() method.
     *
     * Assigning a default-constructed PointCloud resets its
     * contents while keeping this class independent of a
     * PointCloud::clear() API.
     */

    pointCloud = PointCloud{};

    packedLabels.clear();
    semanticLabels.clear();
    instanceLabels.clear();
}


// ============================================================
// Load SemanticKITTI frame
// ============================================================

bool SemanticKITTIFrame::load(
    const std::string& pointFilename,
    const std::string& labelFilename)
{
    /*
     * --------------------------------------------------------
     * Reset previous frame
     * --------------------------------------------------------
     *
     * A failed load must never leave stale data from a
     * previous frame.
     */

    clear();


    /*
     * --------------------------------------------------------
     * Load point cloud into temporary storage
     * --------------------------------------------------------
     */

    PointCloud temporaryPointCloud;

    if (!temporaryPointCloud.loadBIN(
            pointFilename))
    {
        return false;
    }


    /*
     * --------------------------------------------------------
     * Load label file
     * --------------------------------------------------------
     */

    std::vector<std::uint8_t> labelBytes;

    if (!readBinaryFile(
            labelFilename,
            labelBytes))
    {
        return false;
    }


    /*
     * --------------------------------------------------------
     * Validate label byte size
     * --------------------------------------------------------
     *
     * SemanticKITTI stores exactly one uint32_t label
     * for every point.
     */

    constexpr std::size_t labelSize =
        sizeof(std::uint32_t);

    if (labelBytes.size() % labelSize != 0)
    {
        return false;
    }


    /*
     * --------------------------------------------------------
     * Calculate label count
     * --------------------------------------------------------
     */

    const std::size_t labelCount =
        labelBytes.size() / labelSize;


    /*
     * --------------------------------------------------------
     * Validate point/label correspondence
     * --------------------------------------------------------
     */

    if (labelCount != temporaryPointCloud.size())
    {
        return false;
    }


    /*
     * --------------------------------------------------------
     * Temporary decoded label storage
     * --------------------------------------------------------
     *
     * Nothing is written into the frame until all validation
     * and decoding succeeds.
     */

    std::vector<std::uint32_t>
        temporaryPackedLabels;

    std::vector<std::uint16_t>
        temporarySemanticLabels;

    std::vector<std::uint16_t>
        temporaryInstanceLabels;


    temporaryPackedLabels.reserve(
        labelCount
    );

    temporarySemanticLabels.reserve(
        labelCount
    );

    temporaryInstanceLabels.reserve(
        labelCount
    );


    /*
     * --------------------------------------------------------
     * Decode labels
     * --------------------------------------------------------
     *
     * SemanticKITTI label layout:
     *
     * bits  0-15 -> semantic class ID
     * bits 16-31 -> instance ID
     *
     * The dataset stores these values as little-endian
     * uint32_t values.
     */

    for (std::size_t i = 0;
         i < labelCount;
         ++i)
    {
        const std::size_t offset =
            i * labelSize;


        const std::uint32_t packedLabel =
            static_cast<std::uint32_t>(
                labelBytes[offset]
            )
            |
            (
                static_cast<std::uint32_t>(
                    labelBytes[offset + 1]
                )
                << 8
            )
            |
            (
                static_cast<std::uint32_t>(
                    labelBytes[offset + 2]
                )
                << 16
            )
            |
            (
                static_cast<std::uint32_t>(
                    labelBytes[offset + 3]
                )
                << 24
            );


        const std::uint16_t semanticID =
            static_cast<std::uint16_t>(
                packedLabel & 0xFFFFu
            );


        const std::uint16_t instanceID =
            static_cast<std::uint16_t>(
                (packedLabel >> 16) & 0xFFFFu
            );


        temporaryPackedLabels.push_back(
            packedLabel
        );

        temporarySemanticLabels.push_back(
            semanticID
        );

        temporaryInstanceLabels.push_back(
            instanceID
        );
    }


    /*
     * --------------------------------------------------------
     * Final consistency check
     * --------------------------------------------------------
     */

    if (temporaryPackedLabels.size() !=
        temporaryPointCloud.size())
    {
        return false;
    }


    if (temporarySemanticLabels.size() !=
        temporaryPointCloud.size())
    {
        return false;
    }


    if (temporaryInstanceLabels.size() !=
        temporaryPointCloud.size())
    {
        return false;
    }


    /*
     * --------------------------------------------------------
     * Commit
     * --------------------------------------------------------
     *
     * Every validation has succeeded.
     *
     * Only now replace the frame's contents.
     */

    pointCloud =
        std::move(
            temporaryPointCloud
        );

    packedLabels =
        std::move(
            temporaryPackedLabels
        );

    semanticLabels =
        std::move(
            temporarySemanticLabels
        );

    instanceLabels =
        std::move(
            temporaryInstanceLabels
        );


    return true;
}


// ============================================================
// Accessors
// ============================================================

std::size_t SemanticKITTIFrame::size() const
{
    return pointCloud.size();
}


bool SemanticKITTIFrame::empty() const
{
    return pointCloud.size() == 0;
}


const Point& SemanticKITTIFrame::getPoint(
    std::size_t index) const
{
    return pointCloud.getPoint(index);
}


std::uint32_t SemanticKITTIFrame::getPackedLabel(
    std::size_t index) const
{
    return packedLabels.at(index);
}


std::uint16_t SemanticKITTIFrame::getSemanticId(
    std::size_t index) const
{
    return semanticLabels.at(index);
}


std::uint16_t SemanticKITTIFrame::getInstanceId(
    std::size_t index) const
{
    return instanceLabels.at(index);
}