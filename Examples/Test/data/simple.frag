#version 450

layout (location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D samplerColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	//outFragColor = vec4(inUV.xy, 0.0, 1.0);
	//outFragColor = texture(samplerColor, inUV);

	outFragColor = vec4(1.0);
}