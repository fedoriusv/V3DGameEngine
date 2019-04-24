#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec3 inTang;
layout (location = 3) in vec3 inBitang;
layout (location = 4) in vec2 inUV;

layout (set = 0, binding = 0, std140) uniform UBO
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (location = 0) out vec2 outUV;

void main() 
{
	outUV = inUV;
	vec4 vertex   = ubo.modelMatrix * vec4(inPos, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vertex;
}
