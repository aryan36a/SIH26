#include "ResolutionProfile.h"

#include <stdexcept>

ResolutionProfile::ResolutionProfile()
    : nearDistance(10.0f),
      nearResolution(0.05f),
      midDistance(25.0f),
      midResolution(0.10f),
      farResolution(0.25f) {
}

ResolutionProfile::ResolutionProfile(
    float nearDistance,
    float nearResolution,
    float midDistance,
    float midResolution,
    float farResolution)
    : nearDistance(nearDistance),
      nearResolution(nearResolution),
      midDistance(midDistance),
      midResolution(midResolution),
      farResolution(farResolution) {

    if (nearDistance <= 0.0f) {
        throw std::invalid_argument(
            "Near distance must be greater than zero"
        );
    }

    if (midDistance <= nearDistance) {
        throw std::invalid_argument(
            "Mid distance must be greater than near distance"
        );
    }

    if (nearResolution <= 0.0f) {
        throw std::invalid_argument(
            "Near resolution must be greater than zero"
        );
    }

    if (midResolution <= 0.0f) {
        throw std::invalid_argument(
            "Mid resolution must be greater than zero"
        );
    }

    if (farResolution <= 0.0f) {
        throw std::invalid_argument(
            "Far resolution must be greater than zero"
        );
    }
}

int ResolutionProfile::getBand(float distance) const {

    if (distance < 0.0f) {
        throw std::invalid_argument(
            "Distance cannot be negative"
        );
    }

    if (distance < nearDistance) {
        return 0;
    }

    if (distance < midDistance) {
        return 1;
    }

    return 2;
}

float ResolutionProfile::getResolution(float distance) const {

    const int band = getBand(distance);

    return getResolutionForBand(band);
}

float ResolutionProfile::getResolutionForBand(int band) const {

    switch (band) {

        case 0:
            return nearResolution;

        case 1:
            return midResolution;

        case 2:
            return farResolution;

        default:
            throw std::out_of_range(
                "Resolution band out of range"
            );
    }
}

float ResolutionProfile::getNearDistance() const {
    return nearDistance;
}

float ResolutionProfile::getNearResolution() const {
    return nearResolution;
}

float ResolutionProfile::getMidDistance() const {
    return midDistance;
}

float ResolutionProfile::getMidResolution() const {
    return midResolution;
}

float ResolutionProfile::getFarResolution() const {
    return farResolution;
}