#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

layout (binding = 3) uniform LIGHT 
{
    vec4 lightPosition;
} light;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 color = texture(sampler2D(textureColor, samplerColor), inUV);

    vec3 N = normalize(inNormal);
    vec3 L = normalize(light.lightPosition.xyz - inPosition);
    float diffuseKoeff = max(dot(N, L), 0.01);
    outFragColor = vec4(diffuseKoeff * color.rgb, 1.0);
  
    if (diffuseKoeff > 0.01)
    {
        vec3 V = normalize(-inPosition);
        vec3 R = reflect(-L, N);
        float specularKoeff = pow(max(dot(R, V), 0.0), 64.0);
        outFragColor.xyz += specularKoeff * vec3(0.75);
    }
}