#version 450

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} ubo;

void main() 
{
    vec4 position = ubo.modelMatrix *  vec4(gl_VertexIndex * 1.0, 0.0, 0.0, 1.0);
    mat4 view = ubo.viewMatrix;
    
    gl_Position = ubo.projectionMatrix * view * position;
}