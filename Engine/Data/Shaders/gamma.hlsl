#include "global.hlsli"

struct GammaBuffer
{
    int lod;
};
[[vk::binding(1, 1)]] ConstantBuffer<GammaBuffer> CB_Gamma : register(b0, space1);

[[vk::binding(2, 1)]] SamplerState colorSampler : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D colorTexture    : register(t1, space1);

struct PS_INTPUT
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXTURE;
};

float4 gamma_ps(PS_INTPUT Input) : SV_TARGET0
{
    float gamma = 2.2f;
    float4 fragColor = colorTexture.SampleLevel(colorSampler, Input.UV, CB_Gamma.lod);
    fragColor.rgb = pow(fragColor.rgb, float3(1.0 / gamma));

    return fragColor;
}

