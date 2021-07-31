#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform sampler samplerNormal;
layout (binding = 3) uniform texture2D textureColor;
layout (binding = 4) uniform texture2D textureNormal;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (binding = 5) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = texture(sampler2D(textureColor, samplerColor), inUV);
    vec3 normalDiffuse = texture(sampler2D(textureNormal, samplerNormal), inUV).xyz;
    vec3 normal = normalDiffuse * 2.0 - 1.0;
    
    outFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    mat3 TBN = (mat3(normalize(inTangent), normalize(inBitangent), normalize(inNormal)));
    normal = TBN * normal;
    
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightDir = normalize(light.lights[l].position.xyz - inPosition);
        outFragColor += colorDiffuse * max(dot(normal, lightDir), 0.01) * light.lights[l].color;
    }
}