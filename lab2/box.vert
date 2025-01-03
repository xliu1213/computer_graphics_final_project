#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexUV;        // vertex UV coordinates for texture mapping
layout(location = 3) in vec3 vertexNormal;

// Output data, to be interpolated for each fragment
out vec3 color;
out vec2 uv;    // Pass UV to fragment shader
out vec3 worldPosition;
out vec3 worldNormal;

// Matrix for vertex transformation
uniform mat4 MVP;

uniform mat4 lightSpaceTransformMatrix; // for shadow mapping
out vec4 lightSpacePosition; // for shadow mapping

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);
    
    // Pass vertex color to the fragment shader
    color = vertexColor;

    // Pass UV to the fragment shader
    uv = vertexUV;

    worldPosition = vertexPosition;
    worldNormal = vertexNormal;

    lightSpacePosition = lightSpaceTransformMatrix * vec4(worldPosition, 1.0); // for shadow mapping
}