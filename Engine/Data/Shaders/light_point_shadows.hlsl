#include "global.hlsli"
#include "viewport.hlsli"
#include "lighting_common.hlsli"
#include "gbuffer_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

typedef VS_GBUFFER_STANDARD_INPUT VS_SHADOW_STANDARD_INPUT;

///////////////////////////////////////////////////////////////////////////////////////

struct ShadowBuffer
{
    float4x4 lightSpaceMatrix[6];
    float4x4 modelMatrix;
    float bias;
};
[[vk::binding(0, 0)]] ConstantBuffer<ShadowBuffer> cb_PunctualShadowBuffer : register(b0, space0);

///////////////////////////////////////////////////////////////////////////////////////

float4 point_shadows_vs(VS_SHADOW_STANDARD_INPUT Input, uint ViewId : SV_ViewID) : SV_POSITION
{
    float4 position = mul(cb_PunctualShadowBuffer.modelMatrix, float4(Input.Position, 1.0));
    position = mul(cb_PunctualShadowBuffer.lightSpaceMatrix[ViewId], position);
    
    //Apply bias
    float depth = position.z / position.w;
    depth += cb_PunctualShadowBuffer.bias;
    position.z = depth * position.w;
    
    return position;
}

///////////////////////////////////////////////////////////////////////////////////////

void shadows_ps(float4 Position : SV_POSITION) : SV_DEPTH
{
    //nothing
}

///////////////////////////////////////////////////////////////////////////////////////