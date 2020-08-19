#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

struct Light
{
    vec4 direction;
    vec4 diffuse;
};

layout (binding = 3) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = texture(sampler2D(textureColor, samplerColor), inUV);
    vec3 normal = normalize(inNormal);
    
    outFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightDir = normalize(light.lights[l].direction.xyz);
        outFragColor += colorDiffuse * max(dot(normal, lightDir), 0.01) * light.lights[l].diffuse;
    }
}