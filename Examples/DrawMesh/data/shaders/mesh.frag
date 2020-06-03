#version 450

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inView;
layout (location = 3) in vec3 inLight;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 color = texture(sampler2D(textureColor, samplerColor), inUV);

	//vec3 N = normalize(inNormal);
	//vec3 L = normalize(inLight);
	//vec3 V = normalize(inView);
	//vec3 R = reflect(-L, N);
	//vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	//vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
    
	//outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
    outFragColor = color;
}