#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

#ifndef DEBUG_PUNCTUAL_SHADOWMAPS
#define DEBUG_PUNCTUAL_SHADOWMAPS 0
#endif

struct PunctualLightBuffer
{
    matrix lightSpaceMatrix[6];
    float2 clipNearFar;
    float2 viewSliceOffest;
    float3 position;
    float4 color;
    float4 attenuation;
    float  intensity;
    float  temperature;
    float  shadowBaseBias;
    float  applyShadow;
};

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport          : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model          : register(b1, space1);
[[vk::binding(2, 1)]] ConstantBuffer<PunctualLightBuffer> cb_Light  : register(b2, space1);
[[vk::binding(3, 1)]] SamplerState s_SamplerState                   : register(s0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureBaseColor                  : register(t0, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureNormal                     : register(t1, space1);
[[vk::binding(6, 1)]] Texture2D t_TextureMaterial                   : register(t2, space1);
[[vk::binding(7, 1)]] Texture2D t_TextureDepth                      : register(t3, space1);
[[vk::binding(8, 1)]] TextureCube t_TextureShadowmaps               : register(t5, space1);

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

float punctual_light_shadow(in float3 WorldPos, in float3 Normal, in float cs_position_w)
{
    float shadow = 0.0;
    
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
    float3 bias = cb_Light.shadowBaseBias + slopeBias;//+viewBias;
    float3 offsetPos = WorldPos + Normal * bias;
    
    uint face = cubemap_face(lightDirection);
    float4 lightModelViewProj = mul(cb_Light.lightSpaceMatrix[face], float4(offsetPos, 1.0));
    float3 shadowCoord = lightModelViewProj.xyz / lightModelViewProj.w;

    float dist = t_TextureShadowmaps.Sample(s_SamplerState, normalize(lightDirection)).r;

    float depthMap = linearize_depth(dist, cb_Light.clipNearFar.x, cb_Light.clipNearFar.y);
    float depthFrag = linearize_depth(shadowCoord.z, cb_Light.clipNearFar.x, cb_Light.clipNearFar.y);
    if (depthMap < depthFrag)
    {
        shadow = 1.0;
    }

    return shadow;
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
        float3 worldPos = reconstruct_WorldPos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, positionScreenUV, depth);
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

        float3 lightDirection = worldPos - cb_Light.position.xyz;
        float shadow = cb_Light.applyShadow > 0.0 ? punctual_light_shadow(worldPos, normals, depth) : 0.0;
        float4 color = cook_torrance_BRDF(cb_Viewport, light, environment, worldPos, lightDistance, albedo, normals, roughness, metallic, depth, 1.0 - shadow);
#if DEBUG_PUNCTUAL_SHADOWMAPS
        color.rgb = lerp(color.rgb, float3(1.0, 1.0, 1.0), shadow);
#endif
        return float4(color.rgb, 1.0);
    }
    
    return float4(0.0, 0.0, 0.0, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////