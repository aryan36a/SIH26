#version 330 core

in float vElevation;
in float vIntensity;

out vec4 FragColor;

uniform float uMinElevation;
uniform float uMaxElevation;

void main()
{
    float elevationRange =
        uMaxElevation - uMinElevation;

    float value = 0.5;

    if (elevationRange > 0.000001)
    {
        value =
            (vElevation - uMinElevation) /
            elevationRange;
    }

    value = clamp(
        value,
        0.0,
        1.0
    );

    /*
     * Elevation visualization.
     *
     * Low elevation:
     * dark blue
     *
     * Middle:
     * green
     *
     * High:
     * light yellow
     */

    vec3 lowColor =
        vec3(
            0.05,
            0.15,
            0.45
        );

    vec3 middleColor =
        vec3(
            0.10,
            0.65,
            0.25
        );

    vec3 highColor =
        vec3(
            0.95,
            0.85,
            0.25
        );

    vec3 color;

    if (value < 0.5)
    {
        color =
            mix(
                lowColor,
                middleColor,
                value * 2.0
            );
    }
    else
    {
        color =
            mix(
                middleColor,
                highColor,
                (value - 0.5) * 2.0
            );
    }

    FragColor =
        vec4(
            color,
            1.0
        );
}