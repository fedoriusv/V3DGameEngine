#version 450

layout (location = 0) in vec3 inAttribute0_vec3;
layout (location = 1) in vec4 inAttribute1_vec4;
layout (location = 2) in vec2 inAttribute2_vec2;

layout (set = 0, binding = 1, std140) uniform UBO01_size192
{
    mat4 matrix0_01;
    mat4 matrix1_01;
    mat4 matrix2_01;
} ubo01_size192;

layout (set = 0, binding = 0, std140) uniform UBO00_size64
{
    mat4 matrix0_00;
} ubo00_size64;

layout (set = 1, binding = 1, std140) uniform UBO11_size256
{
    mat4 matrix0_11[2];
    mat4 matrix1_11[2];
} ubo11_size256;

layout (location = 0) out vec4 outAttribute0_vec4;
layout (location = 1) out vec2 outAttribute1_vec2;
layout (location = 2) out vec3 outAttribute2_vec3;

void main()
{
    vec4 vertex = ubo01_size192.matrix1_01 * vec4(inAttribute0_vec3, 1.0);
    outAttribute0_vec4 = vertex;
    outAttribute1_vec2 = inAttribute2_vec2;
    outAttribute2_vec3 = mat3(ubo01_size192.matrix0_01) * inAttribute1_vec4.xyz;   
    
    gl_Position = ubo00_size64.matrix0_00 * ubo11_size256.matrix1_11[1] * vertex;
}
