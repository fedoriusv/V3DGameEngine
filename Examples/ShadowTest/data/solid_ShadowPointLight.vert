#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewMatrix;
    vec4 lightPosVector;
    vec4 viewPosVector;
} ubo;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outLightPos;
layout (location = 3) out vec3 outViewDir;
layout (location = 4) out vec2 outUV;

void main() 
{
    vec4 position = ubo.modelMatrix *  vec4(inPosition.xyz, 1.0);
 
    outPosition = position.xyz; 
    outNormal = (ubo.normalMatrix * vec4(inNormal, 0.0)).xyz;
    outLightPos = ubo.lightPosVector.xyz;
    outViewDir = ubo.viewPosVector.xyz - position.xyz;
    outUV = inUV;
    
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * position;
}