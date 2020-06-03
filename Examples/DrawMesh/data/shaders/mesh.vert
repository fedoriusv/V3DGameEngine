#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
	vec4 lightPos;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outView;
layout (location = 3) out vec3 outLight;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
    vec4 position = ubo.modelMatrix *  vec4(inPosition.xyz, 1.0);
	vec3 lightPosition = mat3(ubo.modelMatrix) * ubo.lightPos.xyz;
    
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * position;
    outNormal = mat3(ubo.modelMatrix) * inNormal;
    outUV = inUV;
	outLight = lightPosition - position.xyz;
	outView = -position.xyz;
}