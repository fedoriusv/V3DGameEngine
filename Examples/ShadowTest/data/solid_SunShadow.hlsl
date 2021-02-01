struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXTURE;
};

struct VS_OUTPUT
{
    float4 Pos          : SV_POSITION;
    
    float4 LightSpace   : LIGHTSPACE;  
    float3 Position     : POSITION;    
    float3 Normal       : NORMAL;
    float2 UV           : TEXTURE;
};

struct VS_Buffer 
{
    matrix projectionMatrix;
    matrix modelMatrix;
    matrix normalMatrix;
    matrix viewMatrix;
    matrix lightSpaceMatrix;
};
ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

VS_OUTPUT main_VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    const matrix biasMatrix = matrix
        (
            0.5, 0.0, 0.0, 0.5,
            0.0,-0.5, 0.0, 0.5,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    
    output.Position = position.xyz;
    output.LightSpace = mul(biasMatrix, mul(vs_buffer.lightSpaceMatrix, position));
    output.Normal = mul(vs_buffer.normalMatrix, float4(input.Normal, 1.0)).xyz;
    output.UV = input.UV;
    
    output.Pos = mul(vs_buffer.projectionMatrix, mul(vs_buffer.viewMatrix, position));
    
    return output;
}


SamplerComparisonState shadowSampler  : register(s1, space0);
Texture2D shadowMap                   : register(t2, space0);

struct FS_Buffer
{
    float4 lightDirection;
    float4 viewPosition;
    bool   enablePCF;
};
ConstantBuffer<FS_Buffer> fs_buffer : register(b3, space0);

float depthTextureProj(float4 shadowCoord, float2 offset)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
    {
        float dist = shadowMap.SampleCmpLevelZero(shadowSampler, shadowCoord.xy + offset, shadowCoord.z);
        if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
        {
            shadow = 0.2;
        }
    }
    
    return shadow;
}

float percentageCloserFiltering(float4 shadowCoord)
{
    float width = 0, height = 0;
    shadowMap.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);
    
    float shadow = 0.0;
    int count = 0;
    int range = 1;
    for(int x = -range; x <= range; ++x)
    {
        for(int y = -range; y <= range; ++y)
        {
            shadow += depthTextureProj(shadowCoord, float2(x, y) * texelSize);
            ++count;
        }    
    }
    
    return shadow / count;
}

float shadowMask(float4 lightSpace)
{
    float shadow = 1.0;
    float3 shadowCoord = lightSpace.xyz / lightSpace.w;
    if (fs_buffer.enablePCF)
    {
        shadow = percentageCloserFiltering(float4(shadowCoord, lightSpace.w));
    }
    else
    {
        shadow = depthTextureProj(float4(shadowCoord, lightSpace.w), float2(0.0, 0.0));
    }
    
    return shadow;
}

float4 main_FS(VS_OUTPUT input) : SV_TARGET0
{
    float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(fs_buffer.lightDirection.xyz);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    float4 outFragColor = diffuseKoeff * diffuseColor;

    float shadow = shadowMask(input.LightSpace);
    if (shadow == 1.0 && diffuseKoeff > 0.01)
    {
        float4 specularColor = float4(1.0, 1.0, 1.0, 1.0);
        float3 reflectDir = reflect(-lightDir, normal);
        float3 viewDir = normalize(fs_buffer.viewPosition.xyz - input.Position);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 512.0);
    }
    
    outFragColor = float4((outFragColor.xyz * shadow), 1.0);
    return outFragColor;
}