#include "global.hlsli"
#include "viewport.hlsli"
#include "offscreen_common.hlsli"
#include "lighting_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> cb_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> cb_Model : register(b1, space1);
[[vk::binding(2, 1)]] ConstantBuffer<LightBuffer> cb_Light : register(b2, space1);
[[vk::binding(3, 1)]] SamplerState s_SamplerState          : register(s0, space1);
[[vk::binding(4, 1)]] Texture2D t_TextureBaseColor         : register(t0, space1);
[[vk::binding(5, 1)]] Texture2D t_TextureNormal            : register(t1, space1);
[[vk::binding(6, 1)]] Texture2D t_TextureMaterial          : register(t2, space1);
[[vk::binding(7, 1)]] Texture2D t_TextureDepth             : register(t3, space1);
#if WORLD_POS_ATTACHMENT
[[vk::binding(8, 1)]] Texture2D t_TextureWorldPos          : register(t4, space1);
#endif

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

[[vk::location(0)]] float4 deffered_lighting_ps(PS_OFFSCREEN_INPUT Input) : SV_TARGET0
{
    float3 albedo = t_TextureBaseColor.SampleLevel(s_SamplerState, Input.UV, 0).rgb;
    float3 normals = t_TextureNormal.SampleLevel(s_SamplerState, Input.UV, 0).rgb * 2.0 - 1.0;
    float4 material = t_TextureMaterial.SampleLevel(s_SamplerState, Input.UV, 0);
    float roughness = material.r;
    float metallic = material.g;
    
    float depth = t_TextureDepth.SampleLevel(s_SamplerState, Input.UV, 0).r;
    if (depth > 0.0)
    {
        float3 worldPos = reconstruct_WorldPos(cb_Viewport.invProjectionMatrix, cb_Viewport.invViewMatrix, Input.UV, depth);
        
        EnvironmentBuffer environment;
        environment.wetness = 0.0f;
    
        float4 color = cook_torrance_BRDF(cb_Viewport, cb_Light, environment, worldPos, cb_Light.direction_range.xyz, 0.0, albedo, normals, roughness, metallic, depth);
        return float4(color.rgb, 1.0);
    }
    
    return float4(0.0, 0.0, 0.0, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

[[vk::location(0)]] float4 light_volume_ps(PS_SIMPLE_INPUT Input) : SV_TARGET0
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
        clip(cb_Light.direction_range.x - lightDistance);
        
        EnvironmentBuffer environment;
        environment.wetness = 0.f;

        float3 lightDirection = worldPos - cb_Light.position.xyz;
        float4 color = cook_torrance_BRDF(cb_Viewport, cb_Light, environment, worldPos, lightDirection, lightDistance, albedo, normals, roughness, metallic, depth);
        return float4(color.rgb, 1.0);
    }
    
    return float4(0.0, 0.0, 0.0, 1.0);
}