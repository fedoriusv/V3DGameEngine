struct VS_ATTRIBUTES
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXTURE;
};

struct PS_ATTRIBUTES
{
    float4 Pos : SV_POSITION;
    
    float4 LightModelViewProj[SHADOWMAP_CASCADE_COUNT] : LIGHTSPACE;
    float4 PositionModelView : POSITION0;       
    float3 PositionModel : POSITION1;
    float3 Normal : NORMAL;
    float2 UV : TEXTURE;
};

struct VS_Buffer 
{
    matrix projectionMatrix;
    matrix modelMatrix;
    matrix normalMatrix;
    matrix viewMatrix;
    matrix lightSpaceMatrix[SHADOWMAP_CASCADE_COUNT];
};
ConstantBuffer<VS_Buffer> vs_buffer : register(b0, space0);

PS_ATTRIBUTES main_VS(VS_ATTRIBUTES input)
{
    PS_ATTRIBUTES output;
    
    const matrix biasMatrix = matrix
        (
            0.5, 0.0, 0.0, 0.5,
            0.0,-0.5, 0.0, 0.5,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
        
    float4 position = mul(vs_buffer.modelMatrix, float4(input.Position, 1.0));
    float4 viewPosition = mul(vs_buffer.viewMatrix, position);
    
    output.PositionModel = position.xyz;
    output.PositionModelView = viewPosition;
    output.Normal = mul(vs_buffer.normalMatrix, float4(input.Normal, 1.0)).xyz;
    output.UV = input.UV;
    
    for (uint i = 0; i < SHADOWMAP_CASCADE_COUNT; ++i )
    {
        output.LightModelViewProj[i] = mul(biasMatrix, mul(vs_buffer.lightSpaceMatrix[i], position));
    }

    output.Pos = mul(vs_buffer.projectionMatrix, viewPosition);
    return output;
}

SamplerComparisonState shadowSampler : register(s1, space0);
Texture2DArray cascadedShadowMap : register(t2, space0);

struct FS_Buffer
{
    float4 lightDirection;
    float4 viewPosition;
    float4 cascadeSplits;
};
ConstantBuffer<FS_Buffer> fs_buffer : register(b3, space0);

float depthTextureProj(float4 shadowCoord, float2 offset, uint cascadeIndex)
{
    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) 
    {
        float dist = cascadedShadowMap.SampleCmpLevelZero(shadowSampler, float3(shadowCoord.xy, cascadeIndex), shadowCoord.z, offset);
        if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
        {
            shadow = 0.2;
        }
    }
    
    return shadow;
}

float shadowMask(float4 lightSpace, uint cascadeIndex)
{
    float3 shadowCoord = lightSpace.xyz / lightSpace.w;
    return depthTextureProj(float4(shadowCoord, lightSpace.w), float2(0.0, 0.0), cascadeIndex);
}

float4 main_FS(PS_ATTRIBUTES input) : SV_TARGET0
{
    float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(fs_buffer.lightDirection.xyz);
    float diffuseKoeff = max(dot(normal, lightDir), 0.01);
    float4 outFragColor = diffuseKoeff * diffuseColor;

    uint cascadeIndex = 0;
    for (uint i = 0; i < SHADOWMAP_CASCADE_COUNT; ++i)
    {
        if (input.PositionModelView.z <= fs_buffer.cascadeSplits[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float shadow = shadowMask(input.LightModelViewProj[cascadeIndex], cascadeIndex);
    
    if (shadow == 1.0 && diffuseKoeff > 0.01)
    {
        float4 specularColor = float4(1.0, 1.0, 1.0, 1.0);
        float3 reflectDir = reflect(-lightDir, normal);
        float3 viewDir = normalize(fs_buffer.viewPosition.xyz - input.PositionModel);
        outFragColor += specularColor * pow(max(dot(reflectDir, viewDir), 0.0), 512.0);
    }
    
    outFragColor = float4((outFragColor.xyz * shadow), 1.0);
#if SHADER_DEBUG
    switch(cascadeIndex) 
    {
    case 0 :
        outFragColor.rgb *= float3(0.25f, 0.25f, 1.0f);      
        break;
    case 1 :
        outFragColor.rgb *= float3(0.25f, 1.0f, 0.25f);
        break;
    case 2 :
        outFragColor.rgb *= float3(1.0f, 1.0f, 0.25f);
        break;
    case 3 :
        outFragColor.rgb *= float3(1.0f, 0.25f, 0.25f);
        break;
    }
#endif
    return outFragColor;
}