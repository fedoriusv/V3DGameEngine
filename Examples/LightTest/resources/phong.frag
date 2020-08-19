#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;

struct Light
{
    vec4 direction;
    vec4 diffuse;
    vec4 specular;
};

layout (binding = 3) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (binding = 4) uniform UBO 
{
    vec4 viewPosition;
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuseColor = texture(sampler2D(textureColor, samplerColor), inUV);
    vec3 normal = normalize(inNormal);
    
    vec3 viewDir = normalize(ubo.viewPosition.xyz - inPosition);
    vec3 r = reflect(-viewDir, normal);
    
    outFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    for (int l = 0; l < 1; ++l)
    {
        vec3 lightDir = normalize(light.lights[l].direction.xyz);
        //vec3 lightDir = normalize(light.lights[l].direction.xyz - inPosition);
        float diffuseKoeff = max(dot(normal, lightDir), 0.01);
        outFragColor += diffuseKoeff * diffuseColor * light.lights[l].diffuse;
        
        if (diffuseKoeff > 0.01)
        {
            outFragColor += pow(max(dot(r, lightDir), 0.01), 8.0) * light.lights[l].specular;
        }
    }
}