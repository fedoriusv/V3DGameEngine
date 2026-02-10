#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

#ifndef SHADOWMAP_CASCADE_COUNT
#define SHADOWMAP_CASCADE_COUNT 4
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
    float4 shadowMapResolution;
    float4 directionLight;
    float  enablePCF;
};

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport                  : register(b0, space0);

[[vk::binding(0, 1)]] ConstantBuffer<ShadowmapBuffer> cb_ShadowmapBuffer    : register(b0, space1);
[[vk::binding(1, 1)]] SamplerState s_SamplerState                           : register(s0, space1);
[[vk::binding(2, 1)]] SamplerComparisonState s_ShadowSamplerState           : register(s1, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureDepth                              : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureNormals                            : register(t1, space1);
[[vk::binding(5, 1)]] Texture2DArray t_DirectionCascadeShadows              : register(t2, space1);
//[[vk::binding(6, 1)]] TextureCube t_PunctualShadows                         : register(t3, space1);

///////////////////////////////////////////////////////////////////////////////////////

float depth_projection(Texture2DArray shadowmap, in float4 shadowCoord, in float2 offset, in uint cascadeIndex)
{
    float shadow = 0.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = shadowmap.SampleCmpLevelZero(s_ShadowSamplerState, float3(shadowCoord.xy + offset, (float) cascadeIndex), shadowCoord.z);
        if (shadowCoord.w > 0.0 && dist >= shadowCoord.z)
        {
            shadow = 1.0;
        }
    }

    return shadow;
}

float depth_projection_PCF_3x3(Texture2DArray shadowmap, in float2 scaleFactor, in float4 shadowCoord, in uint cascadeIndex)
{
    static const uint kernelSize = 9;
    static const float2 kernel[kernelSize] =
    {
        float2(-1.0, -1.0), float2(0.0, -1.0), float2(1.0, -1.0),
        float2(-1.0,  0.0), float2(0.0,  0.0), float2(1.0,  0.0),
        float2(-1.0,  1.0), float2(0.0,  1.0), float2(1.0,  1.0)
    };

    float shadow = 0.0;
    [unroll(kernelSize)]
    for (uint i = 0; i < kernelSize; ++i)
    {
        float2 offset = kernel[i] * scaleFactor;
        shadow += depth_projection(shadowmap, shadowCoord, offset, cascadeIndex);
    }
    
    return shadow / (float)kernelSize;
}


float direction_light_shadows(in float3 worldPos, in float3 normal, out float cascadeID)
{
    uint cascadeIndex = 0;
    float4 viewPosition = mul(cb_Viewport.viewMatrix, float4(worldPos, 1.0));
    for (uint i = 0; i < SHADOWMAP_CASCADE_COUNT; ++i)
    {
        if (viewPosition.z <= cb_ShadowmapBuffer.cascade[i].cascadeSplit)
        {
            cascadeIndex = i;
            break;
        }
    }
    
    static const matrix biasUVMatrix = matrix
        (
            0.5,  0.0, 0.0, 0.5,
            0.0, -0.5, 0.0, 0.5,
            0.0,  0.0, 1.0, 0.0,
            0.0,  0.0, 0.0, 1.0
        );
    
    float NdotL = saturate(dot(normal, cb_ShadowmapBuffer.directionLight.xyz));
    float NdotV = saturate(dot(normal, cb_Viewport.cameraPosition.xyz));
    float slopeBias = max(0.001 * (1.0 - NdotL), 0.0001);
    float viewBias = max(0.001 * (1.0 - NdotV), 0.0001);
    float2 texelSize = 1.0 / cb_ShadowmapBuffer.shadowMapResolution.xy;
    float bias = cb_ShadowmapBuffer.cascade[cascadeIndex].baseBias + slopeBias * cb_ShadowmapBuffer.cascade[cascadeIndex].slopeBias + viewBias;
    float3 offsetPos = worldPos + normal * bias;
    
    float4 lightModelViewProj = mul(biasUVMatrix, mul(cb_ShadowmapBuffer.cascade[cascadeIndex].lightSpaceMatrix, float4(offsetPos, 1.0)));
    float3 shadowCoord = lightModelViewProj.xyz / lightModelViewProj.w;
    
    cascadeID = (float)cascadeIndex;
    if (cb_ShadowmapBuffer.enablePCF)
    {
        float2 texelSize = 1.0 / cb_ShadowmapBuffer.shadowMapResolution.xy;
        float2 scaleFactor = texelSize * 0.75;
        return depth_projection_PCF_3x3(t_DirectionCascadeShadows, scaleFactor, float4(shadowCoord, lightModelViewProj.w), cascadeIndex);
    }

    return depth_projection(t_DirectionCascadeShadows, float4(shadowCoord, lightModelViewProj.w), float2(0.0, 0.0), cascadeIndex);
}

//float point_light_shadows(in float3 worldPos)
//{
//    float shadow = 0.0;
//    for (uint i = 0; i < cb_ShadowmapBuffer.countLights; ++i)
//    {
//        float3 lightDirection = worldPos - cb_ShadowmapBuffer.punctualLight[i].position.rgb;
//        float dist = t_PunctualShadows.Sample(s_SamplerState, lightDirection).r;
//        //float linearDepth = linearize_depth(dist, cb_ShadowmapBuffer.punctualLight[i].clipNearFar.y, cb_ShadowmapBuffer.punctualLight[i].clipNearFar.x);
        
//        float lightDistance = length(lightDirection) / (cb_ShadowmapBuffer.punctualLight[i].clipNearFar.y - cb_ShadowmapBuffer.punctualLight[i].clipNearFar.x);
//        if (dist < lightDistance)
//        {
//            shadow += 1.0;
//        }
//    }

//    return shadow;
//}

[[vk::location(0)]] float4 screen_space_shadow_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 normal = t_TextureNormals.Sample(s_SamplerState, Input.UV).rgb * 2.0 - 1.0;
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, Input.UV, 0).r;
    if (depth > 0.0) //TODO move to stencil test
    {
        float3 worldPos = reconstruct_WorldPos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, Input.UV, depth);
        
        float cascadeID = 0.0;
        float directShadow = direction_light_shadows(worldPos, normal, cascadeID);
        
        return float4(directShadow, cascadeID, 0.0, 0.0);
    }
    
    return float4(0.0, 0.0, 0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////