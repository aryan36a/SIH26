#version 330 core

// Must match AdaptiveCellMesh::Vertex layout:
//   float x, float y, float z, float intensity
layout(location = 0) in vec3  aPosition;  // (x, y, z) in world space
layout(location = 1) in float aIntensity; // pre-normalized [0..1]
layout(location = 2) in vec3  aNormal;

uniform mat4 uView;
uniform mat4 uProjection;

out float vElevation;   // world-space Z, passed for fragment colormap
out float vIntensity;   // pre-normalized intensity
out vec3  vNormal;

void main()
{
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
    vElevation  = aPosition.z;   // Z is the elevation axis
    vIntensity  = aIntensity;
    vNormal     = aNormal;
}