#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aIntensity;

uniform mat4 uView;
uniform mat4 uProjection;

out float vIntensity;

void main()
{
    gl_Position =
        uProjection *
        uView *
        vec4(
            aPosition,
            1.0
        );

    vIntensity =
        aIntensity;

    gl_PointSize = 2.0;
}