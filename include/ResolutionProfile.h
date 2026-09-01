#pragma once

class ResolutionProfile {
private:
    float nearDistance;
    float nearResolution;

    float midDistance;
    float midResolution;

    float farResolution;

public:
    /*
     * Default profile:
     *
     * 0m  - 10m  -> 0.05m
     * 10m - 25m  -> 0.10m
     * 25m+       -> 0.25m
     */
    ResolutionProfile();

    ResolutionProfile(
        float nearDistance,
        float nearResolution,
        float midDistance,
        float midResolution,
        float farResolution
    );

    /*
     * Return the resolution that should be used
     * at a particular distance from the sensor.
     */
    float getResolution(float distance) const;

    /*
     * Return the resolution band:
     *
     * 0 -> near
     * 1 -> mid
     * 2 -> far
     */
    int getBand(float distance) const;

    /*
     * Return the resolution associated with
     * a specific band.
     */
    float getResolutionForBand(int band) const;

    float getNearDistance() const;
    float getNearResolution() const;

    float getMidDistance() const;
    float getMidResolution() const;

    float getFarResolution() const;
};