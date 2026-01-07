#ifndef _GBUFFER_COMMON_HLSL_
#define _GBUFFER_COMMON_HLSL_

#include "global.hlsli"
#include "viewport.hlsli"
#include "lighting_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct VS_GBUFFER_STANDARD_INPUT
{
    [[vk::location(0)]] float3 Position    : IN_POSITION;
    [[vk::location(1)]] float3 Normal      : IN_NORMAL;
    [[vk::location(2)]] float3 Tangent     : IN_TANGENT;
    [[vk::location(3)]] float3 Bitangent   : IN_BITANGENT;
    [[vk::location(4)]] float2 UV          : IN_TEXTURE;
};

struct VS_GBUFFER_STANDARD_OUTPUT
{
    float4                     Position    : SV_POSITION;
    [[vk::location(0)]] float4 ClipPos     : CLIPPOS;
    [[vk::location(1)]] float4 PrevClipPos : PREVPOSITION;
    [[vk::location(2)]] float3 WorldPos    : WORLDPOS;
    [[vk::location(3)]] float3 Normal      : NORMAL;
    [[vk::location(4)]] float3 Tangent     : TANGENT;
    [[vk::location(5)]] float3 Bitangent   : BITANGENT;
    [[vk::location(6)]] float2 UV          : TEXTURE;
};

typedef VS_GBUFFER_STANDARD_OUTPUT PS_GBUFFER_STANDARD_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

struct PS_GBUFFER_STRUCT
{
    [[vk::location(0)]] float4 BaseColor   : SV_TARGET0; // RGB = BaseColor, A = Opacity
    [[vk::location(1)]] float4 Normal      : SV_TARGET1; // RGB = Normal (world), A = unused
    [[vk::location(2)]] float4 Material    : SV_TARGET2; // R = Roughness, G = Metalness, B = ObjectID, A = unused
    [[vk::location(3)]] float2 Velocity    : SV_TARGET3; // RG = Velocity
};

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT _gbuffer_standard_vs(
    in VS_GBUFFER_STANDARD_INPUT Input, 
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model)
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
    float2 prevPos = (prevPosition.xy / prevPosition.w) * 0.5 + 0.5;
    return (pos - prevPos).xy;
}

PS_GBUFFER_STRUCT _gbuffer_standard_ps(
    in PS_GBUFFER_STANDARD_INPUT Input,
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model,
    in float3 Albedo,
    in float3 Normal,
    in float Roughness,
    in float Metalness)
{
    PS_GBUFFER_STRUCT Output;
    
    float2 velocity = calc_velocity(Input.ClipPos, Input.PrevClipPos);
    
    float3 N = normalize(Input.Normal);
    float3 B = normalize(Input.Bitangent);
    float3 T = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    float3 normal = normalize(mul(Normal, TBN));

    Output.BaseColor = float4(Albedo * Model.tintColour.rgb, 1.0);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(Roughness, Metalness, (float)Model.objectID, 0.0);
    Output.Velocity = velocity;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT _gbuffer_standard_alpha_ps(
    in PS_GBUFFER_STANDARD_INPUT Input,
    in uniform ConstantBuffer<Viewport> Viewport,
    in uniform ConstantBuffer<ModelBuffer> Model,
    in float3 Albedo,
    in float Opacity,
    in float3 Normal,
    in float Roughness,
    in float Metalness)
{
    PS_GBUFFER_STRUCT Output;

    float2 velocity = calc_velocity(Input.ClipPos, Input.PrevClipPos);
    
    float3 N = normalize(Input.Normal);
    float3 B = normalize(Input.Bitangent);
    float3 T = normalize(Input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    float3 normal = normalize(mul(TBN, Normal));

    Output.BaseColor = float4(Albedo * Model.tintColour.rgb, Opacity * Model.tintColour.a);
    Output.Normal = float4(normal * 0.5 + 0.5, 0.0);
    Output.Material = float4(Roughness, Metalness, (float)Model.objectID, 0.0);
    Output.Velocity = velocity;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

#endif //_GBUFFER_COMMON_HLSL_