#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport  : register(b0, space0);

[[vk::binding(1, 1)]] SamplerState s_SamplerLinear          : register(s0, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerPoint           : register(s1, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureBaseColor          : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureHistory            : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureVelocity           : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 main_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 currentColor = t_TextureBaseColor.SampleLevel(s_SamplerPoint, Input.UV, 0).xyz;
    float2 velocity = t_TextureVelocity.SampleLevel(s_SamplerPoint, Input.UV, 0).xy;
    
    float2 prevousPixelPos = Input.UV - velocity;
    float3 historyColor = t_TextureHistory.SampleLevel(s_SamplerLinear, prevousPixelPos, 0).xyz;
    
    float3 nearColor0 = t_TextureBaseColor.SampleLevel(s_SamplerPoint, Input.UV, 0, int2(1, 0)).xyz;
    float3 nearColor1 = t_TextureBaseColor.SampleLevel(s_SamplerPoint, Input.UV, 0, int2(0, 1)).xyz;
    float3 nearColor2 = t_TextureBaseColor.SampleLevel(s_SamplerPoint, Input.UV, 0, int2(-1, 0)).xyz;
    float3 nearColor3 = t_TextureBaseColor.SampleLevel(s_SamplerPoint, Input.UV, 0, int2(0, -1)).xyz;
    float3 boxMin = min(currentColor, min(nearColor0, min(nearColor1, min(nearColor2, nearColor3))));
    float3 boxMax = max(currentColor, max(nearColor0, max(nearColor1, max(nearColor2, nearColor3))));
    
    historyColor = clamp(historyColor, boxMin, boxMax);
    
    float modulationFactor = 0.9;
    float3 color = lerp(currentColor, historyColor, modulationFactor);
    
    return float4(color, 1.0);
}