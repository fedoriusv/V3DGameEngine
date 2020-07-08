#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} ubo;

layout (location = 0) out vec2 outUV;

void main() 
{
    outUV = inUV;
    
    vec4 position = ubo.modelMatrix *  vec4(inPosition.xyz, 1.0);
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * position;
}