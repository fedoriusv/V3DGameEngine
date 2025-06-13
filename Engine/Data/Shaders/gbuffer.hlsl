#include "global.hlsli"
#include "gbuffer_common.hlsli"
#include "lighting.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState            : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo              : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal              : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial            : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_standard_vs(VS_GBUFFER_STANDARD_INPUT input)
{
    return _gbuffer_standard_vs(input, CB_Viewport, CB_Model);
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_standard_ps(PS_GBUFFER_STANDARD_INPUT input)
{
    return _gbuffer_standard_ps(input, CB_Viewport, CB_Model, textureAlbedo, textureNormal, textureMaterial, samplerState);
}

///////////////////////////////////////////////////////////////////////////////////////

void gbuffer_depth_ps(PS_GBUFFER_STANDARD_INPUT input)
{
    //nothing
}