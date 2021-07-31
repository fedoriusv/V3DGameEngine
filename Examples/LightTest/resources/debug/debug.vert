#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} ps_ubo;

void main() 
{
    vec4 position = ps_ubo.modelMatrix *  vec4(inPosition.xyz, 1.0);
    gl_Position = ps_ubo.projectionMatrix * ps_ubo.viewMatrix * position;
}