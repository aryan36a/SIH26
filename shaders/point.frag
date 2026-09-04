#version 330 core

in  float vIntensity;
out vec4  FragColor;

uniform float uMinIntensity;
uniform float uMaxIntensity;

void main()
{
    // Circular sprite — discard fragments outside the circle
    vec2  coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0) discard;

    float range = uMaxIntensity - uMinIntensity;
    float t     = (range > 1e-6)
        ? clamp((vIntensity - uMinIntensity) / range, 0.0, 1.0)
        : 0.5;

    // Thermal colormap: dark-blue → cyan → green → yellow → red
    vec3 color;
    if      (t < 0.25) color = mix(vec3(0.02, 0.02, 0.20), vec3(0.00, 0.60, 1.00), t / 0.25);
    else if (t < 0.50) color = mix(vec3(0.00, 0.60, 1.00), vec3(0.00, 1.00, 0.30), (t - 0.25) / 0.25);
    else if (t < 0.75) color = mix(vec3(0.00, 1.00, 0.30), vec3(1.00, 1.00, 0.00), (t - 0.50) / 0.25);
    else               color = mix(vec3(1.00, 1.00, 0.00), vec3(1.00, 0.10, 0.02), (t - 0.75) / 0.25);

    FragColor = vec4(color, 1.0);
}