#include "global.hlsl"

struct VS_INPUT
{
    [[vk::location(0)]] float3 Position  : IN_POSITION;
    [[vk::location(1)]] float3 Normal    : IN_NORMAL;
    [[vk::location(2)]] float3 Tangent   : IN_TANGENT;
    [[vk::location(3)]] float3 Bitangent : IN_BITANGENT;
    [[vk::location(4)]] float2 UV        : IN_TEXTURE;
};

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    float4   tint;
    uint64_t objectID;
};
[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b0, space1);

struct VS_OUTPUT
{
    float4                     Position  : SV_POSITION;
    [[vk::location(0)]] float3 WorldPos  : POSITION;
    [[vk::location(1)]] float3 Normal    : NORMAL;
    [[vk::location(2)]] float3 Tangent   : TANGENT;
    [[vk::location(3)]] float3 Bitangent : BITANGENT;
    [[vk::location(4)]] float2 UV        : TEXTURE;
};

VS_OUTPUT gbuffer_standard_vs(VS_INPUT Input)
{
    VS_OUTPUT Output;
   
    float4 position = mul(CB_Model.modelMatrix, float4(Input.Position, 1.0));
    
    Output.Position = mul(viewport.projectionMatrix, mul(viewport.viewMatrix, position));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = mul((float3x3) CB_Model.normalMatrix, Input.Normal);
    Output.Tangent = mul((float3x3) CB_Model.normalMatrix, Input.Tangent);
    Output.Bitangent = mul((float3x3) CB_Model.normalMatrix, Input.Bitangent);
    Output.UV = Input.UV;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo   : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal   : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial : register(t2, space1);

struct PS_GBUFFER_OUTPUT
{
    [[vk::location(0)]] float4 Albedo   : SV_TARGET0; // RGB = Albedo, A = unused
    [[vk::location(1)]] float4 Normal   : SV_TARGET1; // RGB = Normal (world), A = unused
    [[vk::location(2)]] float4 Material : SV_TARGET2; // RGB = specular/metalness/etc., A = roughness or smoothness
};
typedef VS_OUTPUT PS_INPUT;

PS_GBUFFER_OUTPUT gbuffer_standard_ps(PS_INPUT Input)
{
    PS_GBUFFER_OUTPUT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    
    float3 N = normalize(Input.Normal);
    float3 T = normalize(Input.Bitangent);
    float3 B = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));
    
    Output.Albedo = float4(albedo * CB_Model.tint.rgb, 1.0);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, 0.0, 0.0);

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

void gbuffer_depth_ps(PS_INPUT Input)
{
    //nothing
}