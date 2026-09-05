#version 330 core

layout(location = 0) in vec3  aPosition;

uniform mat4  uView;
uniform mat4  uProjection;
uniform float uPointBaseSize;   // world-space base size (metres)

out float vElevation;

void main()
{
    vec4 cameraPos = uView * vec4(aPosition, 1.0);

    gl_Position = uProjection * cameraPos;

    // Keep raw points readable without turning near returns into squares.
    float depth    = max(-cameraPos.z, 0.001);
    float rawSize  = uPointBaseSize * 55.0 / depth;
    gl_PointSize   = clamp(rawSize, 1.5, 5.0);

    vElevation = aPosition.z;
}