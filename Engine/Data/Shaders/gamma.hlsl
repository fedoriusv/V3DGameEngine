#include "global.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 1)]] SamplerState colorSampler : register(s0, space1);
[[vk::binding(1, 1)]] Texture2D colorTexture    : register(t1, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 gamma_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float gamma = 2.2f;
    float4 fragColor = colorTexture.SampleLevel(colorSampler, Input.UV, 0);
    fragColor.rgb = pow(fragColor.rgb, float3(1.0 / gamma));

    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////////////
