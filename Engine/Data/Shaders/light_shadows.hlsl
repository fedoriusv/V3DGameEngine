#include "global.hlsli"
#include "viewport.hlsli"
#include "lighting_common.hlsli"
#include "gbuffer_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

typedef VS_GBUFFER_STANDARD_INPUT VS_SHADOW_STANDARD_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

struct ShadowBuffer
{
    float4x4 lightSpaceMatrix;
    float4x4 modelMatrix;
};
[[vk::binding(0, 0)]] ConstantBuffer<ShadowBuffer> cb_ShadowBuffer : register(b0, space0);

///////////////////////////////////////////////////////////////////////////////////////

float4 light_shadows_vs(VS_SHADOW_STANDARD_INPUT Input) : SV_POSITION
{
    float4 position = mul(cb_ShadowBuffer.modelMatrix, float4(Input.Position, 1.0));
    position = mul(cb_ShadowBuffer.lightSpaceMatrix, position);
    
    return position;
}

void light_shadows_ps(float4 Position : SV_POSITION) : SV_DEPTH
{
    //nothing
}

///////////////////////////////////////////////////////////////////////////////////////