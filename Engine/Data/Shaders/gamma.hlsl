#include "global.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

struct GammaBuffer
{
    int lod;
};
[[vk::binding(0, 1)]] ConstantBuffer<GammaBuffer> CB_Gamma : register(b0, space1);

[[vk::binding(1, 1)]] SamplerState colorSampler : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D colorTexture    : register(t1, space1);

///////////////////////////////////////////////////////////////////////////////////////

float4 gamma_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float gamma = 2.2f;
    float4 fragColor = colorTexture.SampleLevel(colorSampler, input.UV, CB_Gamma.lod);
    fragColor.rgb = pow(fragColor.rgb, float3(1.0 / gamma));

    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////////////
