#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

/*
 * Semantic classes used by the mapping layer.
 *
 * These are deliberately generic.
 *
 * The AI model does NOT need to use these exact
 * numeric IDs internally. A later adapter can
 * translate model-specific classes into this
 * representation.
 */
enum class SemanticClass : std::uint8_t {
    Unknown = 0,
    Ground = 1,
    Road = 2,
    Sidewalk = 3,
    Vegetation = 4,
    Vehicle = 5,
    Person = 6,
    Building = 7,
    Obstacle = 8
};

/*
 * Maximum number of semantic classes tracked
 * by a cell.
 *
 * This keeps the initial implementation simple
 * and deterministic.
 */
constexpr std::size_t SEMANTIC_CLASS_COUNT = 9;

/*
 * A single 2.5D map cell.
 *
 * The cell stores geometric information as well
 * as semantic evidence.
 */
struct MapCell {

    /*
     * --------------------------------------------------
     * Geometric state
     * --------------------------------------------------
     */

    float elevation = 0.0f;

    float intensity = 0.0f;

    std::size_t pointCount = 0;

    float elevationSum = 0.0f;

    float minimumElevation = 0.0f;

    float maximumElevation = 0.0f;


    /*
     * --------------------------------------------------
     * Semantic state
     * --------------------------------------------------
     *
     * semanticEvidence[class] stores how many
     * observations contributed evidence for that class.
     */
    std::array<std::size_t, SEMANTIC_CLASS_COUNT>
        semanticEvidence{};

    /*
     * Current representative semantic class.
     */
    SemanticClass semanticClass =
        SemanticClass::Unknown;

    /*
     * Confidence in the representative semantic class.
     *
     * Range:
     *
     *     0.0 → no confidence
     *     1.0 → complete confidence
     */
    float semanticConfidence = 0.0f;
};