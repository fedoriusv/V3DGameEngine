#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (binding = 3) uniform UBO 
{
    Light lights[LIGHT_COUNT];
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = texture(sampler2D(textureColor, samplerColor), inUV);
    vec3 normal = normalize(inNormal);
    
    outFragColor = vec4(0.0);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightDir = normalize(ubo.lights[l].position.xyz);
        outFragColor += colorDiffuse * max(dot(normal, lightDir), 0.01) * ubo.lights[l].color;
    }
}