#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureDiffuse;
layout (binding = 3) uniform texture2D textureSpecular;

layout (binding = 4) uniform UBO 
{
    vec4 viewPosition;
    vec4 lightPosition;
    vec4 lightColor;
} ubo;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 colorDiffuse = texture(sampler2D(textureDiffuse, samplerColor), inUV);
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(ubo.lightPosition.xyz - inPos);
    float diffuseIntensity = max(dot(normal, lightDir), 0.01);
    vec4 diffuse = colorDiffuse * diffuseIntensity;
    
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;
    float distance = length(ubo.lightPosition.xyz - inPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance)); 
    
    vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
    if (diffuseIntensity > 0.1)
    {
        float specPower = 8.0;
        vec4 specularColor = texture(sampler2D(textureSpecular, samplerColor), inUV);
        vec3 viewDir = normalize(ubo.viewPosition.xyz - inPos);
        vec3 r = reflect(-lightDir, normal);
        specular = specularColor * pow(max(dot(r, viewDir), 0.1), specPower);
    }
    outFragColor = (diffuse + specular) * attenuation * ubo.lightColor;
}