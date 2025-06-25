#include "global.hlsli"
#include "viewport.hlsli"
#include "gbuffer_common.hlsli"
#include "lighting.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState            : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureAlbedo              : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal              : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial            : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D textureMask                : register(t3, space1);

///////////////////////////////////////////////////////////////////////////////////////

VS_GBUFFER_STANDARD_OUTPUT gbuffer_standard_vs(VS_GBUFFER_STANDARD_INPUT Input)
{
    return _gbuffer_standard_vs(Input, CB_Viewport, CB_Model);
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_standard_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    return _gbuffer_standard_ps(Input, CB_Viewport, CB_Model, textureAlbedo, textureNormal, textureMaterial, samplerState);
}

///////////////////////////////////////////////////////////////////////////////////////

PS_GBUFFER_STRUCT gbuffer_masked_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    PS_GBUFFER_STRUCT GBubfferStruct = _gbuffer_standard_alpha_ps(Input, CB_Viewport, CB_Model, textureAlbedo, textureNormal, textureMaterial, samplerState);
    
    float3 color = GBubfferStruct.BaseColor.rgb;
    float opacity = GBubfferStruct.BaseColor.a;
    
    
    float mask = textureMask.SampleLevel(samplerState, Input.UV, 0).r;
    if (mask > opacity)
    {
        discard;
    }
    
    return GBubfferStruct;

}

///////////////////////////////////////////////////////////////////////////////////////

void gbuffer_depth_ps(PS_GBUFFER_STANDARD_INPUT Input)
{
    //nothing
}