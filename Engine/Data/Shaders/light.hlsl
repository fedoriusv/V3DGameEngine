#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);
[[vk::binding(1, 1)]] ConstantBuffer<LightBuffer> CB_Light : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D textureBaseColor : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D textureNormal : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D textureMaterial : register(t2, space1);
[[vk::binding(6, 1)]] Texture2D textureDepth : register(t3, space1);

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 deffered_lighting_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 albedo = textureBaseColor.SampleLevel(samplerState, Input.UV, 0).rgb;
    float3 normals = textureNormal.SampleLevel(samplerState, Input.UV, 0).rgb * 2.0 - 1.0;
    float4 material = textureMaterial.SampleLevel(samplerState, Input.UV, 0);
    float roughness = material.r;
    float metallic = material.g;
    
    float depth = textureDepth.SampleLevel(samplerState, Input.UV, 0).r;
    float4x4 invViewProject = mul(CB_Viewport.invProjectionMatrix, CB_Viewport.invViewMatrix);
    float3 worldPos = reconstruct_worldPos(invViewProject, Input.UV, depth);
    
    float4 color = cook_torrance_BRDF(CB_Viewport, CB_Light, worldPos, albedo, normals, metallic, roughness, depth);

    return color;
}

///////////////////////////////////////////////////////////////////////////////////////