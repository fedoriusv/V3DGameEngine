#version 450

layout (location = 0) in vec4 inLightSpace;
layout (location = 1) in vec3 inPosition;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

//layout (binding = 1) uniform sampler colorSampler;
//layout (binding = 2) uniform texture2D colorTexture;

layout (binding = 3) uniform sampler shadowSampler;
layout (binding = 4) uniform texture2D shadowMap;

layout (binding = 5, std140) uniform UBO
{
    vec4 lightPosition;
    vec4 viewPosition;
} ubo;

layout (location = 0) out vec4 outFragColor;

float shadowCalculation(vec4 lightSpace)
{
    vec3 shadowCoord = lightSpace.xyz / lightSpace.w;
    
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
    {
        float dist = texture(sampler2D(shadowMap, shadowSampler), shadowCoord.xy).r;
        if ( lightSpace.w > 0.0 && dist < shadowCoord.z ) 
        {
            shadow = 0.2;
        }
    }
    
    return shadow;
}

void main() 
{
    vec4 diffuseColor = vec4(1.0); //texture(sampler2D(colorTexture, colorSampler), inUV);
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(ubo.lightPosition.xyz - inPosition);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    outFragColor = diffuseKoeff * diffuseColor;

    if (diffuseKoeff > 0.01)
    {
        vec4 specularColor = vec4(1.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 viewDir = normalize(ubo.viewPosition.xyz - inPosition);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 8.0);
    }
    
    float shadow = shadowCalculation(inLightSpace); 
    outFragColor = shadow * outFragColor;
}