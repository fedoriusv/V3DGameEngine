#version 450

layout (location = 0) in vec3 inAttribute0_vec3;
layout (location = 1) in vec2 inAttribute1_vec2;
layout (location = 2) in vec4 inAttribute2_vec4;

layout (set = 0, binding = 1, std140) uniform UBO01_size192
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo192;

layout (set = 0, binding = 0, std140) uniform UBO00_size64
{
	mat4 projectionMatrix;
} ubo64;

layout (set = 1, binding = 1, std140) uniform UBO11_size128
{
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo128;

layout (location = 0) out vec2 outAttribute0_vec2;
layout (location = 1) out vec4 outAttribute1_vec4;

void main() 
{
	outAttribute0_vec2 = inAttribute1_vec2;
	vec4 vertex   = ubo192.modelMatrix * vec4(inAttribute0_vec3, 1.0);
	gl_Position = ubo64.projectionMatrix * ubo128.viewMatrix * vertex;
}
