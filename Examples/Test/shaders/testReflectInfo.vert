#version 450

layout (location = 0) in vec3 inAttribute0_vec3;
layout (location = 1) in vec4 inAttribute1_vec4;
layout (location = 2) in vec2 inAttribute2_vec2;

layout (set = 0, binding = 1, std140) uniform UBO01_size192
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} ubo01_size192;

layout (set = 0, binding = 0, std140) uniform UBO00_size64
{
    mat4 projectionMatrix;
} ubo00_size64;

layout (set = 1, binding = 1, std140) uniform UBO11_size128
{
    mat4 modelMatrix[2];
    mat4 viewMatrix[2];
} ubo11_size128;

layout (location = 0) out vec4 outAttribute0_vec4;
layout (location = 1) out vec2 outAttribute1_vec2;

void main()
{
    outAttribute1_vec2 = inAttribute2_vec2;
    vec4 vertex = ubo01_size192.modelMatrix * vec4(inAttribute0_vec3, 1.0);
    outAttribute0_vec4 = ubo00_size64.projectionMatrix * ubo11_size128.viewMatrix[1] * vertex;

    gl_Position = outAttribute0_vec4;
}
