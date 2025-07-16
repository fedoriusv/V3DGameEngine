#include "global.hlsli"
#include "viewport.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct VS_SIMPLE_INPUT
{
    [[vk::location(0)]] float3 Position : IN_POSITION;
    [[vk::location(1)]] float3 Normal   : IN_NORMAL;
    [[vk::location(2)]] float2 UV       : IN_TEXTURE;
};

struct VS_SIMPLE_OUTPUT
{
    float4                     Position : SV_POSITION;
    [[vk::location(0)]] float4 ClipPos  : CLIPPOS;
    [[vk::location(2)]] float3 WorldPos : WORLDPOS;
    [[vk::location(3)]] float3 Normal   : NORMAL;
    [[vk::location(6)]] float2 UV       : TEXTURE;
};

typedef VS_SIMPLE_OUTPUT PS_SIMPLE_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);
[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT simple_vs(VS_SIMPLE_INPUT Input)
{
    VS_SIMPLE_OUTPUT Output;
   
    float4 position = mul(CB_Model.modelMatrix, float4(Input.Position, 1.0));
    
    Output.ClipPos = mul(CB_Viewport.projectionMatrix, mul(CB_Viewport.viewMatrix, position));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = normalize(mul((float3x3) CB_Model.normalMatrix, Input.Normal));
    Output.UV = Input.UV;

    Output.Position = Output.ClipPos;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

float4 simple_unlit_ps(PS_SIMPLE_INPUT Input) : SV_TARGET0
{
    return float4(CB_Model.tint.rgb, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////