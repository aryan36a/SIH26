#version 330 core

in  float vElevation;
out vec4  FragColor;

uniform float uMinElevation;
uniform float uMaxElevation;

void main()
{
    // Circular sprite — discard fragments outside the circle
    vec2  coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0) discard;

    float range = uMaxElevation - uMinElevation;
    float t     = (range > 1e-6)
        ? clamp((vElevation - uMinElevation) / range, 0.0, 1.0)
        : 0.5;

    // Perceptually smoother thermal map. The percentile range is supplied
    // by the renderer, so normal terrain uses the full palette while outlier
    // returns are clipped instead of flattening the entire scene.
    vec3 color;
    if (t < 0.20)
        color = mix(vec3(0.02, 0.04, 0.18), vec3(0.00, 0.45, 0.90), t / 0.20);
    else if (t < 0.45)
        color = mix(vec3(0.00, 0.45, 0.90), vec3(0.00, 0.85, 0.45), (t - 0.20) / 0.25);
    else if (t < 0.70)
        color = mix(vec3(0.00, 0.85, 0.45), vec3(1.00, 0.85, 0.05), (t - 0.45) / 0.25);
    else
        color = mix(vec3(1.00, 0.85, 0.05), vec3(0.95, 0.12, 0.02), (t - 0.70) / 0.30);

    FragColor = vec4(color, 1.0);
}