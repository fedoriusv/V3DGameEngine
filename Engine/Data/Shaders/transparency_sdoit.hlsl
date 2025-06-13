#include "global.hlsli"
#include "gbuffer_common.hlsli"
#include "offscreen_common.hlsli"

///////////////////////////////////////////////////////////////////////////////////////

[[vk::binding(0, 0)]] ConstantBuffer<Viewport> CB_Viewport : register(b0, space0);

[[vk::binding(1, 1)]] ConstantBuffer<ModelBuffer> CB_Model : register(b1, space1);
[[vk::binding(2, 1)]] SamplerState samplerState : register(s0, space1);
[[vk::binding(3, 1)]] Texture2D texture0 : register(t0, space1);
[[vk::binding(4, 1)]] Texture2D texture1 : register(t1, space1);
[[vk::binding(5, 1)]] Texture2D texture2 : register(t2, space1);

///////////////////////////////////////////////////////////////////////////////////////

//pass 1
[[vk::location(0)]] float transparency_alpha_ps(PS_GBUFFER_STANDARD_INPUT input) : SV_TARGET0
{
    float alpha = CB_Model.tint.a;
    return alpha;
}

///////////////////////////////////////////////////////////////////////////////////////

//pass 2
void stochastic_depth_ps(PS_GBUFFER_STANDARD_INPUT input, uint PrimitiveID : SV_PRIMITIVEID)
{
    float2 positionScreenUV = input.Position.xy * (1.0 / CB_Viewport.viewportSize.xy);

    //float3 alpha = texture0.Sample(samplerState, input.UV).a;
    float alpha = CB_Model.tint.a;

    float2 noiseSize = CB_Viewport.viewportSize.xy / 64.0;
    const float S = 8.0;
    float noiseAccum = 0.0;
    for (int s = 0; s < S; ++s)
    {
        float2 offset = CB_Viewport.random.xy * (PrimitiveID + 1) * (s + 1);
        //float noise = rand(input.Position.xy * viewport.random.xy);
        float noise = texture1.Sample(samplerState, positionScreenUV /*input.UV*/ + offset).a;
        noiseAccum += noise;
        //if (noise >= alpha)
        //{
        //    discard;
        //}
    }
    
    noiseAccum = noiseAccum / S;
    if (noiseAccum >= alpha)
    {
        discard;
    }
}

///////////////////////////////////////////////////////////////////////////////////////

//pass 3
[[vk::location(0)]] float4 transparency_accumulate_color_ps(PS_GBUFFER_STANDARD_INPUT input) : SV_TARGET0
{
    float3 albedo = texture0.Sample(samplerState, input.UV).rgb;
    float3 normal = texture1.Sample(samplerState, input.UV).rgb * 2.0 - 1.0;
    float metalness = texture2.Sample(samplerState, input.UV).r;
    float roughness = texture2.Sample(samplerState, input.UV).g;
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Bitangent);
    float3 B = normalize(input.Tangent);
    float3x3 TBN = float3x3(T, B, N);
    normal = mul(TBN, normalize(normal));
    
    float4 color = float4(albedo * CB_Model.tint.rgb * CB_Model.tint.a, CB_Model.tint.a);
    return color;
}

///////////////////////////////////////////////////////////////////////////////////////

//pass 4
[[vk::location(0)]] float4 stochastic_transparency_final_ps(PS_OFFSCREEN_INPUT input) : SV_TARGET0
{
    float totalAlpha = texture1.Sample(samplerState, input.UV).r;
    float correctedAlpha = 1.0 - exp(-totalAlpha);
    
    float3 baseColor = texture0.Sample(samplerState, input.UV).rgb;
    float4 transparencyColor = texture2.Sample(samplerState, input.UV);
    //transparencyColor.rgb /= max(1.0, transparencyColor.a);
    //transparencyColor.rgb *= correctedAlpha;
    
    float3 color = baseColor.rgb * (1 - transparencyColor.a) + transparencyColor.rgb;
    //float3 color = lerp(baseColor.rgb, transparencyColor.rgb, correctedAlpha);
    
    return float4(color, 1.0);
}

///////////////////////////////////////////////////////////////////////////////////////

