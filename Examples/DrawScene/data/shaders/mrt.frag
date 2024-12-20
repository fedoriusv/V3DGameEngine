#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform sampler2D samplerColor;
layout (set = 0, binding = 2) uniform sampler2D samplerSpecular;
layout (set = 0, binding = 3) uniform sampler2D samplerNormal;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inUV;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out uvec4 outAlbedo;

//layout (constant_id = 0) const float NEAR_PLANE = 1.0f;
//layout (constant_id = 1) const float FAR_PLANE = 512.0f;
//layout (constant_id = 2) const int ENABLE_DISCARD = 0;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main() 
{
	outPosition = vec4(inWorldPos, linearDepth(gl_FragCoord.z));

	vec4 color = texture(samplerColor, inUV);

	// Discard by alpha for transparent objects if enabled via specialization constant
	if (ENABLE_DISCARD == 0)
	{
		vec3 N = normalize(inNormal);
		vec3 B = normalize(inBitangent);
		vec3 T = normalize(inTangent);
		mat3 TBN = mat3(T, B, N);
		vec3 nm = texture(samplerNormal, inUV).xyz * 2.0 - vec3(1.0);
		nm = TBN * normalize(nm);
		outNormal = vec4(nm * 0.5 + 0.5, 0.0);
	}
	else
	{
		outNormal = vec4(normalize(inNormal) * 0.5 + 0.5, 0.0);
		if (color.a < 0.5)
		{
			discard;
		}
	}

	// Pack
	float specular = texture(samplerSpecular, inUV).r;

	outAlbedo.r = packHalf2x16(color.rg);
	outAlbedo.g = packHalf2x16(color.ba);
	outAlbedo.b = packHalf2x16(vec2(specular, 0.0));
}