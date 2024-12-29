#version 330 core

layout(location = 0) in vec3 Pos;

uniform mat4 lightSpaceTransformMatrix;

void main()
{
    gl_Position = lightSpaceTransformMatrix * vec4(Pos, 1.0);
}