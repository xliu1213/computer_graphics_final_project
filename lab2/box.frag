#version 330 core

in vec3 color;

in vec2 uv;

uniform sampler2D textureSampler;

out vec3 finalColor;

void main()
{
	vec4 texColor = texture(textureSampler, uv);  // Perform texture lookup using UV coordinates
    finalColor = color * texColor.rgb;            // Modulate the RGB components of the texture color with the vertex color
}
