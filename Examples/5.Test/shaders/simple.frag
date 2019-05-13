#version 450

layout (location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D samplerColor;
layout (set = 0, binding = 1) uniform UBO
{
    vec4 value;
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	//outFragColor = vec4(inUV.xy, 0.0, 1.0);
	//outFragColor = texture(samplerColor, inUV);
    
    //vec4 result1 = (ubo.value / 2.0) + 1.0;
    //vec4 result2 = (ubo.value / 2.0) - 1.0;
    //vec4 result3 = (ubo.value / -2.0) + 1.0;

    vec4 result11 = (ubo.value * 0.5) + 1.0;
    vec4 result22 = (ubo.value * 0.5) - 1.0;
    vec4 result33 = (ubo.value * -0.5) + 1.0;

	outFragColor = vec4(result11.x, result22.y, result33.z, 1.0);
}