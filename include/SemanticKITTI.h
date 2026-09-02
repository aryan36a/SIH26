#pragma once

#include "PointCloud.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


class SemanticKITTIFrame
{
public:

    // ========================================================
    // Loading
    // ========================================================

    bool load(
        const std::string& pointFilename,
        const std::string& labelFilename
    );


    // ========================================================
    // Reset
    // ========================================================

    void clear();


    // ========================================================
    // Frame information
    // ========================================================

    std::size_t size() const;

    bool empty() const;


    // ========================================================
    // Point access
    // ========================================================

    const Point& getPoint(
        std::size_t index
    ) const;


    // ========================================================
    // Label access
    // ========================================================

    /*
     * Complete SemanticKITTI packed label.
     *
     * bits  0-15 -> semantic class ID
     * bits 16-31 -> instance ID
     */
    std::uint32_t getPackedLabel(
        std::size_t index
    ) const;


    /*
     * Semantic class ID.
     */
    std::uint16_t getSemanticId(
        std::size_t index
    ) const;


    /*
     * Instance ID.
     */
    std::uint16_t getInstanceId(
        std::size_t index
    ) const;


private:

    // ========================================================
    // Point data
    // ========================================================

    PointCloud pointCloud;


    // ========================================================
    // SemanticKITTI labels
    // ========================================================

    /*
     * Original 32-bit packed label.
     */
    std::vector<std::uint32_t> packedLabels;


    /*
     * Lower 16 bits of packedLabels.
     */
    std::vector<std::uint16_t> semanticLabels;


    /*
     * Upper 16 bits of packedLabels.
     */
    std::vector<std::uint16_t> instanceLabels;
};