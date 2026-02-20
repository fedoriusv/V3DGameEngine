#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"
#include "shadow_common.hlsli"

#ifndef DEBUG_PUNCTUAL_SHADOWMAPS
#define DEBUG_PUNCTUAL_SHADOWMAPS 0
#endif

#define POINT_LIGHT 1
#define SPOT_LIGHT  2

struct PunctualLightBuffer
{
    float3 position;
    float4 color;
    float4 attenuation;
    float  intensity;
    float  temperature;

    float2 clipNearFar;
    matrix lightSpaceMatrix[6];
    float2 shadowMapResolution;
    float  shadowBaseBias;
    uint   shadowSliceOffset;
    uint   shadowFaceMask;
    uint   shadowPCFMode;
    
    uint  lightType;
    float CosOuterAngle;
    float3 direction;
};

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport          : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model          : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState s_SamplerState                   : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D t_TextureBaseColor                  : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureNormal                     : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureMaterial                   : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D t_TextureDepth                      : register(t3, space1);

[[vk::binding(7, 2)]] ConstantBuffer<PunctualLightBuffer> cb_Light  : register(b2, space2);
[[vk::binding(8, 2)]] Texture2DArray t_TextureShadowmaps            : register(t5, space2);
[[vk::binding(9, 2)]] SamplerComparisonState s_ShadowSamplerState   : register(s1, space2);

///////////////////////////////////////////////////////////////////////////////////////

VS_SIMPLE_OUTPUT main_vs(VS_SIMPLE_INPUT Input)
{
    VS_SIMPLE_OUTPUT Output;
   
    float4 position = mul(cb_Model.modelMatrix, float4(Input.Position, 1.0));
    
    Output.ClipPos = mul(cb_Viewport.projectionMatrix, mul(cb_Viewport.viewMatrix, position));
    Output.WorldPos = position.xyz / position.w;
    Output.Normal = normalize(mul((float3x3) cb_Model.normalMatrix, Input.Normal));
    Output.UV = Input.UV;

    Output.Position = Output.ClipPos;

    return Output;
}

///////////////////////////////////////////////////////////////////////////////////////

float punctual_light_shadow(in float3 WorldPos, in float3 Normal)
{
    float shadow = 0.0;
    const float2 scaleFactor = 0.75.xx;
    
    float3 lightDirection = WorldPos - cb_Light.position.rgb;
    float lightDistance = length(lightDirection);
    if (lightDistance > cb_Light.clipNearFar.y)
    {
        return 0.0;
    }
    
    float NdotL = saturate(dot(Normal, normalize(lightDirection)));
    float NdotV = saturate(dot(Normal, cb_Viewport.cameraPosition.xyz));
    float slopeBias = max(0.001 * (1.0 - NdotL), 0.0001);
    float viewBias = max(0.001 * (1.0 - NdotV), 0.0001);
    float3 bias = cb_Light.shadowBaseBias + slopeBias + viewBias;
    float3 offsetPos = WorldPos + Normal * bias;
    
    uint face = _cubemap_face_id(lightDirection);
    float2 uv = _cubemap_face_UV(lightDirection, face);
    
    float4 lightModelViewProj = mul(cb_Light.lightSpaceMatrix[face], float4(offsetPos, 1.0));
    float3 shadowCoord = lightModelViewProj.xyz / lightModelViewProj.w;

    if (cb_Light.shadowPCFMode == 1)
    {
        return shadow_linear_sample_PCF_3x3(
            t_TextureShadowmaps, s_SamplerState, cb_Light.shadowMapResolution, cb_Light.clipNearFar, float4(uv, shadowCoord.z, lightModelViewProj.w), cb_Light.shadowSliceOffset + face, scaleFactor, cb_Light.shadowBaseBias);

    }
    else if (cb_Light.shadowPCFMode == 2)
    {
        return shadow_linear_sample_PCF_5x5(
            t_TextureShadowmaps, s_SamplerState, cb_Light.shadowMapResolution, cb_Light.clipNearFar, float4(uv, shadowCoord.z, lightModelViewProj.w), cb_Light.shadowSliceOffset + face, scaleFactor, cb_Light.shadowBaseBias);
    }
    else if (cb_Light.shadowPCFMode == 3)
    {
        return shadow_linear_sample_PCF_9x9(
            t_TextureShadowmaps, s_SamplerState, cb_Light.shadowMapResolution, cb_Light.clipNearFar, float4(uv, shadowCoord.z, lightModelViewProj.w), cb_Light.shadowSliceOffset + face, scaleFactor, cb_Light.shadowBaseBias);
    }
    
    return shadow_linear_sample_PCF_1x1(
        t_TextureShadowmaps, s_SamplerState, cb_Light.shadowMapResolution, cb_Light.clipNearFar, float4(uv, shadowCoord.z, lightModelViewProj.w), cb_Light.shadowSliceOffset + face, cb_Light.shadowBaseBias * 1.0);;
}

[[vk::location(0)]] float4 light_accumulation_ps(PS_SIMPLE_INPUT Input) : SV_TARGET0
{
    float2 positionScreenUV = Input.Position.xy * (1.0 / cb_Viewport.viewportSize.xy);
    float3 albedo = t_TextureBaseColor.SampleLevel(s_SamplerState, positionScreenUV, 0).rgb;
    float3 normals = t_TextureNormal.SampleLevel(s_SamplerState, positionScreenUV, 0).rgb * 2.0 - 1.0;
    float4 material = t_TextureMaterial.SampleLevel(s_SamplerState, positionScreenUV, 0);
    float roughness = material.r;
    float metallic = material.g;
    
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, positionScreenUV, 0).r;
    if (depth > 0.0) //TODO move to stencil test
    {
        float3 worldPos = _reconstruct_world_pos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, positionScreenUV, depth);
        
        if (cb_Light.lightType == POINT_LIGHT)
        {
            float lightDistance = distance(worldPos, cb_Light.position.xyz);
            float radius = cb_Light.attenuation.w;
            clip(radius - lightDistance);
        
            EnvironmentBuffer environment;
            environment.wetness = 0.f;
            environment.shadowSaturation = 0.01f; //temp
        
            LightBuffer light;
            light.direction = worldPos - cb_Light.position.xyz;
            light.color = cb_Light.color;
            light.attenuation = cb_Light.attenuation;
            light.intensity = cb_Light.intensity;
            light.temperature = cb_Light.temperature;

            float shadow = cb_Light.shadowFaceMask > 0.0 ? punctual_light_shadow(worldPos, normals) : 0.0;
            float4 color = cook_torrance_BRDF(cb_Viewport, light, environment, worldPos, lightDistance, albedo, normals, roughness, metallic, depth, 1.0 - shadow);
#if DEBUG_PUNCTUAL_SHADOWMAPS
        color.rgb = lerp(color.rgb, float3(1.0, 1.0, 1.0), shadow);
#endif
            return float4(color.rgb, 1.0);
        }
        else if (cb_Light.lightType == SPOT_LIGHT)
        {
            //float lightDistance = distance(worldPos, cb_Light.position.xyz);
            float range = cb_Light.attenuation.w;
            //clip(range - lightDistance);
            
            float3 V = worldPos - cb_Light.position.xyz;
            float axial = dot(V, cb_Light.direction);

            if (axial <= 0 || axial >= range)
                discard;

            //float cosAngle = dot(normalize(V), cb_Light.direction);
            //if (cosAngle <= cb_Light.CosOuterAngle)
            //    discard;
            
            LightBuffer light;
            light.direction = worldPos - cb_Light.position.xyz;
            light.color = cb_Light.color;
            light.attenuation = cb_Light.attenuation;
            light.intensity = cb_Light.intensity;
            light.temperature = cb_Light.temperature;
            
            float4 color = light.color;
            return float4(color.rgb, 1.0);
        }
    }
    
    return float4(0.0, 0.0, 0.0, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////