#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "shadow_common.hlsli"

#ifndef SHADOWMAP_CASCADE_COUNT
#define SHADOWMAP_CASCADE_COUNT 4
#endif

#ifndef SHADOWMAP_CASCADE_BLEND
#define SHADOWMAP_CASCADE_BLEND 1
#endif


struct DirectionLightShadowmapCascade
{
    matrix lightSpaceMatrix;
    float  cascadeSplit;
    float  baseBias;
    float  slopeBias;
};

struct ShadowmapBuffer
{
    
    DirectionLightShadowmapCascade cascade[SHADOWMAP_CASCADE_COUNT];
    float4 directionLight;
    float2 shadowMapResolution;
    float  PCFMode;
    float  texelScale;
};

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport                  : register(b0, space0);

[[vk::binding(0, 1)]] ConstantBuffer<ShadowmapBuffer> cb_ShadowmapBuffer    : register(b0, space1);
[[vk::binding(1, 1)]] SamplerState s_SamplerState                           : register(s0, space1);
[[vk::binding(2, 1)]] Texture2D t_TextureDepth                              : register(t0, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureNormals                            : register(t1, space1);
[[vk::binding(4, 1)]] Texture2DArray t_DirectionCascadeShadows              : register(t2, space1);
[[vk::binding(5, 1)]] SamplerComparisonState s_ShadowSamplerState           : register(s1, space1);

///////////////////////////////////////////////////////////////////////////////////////

float shadowmap_cascade_blend(in float CurrentShadowFactor, in float NextShadowFactor, in float View, in int CascadeIndex)
{
    uint nextCascadeIndex = min(CascadeIndex + 1, SHADOWMAP_CASCADE_COUNT - 1);

    float cascadeStart = CascadeIndex > 0 ? cb_ShadowmapBuffer.cascade[CascadeIndex - 1].cascadeSplit : 0.0;
    float cascadeEnd = cb_ShadowmapBuffer.cascade[CascadeIndex].cascadeSplit;
    float cascadeLength = cascadeEnd - cascadeStart;

    float blendRegion = cascadeLength * 0.2; //20% overlap
    float blendStart = cascadeEnd - blendRegion;
    float blendFactor = smoothstep(blendStart, cascadeEnd, View);

    return lerp(CurrentShadowFactor, NextShadowFactor, blendFactor);
}

float direction_light_shadows(in float3 WorldPos, in float3 Normal, out float CascadeID)
{
    int cascadeIndex = 0;
    float4 viewPosition = mul(cb_Viewport.viewMatrix, float4(WorldPos, 1.0));
    for (uint i = 0; i < SHADOWMAP_CASCADE_COUNT; ++i)
    {
        if (viewPosition.z <= cb_ShadowmapBuffer.cascade[i].cascadeSplit)
        {
            cascadeIndex = i;
            break;
        }
    }
    CascadeID = (float) cascadeIndex;
    
    static const matrix biasUVMatrix = matrix
        (
            0.5,  0.0, 0.0, 0.5,
            0.0, -0.5, 0.0, 0.5,
            0.0,  0.0, 1.0, 0.0,
            0.0,  0.0, 0.0, 1.0
        );
    
    float2 texelSize = rcp(cb_ShadowmapBuffer.shadowMapResolution);
    float NdotL = saturate(dot(Normal, cb_ShadowmapBuffer.directionLight.xyz));
    float NdotV = saturate(dot(Normal, cb_Viewport.cameraPosition.xyz));
    float slopeBias = max(0.001 * (1.0 - NdotL), 0.0001);
    float viewBias = max(0.001 * (1.0 - NdotV), 0.0001);
    
    float bias = cb_ShadowmapBuffer.cascade[cascadeIndex].baseBias + slopeBias * cb_ShadowmapBuffer.cascade[cascadeIndex].slopeBias + viewBias;
    float3 ws_offsetPos = WorldPos + Normal * bias;
    
    float4 lightModelViewProj = mul(biasUVMatrix, mul(cb_ShadowmapBuffer.cascade[cascadeIndex].lightSpaceMatrix, float4(ws_offsetPos, 1.0)));
    float3 cs_shadow = lightModelViewProj.xyz / lightModelViewProj.w;
#if SHADOWMAP_CASCADE_BLEND
    uint nextCascadeIndex = min(cascadeIndex + 1, SHADOWMAP_CASCADE_COUNT - 1);
    
    float biasNext = cb_ShadowmapBuffer.cascade[nextCascadeIndex].baseBias + slopeBias * cb_ShadowmapBuffer.cascade[nextCascadeIndex].slopeBias + viewBias;
    float3 ws_offsetPosNextCascade = WorldPos + Normal * biasNext;
    
    float4 lightModelViewProjNextCascade = mul(biasUVMatrix, mul(cb_ShadowmapBuffer.cascade[nextCascadeIndex].lightSpaceMatrix, float4(ws_offsetPosNextCascade, 1.0)));
    float3 cs_shadowNextCascade = lightModelViewProjNextCascade.xyz / lightModelViewProjNextCascade.w;
#endif

    if (cb_ShadowmapBuffer.PCFMode > 0)
    {
        float shadowFactor = shadow_sample_PCF_3x3(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadow, cascadeIndex);
#if SHADOWMAP_CASCADE_BLEND
        float nextShadowFactor = shadow_sample_PCF_3x3(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadowNextCascade, nextCascadeIndex);
        shadowFactor = shadowmap_cascade_blend(shadowFactor, nextShadowFactor, viewPosition.z, cascadeIndex);
#endif
        return shadowFactor;
    }
    else if (cb_ShadowmapBuffer.PCFMode == 2)
    {
        float shadowFactor = shadow_sample_PCF_5x5(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadow, cascadeIndex);
#if SHADOWMAP_CASCADE_BLEND
        float nextShadowFactor = shadow_sample_PCF_5x5(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadowNextCascade, nextCascadeIndex);
        shadowFactor = shadowmap_cascade_blend(shadowFactor, nextShadowFactor, viewPosition.z, cascadeIndex);
#endif
        return shadowFactor;
    }
    else if (cb_ShadowmapBuffer.PCFMode == 3)
    {
        float shadowFactor = shadow_sample_PCF_9x9(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadow, cascadeIndex);
#if SHADOWMAP_CASCADE_BLEND
        float nextShadowFactor = shadow_sample_PCF_9x9(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cb_ShadowmapBuffer.texelScale, cs_shadowNextCascade, nextCascadeIndex);
        shadowFactor = shadowmap_cascade_blend(shadowFactor, nextShadowFactor, viewPosition.z, cascadeIndex);
#endif
        return shadowFactor;
    }

    float shadowFactor = shadow_sample(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cs_shadow, cascadeIndex);
#if SHADOWMAP_CASCADE_BLEND
    float nextShadowFactor = shadow_sample(t_DirectionCascadeShadows, s_ShadowSamplerState, cb_ShadowmapBuffer.shadowMapResolution, cs_shadowNextCascade, nextCascadeIndex);
    shadowFactor = shadowmap_cascade_blend(shadowFactor, nextShadowFactor, viewPosition.z, cascadeIndex);
#endif
    return shadowFactor;
}

[[vk::location(0)]] float4 screen_space_shadow_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 normal = t_TextureNormals.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, Input.UV, 0).r;
    if (depth > 0.0) //TODO move to stencil test
    {
        float3 worldPos = _reconstruct_world_pos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, Input.UV, depth);
        
        float cascadeID = 0.0;
        float directShadow = direction_light_shadows(worldPos, normal, cascadeID);
        
        return float4(saturate(directShadow), cascadeID, 0.0, 0.0);
    }
    
    return float4(0.0, 0.0, 0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////