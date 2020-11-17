#version 450
#extension GL_EXT_samplerless_texture_functions : require

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
    vec4 lightDirection;
    vec4 viewPosition;
    bool enablePCF;
} fs_buffer;

layout (location = 0) out vec4 outFragColor;

float depthTextureProj(vec4 shadowCoord, vec2 offset)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
    {
        float dist = texture(sampler2D(shadowMap, shadowSampler), shadowCoord.xy + offset).r;
        if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
        {
            shadow = 0.2;
        }
    }
    
    return shadow;
}

float percentageCloserFiltering(vec4 shadowCoord)
{
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    float shadow = 0.0;
    int count = 0;
    int range = 1;
    for(int x = -range; x <= range; ++x)
    {
        for(int y = -range; y <= range; ++y)
        {
            shadow += depthTextureProj(shadowCoord, vec2(x, y) * texelSize);
            ++count;
        }    
    }
    
    return shadow / count;
}

float shadowMask(vec4 lightSpace)
{
    vec3 shadowCoord = lightSpace.xyz / lightSpace.w;
    if (fs_buffer.enablePCF)
    {
        return percentageCloserFiltering(vec4(shadowCoord, lightSpace.w));
    }

    return depthTextureProj(vec4(shadowCoord, lightSpace.w), vec2(0.0, 0.0));
}

void main() 
{
    vec4 diffuseColor = vec4(1.0); //texture(sampler2D(colorTexture, colorSampler), inUV);
    vec3 normal = normalize(inNormal);
    vec3 lightDir = normalize(fs_buffer.lightDirection.xyz);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    outFragColor = diffuseKoeff * diffuseColor;

    if (diffuseKoeff > 0.01)
    {
        vec4 specularColor = vec4(1.0);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 viewDir = normalize(fs_buffer.viewPosition.xyz - inPosition);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 512.0);
    }
    
    float shadow = shadowMask(inLightSpace); 
    outFragColor = vec4((outFragColor.xyz * shadow), 1.0);
}