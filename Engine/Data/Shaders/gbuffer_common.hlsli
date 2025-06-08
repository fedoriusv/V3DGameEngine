#ifndef _GBUFFER_COMMON_HLSL_
#define _GBUFFER_COMMON_HLSL_

#include "global.hlsli"

struct VS_GBUFFER_STANDARD_INPUT
{
    [[vk::location(0)]] float3 Position     : IN_POSITION;
    [[vk::location(1)]] float3 Normal       : IN_NORMAL;
    [[vk::location(2)]] float3 Tangent      : IN_TANGENT;
    [[vk::location(3)]] float3 Bitangent    : IN_BITANGENT;
    [[vk::location(4)]] float2 UV           : IN_TEXTURE;
};

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

struct ModelBuffer
{
    float4x4 modelMatrix;
    float4x4 prevModelMatrix;
    float4x4 normalMatrix;
    float4   tint;
    uint64_t objectID;
};
[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b0, space1);

VS_GBUFFER_STANDARD_OUTPUT gbuffer_standard_vs(VS_GBUFFER_STANDARD_INPUT Input)
{
    VS_GBUFFER_STANDARD_OUTPUT Output;
   
    float4 position = mul(CB_Model.modelMatrix, float4(Input.Position, 1.0));
    float4 prevPosition = mul(CB_Model.prevModelMatrix, float4(Input.Position, 1.0));
    
    Output.Position = mul(viewport.projectionMatrix, mul(viewport.viewMatrix, position));
    Output.PrevPosition = mul(viewport.prevProjectionMatrix, mul(viewport.prevViewMatrix, prevPosition));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = mul((float3x3) CB_Model.normalMatrix, Input.Normal);
    Output.Tangent = mul((float3x3) CB_Model.normalMatrix, Input.Tangent);
    Output.Bitangent = mul((float3x3) CB_Model.normalMatrix, Input.Bitangent);
    Output.UV = Input.UV;

    return Output;
}

#endif //_GBUFFER_COMMON_HLSL_