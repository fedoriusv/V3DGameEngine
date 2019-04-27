#version 450

layout (location = 0) in vec2 inUV;

//layout (set = 0, binding = 1) uniform sampler2DMS samplerColor;
layout (set = 0, binding = 1) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outFragColor;

#define NUM_SAMPLES 8

vec4 resolve(sampler2DMS tex, ivec2 uv)
{
	vec4 result = vec4(0.0);
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		vec4 val = texelFetch(tex, uv, i);
		result += val;
	}
	// Average resolved samples
	return result / float(NUM_SAMPLES);
}

void main() 
{
	outFragColor = texture(samplerColor, inUV).rrrr;

//	ivec2 attDim = textureSize(samplerColor);
//	ivec2 UV = ivec2(inUV * attDim);
//	outFragColor = resolve(samplerColor, UV).rrrr;
}