#version 330 core

in vec3 color;
in vec3 worldPosition; // lighting
in vec3 worldNormal; // lighting

out vec3 finalColor;

in vec2 uv;

uniform vec3 lightPosition; // lighting
uniform vec3 lightIntensity; // lighting
uniform float exposure; // lighting
uniform sampler2D textureSampler;

void main()
{
	vec4 texColor = texture(textureSampler, uv);  // Perform texture lookup using UV coordinates
    // Normalized normal vector
    vec3 N = normalize(worldNormal);

    // Light direction vector (from fragment to light)
    vec3 L = normalize(lightPosition - worldPosition);

    // Lambertian BRDF
    vec3 BRDF = color / 3.14159; // Assuming diffuse Lambertian reflectance

    // Cosine of the angle between the light direction and the surface normal
    float cosine = max(dot(N, L), 0.0);

    // Light source irradiance (attenuated by distance squared)
    vec3 lightSourceIrradiance = lightIntensity / (4.0 * 3.14159 * pow(length(lightPosition - worldPosition), 2.0));

    // Diffuse lighting computation
    vec3 diffuse = BRDF * cosine * lightSourceIrradiance;

    // Apply exposure before tone mapping
    vec3 mapped = diffuse * exposure;

    // Tone mapping (Reinhard tone mapping)
    vec3 toneMapping = mapped / (1.0 + mapped);

    // Gamma correction (sRGB gamma of 2.2)
    vec3 lightingColor = pow(toneMapping, vec3(1.0 / 2.2));

    // Combine lighting with texture color and vertex color
    finalColor = texColor.rgb * lightingColor; // Modulate texture color with lighting
}
