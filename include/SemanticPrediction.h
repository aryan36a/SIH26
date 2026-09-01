#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "MapCell.h"

/*
 * Semantic prediction associated with
 * one LiDAR point.
 */
struct SemanticPointPrediction {

    /*
     * Index of the point in PointCloud.
     */
    std::size_t pointIndex = 0;

    /*
     * Predicted semantic class.
     */
    SemanticClass semanticClass =
        SemanticClass::Unknown;

    /*
     * Confidence of the prediction.
     *
     * Expected range:
     *
     *     0.0 → 1.0
     */
    float confidence = 0.0f;
};


/*
 * Collection of semantic predictions
 * for one LiDAR frame.
 */
class SemanticPrediction {

private:

    std::vector<
        SemanticPointPrediction
    > predictions;

public:

    void clear() {
        predictions.clear();
    }


    void reserve(
        std::size_t count) {

        predictions.reserve(
            count
        );
    }


    void addPrediction(
        const SemanticPointPrediction &prediction) {

        predictions.push_back(
            prediction
        );
    }


    std::size_t size() const {
        return predictions.size();
    }


    const SemanticPointPrediction &
    getPrediction(
        std::size_t index) const {

        return predictions.at(
            index
        );
    }


    bool empty() const {
        return predictions.empty();
    }
};