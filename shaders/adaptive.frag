#version 330 core

in  float vElevation;
in  float vIntensity;
in  vec3  vNormal;
out vec4  FragColor;

uniform float uMinElevation;   // robust low end (2nd percentile)
uniform float uMaxElevation;   // robust high end (98th percentile)

void main()
{
    // --------------------------------------------------------
    // Elevation-based color (primary visual variable)
    // Color MUST NOT depend on XY position, radial distance,
    // or camera distance — only on Z.
    // --------------------------------------------------------
    float range = uMaxElevation - uMinElevation;
    float t     = (range > 1e-4)
        ? clamp((vElevation - uMinElevation) / range, 0.0, 1.0)
        : 0.5;

    // Restrained thermal map: elevation remains readable without neon blocks.
    vec3 color;
    if      (t < 0.25) color = mix(vec3(0.04, 0.09, 0.28), vec3(0.00, 0.42, 0.68), t / 0.25);
    else if (t < 0.50) color = mix(vec3(0.00, 0.42, 0.68), vec3(0.12, 0.68, 0.28), (t - 0.25) / 0.25);
    else if (t < 0.75) color = mix(vec3(0.12, 0.68, 0.28), vec3(0.86, 0.70, 0.08), (t - 0.50) / 0.25);
    else               color = mix(vec3(0.86, 0.70, 0.08), vec3(0.86, 0.16, 0.04), (t - 0.75) / 0.25);

    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(vec3(-0.35, -0.45, 0.85));
    float lighting = 0.72 + 0.28 * max(dot(normal, lightDirection), 0.0);
    color *= lighting;

    // --------------------------------------------------------
    // Restrained intensity modulation (±15%)
    // Keeps elevation as primary variable; intensity adds local
    // contrast without corrupting the color meaning.
    // vIntensity is pre-normalized [0..1] by the CPU.
    // --------------------------------------------------------
    float iMod = mix(0.85, 1.15, vIntensity);
    color      = clamp(color * iMod, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}