#include "global.hlsli"
#include "viewport.hlsli"
#include "vfx_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);
[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState          : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureBaseColor         : register(t0, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT main_vs(VS_SIMPLE_INPUT Input)
{
    VS_SIMPLE_OUTPUT Output;
   
    float4 position = mul(cb_Model.modelMatrix, float4(Input.Position, 1.0));
    
    Output.ClipPos = mul(cb_Viewport.projectionMatrix, mul(cb_Viewport.viewMatrix, position));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = normalize(mul((float3x3) cb_Model.normalMatrix, Input.Normal));
    Output.UV = Input.UV;

    Output.Position = Output.ClipPos;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT billboard_vs(uint VertexID : SV_VERTEXID)
{
    VS_SIMPLE_OUTPUT Output = _billboard_vs(cb_Viewport, cb_Model, VertexID);

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

float4 unlit_ps(PS_SIMPLE_INPUT Input) : SV_TARGET0
{
    return float4(cb_Model.tintColour.rgb, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

struct PS_SIMPLE_SELECTABLE_OUTPUT
{
    [[vk::location(0)]] float4 Color    : SV_TARGET0;
    [[vk::location(1)]] float4 Material : SV_TARGET1;
};

PS_SIMPLE_SELECTABLE_OUTPUT unlit_selectable_ps(PS_SIMPLE_INPUT Input)
{
    float3 color = t_TextureBaseColor.Sample(s_SamplerState, Input.UV).rgb;
    
    PS_SIMPLE_SELECTABLE_OUTPUT Output;
    
    Output.Color = float4(color * cb_Model.tintColour.rgb, 1.0);
    Output.Material = float4(0.0, 0.0, cb_Model.objectID, 0.0);

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////