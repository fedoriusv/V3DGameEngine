#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

#ifndef DEBUG_SHADOWMAP_CASCADES
#define DEBUG_SHADOWMAP_CASCADES 0
#endif

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<LightBuffer> cb_Light : register(b2, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureBaseColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureNormal : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureMaterial : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D t_TextureDepth : register(t3, space1);
[[vk::binding(7, 1)]] Texture2D t_TextureScreenSpaceShadows : register(t4, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 deffered_lighting_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 albedo = t_TextureBaseColor.SampleLevel(s_SamplerState, Input.UV, 0).rgb;
    float3 normals = t_TextureNormal.SampleLevel(s_SamplerState, Input.UV, 0).rgb * 2.0 - 1.0;
    float4 material = t_TextureMaterial.SampleLevel(s_SamplerState, Input.UV, 0);
    float roughness = material.r;
    float metallic = material.g;
    
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, Input.UV, 0).r;
    if (depth > 0.0) //TODO move to stencil test
    {
        float3 worldPos = _reconstruct_world_pos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, Input.UV, depth);
        
        EnvironmentBuffer environment;
        environment.wetness = 0.0f;
        environment.shadowSaturation = 0.01f; //temp
    
        float directionShadow = t_TextureScreenSpaceShadows.SampleLevel(s_SamplerState, Input.UV, 0).r;
        float4 color = cook_torrance_BRDF(cb_Viewport, cb_Light, environment, worldPos, 0.0, albedo, normals, roughness, metallic, depth, 1.0 - directionShadow);
#if DEBUG_SHADOWMAP_CASCADES
        uint cascade = (uint)t_TextureScreenSpaceShadows.SampleLevel(s_SamplerState, Input.UV, 0).g;
        switch (cascade)
        {
            case 0:
                color.rgb = lerp(color.rgb, float3(1.0, 0.0, 0.0), directionShadow);
                break;
            case 1:
                color.rgb = lerp(color.rgb, float3(0.0, 1.0, 0.0), directionShadow);
                break;
            case 2:
                color.rgb = lerp(color.rgb, float3(0.0, 0.0, 1.0), directionShadow);
                break;
            case 3:
                color.rgb = lerp(color.rgb, float3(1.0, 1.0, 0.0), directionShadow);
                break;
        }
#else
        color.rgb = lerp(color.rgb, albedo * environment.shadowSaturation, directionShadow);
#endif
        return float4(color.rgb, 1.0);
    }
    
    return float4(0.0, 0.0, 0.0, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////
