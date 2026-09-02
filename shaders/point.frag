#version 330 core

in float vIntensity;

uniform float uMinIntensity;
uniform float uMaxIntensity;

out vec4 FragColor;

void main()
{
    float range =
        uMaxIntensity - uMinIntensity;

    float normalizedIntensity = 0.0;

    if (range > 0.000001)
    {
        normalizedIntensity =
            (vIntensity - uMinIntensity) /
            range;
    }

    normalizedIntensity =
        clamp(
            normalizedIntensity,
            0.0,
            1.0
        );

    /*
     * Intensity visualization.
     *
     * Low intensity:
     *     dark blue
     *
     * Medium:
     *     cyan / green
     *
     * High:
     *     yellow / red
     */

    vec3 color;

    if (normalizedIntensity < 0.25)
    {
        float t =
            normalizedIntensity / 0.25;

        color =
            mix(
                vec3(0.02, 0.02, 0.20),
                vec3(0.00, 0.60, 1.00),
                t
            );
    }
    else if (normalizedIntensity < 0.50)
    {
        float t =
            (normalizedIntensity - 0.25) / 0.25;

        color =
            mix(
                vec3(0.00, 0.60, 1.00),
                vec3(0.00, 1.00, 0.30),
                t
            );
    }
    else if (normalizedIntensity < 0.75)
    {
        float t =
            (normalizedIntensity - 0.50) / 0.25;

        color =
            mix(
                vec3(0.00, 1.00, 0.30),
                vec3(1.00, 1.00, 0.00),
                t
            );
    }
    else
    {
        float t =
            (normalizedIntensity - 0.75) / 0.25;

        color =
            mix(
                vec3(1.00, 1.00, 0.00),
                vec3(1.00, 0.10, 0.02),
                t
            );
    }

    FragColor =
        vec4(
            color,
            1.0
        );
}