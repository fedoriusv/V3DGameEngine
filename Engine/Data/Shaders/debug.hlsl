#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

#define GBUFFER_BASE_COLOR 1
#define GBUFFER_NORNALS 2
#define DEPTH 3
#define LIGHT 4

struct Visualizer
{
    uint mode;
    uint format;
};

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport      : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<Visualizer> cb_Visualizer  : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState               : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_RenderTargetTexture           : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_VisualizeTexture              : register(t1, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 debug_visualizer_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 colorTarget = t_RenderTargetTexture.SampleLevel(s_SamplerState, Input.UV, 0).rgb;
    float4 visualizeTexture = t_VisualizeTexture.SampleLevel(s_SamplerState, Input.UV, 0);

    switch (cb_Visualizer.mode)
    {
        case GBUFFER_BASE_COLOR:
            return float4(visualizeTexture.rgb, 1.0);
        
        case GBUFFER_NORNALS:
            return float4(visualizeTexture.rgb, 1.0);
        
        case DEPTH:
            return float4(visualizeTexture.rrr, 1.0);
        
        case LIGHT:
            return float4(visualizeTexture.rgb, 1.0);
        
        default:
            break;
    }
    
    return float4(colorTarget.rgb, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////