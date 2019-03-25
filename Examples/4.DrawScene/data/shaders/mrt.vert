#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inUV;

layout (set = 0, binding = 0, std140) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
} ubo;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBitangent;
layout (location = 4) out vec2 outUV;

void main() 
{
	vec4 pos = ubo.view * ubo.model * vec4(inPos, 1.0);
	outWorldPos = pos.xyz;

	mat3 normalMatrix = transpose(inverse(mat3(ubo.view * ubo.model)));
	outNormal = normalMatrix * inNormal;

	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outTangent = mNormal * normalize(inTangent);

	outUV = inUV;
	outUV.t = 1.0 - outUV.t;

	outBitangent = inBitangent;

	gl_Position = ubo.projection * pos;
}
