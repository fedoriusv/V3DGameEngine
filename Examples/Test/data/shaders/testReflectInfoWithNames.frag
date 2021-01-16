#version 450

layout (location = 0) in vec4 inAttribute0_vec4;
layout (location = 1) in vec2 inAttribute1_vec2;
layout (location = 2) in vec3 inAttribute2_vec3;

layout (set = 0, binding = 0) uniform sampler samplerColor00;

layout (set = 0, binding = 2) uniform texture2D textureColor02;
layout (set = 1, binding = 0) uniform texture2D textureColor10;
layout (set = 0, binding = 1) uniform texture2D textureColor01;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor.r = texture(sampler2D(textureColor02, samplerColor00), inAttribute1_vec2).r;
    outFragColor.g = texture(sampler2D(textureColor10, samplerColor00), inAttribute1_vec2).g;
    outFragColor.b = texture(sampler2D(textureColor01, samplerColor00), inAttribute1_vec2).b;
    outFragColor.a = 0.0;
}