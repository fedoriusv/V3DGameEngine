#ifndef _GBUFFER_COMMON_HLSL_
#define _GBUFFER_COMMON_HLSL_

#include "global.hlsli"
#include "viewport.hlsli"

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
    float4                     Position    : SV_POSITION;
    [[vk::location(0)]] float4 ClipPos     : POSITION;
    [[vk::location(1)]] float4 PrevClipPos : PREVPOSITION;
    [[vk::location(2)]] float3 WorldPos    : POSITION;
    [[vk::location(3)]] float3 Normal      : NORMAL;
    [[vk::location(4)]] float3 Tangent     : TANGENT;
    [[vk::location(5)]] float3 Bitangent   : BITANGENT;
    [[vk::location(6)]] float2 UV          : TEXTURE;
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
    in VS_GBUFFER_STANDARD_INPUT Input, 
    in ConstantBuffer<Viewport> Viewport,
    in ConstantBuffer<ModelBuffer> Model)
{
    VS_GBUFFER_STANDARD_OUTPUT Output;
   
    float4 position = mul(Model.modelMatrix, float4(Input.Position, 1.0));
    float4 prevPosition = mul(Model.prevModelMatrix, float4(Input.Position, 1.0));
    
    Output.ClipPos = mul(Viewport.projectionMatrix, mul(Viewport.viewMatrix, position));
    Output.PrevClipPos = mul(Viewport.prevProjectionMatrix, mul(Viewport.prevViewMatrix, prevPosition));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = normalize(mul((float3x3) Model.normalMatrix, Input.Normal));
    Output.Tangent = normalize(mul((float3x3) Model.normalMatrix, Input.Tangent));
    Output.Bitangent = normalize(mul((float3x3) Model.normalMatrix, Input.Bitangent));
    Output.UV = Input.UV;
    
    float2 jitterOffset = Viewport.cameraJitter * Output.ClipPos.w;
    Output.Position = Output.ClipPos + float4(jitterOffset, 0.0, 0.0);

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

float2 calc_velocity(float4 position, float4 prevPosition)
{
    float2 pos = (prevPosition.xy / prevPosition.w) * 0.5 + 0.5;
    //pos.y = 1.0 - pos.y;
    
    float2 prevPos = (prevPosition.xy / prevPosition.w) * 0.5 + 0.5;
    //prevPos.y = 1.0 - prevPos.y;
    
    return (pos - prevPos).xy;
}

PS_GBUFFER_STRUCT _gbuffer_standard_ps(
    in PS_GBUFFER_STANDARD_INPUT Input, 
    in ConstantBuffer<Viewport> Viewport,
    in ConstantBuffer<ModelBuffer> Model,
    in Texture2D textureAlbedo,
    in Texture2D textureNormal,
    in Texture2D textureMaterial,
    in SamplerState samplerState)
{
    PS_GBUFFER_STRUCT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    float2 velocity = calc_velocity(Input.ClipPos, Input.PrevClipPos);
    
    float3 N = normalize(Input.Normal);
    float3 B = normalize(Input.Tangent);
    float3 T = normalize(Input.Bitangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));

    Output.BaseColor = float4(albedo * Model.tint.rgb, 1.0);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, Model.objectID, 0.0);
    Output.Velocity = velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT _gbuffer_standard_alpha_ps(
    in PS_GBUFFER_STANDARD_INPUT Input,
    in ConstantBuffer<Viewport> Viewport,
    in ConstantBuffer<ModelBuffer> Model,
    in Texture2D textureAlbedo,
    in Texture2D textureNormal,
    in Texture2D textureMaterial,
    in SamplerState samplerState)
{
    PS_GBUFFER_STRUCT Output;
    
    float3 albedo = textureAlbedo.Sample(samplerState, Input.UV).rgb;
    float3 normal = textureNormal.Sample(samplerState, Input.UV).rgb * 2.0 - 1.0;
    float metalness = textureMaterial.Sample(samplerState, Input.UV).r;
    float roughness = textureMaterial.Sample(samplerState, Input.UV).g;
    float2 velocity = calc_velocity(Input.ClipPos, Input.PrevClipPos);
    
    float3 N = normalize(Input.Normal);
    float3 B = normalize(Input.Tangent);
    float3 T = normalize(Input.Bitangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));

    Output.BaseColor = float4(albedo * Model.tint.rgb, Model.tint.a);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(roughness, metalness, Model.objectID, 0.0);
    Output.Velocity = velocity;
    
    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GBUFFER_COMMON_HLSL_