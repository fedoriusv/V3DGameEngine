#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

layout (binding = 3) uniform UBO 
{
    vec4 lightPosition;
    vec4 lightColor;
} light;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 color = texture(sampler2D(textureColor, samplerColor), inUV);
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(light.lightPosition.xyz);
    
    outFragColor = color * max(dot(normal, lightDir), 0.1) * light.lightColor;
}