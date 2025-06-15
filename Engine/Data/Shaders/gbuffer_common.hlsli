#ifndef _GBUFFER_COMMON_HLSL_
#define _GBUFFER_COMMON_HLSL_

#include "global.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct VS_GBUFFER_STANDARD_INPUT
{
    [[vk::location(0)]] float3 Position     : IN_POSITION;
    [[vk::location(1)]] float3 Normal       : IN_NORMAL;
    [[vk::location(2)]] float3 Tangent      : IN_TANGENT;
    [[vk::location(3)]] float3 Bitangent    : IN_BITANGENT;
    [[vk::location(4)]] float2 UV           : IN_TEXTURE;
};

///////////////////////////////////////////////////////////////////////////////////////

struct VS_GBUFFER_STANDARD_OUTPUT
{
    float4                     Position     : SV_POSITION;
    [[vk::location(0)]] float4 PrevPosition : PREVPOSITION;
    [[vk::location(1)]] float3 WorldPos     : POSITION;
    [[vk::location(2)]] float3 Normal       : NORMAL;
    [[vk::location(3)]] float3 Tangent      : TANGENT;
    [[vk::location(4)]] float3 Bitangent    : BITANGENT;
    [[vk::location(5)]] float2 UV           : TEXTURE;
};

typedef VS_GBUFFER_STANDARD_OUTPUT PS_GBUFFER_STANDARD_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

struct PS_GBUFFER_STRUCT
{
    [[vk::location(0)]] float4 BaseColor : SV_TARGET0; // RGB = BaseColor, A = unused
    [[vk::location(1)]] float4 Normal    : SV_TARGET1; // RGB = Normal (world), A = unused
    [[vk::location(2)]] float4 Material  : SV_TARGET2; // R = Roughness, G = Metalness, B = objectID, A = unused
    [[vk::location(3)]] float2 Velocity  : SV_TARGET3; // RG = Velocity
};

///////////////////////////////////////////////////////////////////////////////////////

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4   tint;
    uint64_t objectID;
};

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT _gbuffer_standard_vs(
    in VS_GBUFFER_STANDARD_INPUT input, 
    in ConstantBuffer<Viewport> viewport,
    in ConstantBuffer<ModelBuffer> model)
{
    VS_GBUFFER_STANDARD_OUTPUT Output;
   
    float4 position = mul(model.modelMatrix, float4(input.Position, 1.0));
    float4 prevPosition = mul(model.prevModelMatrix, float4(input.Position, 1.0));
    
    Output.Position = mul(viewport.projectionMatrix, mul(viewport.viewMatrix, position));
    Output.PrevPosition = mul(viewport.prevProjectionMatrix, mul(viewport.prevViewMatrix, prevPosition));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = mul((float3x3) model.normalMatrix, input.Normal);
    Output.Tangent = mul((float3x3) model.normalMatrix, input.Tangent);
    Output.Bitangent = mul((float3x3) model.normalMatrix, input.Bitangent);
    Output.UV = input.UV;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT _gbuffer_standard_ps(
    in PS_GBUFFER_STANDARD_INPUT input, 
    in ConstantBuffer<Viewport> viewport,
    in ConstantBuffer<ModelBuffer> model,
    in Texture2D textureAlbedo,
    in Texture2D textureNormal,
    in Texture2D textureMaterial,
    in SamplerState samplerState)
{
    PS_GBUFFER_STRUCT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, input.UV).g;
    float2 velocity = float2(input.Position.xy / input.Position.w - input.PrevPosition.xy / input.PrevPosition.w);
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Bitangent);
    float3 B = normalize(input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));

    Output.BaseColor = float4(albedo * model.tint.rgb, 1.0);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, model.objectID, 0.0);
    Output.Velocity = velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT _gbuffer_standard_alpha_ps(
    in PS_GBUFFER_STANDARD_INPUT input,
    in ConstantBuffer<Viewport> viewport,
    in ConstantBuffer<ModelBuffer> model,
    in Texture2D textureAlbedo,
    in Texture2D textureNormal,
    in Texture2D textureMaterial,
    in SamplerState samplerState)
{
    PS_GBUFFER_STRUCT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, input.UV).g;
    float2 velocity = float2(input.Position.xy / input.Position.w - input.PrevPosition.xy / input.PrevPosition.w);
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Bitangent);
    float3 B = normalize(input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));

    Output.BaseColor = float4(albedo * model.tint.rgb, model.tint.a);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, model.objectID, 0.0);
    Output.Velocity = velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GBUFFER_COMMON_HLSL_