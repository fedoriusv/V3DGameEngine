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
    mat4 lightSpaceMatrix;
} vs_buffer;

layout (location = 0) out vec4 outLightSpace;
layout (location = 1) out vec3 outPosition;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec2 outUV;

const mat4 biasMatrix = mat4
    ( 
        0.5, 0.0, 0.0, 0.0,
        0.0,-0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0
    );
    
void main() 
{
    vec4 position = vs_buffer.modelMatrix *  vec4(inPosition.xyz, 1.0);
 
    outPosition = position.xyz; 
    outNormal = (vs_buffer.normalMatrix * vec4(inNormal, 0.0)).xyz;
    outLightSpace = biasMatrix * vs_buffer.lightSpaceMatrix * position;
    outUV = inUV;
    
    gl_Position = vs_buffer.projectionMatrix * vs_buffer.viewMatrix * position;
}