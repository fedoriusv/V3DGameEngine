#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 modelMatrix;
    mat4 normalMatrix;   

} ubo;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    vec4 position = ubo.modelMatrix *  vec4(inPosition.xyz, 1.0);
    
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * position;
    outPosition = position.xyz / position.w;
    outNormal = mat3(ubo.normalMatrix) * inNormal;
    outUV = inUV;
}