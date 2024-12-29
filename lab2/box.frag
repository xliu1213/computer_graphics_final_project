#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal; 

in vec2 uv;

uniform sampler2D textureSampler;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform float exposure;

in vec4 lightSpacePosition; // for shadow mapping
uniform sampler2D shadowMap; // for shadow mapping

float CalcShadowFactor() {
    vec3 Coords = lightSpacePosition.xyz / lightSpacePosition.w;

    // Transform to [0, 1] range for all coordinates
    Coords = Coords * 0.5 + 0.5;

    // Early exit for fragments outside the light frustum
    if (Coords.z > 1.0) return 1.0;

    // Retrieve depth from shadow map
    float Depth = texture(shadowMap, Coords.xy).r;
    float bias = 0.0025;

    // Compare depths with bias
    return (Coords.z >= Depth + bias) ? 0.5 : 1.0;
}

void main()
{
	vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 BRDF = color / 3.14159;
    float cosine = max(dot(N, L), 0);
    vec3 lightSourceIrradiance = lightIntensity / (4 * 3.14159 * pow(length(lightPosition - worldPosition), 2.0));
    vec3 diffuse = BRDF * cosine * lightSourceIrradiance; 
    vec3 mapped = diffuse * exposure; // Apply exposure before tone mapping

    // for shadow mapping
    float shadow = CalcShadowFactor();  
    mapped = mapped * shadow;

    vec3 toneMapping = mapped / (1 + mapped);
	vec4 texColor = texture(textureSampler, uv);  // Perform texture lookup using UV coordinates
    finalColor = pow(toneMapping, vec3(1 / 2.2)) + texColor.rgb;            // Modulate the RGB components of the texture color with the vertex color
}