#version 330 core

in float intensity;

out vec4 FragColor;

uniform float uMinIntensity;
uniform float uMaxIntensity;

void main()
{
    float range =
        uMaxIntensity - uMinIntensity;

    float value = 0.0;

    if (range > 0.0)
    {
        value =
            (intensity - uMinIntensity) /
            range;
    }

    value = clamp(
        value,
        0.0,
        1.0
    );

    FragColor = vec4(
        value,
        value,
        value,
        1.0
    );
}