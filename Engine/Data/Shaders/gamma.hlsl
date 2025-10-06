#include "global.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 1)]] SamplerState s_ColorSampler : register(s0, space1);
[[vk::binding(1, 1)]] Texture2D t_ColorTexture    : register(t1, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 gamma_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float gamma = 2.2f;
    float4 fragColor = t_ColorTexture.SampleLevel(s_ColorSampler, Input.UV, 0);
    fragColor.rgb = pow(fragColor.rgb, 1.0 / gamma);

    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////////////
