#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

#ifndef SHADOWMAP_CASCADE_COUNT
#define SHADOWMAP_CASCADE_COUNT 4
#endif

struct ShadowCascade
{
    matrix lightSpaceMatrix;
    float cascadeSplit;
};

struct CascadeShadowBuffer
{
    ShadowCascade cascade[SHADOWMAP_CASCADE_COUNT];
    float3 lightDirection;
};

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport                       : register(b0, space0);

[[vk::binding(0, 1)]] ConstantBuffer<CascadeShadowBuffer> cb_CascadeShadowBuffer : register(b0, space1);
[[vk::binding(1, 1)]] SamplerState s_SamplerState                                : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D t_TextureDepth                                   : register(t0, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureNormals                                 : register(t1, space1);
[[vk::binding(4, 1)]] SamplerComparisonState s_ShadowSamplerState                : register(s1, space1);
[[vk::binding(5, 1)]] Texture2DArray t_CascadeShadows                            : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

float depthTextureProj(float4 shadowCoord, float2 offset, uint cascadeIndex)
{
    float shadow = 0.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = t_CascadeShadows.SampleCmpLevelZero(s_ShadowSamplerState, float3(shadowCoord.xy + offset, (float) cascadeIndex), shadowCoord.z);
        if (shadowCoord.w > 0.0 && dist >= shadowCoord.z)
        {
            shadow = 1.0;
        }
    }

    return shadow;
}

float directionLightCascadeShadowDepthMask(in float3 worldPos, out float cascadeID)
{
    uint cascadeIndex = 0;
    float4 viewPosition = mul(cb_Viewport.viewMatrix, float4(worldPos, 1.0));
    for (uint i = 0; i < SHADOWMAP_CASCADE_COUNT; ++i)
    {
        if (viewPosition.z <= cb_CascadeShadowBuffer.cascade[i].cascadeSplit)
        {
            cascadeIndex = i;
            break;
        }
    }
    
    const matrix biasUVMatrix = matrix
        (
            0.5,  0.0, 0.0, 0.5,
            0.0, -0.5, 0.0, 0.5,
            0.0,  0.0, 1.0, 0.0,
            0.0,  0.0, 0.0, 1.0
        );
    
    float4 lightModelViewProj = mul(biasUVMatrix, mul(cb_CascadeShadowBuffer.cascade[cascadeIndex].lightSpaceMatrix, float4(worldPos, 1.0)));
    float3 shadowCoord = lightModelViewProj.xyz / lightModelViewProj.w;

    cascadeID = (float)cascadeIndex;
    return depthTextureProj(float4(shadowCoord, lightModelViewProj.w), float2(0.0, 0.0), cascadeIndex);
}

[[vk::location(0)]] float4 screen_space_shadow_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 normal = t_TextureNormals.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float bias = max(0.001 * (1.0 - dot(normal, cb_CascadeShadowBuffer.lightDirection)), 0.0001);
    
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, Input.UV, 0).r;
    if (depth > 0.0) //TODO move to stencil test
    {
        float3 worldPos = reconstruct_WorldPos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, Input.UV, depth);
        worldPos.z += bias;
        
        float cascadeID = 0.0;
        float shadow = directionLightCascadeShadowDepthMask(worldPos, cascadeID);
        return float4(shadow, cascadeID, 0.0, 0.0);
    }
    
    return float4(0.0, 0.0, 0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////