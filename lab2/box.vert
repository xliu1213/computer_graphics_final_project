#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal; // for lighting
layout(location = 3) in vec2 vertexUV;        // vertex UV coordinates for texture mapping

// Output data, to be interpolated for each fragment
out vec3 color;
out vec3 worldPosition; // for lighting
out vec3 worldNormal; // for lighting
out vec2 uv;    // Pass UV to fragment shader

// Matrix for vertex transformation
uniform mat4 MVP;

void main() {
    // Transform vertex
    gl_Position =  MVP * vec4(vertexPosition, 1);
    
    // Pass vertex color to the fragment shader
    color = vertexColor;

    // Pass UV to the fragment shader
    uv = vertexUV;

    // Transform vertex position to world space
    worldPosition = vertexPosition;

    // Transform normal vector to world space (using normal matrix)
    worldNormal = vertexNormal;
}
