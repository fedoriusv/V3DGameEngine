#include "global.hlsli"
#include "gbuffer_common.hlsli"
#include "lighting.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo   : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal   : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial : register(t2, space1);

typedef VS_GBUFFER_STANDARD_OUTPUT PS_GBUFFER_STANDARD_INPUT;

struct PS_GBUFFER_STRUCT
{
    [[vk::location(0)]] float4 Albedo   : SV_TARGET0; // RGB = Albedo, A = unused
    [[vk::location(1)]] float4 Normal   : SV_TARGET1; // RGB = Normal (world), A = unused
    [[vk::location(2)]] float4 Material : SV_TARGET2; // R = Roughness, G = Metalness, B = objectID, A = unused
    [[vk::location(3)]] float2 Velocity : SV_TARGET3; // RG = Velocity
};

PS_GBUFFER_STRUCT gbuffer_standard_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    PS_GBUFFER_STRUCT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    float2 velocity = float2(Input.Position.xy / Input.Position.w - Input.PrevPosition.xy / Input.PrevPosition.w);
    
    float3 N = normalize(Input.Normal);
    float3 T = normalize(Input.Bitangent);
    float3 B = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));

    Output.Albedo = float4(albedo * CB_Model.tint.rgb, 1.0);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, CB_Model.objectID, 0.0);
    Output.Velocity = velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

void gbuffer_depth_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    //nothing
}