#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNorm;
layout (location = 2) in vec2 inUV;

layout (set = 0, binding = 0, std140) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec3 viewPos;
} ubo;

layout (location = 0) out vec2 outUV;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	outUV = inUV;

	mat4 skypos = ubo.model;
	skypos[3].xyz = ubo.viewPos;

	vec4 vertex = skypos * vec4(inPos, 1.0);
	gl_Position = ubo.projection * ubo.view * vertex;
}
