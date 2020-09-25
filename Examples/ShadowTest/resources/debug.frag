#version 450

layout (binding = 1) uniform UBO 
{
    vec4 color;
} debugColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor = debugColor.color;
}