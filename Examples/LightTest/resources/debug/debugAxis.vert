#version 450

layout (binding = 0) uniform UBO 
{
    mat4 projectionMatrix;
    mat4 modelMatrix;
    mat4 viewMatrix;
} vs_ubo;

void main() 
{
    vec4 position = vs_ubo.modelMatrix *  vec4(gl_VertexIndex * 1.0, 0.0, 0.0, 1.0);
    mat4 view = vs_ubo.viewMatrix;
    
    gl_Position = vs_ubo.projectionMatrix * view * position;
}