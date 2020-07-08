#version 450

layout (location = 0) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 color = texture(sampler2D(textureColor, samplerColor), inUV);
    outFragColor = color;
}