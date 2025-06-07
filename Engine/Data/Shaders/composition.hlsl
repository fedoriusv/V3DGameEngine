#include "global.hlsli"

struct PS_INPUT
{
    [[vk::location(0)]] float2 UV : TEXTURE;
};

[[vk::binding(1, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D textureAlbedo   : register(t0, space1);
[[vk::binding(3, 1)]] Texture2D textureNormal   : register(t1, space1);
[[vk::binding(4, 1)]] Texture2D textureMaterial : register(t2, space1);

float4 main_ps(PS_INPUT Input) : SV_TARGET0
{
    float4 fragColor = textureAlbedo.Sample(samplerState, Input.UV);
    return fragColor;
}