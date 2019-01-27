#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (set = 0, binding = 0, std140) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (location = 0) out vec3 outColor;

void main() 
{
	outColor = inColor;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
