#version 330 core

layout(location = 0) in vec3  aPosition;
layout(location = 1) in float aIntensity;

uniform mat4  uView;
uniform mat4  uProjection;
uniform float uPointBaseSize;   // world-space base size (metres)

out float vIntensity;

void main()
{
    vec4 cameraPos = uView * vec4(aPosition, 1.0);

    gl_Position = uProjection * cameraPos;

    // Keep raw points readable without turning near returns into squares.
    float depth    = max(-cameraPos.z, 0.001);
    float rawSize  = uPointBaseSize * 80.0 / depth;
    gl_PointSize   = clamp(rawSize, 2.0, 8.0);

    vIntensity = aIntensity;
}