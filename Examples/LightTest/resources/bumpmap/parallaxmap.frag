#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inUV;

layout (binding = 1) uniform sampler samplerColor;
layout (binding = 2) uniform texture2D textureColor;
layout (binding = 3) uniform sampler samplerFilter;
layout (binding = 4) uniform texture2D textureNormal;
layout (binding = 5) uniform texture2D textureHeight;

struct Light
{
    vec4 position;
    vec4 diffuse;
    vec4 specular;
};

layout (binding = 6) uniform LIGHT 
{
    Light lights[LIGHT_COUNT];
} light;

layout (binding = 7) uniform UBO 
{
    vec4 viewPosition;
} ubo;

layout (location = 0) out vec4 outFragColor;

vec2 parallaxMappingOcclusion(texture2D heightTexture, sampler texFilter, vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    
    // calculate the size of each layer
    float layerHeight = 1.0 / numLayers;
    // depth of current layer
    float currentLayerHeight = 0.0;
    
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * HEIGHT_SCALE; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentHeightMapValue = texture(sampler2D(heightTexture, texFilter), currentTexCoords).r;
      
    while(currentLayerHeight < currentHeightMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get heightmap value at current texture coordinates
        currentHeightMapValue = texture(sampler2D(heightTexture, texFilter), currentTexCoords).r;
        // get height of next layer
        currentLayerHeight += layerHeight;
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get height after and before collision for linear interpolation
    float afterHeight  = currentHeightMapValue - currentLayerHeight;
    float beforeHeight = texture(sampler2D(heightTexture, texFilter), prevTexCoords).r - currentLayerHeight + layerHeight;
 
    // interpolation of texture coordinates
    float weight = afterHeight / (afterHeight - beforeHeight);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec2 parallaxMapping(texture2D heightTexture, sampler texFilter, vec2 texCoords, vec3 viewDir)
{
    float height =  texture(sampler2D(heightTexture, texFilter), texCoords).r;
    vec2 P = viewDir.xy / viewDir.z * (height * HEIGHT_SCALE); 
    
    return texCoords - P;
}

void main() 
{
    outFragColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    mat3 invertedTBN = transpose(mat3(normalize(inTangent), normalize(inBitangent), normalize(inNormal)));
    vec3 tangentPosition = invertedTBN * inPosition;
    vec3 tangentViewPosition = invertedTBN * ubo.viewPosition.xyz;
    vec3 tangentViewDir = normalize(tangentViewPosition - tangentPosition);
    
    vec2 offsetCoords = inUV;
#if 1
    offsetCoords = parallaxMappingOcclusion(textureHeight, samplerColor, offsetCoords, tangentViewDir);
#else
    offsetCoords = parallaxMapping(textureHeight, samplerColor, offsetCoords, tangentViewDir);
#endif
    if(offsetCoords.x > 1.0 || offsetCoords.y > 1.0 || offsetCoords.x < 0.0 || offsetCoords.y < 0.0)
    {
        discard;
    }
    
    vec4 colorDiffuse = texture(sampler2D(textureColor, samplerColor), offsetCoords);
    vec3 normalDiffuse = texture(sampler2D(textureNormal, samplerColor), offsetCoords).xyz;
    vec3 normal = normalDiffuse * 2.0 - 1.0;
    
    for (int l = 0; l < LIGHT_COUNT; ++l)
    {
        vec3 lightPos = (invertedTBN * light.lights[l].position.xyz) - tangentPosition;
        float lightDistance = length(lightPos);
        float attenuation = 1.0 / (CONSTANT + LINEAR * lightDistance + QUADRATIC * (lightDistance * lightDistance));        
        
        vec3 lightDir = normalize(lightPos);
        float diffuseIntensive = max(dot(normal, lightDir), 0.01);
        outFragColor += diffuseIntensive * colorDiffuse * light.lights[l].diffuse * attenuation;
        
        if (diffuseIntensive > 0.01)
        {
            vec3 reflectDir = reflect(-lightDir, normal);
            outFragColor += light.lights[l].specular * pow(max(dot(tangentViewDir, reflectDir), 0.0), 8.0);
        }
    }
}