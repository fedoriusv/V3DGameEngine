#include "global.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] SamplerState samplerState  : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D textureBaseColor : register(t0, space1);
[[vk::binding(3, 1)]] Texture2D textureNormal    : register(t1, space1);
[[vk::binding(4, 1)]] Texture2D textureMaterial  : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

float4 main_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float4 fragColor = textureBaseColor.Sample(samplerState, input.UV);
    return fragColor;
}